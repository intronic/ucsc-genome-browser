/*	hgWigText.c - functions for wiggle tracks in hgText	*/

#include "common.h"
#include "wiggle.h"
#include "hgColors.h"
#include "web.h"
#include "cheapcgi.h"
#include "hCommon.h"

extern boolean typeWiggle;
extern boolean typeWiggle2;

/*	wiggle constraints, two for two possible tables */
extern char *wigConstraint[2];

/*	wiggle data value constraints, two possible tables, and two
 *	values for a possible range
 */
extern double wigDataConstraint[2][2];

/*	to select one of the two tables	*/
#define WIG_TABLE_1	0
#define WIG_TABLE_2	1
#define MAX_LINES_OUTPUT	150000

extern boolean (*wiggleCompare[2])(int tableId, double value,
    boolean summaryOnly, struct wiggle *wiggle);

extern char *database;
extern char *chrom;
extern int winStart;
extern int winEnd;
extern boolean tableIsSplit;
extern boolean allGenome;
extern char *getTableDb();
extern void getFullTableName(char *dest, char *newChrom, char *table);
extern char *getTableName();
extern char *getTable2Name();
extern char *getTrackName();
extern char *getTableVar();
extern void checkTableExists(char *table);
extern boolean existsAndEqual(char *var, char *value);
extern struct hTableInfo *getOutputHti();
extern void saveOutputOptionsState();
extern void saveIntersectOptionsState();
extern void preserveConstraints(char *fullTblName, char *db, char *tableId);
extern void preserveTable2();
extern void displayPosition();
extern char *customTrackPseudoDb;
#define getOutputPhase      "Get results"
#define getCtPhase          "Get Custom Track"
#define getCtBedPhase       "Get Custom Track File"
#define getWigglePhase      "Get data"
#define getCtWigglePhase    "Get Custom Track Data File"
extern struct cart *cart;
extern char *freezeName;
extern char fullTableName[256];
extern char *httpFormMethod;

/* Droplist menu for custom track visibility: */
static char *ctWigVisMenu[] =
{
    "hide",
    "dense",
    "full",
};
static int ctWigVisMenuSize = sizeof(ctWigVisMenu)/sizeof(char *);

/*********   wiggle compare functions   ***********************************/
static boolean wigInRange(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
    ret = ((wiggle->lowerLimit <= wigDataConstraint[tableId][1]) &&
      (wiggle->lowerLimit+wiggle->dataRange >= wigDataConstraint[tableId][0]));
else
    ret = (value >= wigDataConstraint[tableId][0] &&
	value <= wigDataConstraint[tableId][1]);
return ret;
}
static boolean wigLessThan(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
   ret = (wiggle->lowerLimit < wigDataConstraint[tableId][0]);
else
   ret = (value < wigDataConstraint[tableId][0]);
return ret;
}
static boolean wigLessEqual(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
   ret = (wiggle->lowerLimit <= wigDataConstraint[tableId][0]);
else
   ret = (value <= wigDataConstraint[tableId][0]);
return ret;
}
static boolean wigEqual(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
   ret = ((wiggle->lowerLimit < wigDataConstraint[tableId][0]) &&
	(wiggle->lowerLimit+wiggle->dataRange > wigDataConstraint[tableId][0]));
else
   ret = (value == wigDataConstraint[tableId][0]);
return ret;
}
static boolean wigNotEqual(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
   ret = ((wiggle->lowerLimit > wigDataConstraint[tableId][0]) ||
	(wiggle->lowerLimit+wiggle->dataRange < wigDataConstraint[tableId][0]));
else
   ret = (value != wigDataConstraint[tableId][0]);
return ret;
}
static boolean wigGreaterEqual(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
   ret=(wiggle->lowerLimit+wiggle->dataRange >= wigDataConstraint[tableId][0]);
else
   ret = (value >= wigDataConstraint[tableId][0]);
return ret;
}
static boolean wigGreaterThan(int tableId, double value, boolean summaryOnly,
    struct wiggle *wiggle)
{
boolean ret = FALSE;
if (summaryOnly)
   ret = (wiggle->lowerLimit+wiggle->dataRange > wigDataConstraint[tableId][0]);
else
   ret = (value > wigDataConstraint[tableId][0]);
return ret;
}

