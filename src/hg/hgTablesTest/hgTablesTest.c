/* hgTablesTest - Test hgTables web page. */
#include "common.h"
#include "memalloc.h"
#include "linefile.h"
#include "hash.h"
#include "htmshell.h"
#include "portable.h"
#include "options.h"
#include "errCatch.h"
#include "ra.h"
#include "htmlPage.h"
#include "../hgTables/hgTables.h"
#include "hdb.h"
#include "qa.h"
#include "chromInfo.h"

static char const rcsid[] = "$Id: hgTablesTest.c,v 1.14 2004/11/08 19:47:07 kent Exp $";

/* Command line variables. */
char *clOrg = NULL;	/* Organism from command line. */
char *clDb = NULL;	/* DB from command line */
int clGroups = BIGNUM;	/* Number of groups to test. */
int clTracks = 2;	/* Number of track to test. */
int clTables = 2;	/* Number of tables to test. */
int clDbs = 1;		/* Number of databases per organism. */
int clOrgs = 2;		/* Number of organisms to test. */

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgTablesTest - Test hgTables web page\n"
  "usage:\n"
  "   hgTablesTest url log\n"
  "options:\n"
  "   -org=Human - Restrict to Human (or Mouse, Fruitfly, etc.)\n"
  "   -db=hg17 - Restrict to particular database\n"
  "   -orgs=N - Number of organisms to test.  Default %d\n"
  "   -dbs=N - Number of databases per organism to test. Default %d\n"
  "   -groups=N - Number of groups to test (default all)\n"
  "   -tracks=N - Number of tracks per group to test (default %d)\n"
  "   -tables=N - Number of tables per track to test (deault %d)\n"
  "   -verbose=N - Set to 0 for silent operation, 2 for debugging\n"
  , clOrgs, clDbs, clTracks, clTables);
}

FILE *logFile;	/* Log file. */

static struct optionSpec options[] = {
   {"org", OPTION_STRING},
   {"db", OPTION_STRING},
   {"orgs", OPTION_INT},
   {"dbs", OPTION_INT},
   {"search", OPTION_STRING},
   {"groups", OPTION_INT},
   {"tracks", OPTION_INT},
   {"tables", OPTION_INT},
   {NULL, 0},
};

struct tablesTest
/* Test on one column. */
    {
    struct tablesTest *next;
    struct qaStatus *status;	/* Result of test. */
    char *info[6];
    };

enum tablesTestInfoIx {
   ntiiType,
   ntiiOrg,
   ntiiDb,
   ntiiGroup,
   ntiiTrack,
   ntiiTable,
   ntiiTotalCount,
};

char *tablesTestInfoTypes[] =
   { "type", "organism", "db", "group", "track", "table"};

struct tablesTest *tablesTestList = NULL;	/* List of all tests, latest on top. */

struct tablesTest *tablesTestNew(struct qaStatus *status,
	char *type, char *org, char *db, char *group, 
	char *track, char *table)
/* Save away test test results. */
{
struct tablesTest *test;
AllocVar(test);
test->status = status;
test->info[ntiiType] = cloneString(naForNull(type));
test->info[ntiiOrg] = cloneString(naForNull(org));
test->info[ntiiDb] = cloneString(naForNull(db));
test->info[ntiiGroup] = cloneString(naForNull(group));
test->info[ntiiTrack] = cloneString(naForNull(track));
test->info[ntiiTable] = cloneString(naForNull(table));
slAddHead(&tablesTestList, test);
return test;
}

void tablesTestLogOne(struct tablesTest *test, FILE *f)
/* Log test result to file. */
{
int i;
for (i=0; i<ArraySize(test->info); ++i)
    fprintf(f, "%s ", test->info[i]);
fprintf(f, "%s\n", test->status->errMessage);
}

struct htmlPage *quickSubmit(struct htmlPage *basePage,
	char *org, char *db, char *group, char *track, char *table,
	char *testName, char *button, char *buttonVal)
