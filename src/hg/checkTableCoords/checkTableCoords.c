/* checkTableCoords - Check several invariants on genomic coordinates of 
 * each item in all table in the specified db (or just the specified table). */
#include "common.h"
#include "options.h"
#include "verbose.h"
#include "jksql.h"
#include "hash.h"
#include "dystring.h"
#include "hdb.h"
#include "portable.h"

static char const rcsid[] = "$Id: checkTableCoords.c,v 1.13 2006/04/04 23:57:26 angie Exp $";

/* Default parameter values */
char *db = NULL;                        /* first arg */
char *theTable = NULL;                  /* -table option or second arg */
int hoursOld = 0;                       /* -daysOld, -hoursOld options */
char *alwaysExclude = "trackDb*,hgFindSpec*,tableDescriptions,all_mrna,all_est";
                                        /* always exclude these patterns */
char *genbankExclude = "*mrna,*_est,*intronEst,xeno*,ref*,gb*,mgc*,mrna,"
                       "author,cds,description,geneName,imageClone,"
                       "mrnaClone,estOrientInfo,mrnaOrientInfo,productName";
                                        /* expand "genbank" to these patterns */
struct slName *excludePatterns = NULL;  /* -exclude option + alwaysExclude */
boolean ignoreBlocks = FALSE;           /* skip block checks to save time */
boolean verboseBlocks = FALSE;          /* more details about block problems */
boolean justList = FALSE;               /* list tables to check, then exit */
boolean allowThickZero = TRUE;          /* OK for thickStart==thickEnd==0 */

/* Command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"table",         OPTION_STRING},
    {"daysOld",       OPTION_INT},
    {"hoursOld",      OPTION_INT},
    {"exclude",       OPTION_STRING},
    {"ignoreBlocks",  OPTION_BOOLEAN},
    {"verboseBlocks", OPTION_BOOLEAN},
    {"justList",      OPTION_BOOLEAN},
    {NULL, 0}
};

/* Err strings for reportErrors(), all formatted with a %s followed by a %d */
#define START_LT_ZERO "%s.%s has %d records with start < 0.\n"
#define END_LT_START "%s.%s has %d records with end < start.\n"
#define END_GT_CHROMSIZE "%s.%s has %d records with end > chromSize.\n"
#define CDSSTART_LT_START "%s.%s has %d records with cdsStart < start.\n"
#define CDSEND_LT_CDSSTART "%s.%s has %d records with cdsEnd < cdsStart.\n"
#define CDSEND_GT_END "%s.%s has %d records with cdsEnd > end.\n"
#define ONLY_CDSSTART_ZERO "%s.%s has %d records where cdsStart is 0 but not cdsEnd.\n"
#define SPLIT_WRONG_CHROM "%s.%s has %d records with chrom inconsistent with table name.\n"
#define BAD_CHROM "%s.%s has %d records with chrom not described in chromInfo.\n"
#define BLOCKSTART_NOT_START "%s.%s has %d records with blockStart[0] != start.\n"
#define BLOCKSTART_LT_START "%s.%s has %d records with blockStart[i] < start.\n"
#define BLOCKEND_LT_BLOCKSTART "%s.%s has %d records with blockEnd[i] < blockStart[i].\n"
#define BLOCKS_NOT_ASCEND "%s.%s has %d records with blocks not in ascending order.\n"
#define BLOCKS_OVERLAP "%s.%s has %d records with overlapping blocks.\n"
#define BLOCKEND_GT_END "%s.%s has %d records with blockEnd[i] > end.\n"
#define BLOCKEND_NOT_END "%s.%s has %d records with blockEnd[n-1] != end.\n"


void usage()
{
errAbort(
"checkTableCoords - check invariants on genomic coords in table(s).\n"
"usage:\n"
"  checkTableCoords database [tableName]\n"
"Searches for illegal genomic coordinates in all tables in database\n"
"unless narrowed down using options.  Uses ~/.hg.conf to determine\n"
"genome database connection info.  For psl/alignment tables, checks\n"
"target coords only.\n"
"options:\n"
"  -table=tableName  Check this table only.  (Default: all tables)\n"
"  -daysOld=N        Check tables that have been modified at most N days ago.\n"
"  -hoursOld=N       Check tables that have been modified at most N hours ago.\n"
"                    (days and hours are additive)\n"
"  -exclude=patList  Exclude tables matching any pattern in comma-separated\n"
"                    patList.  patList can contain wildcards (*?) but should\n"
"                    be escaped or single-quoted if it does.  patList can\n"
"                    contain \"genbank\" which will be expanded to all tables\n"
"                    generated by the automated genbank build process.\n"
"  -ignoreBlocks     To save time (but lose coverage), skip block coord checks.\n"
"  -verboseBlocks    Print out more details about illegal block coords, since \n"
"                    they can't be found by simple SQL queries.\n"
);
}