void wiggleConstraints(char *cmp, char *pat, int tableIndex)
{
wigConstraint[tableIndex] = cmp;
wiggleCompare[tableIndex] = NULL;
if (strlen(pat)>0)
    {
    if (sameWord(cmp,"in range"))
	{
	char *rangeValues[2];
	char *clone = cloneString(pat);
	int records = 0;
	char *comma = strchr(pat,',');

	if (comma == (char *)NULL)
	    records = chopByWhite(clone,rangeValues,2);
	else
	    records = chopByChar(clone,',',rangeValues,2);

	if (records == 2)
	    {
	    trimSpaces(rangeValues[0]);
	    trimSpaces(rangeValues[1]);
	    wigDataConstraint[tableIndex][0] = sqlDouble(rangeValues[0]);
	    wigDataConstraint[tableIndex][1] = sqlDouble(rangeValues[1]);
	    if (wigDataConstraint[tableIndex][0] >
		    wigDataConstraint[tableIndex][1])
		{
		double d = wigDataConstraint[tableIndex][1];
		wigDataConstraint[tableIndex][1] =
		    wigDataConstraint[tableIndex][0];
		wigDataConstraint[tableIndex][0] = d;
		}
            wiggleCompare[tableIndex] = wigInRange;
	    }
	freeMem(clone);
	if (wigDataConstraint[tableIndex][1] ==
		wigDataConstraint[tableIndex][0])
	    errAbort("For \"in range\" constraint, you must give two numbers separated by whitespace or comma.");
	}
    else
	{
	wigDataConstraint[tableIndex][0] = sqlDouble(pat);
	if (sameWord(cmp,"<")) wiggleCompare[tableIndex] = wigLessThan;
	else if (sameWord(cmp,"<=")) wiggleCompare[tableIndex] = wigLessEqual;
	else if (sameWord(cmp,"=")) wiggleCompare[tableIndex] = wigEqual;
	else if (sameWord(cmp,"!=")) wiggleCompare[tableIndex] = wigNotEqual;
	else if (sameWord(cmp,">=")) wiggleCompare[tableIndex]=wigGreaterEqual;
	else if (sameWord(cmp,">")) wiggleCompare[tableIndex] = wigGreaterThan;
	}
    }
}

static void wigStatsCalc(unsigned count, double wigSumData,
    double wigSumSquares, double *mean, double *variance, double *stddev)
{
*mean = 0.0;
*variance = 0.0;
*stddev = 0.0;
if (count > 0)
    *mean = wigSumData / count;
if (count > 1)
    {
    *variance = (wigSumSquares - ((wigSumData*wigSumData)/count)) / (count-1);
    if (*variance > 0.0)
	*stddev = sqrt(*variance);
    }
}
static void wigStatsRow(char *chrom, unsigned start, unsigned end,
    int span, unsigned count, double wigUpperLimit, double wigLowerLimit,
    double mean, double variance, double stddev)
{
printf("<TR><TH ALIGN=LEFT> %s </TH>\n", chrom);
printf("\t<TD ALIGN=RIGHT> %u </TD>\n", start);
printf("\t<TD ALIGN=RIGHT> %u </TD>\n", end);
printf("\t<TD ALIGN=RIGHT> %u </TD>\n", count);
printf("\t<TD ALIGN=RIGHT> %d </TD>\n", span);
printf("\t<TD ALIGN=RIGHT> %u </TD>\n", count*span);
printf("\t<TD ALIGN=RIGHT> %g </TD>\n", wigLowerLimit);
printf("\t<TD ALIGN=RIGHT> %g </TD>\n", wigUpperLimit);
printf("\t<TD ALIGN=RIGHT> %g </TD>\n", wigUpperLimit - wigLowerLimit);
printf("\t<TD ALIGN=RIGHT> %g </TD>\n", mean);
printf("\t<TD ALIGN=RIGHT> %g </TD>\n", variance);
printf("\t<TD ALIGN=RIGHT> %g </TD>\n", stddev);
printf("<TR>\n");
}