/* Submit page and record info.  Return NULL if a problem. */
{
struct tablesTest *test;
struct qaStatus *qs;
struct htmlPage *page;
verbose(2, "quickSubmit(%p, %s, %s, %s, %s, %s, %s, %s, %s)\n",
	basePage, org, db, group, track, table, testName, button, buttonVal);
if (basePage != NULL)
    {
    if (db != NULL)
	htmlPageSetVar(basePage, NULL, "db", db);
    if (org != NULL)
	htmlPageSetVar(basePage, NULL, "org", org);
    if (group != NULL)
        htmlPageSetVar(basePage, NULL, hgtaGroup, group);
    if (track != NULL)
        htmlPageSetVar(basePage, NULL, hgtaTrack, track);
    if (table != NULL)
        htmlPageSetVar(basePage, NULL, hgtaTable, table);
    qs = qaPageFromForm(basePage, basePage->forms, 
	    button, buttonVal, &page);
    test = tablesTestNew(qs, testName, org, db, group, track, table);
    }
return page;
}

void serialSubmit(struct htmlPage **pPage,
	char *org, char *db, char *group, char *track, char *table,
	char *testName, char *button, char *buttonVal)
/* Submit page, replacing old page with new one. */
{
struct htmlPage *oldPage = *pPage;
if (oldPage != NULL)
    {
    *pPage = quickSubmit(oldPage, org, db, group, track, table,
    	testName, button, buttonVal);
    htmlPageFree(&oldPage);
    }
}

int tableSize(char *db, char *table)
/* Return number of rows in table. */
{
struct sqlConnection *conn = sqlConnect(db);
int size = sqlTableSize(conn, table);
sqlDisconnect(&conn);
return size;
}

void quickErrReport()
/* Report error at head of list if any */
{
struct tablesTest *test = tablesTestList;
if (test->status->errMessage != NULL)
    tablesTestLogOne(test, stderr);
}

void testSchema(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table)
/* Make sure schema page comes up. */
{
struct htmlPage *schemaPage = quickSubmit(tablePage, org, db, group,
        track, table, "schema", hgtaDoSchema, "submit");
htmlPageFree(&schemaPage);
}

void testSummaryStats(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table)
/* Make sure summary stats page comes up. */
{
if (htmlFormVarGet(mainForm, hgtaDoSummaryStats) != NULL)
    {
    struct htmlPage *statsPage = quickSubmit(tablePage, org, db, group,
    	track, table, "summaryStats", hgtaDoSummaryStats, "submit");
    htmlPageFree(&statsPage);
    }
}

boolean varIncludesType(struct htmlForm *form, char *var, char *value)
/* Return TRUE if value is one of the options for var. */
{
struct htmlFormVar *formVar = htmlFormVarGet(form, var);
if (formVar == NULL)
    errAbort("Couldn't find %s variable in form", var);
return slNameInList(formVar->values, value);
}

boolean outTypeAvailable(struct htmlForm *form, char *value)
/* Return true if outType options include value. */
{
return varIncludesType(form, hgtaOutputType, value);
}

int countNoncommentLines(char *s)
/* Count number of lines in s that don't start with # */
{
int count = 0;
s = skipLeadingSpaces(s);
while (s != NULL && s[0] != 0)
    {
    if (s[0] != '#')
	++count;
    s = strchr(s, '\n');
    s = skipLeadingSpaces(s);
    }
return count;
}

int testAllFields(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table)
/* Get all fields and return count of rows. */
{
struct htmlPage *outPage;
int rowCount = 0;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "primaryTable");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "allFields", hgtaDoTopSubmit, "submit");
rowCount = countNoncommentLines(outPage->htmlText);
htmlPageFree(&outPage);
return rowCount;
}

struct htmlFormVar *findPrefixedVar(struct htmlFormVar *list, char *prefix)
/* Find first var with given prefix in list. */
{
struct htmlFormVar *var;
for (var = list; var != NULL; var = var->next)
    {
    if (startsWith(prefix, var->name))
        return var;
    }
return NULL;
}

void checkExpectedSimpleRows(struct htmlPage *outPage, int expectedRows)
/* Make sure we got the number of rows we expect. */
{
if (outPage != NULL)
    {
    int rowCount = countNoncommentLines(outPage->htmlText);
    if (rowCount != expectedRows)
	qaStatusSoftError(tablesTestList->status, 
		"Got %d rows, expected %d", rowCount, expectedRows);
    }
}

void testOneField(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table, 
     int expectedRows)