struct slName *getTableNames(struct sqlConnection *conn)
/* Return a list of names of tables that have not been excluded by 
 * command line options. */
{
char *query = hoursOld ? "show table status" : "show tables";
struct sqlResult *sr = sqlGetResult(conn, query);
struct slName *tableList = NULL;
char **row = NULL;
int startTime = clock1();
int ageThresh = hoursOld * 3600;

while((row = sqlNextRow(sr)) != NULL)
    {
    struct slName *tableName = NULL;
    struct slName *pat = NULL;
    boolean gotMatch = FALSE;
    if (hoursOld)
	{
	int tableUpdateTime = sqlDateToUnixTime(row[11]);
	int ageInSeconds = startTime - tableUpdateTime;
	if (ageInSeconds > ageThresh)
	    continue;
	}
    for (pat = excludePatterns;  pat != NULL;  pat=pat->next)
	{
	if (wildMatch(pat->name, row[0]))
	    {
	    gotMatch = TRUE;
	    break;
	    }
	}
    if (gotMatch)
	continue;
    if (verboseLevel() >= 3 || justList)
	fprintf(stderr, "Adding %s\n", row[0]);
    tableName = newSlName(row[0]);
    slAddHead(&tableList, tableName);
    }
sqlFreeResult(&sr);
if (justList)
    exit(0);
slReverse(&tableList);
return tableList;
}


boolean reportErrors(char *errFormat, char *table, int numErrors)
/* If numErrors != 0, print it out in %s,%d-formatted errFormat & ret TRUE. */
{
boolean gotError = (numErrors != 0);
if (gotError)
    printf(errFormat, hGetDb(), table, numErrors);
return gotError;
}


boolean splitAndNonSplitExist(struct sqlConnection *conn, char *table,
			      char *tableChrom)
/* Warn and return TRUE if table exists in both split and non-split versions 
 * (except for mrna) */
{
boolean gotBoth = FALSE;
if (! startsWith(tableChrom, table))
    {
    char splitTable[512];
    safef(splitTable, sizeof(splitTable), "%s_%s", tableChrom, table);
    if (sqlTableExists(conn, splitTable))
	{
	if (! sameString(table, "mrna"))
	    printf("%s: Table %s exists in both split and non-split forms!  "
		   "Ignoring the non-split form.\n", db, table);
	gotBoth = TRUE;
	}
    }
return(gotBoth);
}


boolean checkSplitTableOnlyChrom(struct bed *bedList, char *table,
				 struct hTableInfo *hti, char *tableChrom)
/* invariant: if table is split and has a chrom field, then all 
 * entries in the table will have chrom matching the table name.
 * Return TRUE if any errors. */
{
struct bed *bed = NULL;
boolean gotError = FALSE;
int errCount = 0;
for (bed = bedList;  bed != NULL;  bed = bed->next)
    {
    if (! sameString(bed->chrom, tableChrom))
	{
	verbose(2, "%s.%s item %s %s:%d-%d has %s = \"%s\" "
		"inconsistent with split table name\n",
		db, table,
		bed->name, bed->chrom, bed->chromStart, bed->chromEnd,
		hti->chromField, bed->chrom);
	errCount++;
	}
    gotError |= reportErrors(SPLIT_WRONG_CHROM, table, errCount);
    }
return(gotError);
}

boolean checkStartEnd(struct bed *bedList, char *table, struct hTableInfo *hti,
		      int chromSize)
/* invariant: 0 <= start <= end <= chromSize.  Return TRUE if any problems. */
/* NOTE: to save time, all elements of bedList are assumed to be from the 
 * same chrom!  so chromSize can be applied to all. */
{
struct bed *bed = NULL;
boolean gotError = 0;
int startLTZ = 0, endLTStart = 0, endGTChrom = 0;
for (bed = bedList;  bed != NULL;  bed = bed->next)
    {
    if (bed->chromStart < 0)
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s < 0\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->startField);
	startLTZ++;
	}
    if (bed->chromEnd < bed->chromStart)
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s < %s\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->endField, hti->startField);
	endLTStart++;
	}
    if (bed->chromEnd > chromSize)
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s > chromSize %d\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->endField, chromSize);
	endGTChrom++;
	}
    }
