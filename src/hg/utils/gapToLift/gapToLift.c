/* gapToLift - create lift file from gap table(s). */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"
#include "chromInfo.h"
#include "agpGap.h"
#include "hdb.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "gapToLift - create lift file from gap table(s)\n"
  "usage:\n"
  "   gapToLift [options] db liftFile.lft\n"
  "       uses gap table(s) from specified db.  Writes to liftFile.lft\n"
  "       generates lift file segements separated by non-bridged gaps.\n"
  "options:\n"
  "   -chr=chrN - work only on given chrom\n"
  "   -sane - perform coordinate sanity checks on gaps\n"
  "   -bedFile=fileName.bed - output segments to fileName.bed"
  );
}

/* options */
static char *workChr = NULL;	/* work only on this given chrom name */
static boolean sane = FALSE;	/* if TRUE, perform sanity checks on gaps */
static FILE *bedFile = NULL; /* when requested, output segments to bed file */
static char *bedFileName = NULL; /* output to bedFileName name */


static struct optionSpec options[] = {
   {"chr", OPTION_STRING},
   {"sane", OPTION_BOOLEAN},
   {"bedFile", OPTION_STRING},
   {NULL, 0},
};

static struct hash *cInfoHash = NULL;

static struct chromInfo *loadChromInfo(struct sqlConnection *conn)
{
struct chromInfo *ret = NULL;
char **row;
char query[256];
int chromCount = 0;

cInfoHash = newHash(0);

if (workChr)
    safef(query, ArraySize(query), "SELECT * FROM chromInfo WHERE "
	"chrom='%s' ORDER BY chrom DESC", workChr);
else
    safef(query, ArraySize(query),
	"SELECT * FROM chromInfo ORDER BY chrom DESC");

struct sqlResult *sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct chromInfo *el;
    struct chromInfo *ci;
    AllocVar(ci);
    el = chromInfoLoad(row);
    ci->chrom = cloneString(el->chrom);
    ci->size = el->size;
    slAddHead(&ret, ci);
    hashAddInt(cInfoHash, el->chrom, el->size);
    ++chromCount;
    }
sqlFreeResult(&sr);
verbose(2,"#\tchrom count: %d\n", chromCount);
return (ret);
}

static struct agpGap *loadAllGaps(struct sqlConnection *conn,
	char *db, struct chromInfo *cInfoList)
/*	fetch all gaps, returns list of gaps */
{ 
struct agpGap *gapList = NULL;
struct chromInfo *cInfo;
int gapCount = 0;
int prevEnd = 0;
int prevStart = 0;
char *prevChr = NULL;

for (cInfo = cInfoList; cInfo; cInfo = cInfo->next)
    {
    char **row;
    int rowOffset;
    struct sqlResult *sr = hRangeQuery(conn, "gap", cInfo->chrom, 0,
	cInfo->size, NULL, &rowOffset);
    while ((row = sqlNextRow(sr)) != NULL)
	{
	struct agpGap *gap = agpGapLoad(row+rowOffset);
	slAddHead(&gapList, gap);
	++gapCount;
	if (sane)
	    {
	    if (gap->chromStart < 0)
		verbose(1, "WARNING: gap chromStart < 0 at %s:%d-%d\n",
			gap->chrom, gap->chromStart, gap->chromEnd);
	    if (gap->chromEnd > cInfo->size)
		verbose(1, "WARNING: gap chromEnd > chromSize(%d) "
		    "at %s:%d-%d\n", cInfo->size, gap->chrom,
			gap->chromStart, gap->chromEnd);
	    if (gap->chromEnd == cInfo->size)
		verbose(1, "WARNING: gap at end of chromosome "
		    "at %s:%d-%d\n", gap->chrom,
			gap->chromStart, gap->chromEnd);
	    if (gap->chromStart >= gap->chromEnd)
		verbose(1, "WARNING: gap chromStart >= chromEnd at %s:%d-%d\n",
		    gap->chrom, gap->chromStart, gap->chromEnd);
	    if (prevEnd > 0)
		{
		if (sameWord(prevChr, gap->chrom) &&
			(prevEnd >= gap->chromStart))
		    verbose(1,"WARNING: overlapping gap at "
			"%s:%d-%d and %s:%d-%d\n",
			    gap->chrom, prevStart, prevEnd, gap->chrom,
				gap->chromStart, gap->chromEnd);
		}
	    else
		{
		prevStart = gap->chromStart;
		prevEnd = gap->chromEnd;
		}
	    if (isNotEmpty(prevChr))
		{
		if (differentWord(prevChr, gap->chrom))
		    {
		    freeMem(prevChr);
		    prevChr = cloneString(gap->chrom);
		    }
		}
	    else
		prevChr = cloneString(gap->chrom);
	    prevStart = gap->chromStart;
	    prevEnd = gap->chromEnd;
	    }
	}
    sqlFreeResult(&sr);
    }
slReverse(&gapList);
verbose(2,"#\tfound %d gaps\n", gapCount);
return (gapList);
}