/* Get one field and make sure the count agrees with expected. */
{
struct htmlPage *outPage;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "selectedFields");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "selFieldsPage", hgtaDoTopSubmit, "submit");
if (outPage != NULL)
    {
    struct htmlForm *form = outPage->forms;
    struct htmlFormVar *var;
    int rowCount = 0;
    if (form == NULL)
        errAbort("No forms in select fields page");
    var = findPrefixedVar(form->vars, "hgta_fs.check.");
    if (var == NULL)
        errAbort("No hgta_fs.check. vars in form");
    htmlPageSetVar(outPage, NULL, var->name, "on");
    serialSubmit(&outPage, org, db, group, track, table, "oneField",
    	hgtaDoPrintSelectedFields, "submit");
    checkExpectedSimpleRows(outPage, expectedRows);
    }
htmlPageFree(&outPage);
}
	
void testOutBed(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table, 
     int expectedRows)
/* Get as bed and make sure count agrees with expected. */
{
struct htmlPage *outPage;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "bed");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "bedUiPage", hgtaDoTopSubmit, "submit");
if (outPage != NULL)
    {
    serialSubmit(&outPage, org, db, group, track, table, "outBed",
    	hgtaDoGetBed, "submit");
    checkExpectedSimpleRows(outPage, expectedRows);
    }
htmlPageFree(&outPage);
}

void testOutGff(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table)
/* Get as GFF and make sure no crash. */
{
struct htmlPage *outPage;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "gff");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "outGff", hgtaDoTopSubmit, "submit");
htmlPageFree(&outPage);
}

int countTagsBetween(struct htmlPage *page, char *start, char *end, char *type)
/* Count number of tags of given type (which should be upper case)
 * between start and end. If start is NULL it will start from
 * beginning of page.  If end is NULL it will end at end of page. */
{
int count = 0;
struct htmlTag *tag;
if (start == NULL)
    start = page->htmlText;
if (end == NULL)
    end = start + strlen(start);
for (tag = page->tags; tag != NULL; tag = tag->next)
    {
    if (tag->start >= start && tag->start < end && sameString(tag->name, type))
	{
        ++count;
	}
    }
return count;
}

void testOutHyperlink(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table, 
     int expectedRows)
/* Get as hyperlink and make sure count agrees with expected. */
{
struct htmlPage *outPage;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "hyperlinks");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "outHyperlinks", hgtaDoTopSubmit, "submit");
if (outPage != NULL)
    {
    char *s = stringIn("<!--Content", outPage->htmlText);
    int rowCount;
    if (s == NULL) errAbort("Can't find <!-Content");
    rowCount = countTagsBetween(outPage, s, NULL, "A");
    if (rowCount != expectedRows)
	qaStatusSoftError(tablesTestList->status, 
		"Got %d rows, expected %d", rowCount, expectedRows);
    }
htmlPageFree(&outPage);
}

void testOutCustomTrack(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table)
/* Get as customTrack and make sure nothing explodes. */
{
struct htmlPage *outPage;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "customTrack");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "customTrackUi", hgtaDoTopSubmit, "submit");
if (outPage != NULL)
    {
    struct htmlFormVar *groupVar;
    serialSubmit(&outPage, org, db, group, track, table, "outCustom",
    	hgtaDoGetCustomTrackTb, "submit");
    if (outPage != NULL)
	{
	groupVar = htmlFormVarGet(outPage->forms, hgtaGroup);
	if (!slNameInList(groupVar->values, "user"))
	    {
	    qaStatusSoftError(tablesTestList->status, 
		    "No custom track group after custom track submission");
	    }
	}
    }
htmlPageFree(&outPage);
}

void checkFaOutput(struct htmlPage *page, int expectedCount, boolean lessOk)
/* Check that page contains expected number of sequences.  If lessOk is set
 * (needed to handle some multiply mapped cases in refSeq) then just check
 * that have at least one if expecting any. */
{
if (page != NULL)
    {
    char *s = page->htmlText;
    int count = countChars(page->htmlText, '>');
    if (count != expectedCount)
	{
	if (!lessOk || count > expectedCount || (expectedCount > 0 && count <= 0))
	    qaStatusSoftError(tablesTestList->status, 
		    "Got %d sequences, expected %d", count, expectedCount);
	}
    }
}