gotError |= reportErrors(START_LT_ZERO, table, startLTZ);
gotError |= reportErrors(END_LT_START, table, endLTStart);
gotError |= reportErrors(END_GT_CHROMSIZE, table, endGTChrom);
return gotError;
}

boolean checkCDSStartEnd(struct bed *bedList, char *table,
			 struct hTableInfo *hti)
/* invariant: start <= thickStart <= thickEnd <= end, 
 * unless allowThickZero and thickStart == thickEnd == 0.
 * Return TRUE if any problems. */
{
boolean gotError = FALSE;
struct bed *bed = NULL;
int cdsSLTStart=0, cdsELTCS=0, cdsEGTEnd=0, cdsOnlySZ=0;
for (bed = bedList;  bed != NULL;  bed = bed->next)
    {
    if (bed->thickStart < bed->chromStart &&
	!(allowThickZero && bed->thickStart == 0))
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s (%d) < %s (%d)\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->cdsStartField, bed->thickStart,
		hti->startField, bed->chromStart);
	cdsSLTStart++;
	}
    if (bed->thickEnd < bed->thickStart)
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s (%d) < %s (%d)\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->cdsEndField, bed->thickEnd,
		hti->cdsStartField, bed->thickStart);
	cdsELTCS++;
	}
    if (bed->chromEnd < bed->thickEnd)
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s (%d) > %s (%d)\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->cdsEndField, bed->thickEnd,
		hti->endField, bed->chromEnd);
	cdsEGTEnd++;
	}
    if (allowThickZero &&
	(bed->thickStart == 0 && bed->thickEnd != 0 && bed->chromStart != 0))
	{
	verbose(2, "%s.%s item %s %s:%d-%d: %s (%d) > 0 but not %s (%d)\n",
		db, table, bed->name ? bed->name : "",
		bed->chrom, bed->chromStart, bed->chromEnd,
		hti->cdsEndField, bed->thickEnd,
		hti->cdsStartField, bed->thickStart);
	cdsOnlySZ++;
	}
    }
gotError |= reportErrors(CDSSTART_LT_START, table, cdsSLTStart);
gotError |= reportErrors(CDSEND_LT_CDSSTART, table, cdsELTCS);
gotError |= reportErrors(CDSEND_GT_END, table, cdsEGTEnd);
gotError |= reportErrors(ONLY_CDSSTART_ZERO, table, cdsOnlySZ);
return gotError;
}

boolean checkBlocks(struct bed *bedList, char *table, struct hTableInfo *hti)
/* invariant: blockEnd[i-1] <= blockStart[i] <= blockEnd[i] ... *
 * invariant: blockStarts[0] == start *
 * invariant: blockEnds[n-1] == end *
 * Return TRUE if any problems. */