static void liftOutLine(FILE *out, char *chr, int start, int end,
    int count, int chrSize)
{
if ((end-start) > 0)
    {
    fprintf(out, "%d\t%s.%d\t%d\t%s\t%d\n", start, chr,
	count, end-start, chr, chrSize);
    if (bedFile)
	fprintf(bedFile, "%s\t%d\t%d\t%s.%d\n", chr, start, end, chr, count);
    }
}

static void gapToLift(char *db, char *outFile)
/* gapToLift - create lift file from gap table(s). */
{
FILE *out = mustOpen(outFile, "w");
struct sqlConnection *conn = sqlConnect(db);
struct chromInfo *cInfoList = loadChromInfo(conn);
struct agpGap *gapList = loadAllGaps(conn, db, cInfoList);
struct agpGap *gap;
int start = 0;
int end = 0;
char *prevChr = NULL;
int liftCount = 0;
int chrSize = 0;

if (isNotEmpty(bedFileName))
    {
    bedFile = mustOpen(bedFileName, "w");
    verbose(2,"#\tbed output requested to %s\n", bedFileName);
    }

for (gap = gapList; gap; gap = gap->next)
    {
    verbose(3,"#\t%s\t%d\t%d\t%s\n", gap->chrom, gap->chromStart,
	gap->chromEnd, gap->bridge);

    if (prevChr && sameWord(prevChr, gap->chrom))
	{	/* continuing same segment, check for gap break,
		 *	or gap at end of chrom */
	if (sameWord("no",gap->bridge) || (gap->chromEnd == chrSize))
	    {
	    end = gap->chromStart;
	    liftOutLine(out, gap->chrom, start, end, liftCount++, chrSize);
	    start = gap->chromEnd;
	    end = start;
	    }
	else
	    end = gap->chromEnd;
	}
    else	/* new chrom encountered */
	{ /* output last segment of previous chrom when necessary */
	if (prevChr && differentWord(prevChr, gap->chrom))
	    {
	    if (end < chrSize)
		liftOutLine(out, prevChr, start, chrSize, liftCount++, chrSize);
	    }
	chrSize = hashIntVal(cInfoHash, gap->chrom);
	if (gap->chromStart > 0)
	    {	/* starting first segment at position 0 */
	    start = 0;
	    end = gap->chromStart;
	    /* does the first gap break it ?  Or gap goes to end of chrom. */
	    if (sameWord("no",gap->bridge) || (gap->chromEnd == chrSize))
		{
		liftOutLine(out, gap->chrom, start, end, liftCount++, chrSize);
		start = gap->chromEnd;
		end = start;
		}
	    }
	else	/* first gap is actually the beginning of the chrom */
	    {	/* thus, first segment starts after this first gap */
	    start = gap->chromEnd;
	    end = start;
	    }
	}
    prevChr = gap->chrom;	/* remember prev chrom to detect next chrom */
    }
/* potentially a last one */
if (end < chrSize)
    liftOutLine(out, prevChr, start, chrSize, liftCount++, chrSize);
carefulClose(&out);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
workChr = optionVal("chr", NULL);
bedFileName = optionVal("bedFile", NULL);
sane = optionExists("sane");
gapToLift(argv[1], argv[2]);
return 0;
}