void testOutSequence(struct htmlPage *tablePage, struct htmlForm *mainForm,
     char *org, char *db, char *group, char *track, char *table, 
     int expectedRows)
/* Get as sequence and make sure count agrees with expected. */
{
struct htmlPage *outPage;

htmlPageSetVar(tablePage, NULL, hgtaOutputType, "sequence");
outPage = quickSubmit(tablePage, org, db, group, track, table,
    "seqUi1", hgtaDoTopSubmit, "submit");
if (outPage != NULL)
     {
     struct htmlFormVar *typeVar;

     /* Since some genomic sequence things are huge, this will
      * only test in case where it's a gene prediction. */
     typeVar = htmlFormVarGet(outPage->forms, hgtaGeneSeqType);
     if (typeVar != NULL)
         {
	 struct htmlPage *seqPage;
	 static char *types[] = {"protein", "mRNA"};
	 int i;
	 for (i=0; i<ArraySize(types); ++i)
	     {
	     char *type = types[i];
	     if (slNameInList(typeVar->values, type))
	         {
		 struct htmlPage *page;
		 char testName[128];
		 htmlPageSetVar(outPage, NULL, hgtaGeneSeqType, type);
		 safef(testName, sizeof(testName), "%sSeq", type);
		 page = quickSubmit(outPage, org, db, group, track, table,
		    testName, hgtaDoGenePredSequence, "submit");
		 checkFaOutput(page, expectedRows, TRUE);
		 htmlPageFree(&page);
		 }
	     }
	 htmlPageSetVar(outPage, NULL, hgtaGeneSeqType, "genomic");
	 serialSubmit(&outPage, org, db, group, track, table, "seqUi2",
	    hgtaDoGenePredSequence, "submit");

	 /* On genomic page uncheck intron if it's there, then get results
	  * and count them. */
	 if (htmlFormVarGet(outPage->forms, "hgSeq.intron") != NULL)
	     htmlPageSetVar(outPage, NULL, "hgSeq.intron", NULL);
	 seqPage = quickSubmit(outPage, org, db, group, track, table,
	      "genomicSeq", hgtaDoGenomicDna, "submit");
	 checkFaOutput(seqPage, expectedRows, FALSE);
	 htmlPageFree(&seqPage);
	 }

     }
htmlPageFree(&outPage);
}
	
	
void testOneTable(struct htmlPage *trackPage, char *org, char *db,
	char *group, char *track, char *table)
/* Test stuff on one table. */
{
struct htmlPage *tablePage = quickSubmit(trackPage, org, db, group, 
	track, table, "selectTable", hgtaTable, table);
struct htmlForm *mainForm;

if (tablePage != NULL)
    {
    if ((mainForm = htmlFormGet(tablePage, "mainForm")) == NULL)
	errAbort("Couldn't get main form on tablePage");
    testSchema(tablePage, mainForm, org, db, group, track, table);
    testSummaryStats(tablePage, mainForm, org, db, group, track, table);
    if (outTypeAvailable(mainForm, "bed")) 
        {
	if (outTypeAvailable(mainForm, "primaryTable"))
	    {
	    int rowCount;
	    rowCount = testAllFields(tablePage, mainForm, org, db, group, track, table);
	    testOneField(tablePage, mainForm, org, db, group, track, table, rowCount);
	    testOutSequence(tablePage, mainForm, org, db, group, track, table, rowCount);
	    testOutBed(tablePage, mainForm, org, db, group, track, table, rowCount);
	    testOutHyperlink(tablePage, mainForm, org, db, group, track, table, rowCount);
	    testOutGff(tablePage, mainForm, org, db, group, track, table);
	    if (rowCount > 0)
		testOutCustomTrack(tablePage, mainForm, org, db, group, track, table);
	    }
	}
    else if (outTypeAvailable(mainForm, "primaryTable"))
	{
	if (tableSize(db, table) < 500000)
	    {
	    int rowCount;
	    rowCount = testAllFields(tablePage, mainForm, org, db, group, track, table);
	    testOneField(tablePage, mainForm, org, db, group, track, table, rowCount);
	    }
	}
    verbose(1, "Tested %s %s %s %s %s\n", org, db, group, track, table);
    htmlPageFree(&tablePage);
    }
carefulCheckHeap();
}