/* checked by hGetBedRange: invariant: blockCount = #blocks */
{
boolean gotError = FALSE;
struct bed *bed = NULL;
int bSNotStart=0, bSLTStart=0, bELTBS=0, bENotEnd=0, bEGTEnd=0;
int bNotAscend=0, bOverlap=0;
for (bed = bedList;  bed != NULL;  bed = bed->next)
    {
    int i=0, lastStart=0, lastEnd=0;
    if (bed->chromStarts[0] != 0)
	{
	if (verboseBlocks || verboseLevel() >= 2)
	    verbose(0, "%s.%s item %s %s:%d-%d: blockStarts[0] (relative) should be 0, but is %d\n",
		   db, table, bed->name, bed->chrom,
		   bed->chromStart, bed->chromEnd,
		   bed->chromStarts[0]);
	bSNotStart++;
	}
    lastStart = lastEnd = 0;
    for (i=0;  i < bed->blockCount;  i++)
	{
	if (bed->chromStarts[i] < 0)
	    {
	    if (verboseBlocks || verboseLevel() >= 2)
		verbose(0, "%s.%s item %s %s:%d-%d: start of block %d (%d) is less than start.\n",
		       db, table, bed->name, bed->chrom,
		       bed->chromStart, bed->chromEnd,
		       i, bed->chromStart + bed->chromStarts[i]);
	    bSLTStart++;
	    }
	if (bed->chromStarts[i] < lastStart)
	    {
	    if (verboseBlocks || verboseLevel() >= 2)
		verbose(0, "%s.%s item %s %s:%d-%d: starts of blocks %d and %d not in ascending order.\n",
		       db, table, bed->name, bed->chrom,
		       bed->chromStart, bed->chromEnd,
		       i-1, i);
	    bNotAscend++;
	    }
	if (bed->chromStarts[i] < lastEnd &&
	    i > 0 && bed->chromStarts[i] > bed->chromStarts[i-1])
	    {
	    if (verboseBlocks || verboseLevel() >= 2)
		verbose(0, "%s.%s item %s %s:%d-%d: blocks %d and %d overlap.\n",
		       db, table, bed->name, bed->chrom,
		       bed->chromStart, bed->chromEnd,
		       i-1, i);
	    bOverlap++;
	    }
	if (bed->blockSizes[i] < 0)
	    {
	    if (verboseBlocks || verboseLevel() >= 2)
		verbose(0, "%s.%s item %s %s:%d-%d: blockSize[%d] is %d.\n",
		       db, table, bed->name, bed->chrom,
		       bed->chromStart, bed->chromEnd,
		       i, bed->blockSizes[i]);
	    bELTBS++;
	    }
	if (bed->chromStart + bed->chromStarts[i] + bed->blockSizes[i]
	    > bed->chromEnd)
	    {
	    if (verboseBlocks || verboseLevel() >= 2)
		verbose(0, "%s.%s item %s %s:%d-%d: blockEnd[%d] (%d) > chromEnd.\n",
		       db, table, bed->name, bed->chrom,
		       bed->chromStart, bed->chromEnd,
		       i, (bed->chromStart + bed->chromStarts[i] +
			   bed->blockSizes[i]));
	    bEGTEnd++;
	    }
	lastStart = bed->chromStarts[i];
	lastEnd = bed->chromStarts[i] + bed->blockSizes[i];
	}
    if ((bed->chromStart + lastEnd) != bed->chromEnd)
	{
	if (verboseBlocks || verboseLevel() >= 2)
	    verbose(0, "%s.%s item %s %s:%d-%d: end of last block (%d) is not the same as chromEnd (%d).\n",
		   db, table, bed->name, bed->chrom,
		   bed->chromStart, bed->chromEnd,
		   (bed->chromStart + lastEnd), bed->chromEnd);
	bENotEnd++;
	}
    }
gotError |= reportErrors(BLOCKSTART_NOT_START, table, bSNotStart);
gotError |= reportErrors(BLOCKSTART_LT_START, table, bSLTStart);
gotError |= reportErrors(BLOCKEND_LT_BLOCKSTART, table, bELTBS);
gotError |= reportErrors(BLOCKS_NOT_ASCEND, table, bNotAscend);
gotError |= reportErrors(BLOCKS_OVERLAP, table, bOverlap);
gotError |= reportErrors(BLOCKEND_GT_END, table, bEGTEnd);
gotError |= reportErrors(BLOCKEND_NOT_END, table, bENotEnd);
return gotError;
}


