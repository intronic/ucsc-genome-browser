/* bam - stuff to handle BAM stuff in table browser. */

#ifdef USE_BAM

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "localmem.h"
#include "jksql.h"
#include "cheapcgi.h"
#include "cart.h"
#include "web.h"
#include "bed.h"
#include "hdb.h"
#include "trackDb.h"
#include "obscure.h"
#include "hmmstats.h"
#include "correlate.h"
#include "asParse.h"
#include "bbiFile.h"
#include "bigBed.h"
#include "hubConnect.h"
#include "hgTables.h"
#include "asFilter.h"
#include "bamFile.h"
#include "samAlignment.h"
#if (defined USE_BAM && defined KNETFILE_HOOKS)
#include "knetUdc.h"
#include "udc.h"
#endif//def USE_BAM && KNETFILE_HOOKS


boolean isBamTable(char *table)
/* Return TRUE if table corresponds to a BAM file. */
{
if (isHubTrack(table))
    {
    struct trackDb *tdb = hashFindVal(fullTrackAndSubtrackHash, table);
    return startsWithWord("bam", tdb->type);
    }
else
    return trackIsType(database, table, curTrack, "bam", ctLookupName);
}

char *bamFileName(char *table, struct sqlConnection *conn)
/* Return file name associated with BAM.  This handles differences whether it's
 * a custom or built-in track.  Do a freeMem on returned string when done. */
{
/* Implementation is same as bigWig. */
return bigWigFileName(table, conn);
}

char *bamAsDef = 
"table samAlignment\n"
"\"The fields of a SAM short read alignment, the text version of BAM.\"\n"
"    (\n"
"    string qName;	\"Query template name - name of a read\"\n"
"    ushort flag;	\"Flags.  0x10 set for reverse complement.  See SAM docs for others.\"\n"
"    string rName;	\"Reference sequence name (often a chromosome)\"\n"
"    uint pos;		\"1 based position\"\n"
"    ubyte mapQ;		\"Mapping quality 0-255, 255 is best\"\n"
"    string cigar;	\"CIGAR encoded alignment string.\"\n"
"    string rNext;	\"Ref sequence for next (mate) read. '=' if same as rName, '*' if no mate\"\n"
"    int pNext;		\"Position (1-based) of next (mate) sequence. May be -1 or 0 if no mate\"\n"
"    int tLen;	        \"Size of DNA template for mated pairs.  -size for one of mate pairs\"\n"
"    string seq;		\"Query template sequence\"\n"
"    string qual;	\"ASCII of Phred-scaled base QUALity+33.  Just '*' if no quality scores\"\n"
"    string tagTypeVals; \"Tab-delimited list of tag:type:value optional extra fields\"\n"
"    )\n";

struct asObject *bamAsObj()
/* Return asObject describing fields of BAM */
{
return asParseText(bamAsDef);
}

struct hTableInfo *bamToHti(char *table)
/* Get standard fields of BAM into hti structure. */
{
struct hTableInfo *hti;
AllocVar(hti);
hti->rootName = cloneString(table);
hti->isPos= TRUE;
strcpy(hti->chromField, "rName");
strcpy(hti->startField, "pos");
strcpy(hti->nameField, "qName");
hti->type = cloneString("bam");
return hti;
}

struct slName *bamGetFields(char *table)
/* Get fields of bam as simple name list. */
{
struct asObject *as = bamAsObj();
struct slName *names = asColNames(as);
return names;
}

struct sqlFieldType *bamListFieldsAndTypes()
/* Get fields of bigBed as list of sqlFieldType. */
{
struct asObject *as = bamAsObj();
struct sqlFieldType *list = sqlFieldTypesFromAs(as);
return list;
}

#define BAM_NUM_BUF_SIZE 256