void testOneTrack(struct htmlPage *groupPage, char *org, char *db,
	char *group, char *track, int maxTables)
/* Test a little something on up to maxTables in one track. */
{
struct htmlPage *trackPage = quickSubmit(groupPage, org, db, group, 
	track, NULL, "selectTrack", hgtaTrack, track);
struct htmlForm *mainForm;
struct htmlFormVar *tableVar;
struct slName *table;
int tableIx;

if ((mainForm = htmlFormGet(trackPage, "mainForm")) == NULL)
    errAbort("Couldn't get main form on trackPage");
if ((tableVar = htmlFormVarGet(mainForm, hgtaTable)) == NULL)
    errAbort("Can't find table var");
for (table = tableVar->values, tableIx = 0; 
	table != NULL && tableIx < maxTables; 
	table = table->next, ++tableIx)
    {
    testOneTable(trackPage, org, db, group, track, table->name);
    }
/* Clean up. */
htmlPageFree(&trackPage);
}

void testOneGroup(struct htmlPage *dbPage, char *org, char *db, char *group, 
	int maxTracks)
/* Test a little something on up to maxTracks in one group */
{
struct htmlPage *groupPage = quickSubmit(dbPage, org, db, group, NULL, NULL,
	"selectGroup", hgtaGroup, group);
struct htmlForm *mainForm;
struct htmlFormVar *trackVar;
struct slName *track;
int trackIx;

if ((mainForm = htmlFormGet(groupPage, "mainForm")) == NULL)
    errAbort("Couldn't get main form on groupPage");
if ((trackVar = htmlFormVarGet(mainForm, hgtaTrack)) == NULL)
    errAbort("Can't find track var");
for (track = trackVar->values, trackIx = 0; 
	track != NULL && trackIx < maxTracks; 
	track = track->next, ++trackIx)
    {
    testOneTrack(groupPage, org, db, group, track->name, clTables);
    }

/* Clean up. */
htmlPageFree(&groupPage);
}

void testGroups(struct htmlPage *dbPage, char *org, char *db, int maxGroups)
/* Test a little something in all groups for dbPage. */
{
struct htmlForm *mainForm;
struct htmlFormVar *groupVar;
struct slName *group;
int groupIx;

if ((mainForm = htmlFormGet(dbPage, "mainForm")) == NULL)
    errAbort("Couldn't get main form on dbPage");
if ((groupVar = htmlFormVarGet(mainForm, hgtaGroup)) == NULL)
    errAbort("Can't find group var");
for (group = groupVar->values, groupIx=0; 
	group != NULL && groupIx < maxGroups; 
	group = group->next, ++groupIx)
    {
    if (!sameString("allTables", group->name))
        testOneGroup(dbPage, org, db, group->name, clTracks);
    }
}

void getTestRegion(char *db, char region[256], int regionSize)
/* Look up first chromosome in database and grab five million bases
 * from the middle of it. */
{
struct sqlConnection *conn = sqlConnect(db);
struct sqlResult *sr = sqlGetResult(conn, "select * from chromInfo limit 1");
char **row;
struct chromInfo ci;
int start,end,middle;

if ((row = sqlNextRow(sr)) == NULL)
    errAbort("Couldn't get one row from chromInfo");
chromInfoStaticLoad(row, &ci);
middle = ci.size/2;
start = middle-2500000;
end = middle+2500000;
if (start < 0) start = 0;
if (end > ci.size) end = ci.size;
safef(region, regionSize, "%s:%d-%d", ci.chrom, start+1, end);
verbose(1, "Testing at position %s\n", region);
fprintf(logFile, "Testing at position %s\n", region);
sqlFreeResult(&sr);
sqlDisconnect(&conn);
}

void testDb(struct htmlPage *orgPage, char *org, char *db)
/* Test on one database. */
{
struct htmlPage *dbPage;
char region[256];
htmlPageSetVar(orgPage, NULL, "db", db);
getTestRegion(db, region, sizeof(region));
htmlPageSetVar(orgPage, NULL, "position", region);
htmlPageSetVar(orgPage, NULL, hgtaRegionType, "range");
dbPage = quickSubmit(orgPage, org, db, NULL, NULL, NULL, "selectDb", "submit", "go");
quickErrReport();
if (dbPage != NULL)
    {
    testGroups(dbPage, org, db, clGroups);
    }
htmlPageFree(&dbPage);
}