int checkTableCoords(char *db)
/* Check several invariants (see comments in check*() above), 
 * summarize errors, return nonzero if there are errors. */
{
struct sqlConnection *conn = hAllocOrConnect(db);
struct slName *tableList = NULL, *curTable = NULL;
struct slName *allChroms = NULL;
boolean gotError = FALSE;

hSetDb(db);
allChroms = hAllChromNames();
if (theTable == NULL)
    tableList = getTableNames(conn);
else if (sqlTableExists(conn, theTable))
    tableList = newSlName(theTable);
else
    errAbort("Error: specified table \"%s\" does not exist in database %s.",
	     theTable, db);

for (curTable = tableList;  curTable != NULL;  curTable = curTable->next)
    {
    struct hTableInfo *hti = NULL;
    struct slName *chromList = NULL, *chromPtr = NULL;
    char *table = curTable->name;
    char tableChrom[32], trackName[128], tableChromPrefix[33];
    boolean gotError = FALSE;
    hParseTableName(table, trackName, tableChrom);
    hti = hFindTableInfo(tableChrom, trackName);
    if (hti != NULL && hti->isPos)
	{
	/* watch out for presence of both split and non-split tables; 
	 * hti for non-split will be replaced with hti of split. */
	if (splitAndNonSplitExist(conn, table, tableChrom))
	    continue;
	safef(tableChromPrefix, sizeof(tableChromPrefix), "%s_", tableChrom);
	if (hti->isSplit)
	    chromList = newSlName(tableChrom);
	else
	    chromList = allChroms;
	/* invariant: chrom must be described in chromInfo. */
        /* items with bad chrom will be invisible to hGetBedRange(), so 
	 * catch them here by SQL query. */
	/* The SQL query is too huge for scaffold-based db's, check count: */
	if (hChromCount() <= HDB_MAX_SEQS_FOR_SPLIT)
	    {
	    if (isNotEmpty(hti->chromField))
		{
		struct dyString *bigQuery = newDyString(1024);
		dyStringClear(bigQuery);
		dyStringPrintf(bigQuery, "select count(*) from %s where ",
			       table);
		for (chromPtr=chromList; chromPtr != NULL;
		       chromPtr=chromPtr->next)
		    {
		    dyStringPrintf(bigQuery, "%s != '%s' ",
				   hti->chromField, chromPtr->name);
		    if (chromPtr->next != NULL)
			dyStringAppend(bigQuery, "AND ");
		    }
		gotError |= reportErrors(BAD_CHROM, table,
					 sqlQuickNum(conn, bigQuery->string));
		dyStringFree(&bigQuery);
		}
	    for (chromPtr=chromList; chromPtr != NULL; chromPtr=chromPtr->next)
		{
		char *chrom = chromPtr->name;
		int chromSize = hChromSize(chrom);
		struct bed *bedList = hGetBedRange(table, chrom, 0, chromSize,
						   NULL);
		if (hti->isSplit && isNotEmpty(hti->chromField))
		    gotError |= checkSplitTableOnlyChrom(bedList, table, hti,
							 tableChrom);
		gotError |= checkStartEnd(bedList, table, hti,
					  hChromSize(chrom));
		if (hti->hasCDS)
		    gotError |= checkCDSStartEnd(bedList, table, hti);
		if (hti->hasBlocks && !ignoreBlocks)
		    gotError |= checkBlocks(bedList, table, hti);
		bedFreeList(&bedList);
		}
	    }
	}
    }
return gotError;
}


void processExcludes(char *exclude)
/* Combine alwaysExclude and command like -exclude arg (if given), and 
 * process into a list.  If it contains "genbank", add genbankExclude. */
{
struct dyString *allExcludes = newDyString(512);
char *patterns[128];
int numPats = 0, i = 0;

dyStringAppend(allExcludes, alwaysExclude);
if (exclude != NULL)
    dyStringPrintf(allExcludes, ",%s", exclude);
numPats = chopCommas(allExcludes->string, patterns);
for (i=0;  i < numPats;  i++)
    {
    if (sameWord(patterns[i], "genbank"))
	{
	char *gbPatterns[128];
	int gbNumPats = 0, j = 0;
	char *gbExclude = cloneString(genbankExclude);
	gbNumPats = chopCommas(gbExclude, gbPatterns);
	for (j=0;  j < gbNumPats;  j++)
	    {
	    struct slName *pat = newSlName(gbPatterns[j]);
	    slAddHead(&excludePatterns, pat);
	    }
	freeMem(gbExclude);
	}
    else
	{
	struct slName *pat = newSlName(patterns[i]);
	slAddHead(&excludePatterns, pat);
	}
    }
dyStringFree(&allExcludes);
}


int main(int argc, char *argv[])
{
char *exclude = NULL;
int daysOld = 0;

optionInit(&argc, argv, optionSpecs);
theTable     = optionVal("table", theTable);
daysOld      = optionInt("daysOld", daysOld);
hoursOld     = optionInt("hoursOld", hoursOld) + (24 * daysOld);
exclude      = optionVal("exclude", exclude);
ignoreBlocks = optionExists("ignoreBlocks");
verboseBlocks= optionExists("verboseBlocks");
justList     = optionExists("justList");

processExcludes(exclude);

db = argv[1];
/* Allow "checkTableCoords db table" usage too: */
if (theTable == NULL && argc == 3)
    {
    theTable = argv[2];
    argc = 2;
    }
if (argc != 2)
    usage();
verboseSetLogFile("stdout");
return checkTableCoords(argv[1]);
}