void wigDoStats(char *database, char *table, struct slName *chromList,
    int winStart, int winEnd, int tableId, char *constraints)
{
int spanCount = 0;
struct slName *chromPtr;
char *db = getTableDb();
struct sqlConnection *conn = hAllocOrConnect(db);
struct sqlResult *sr = (struct sqlResult *)NULL;
char query[256];
char **row = (char **)NULL;
int numChroms = slCount(chromList);
char wigFullTableName[256];

if (tableIsSplit)
    {
    char defaultChrom[64];
    hFindDefaultChrom(db, defaultChrom);
    getFullTableName(wigFullTableName, defaultChrom, table);
    snprintf(query, sizeof(query), "show table status like '%s'", wigFullTableName);
    }
else
    snprintf(query, sizeof(query), "show table status like '%s'", table);

sr = sqlMustGetResult(conn,query);
row = sqlNextRow(sr);

// For some reason BORDER=1 does not work in our web.c nested table scheme.
// So use web.c's trick of using an enclosing table to provide a border.  
puts("<P><!--outer table is for border purposes-->" "\n"
     "<TABLE BGCOLOR=\"#"HG_COL_BORDER"\" BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"1\"><TR><TD>");

puts("<TABLE BORDER=\"1\" BGCOLOR=\""HG_COL_INSIDE"\" CELLSPACING=\"0\">");

if (row != NULL)
    {
    printf("<TR><TD COLSPAN=12>\n");
    printf("<TABLE COLS=12 ALIGN=CENTER HSPACE=0>"
	"<TR><TH COLSPAN=1 ALIGN=LEFT> Database: %s </TH><TH COLSPAN=1 ALIGN=CENTER> Table: %s </TH><TH COLSPAN=10 ALIGN=RIGHT>  Last update: %s </TH></TR></TABLE></TD></TR>\n",
	database, table, row[11]);
    }
else
    {
    printf("<TR><TH COLSPAN=6 ALIGN=LEFT> Database: %s </TH><TH COLSPAN=6 ALIGN=RIGHT> Table: %s </TH></TR>\n", database,
	table);
    }

sqlFreeResult(&sr);
hFreeOrDisconnect(&conn);

printf("<TR><TH> Chrom </TH><TH> Data <BR> start </TH>");
printf("<TH> Data <BR> end </TH>");
printf("<TH> # of Data <BR> values </TH><TH> Data <BR> span </TH>");
printf("<TH> Bases <BR> covered </TH><TH> Minimum </TH>");
printf("<TH> Maximum </TH><TH> Range </TH><TH> Mean </TH>");
printf("<TH> Variance </TH><TH> Standard <BR> deviation </TH></TR>\n");

for (chromPtr=chromList;  chromPtr != NULL; chromPtr=chromPtr->next)
    {
    struct wiggleData *wigData;
    char *chrom = chromPtr->name;
    char *tbl = table;
    char wigFullTableName[256];
    if (tableIsSplit)
	{
	getFullTableName(wigFullTableName, chromPtr->name, table);
	tbl = wigFullTableName;
	}

    if (numChroms > 1)
	wigData = wigFetchData(database, tbl, chrom, winStart, winEnd,
	    WIG_ALL_DATA, WIG_DATA_NOT_RETURNED,
		tableId, wiggleCompare[tableId], constraints);
    else
	wigData = wigFetchData(database, tbl, chrom, winStart, winEnd,
	    WIG_ALL_DATA, WIG_DATA_NOT_RETURNED, tableId,
		wiggleCompare[tableId], constraints);

    if (wigData)
	{
	unsigned span = 0;
	double wigLowerLimit = 1.0e+300;
	double wigUpperLimit = -1.0e+300;
	double wigSumData = 0.0;
	double wigSumSquares = 0.0;
	unsigned count = 0;
	unsigned chromStart = BIGNUM;
	unsigned chromEnd = 0;
	struct wiggleData *wdPtr;
	double mean;
	double variance;
	double stddev;
	for (wdPtr = wigData; wdPtr != (struct wiggleData *) NULL;
		    wdPtr= wdPtr->next)
	    {
	    if (span != wdPtr->span)
		{
		++spanCount;
		if (span > 0)
		    {
		    wigStatsCalc(count, wigSumData, wigSumSquares,
			    &mean, &variance, &stddev);
		    wigStatsRow(chrom, chromStart, chromEnd, span, count,
			wigUpperLimit, wigLowerLimit, mean, variance, stddev);
		    }
		span = wdPtr->span;
		wigLowerLimit = 1.0e+300;
		wigUpperLimit = -1.0e+300;
		wigSumData = 0.0;
		wigSumSquares = 0.0;
		count = 0;
		chromStart = BIGNUM;
		chromEnd = 0;
		}
	    wigSumData += wdPtr->sumData;
	    wigSumSquares += wdPtr->sumSquares;
	    count += wdPtr->count;
	    if (wdPtr->lowerLimit < wigLowerLimit)
		    wigLowerLimit = wdPtr->lowerLimit;
	    if (wdPtr->lowerLimit+wdPtr->dataRange > wigUpperLimit)
		    wigUpperLimit = wdPtr->lowerLimit+wdPtr->dataRange;
	    if (wdPtr->chromStart < chromStart)
		    chromStart = wdPtr->chromStart;
	    if (wdPtr->chromEnd > chromEnd)
		    chromEnd = wdPtr->chromEnd;
	    }
	wigStatsCalc(count, wigSumData, wigSumSquares,
	    &mean, &variance, &stddev);
	wigStatsRow(chrom, chromStart, chromEnd, span, count,
	    wigUpperLimit, wigLowerLimit, mean, variance, stddev);
	wigFreeData(wigData);
	}
    else
	{
	printf("<TR><TH ALIGN=LEFT> %s </TH>", chrom);
	printf("<TH COLSPAN=11> No data </TH></TR>\n");
	}
    }
printf("</TABLE>\n");
puts("</TD></TR></TABLE></P>");
}