void testOrg(struct htmlPage *rootPage, struct htmlForm *rootForm, char *org, char *forceDb)
/* Test on organism.  If forceDb is non-null, only test on
 * given database. */
{
struct htmlPage *orgPage;
struct htmlForm *mainForm;
struct htmlFormVar *dbVar;
struct slName *db;
int dbIx;

htmlPageSetVar(rootPage, rootForm, "org", org);
if (forceDb)
    htmlPageSetVar(rootPage, rootForm, "db", forceDb);
else
   htmlPageSetVar(rootPage, rootForm, "db", "0");
orgPage = htmlPageFromForm(rootPage, rootPage->forms, "submit", "Go");
if ((mainForm = htmlFormGet(orgPage, "mainForm")) == NULL)
    errAbort("Couldn't get main form on orgPage");
if ((dbVar = htmlFormVarGet(mainForm, "db")) == NULL)
    errAbort("Couldn't get org var");
for (db = dbVar->values, dbIx=0; db != NULL && dbIx < clDbs; 
	db = db->next, ++dbIx)
    {
    if (forceDb == NULL || sameString(forceDb, db->name))
	testDb(orgPage, org, db->name);
    }
htmlPageFree(&orgPage);
}

void verifyJoinedFormat(char *s)
/* Verify that s consists of lines with two tab-separated fields,
 * and that the second field has some n/a and some comma-separated lists. */
{
char *e;
int lineIx = 0;
boolean gotCommas = FALSE, gotNa = FALSE;

while (s != NULL && s[0] != 0)
    {
    char *row[3];
    int fieldCount;
    ++lineIx;
    e = strchr(s, '\n');
    if (e != NULL)
       *e++ = 0;
    fieldCount = chopTabs(s, row);
    if (fieldCount != 2)
        {
	qaStatusSoftError(tablesTestList->status, 
		"Got %d fields line %s of  joined result, expected 2", 
		fieldCount, lineIx);
	break;
	}
    if (sameString(row[1], "n/a"))
         gotNa = TRUE;
    if (countChars(s, ',') >= 2)
         gotCommas = TRUE;
    s = e;
    }
if (!gotCommas)
    qaStatusSoftError(tablesTestList->status, 
           "Expected some rows in join to have comma separated lists.");
if (!gotNa)
    qaStatusSoftError(tablesTestList->status, 
           "Expected some rows in joint to have n/a.");
}


void testJoining(struct htmlPage *rootPage)
/* Simulate pressing buttons to get a reasonable join on a
 * couple of swissProt tables. */
{
struct htmlPage *allPage, *page;
char *org = NULL, *db = "swissProt", *group = "allTables", *track="swissProt";
int expectedCount = tableSize("swissProt", "taxon");

allPage = quickSubmit(rootPage, org, db, group, "swissProt", 
	"swissProt.taxon", "swissProtTables", NULL, NULL);
if (allPage != NULL)
    {
    int count = testAllFields(allPage, allPage->forms, org, db,
    	group, track, "swissProt.taxon");
    if (count != expectedCount)
	qaStatusSoftError(tablesTestList->status, 
		"Got %d rows in swissProt.taxon, expected %d", count, 
		expectedCount);
    htmlPageSetVar(allPage, NULL, hgtaOutputType, "selectedFields");
    page = quickSubmit(allPage, org, db, group, track, 
    	"swissProt.taxon", "taxonFields", hgtaDoTopSubmit, "submit");
    htmlPageSetVar(page, NULL, "hgta_fs.linked.swissProt.commonName", "on");
    serialSubmit(&page, org, db, group, track, NULL, "taxonLinks",
	hgtaDoSelectFieldsMore, "submit");
    if (page != NULL)
	{
	htmlPageSetVar(page, NULL, "hgta_fs.check.swissProt.taxon.binomial", "on");
	htmlPageSetVar(page, NULL, "hgta_fs.check.swissProt.commonName.val", "on");
	serialSubmit(&page, org, db, group, track, NULL, "taxonJoined",
	    hgtaDoPrintSelectedFields, "submit");
	if (page != NULL)
	    {
	    checkExpectedSimpleRows(page, expectedCount);
	    verifyJoinedFormat(page->htmlText);
	    htmlPageFree(&page);
	    }
	}
    }

htmlPageFree(&allPage);
verbose(1, "Tested joining on swissProt.taxon & commonName\n");
}