void samAlignmentToRow(struct samAlignment *sam, char *numBuf, char *row[SAMALIGNMENT_NUM_COLS])
/* Convert samAlignment data structure to an array of strings, using numBuf to store
 * ascii versions of numbers temporarily */
{
char *numPt = numBuf;
char *numBufEnd = numBuf + BAM_NUM_BUF_SIZE;

row[0] = sam->qName;
row[1] = numPt; numPt += sprintf(numPt, "%u", sam->flag); numPt += 1;
row[2] = sam->rName;
row[3] = numPt; numPt += sprintf(numPt, "%u", sam->pos); numPt += 1;
row[4] = numPt; numPt += sprintf(numPt, "%u", sam->mapQ); numPt += 1;
row[5] = sam->cigar;
row[6] = sam->rNext;
row[7] = numPt; numPt += sprintf(numPt, "%d", sam->pNext); numPt += 1;
row[8] = numPt; numPt += sprintf(numPt, "%d", sam->tLen); numPt += 1;
row[9] = sam->seq;
row[10] = sam->qual;
row[11] = sam->tagTypeVals;
assert(numPt < numBufEnd);
}

void bamTabOut(char *db, char *table, struct sqlConnection *conn, char *fields, FILE *f)
/* Print out selected fields from BAM.  If fields is NULL, then print out all fields. */
{
if (f == NULL) 
    f = stdout;

/* Convert comma separated list of fields to array. */
int fieldCount = chopByChar(fields, ',', NULL, 0);
char **fieldArray;
AllocArray(fieldArray, fieldCount);
chopByChar(fields, ',', fieldArray, fieldCount);

/* Get list of all fields in big bed and turn it into a hash of column indexes keyed by
 * column name. */
struct hash *fieldHash = hashNew(0);
struct slName *bb, *bbList = bamGetFields(table);
int i;
for (bb = bbList, i=0; bb != NULL; bb = bb->next, ++i)
    hashAddInt(fieldHash, bb->name, i);

/* Create an array of column indexes corresponding to the selected field list. */
int *columnArray;
AllocArray(columnArray, fieldCount);
for (i=0; i<fieldCount; ++i)
    {
    columnArray[i] = hashIntVal(fieldHash, fieldArray[i]);
    }

/* Output row of labels */
fprintf(f, "#%s", fieldArray[0]);
for (i=1; i<fieldCount; ++i)
    fprintf(f, "\t%s", fieldArray[i]);
fprintf(f, "\n");

char *fileName = bamFileName(table, conn);
struct asObject *as = bamAsObj();
struct asFilter *filter = NULL;

if (anyFilter())
    {
    filter = asFilterFromCart(cart, db, table, as);
    if (filter)
        {
	fprintf(f, "# Filtering on %d columns\n", slCount(filter->columnList));
	}
    }

/* Loop through outputting each region */
struct region *region, *regionList = getRegions();
for (region = regionList; region != NULL; region = region->next)
    {
    struct lm *lm = lmInit(0);
    struct samAlignment *sam, *samList = bamFetchSamAlignment(fileName, region->chrom,
    	region->start, region->end, lm);
    char *row[SAMALIGNMENT_NUM_COLS];
    char numBuf[BAM_NUM_BUF_SIZE];
    for (sam = samList; sam != NULL; sam = sam->next)
        {
	samAlignmentToRow(sam, numBuf, row);
	if (asFilterOnRow(filter, row))
	    {
	    int i;
	    fprintf(f, "%s", row[columnArray[0]]);
	    for (i=1; i<fieldCount; ++i)
		fprintf(f, "\t%s", row[columnArray[i]]);
	    fprintf(f, "\n");
	    }
	}
    lmCleanup(&lm);
    }

/* Clean up and exit. */
hashFree(&fieldHash);
freeMem(fieldArray);
freeMem(columnArray);
}