void doWiggleCtOptions(boolean doCt)
{
struct hTableInfo *hti = getOutputHti();
char *table = getTableName();
char *table2 = getTable2Name();
char *op = cgiOptionalString("tbIntersectOp");
char *db = getTableDb();
char *outputType = cgiUsualString("outputType", cgiString("phase"));
char *phase = (existsAndEqual("phase", getOutputPhase) ?
	       cgiString("outputType") : cgiString("phase"));
char *setting = NULL;
char buf[256];

saveOutputOptionsState();
saveIntersectOptionsState();

webStart(cart, "Table Browser: %s %s: %s", hOrganism(database), freezeName,
	 phase);
checkTableExists(fullTableName);
printf("<FORM ACTION=\"%s\" NAME=\"mainForm\" METHOD=\"%s\">\n",
       hgTextName(), httpFormMethod);
cartSaveSession(cart);
cgiMakeHiddenVar("db", database);

cgiMakeHiddenVar("table", getTableVar());
displayPosition();
cgiMakeHiddenVar("outputType", outputType);
preserveConstraints(fullTableName, db, NULL);
preserveTable2();
printf("<H3> Select %s Options for %s: </H3>\n",
       (doCt ? "Custom Track" : "DATA"), hti->rootName);
puts("<A HREF=\"/goldenPath/help/hgTextHelp.html#BEDOptions\">"
     "<B>Help</B></A><P>");
puts("<TABLE><TR><TD>");
if (doCt)
    {
    puts("</TD><TD>"
	 "<A HREF=\"/goldenPath/help/customTrack.html\" TARGET=_blank>"
	 "Custom track</A> header: </B>");
    }
else
    {
    cgiMakeCheckBox("tbDoCustomTrack",
		    cartCgiUsualBoolean(cart, "tbDoCustomTrack", FALSE));
    puts("</TD><TD> <B> Include "
	 "<A HREF=\"/goldenPath/help/customTrack.html\" TARGET=_blank>"
	 "custom track</A> header: </B>");
    }
puts("</TD></TR><TR><TD></TD><TD>name=");
if (op == NULL)
    table2 = NULL;
snprintf(buf, sizeof(buf), "tb_%s", hti->rootName);
setting = cgiUsualString("tbCtName", buf);
cgiMakeTextVar("tbCtName", setting, 16);
puts("</TD></TR><TR><TD></TD><TD>description=");
snprintf(buf, sizeof(buf), "table browser query on %s%s%s",
	 table,
	 (table2 ? ", " : ""),
	 (table2 ? table2 : ""));
setting = cgiUsualString("tbCtDesc", buf);
cgiMakeTextVar("tbCtDesc", setting, 50);
puts("</TD></TR><TR><TD></TD><TD>visibility=");
setting = cartCgiUsualString(cart, "tbCtVis", ctWigVisMenu[2]);
cgiMakeDropList("tbCtVis", ctWigVisMenu, ctWigVisMenuSize, setting);
puts("</TD></TR><TR><TD></TD></TR></TABLE>");
if (doCt)
    {
    /*cgiMakeButton("phase", getCtPhase);*/
    cgiMakeButton("phase", getCtWigglePhase);
    }
else
    {
    cgiMakeButton("phase", getWigglePhase);
    }
puts("</FORM>");
webEnd();
}	/*	void doWiggleCtOptions(boolean doCt)	*/