void statsOnSubsets(struct tablesTest *list, int subIx, FILE *f)
/* Report tests of certain subtype. */
{
struct tablesTest *test;
struct hash *hash = newHash(0);
struct slName *typeList = NULL, *type;

fprintf(f, "\n%s subtotals\n", tablesTestInfoTypes[subIx]);

/* Get list of all types in this field. */
for (test = list; test != NULL; test = test->next)
    {
    char *info = test->info[subIx];
    if (!hashLookup(hash, info))
       {
       type = slNameNew(info);
       hashAdd(hash, info, type);
       slAddHead(&typeList, type);
       }
    }
slNameSort(&typeList);
hashFree(&hash);

for (type = typeList; type != NULL; type = type->next)
    {
    struct qaStatistics *stats;
    AllocVar(stats);
    for (test = list; test != NULL; test = test->next)
        {
	if (sameString(type->name, test->info[subIx]))
	    {
	    qaStatisticsAdd(stats, test->status);
	    }
	}
    qaStatisticsReport(stats, type->name, f);
    freez(&stats);
    }
}


void reportSummary(struct tablesTest *list, FILE *f)
/* Report summary of test results. */
{
struct qaStatistics *stats;
struct tablesTest *test;
AllocVar(stats);
int i;

for (i=0; i<ntiiTotalCount; ++i)
    statsOnSubsets(list, i, f);
for (test = list; test != NULL; test = test->next)
    qaStatisticsAdd(stats, test->status);
fprintf(f, "\ngrand total\n");
qaStatisticsReport(stats, "Total", f);
}


void reportAll(struct tablesTest *list, FILE *f)
/* Report all tests. */
{
struct tablesTest *test;
for (test = list; test != NULL; test = test->next)
    {
    if (test->status->errMessage != NULL)
	tablesTestLogOne(test, f);
    }
}

void hgTablesTest(char *url, char *logName)
/* hgTablesTest - Test hgTables web page. */
{
struct htmlPage *rootPage = htmlPageGet(url);
logFile = mustOpen(logName, "w");
htmlPageValidateOrAbort(rootPage);
if (clDb != NULL)
    {
    testDb(rootPage, NULL, clDb);
    }
else
    {
    struct htmlForm *mainForm;
    struct htmlFormVar *orgVar;
    if ((mainForm = htmlFormGet(rootPage, "mainForm")) == NULL)
	errAbort("Couldn't get main form");
    if ((orgVar = htmlFormVarGet(mainForm, "org")) == NULL)
	errAbort("Couldn't get org var");
    if (clOrg != NULL)
	testOrg(rootPage, mainForm, clOrg, clDb);
    else
	{
	struct slName *org;
	int orgIx;
	for (org = orgVar->values, orgIx=0; org != NULL && orgIx < clOrgs; 
		org = org->next, ++orgIx)
	    {
	    testOrg(rootPage, mainForm, org->name, clDb);
	    }
	}
    }
testJoining(rootPage);
htmlPageFree(&rootPage);
slReverse(&tablesTestList);
reportSummary(tablesTestList, stdout);
reportAll(tablesTestList, logFile);
fprintf(logFile, "---------------------------------------------\n");
reportSummary(tablesTestList, logFile);
}

int main(int argc, char *argv[])
/* Process command line. */
{
pushCarefulMemHandler(300000000);
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
clDb = optionVal("db", clDb);
clOrg = optionVal("org", clOrg);
clDbs = optionInt("dbs", clDbs);
clOrgs = optionInt("orgs", clOrgs);
clGroups = optionInt("groups", clGroups);
clTracks = optionInt("tracks", clTracks);
clTables = optionInt("tables", clTables);
if (clOrg != NULL)
   clOrgs = BIGNUM;
hgTablesTest(argv[1], argv[2]);
carefulCheckHeap();
return 0;
}