int cigarWidth(char *cigar, int cigarSize)
/* Return width of alignment as encoded in cigar format string. */
{
int tLength=0;
char *s, *end = cigar + cigarSize;
s = cigar;
while (s < end)
    {
    int digCount = countLeadingDigits(s);
    if (digCount <= 0)
        errAbort("expecting number got %s in cigarWidth", s);
    int n = atoi(s);
    s += digCount;
    char op = *s++;
    switch (op)
	{
	case 'M': // match or mismatch (gapless aligned block)
	    tLength += n;
	    break;
	case 'I': // inserted in query
	    break;
	case 'D': // deleted from query
	case 'N': // long deletion from query (intron as opposed to small del)
	    tLength += n;
	    break;
	case 'S': // skipped query bases at beginning or end ("soft clipping")
	case 'H': // skipped query bases not stored in record's query sequence ("hard clipping")
	case 'P': // P="silent deletion from padded reference sequence" -- ignore these.
	    break;
	default:
	    errAbort("cigarWidth: unrecognized CIGAR op %c -- update me", op);
	}
    }
return tLength;
}

static void addFilteredBedsOnRegion(char *fileName, struct region *region, 
	char *table, struct asFilter *filter, struct lm *bedLm, struct bed **pBedList)
/* Add relevant beds in reverse order to pBedList */
{
struct lm *lm = lmInit(0);
struct samAlignment *sam, *samList = bamFetchSamAlignment(fileName, region->chrom,
    	region->start, region->end, lm);
char *row[SAMALIGNMENT_NUM_COLS];
char numBuf[BAM_NUM_BUF_SIZE];
for (sam = samList; sam != NULL; sam = sam->next)
    {
    samAlignmentToRow(sam, numBuf, row);
    if (asFilterOnRow(filter, row))
        {
	struct bed *bed;
	lmAllocVar(bedLm, bed);
	bed->chrom = lmCloneString(bedLm, sam->rName);
	bed->chromStart = sam->pos - 1;
	bed->chromEnd = bed->chromStart + cigarWidth(sam->cigar, strlen(sam->cigar));
	bed->name = lmCloneString(bedLm, sam->qName);
	slAddHead(pBedList, bed);
	}
    }
lmCleanup(&lm);
}

struct bed *bamGetFilteredBedsOnRegions(struct sqlConnection *conn, 
	char *db, char *table, struct region *regionList, struct lm *lm, 
	int *retFieldCount)
/* Get list of beds from BAM, in all regions, that pass filtering. */
{
/* Figure out bam file name get column info and filter. */
char *fileName = bamFileName(table, conn);
struct asObject *as = bamAsObj();
struct asFilter *filter = asFilterFromCart(cart, db, table, as);

/* Get beds a region at a time. */
struct bed *bedList = NULL;
struct region *region;
for (region = regionList; region != NULL; region = region->next)
    addFilteredBedsOnRegion(fileName, region, table, filter, lm, &bedList);
slReverse(&bedList);
return bedList;
}

void showSchemaBam(char *table)
/* Show schema on bam. */
{
struct sqlConnection *conn = hAllocConn(database);
char *fileName = bamFileName(table, conn);

struct asObject *as = bamAsObj();
hPrintf("<B>Database:</B> %s", database);
hPrintf("&nbsp;&nbsp;&nbsp;&nbsp;<B>Primary Table:</B> %s<br>", table);
hPrintf("<B>BAM File:</B> %s", fileName);
hPrintf("<BR>\n");
hPrintf("<B>Format description:</B> %s<BR>", as->comment);
hPrintf("See the <A HREF=\"%s\" target=_blank>SAM Format Specification</A> for  more details<BR>\n",
	"http://samtools.sourceforge.net/SAM-1.3.pdf");

/* Put up table that describes fields. */
hTableStart();
hPrintf("<TR><TH>field</TH>");
hPrintf("<TH>description</TH> ");
puts("</TR>\n");
struct asColumn *col;
int colCount = 0;
for (col = as->columnList; col != NULL; col = col->next)
    {
    hPrintf("<TR><TD><TT>%s</TT></TD>", col->name);
    hPrintf("<TD>%s</TD></TR>", col->comment);
    ++colCount;
    }
hTableEnd();

/* In a perfect world would print sample rows here.  Maybe later.... */

/* Clean up and go home. */
freeMem(fileName);
hFreeConn(&conn);
}

#endif /* USE_BAM */