void doGetWiggleData(boolean doCt)
/* Find wiggle data and display it */
{
char *db = getTableDb();
char *table = getTableName();
struct wiggleData *wigData = (struct wiggleData *) NULL;
struct wiggleData *wdPtr = (struct wiggleData *) NULL;
struct trackDb *tdb = (struct trackDb *)NULL;
char *track = getTrackName();
unsigned linesOutput = 0;
boolean doCtHdr = (cgiBoolean("tbDoCustomTrack") || doCt);

if (! sameString(customTrackPseudoDb, db))
    {
    struct sqlConnection *conn = hAllocOrConnect(db);
    tdb = hMaybeTrackInfo(conn, track);
    hFreeOrDisconnect(&conn);
    }

saveOutputOptionsState();
saveIntersectOptionsState();

printf("Content-Type: text/plain\n\n");
webStartText();

if (! allGenome)
    wigData = wigFetchData(database, table, chrom, winStart, winEnd,
	WIG_ALL_DATA, WIG_RETURN_DATA, WIG_TABLE_1,
	    wiggleCompare[WIG_TABLE_1], (char *)NULL);

if (wigData)
    {
    unsigned span = 0;
    char *chrom = (char *) NULL;
    char *longLabel;
    unsigned char visibility;
    unsigned char colorR, colorG, colorB;
    unsigned char altColorR, altColorG, altColorB;
    float priority;
    int wordCount;
    char *words[128];
    char *trackType = (char *) NULL;
    char *visibilities[] = {
	"hide",
	"dense",
	"full",
	"pack",
	"squish",
    };
    if (tdb && tdb->type)
	{
	char *typeLine = cloneString(tdb->type);
	longLabel = cloneString(tdb->longLabel);
	priority = tdb->priority;
	wordCount = chopLine(typeLine,words);
	if (wordCount > 0)
	    trackType = words[0];
	colorR = tdb->colorR; colorG = tdb->colorG; colorB = tdb->colorB;
	altColorR = tdb->altColorR; altColorG = tdb->altColorG;
	altColorB = tdb->altColorB;
	visibility = tdb->visibility;
	}
    else
	{
	priority = 42;
	longLabel = cloneString("wiggle data");
	colorR = colorG = colorB = 255;
	altColorR = altColorG = altColorB = 128;
	visibility = 2;
	}

    if (doCtHdr)
	printf("track type=wiggle_0 name=%s description=\"%s\" "
	    "visibility=%s color=%d,%d,%d altColor=%d,%d,%d "
	    "priority=%g\n", table, longLabel, visibilities[visibility],
	    colorR, colorG, colorB, altColorR, altColorG, altColorB, priority);

    for (wdPtr = wigData; (linesOutput < MAX_LINES_OUTPUT) &&
	    (wdPtr != (struct wiggleData *) NULL); wdPtr= wdPtr->next)
	{
	int i;
	struct wiggleDatum *wd;
	if ((chrom == (char *) NULL) || differentWord(chrom,wdPtr->chrom))
	    {
	    if (!doCtHdr)
		printf("#\t");
	    printf("variableStep chrom=%s span=%u\n",
		wdPtr->chrom, wdPtr->span);
	    chrom = wdPtr->chrom;
	    span = wdPtr->span;
	    }
	if (span != wdPtr->span)
	    {
	    if (!doCtHdr)
		printf("#\t");
	    printf("variableStep chrom=%s span=%u\n",
		wdPtr->chrom, wdPtr->span);
	    span = wdPtr->span;
	    }
	wd = wdPtr->data;
	for (i = 0; (linesOutput < MAX_LINES_OUTPUT) && (i < wdPtr->count); ++i)
	    {
	    printf("%u\t%g\n", wd->chromStart, wd->value);
	    ++wd;
	    ++linesOutput;
	    }
	}
    wigFreeData(wigData);
    }
else
    printf("#\tthis data fetch function is under development, expected early April 2004\n");

if (linesOutput >= MAX_LINES_OUTPUT)
    printf("#\tmaximum data output of %u lines reached\n", MAX_LINES_OUTPUT);

webEnd();
}	/*	void doGetWiggleData(boolean doCt)	*/
