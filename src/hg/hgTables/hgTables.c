/* hgTables - Get table data associated with tracks and intersect tracks. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "memalloc.h"
#include "htmshell.h"
#include "cheapcgi.h"
#include "cart.h"
#include "jksql.h"
#include "hdb.h"
#include "web.h"
#include "hui.h"
#include "hgColors.h"
#include "trackDb.h"
#include "grp.h"
#include "hgTables.h"

static char const rcsid[] = "$Id: hgTables.c,v 1.12 2004/07/14 19:02:50 kent Exp $";


void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgTables - Get table data associated with tracks and intersect tracks\n"
  "usage:\n"
  "   hgTables XXX\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

/* Global variables. */
struct cart *cart;	/* This holds cgi and other variables between clicks. */
struct hash *oldVars;	/* The cart before new cgi stuff added. */
char *genome;		/* Name of genome - mouse, human, etc. */
char *database;		/* Name of genome database - hg15, mm3, or the like. */
struct trackDb *fullTrackList;	/* List of all tracks in database. */
struct trackDb *curTrack;	/* Currently selected track. */

/* --------------- HTML Helpers ----------------- */

void hvPrintf(char *format, va_list args)
/* Print out some html.  Check for write error so we can
 * terminate if http connection breaks. */
{
vprintf(format, args);
if (ferror(stdout))
    noWarnAbort();
}

void hPrintf(char *format, ...)
/* Print out some html.  Check for write error so we can
 * terminate if http connection breaks. */
{
va_list(args);
va_start(args, format);
hvPrintf(format, args);
va_end(args);
}

void hPrintSpaces(int count)
/* Print a number of non-breaking spaces. */
{
int i;
for (i=0; i<count; ++i)
    hPrintf("&nbsp;");
}

static void vaHtmlOpen(char *format, va_list args)
/* Start up a page that will be in html format. */
{
puts("Content-Type:text/html\n");
cartVaWebStart(cart, format, args);
}

void htmlOpen(char *format, ...)
/* Start up a page that will be in html format. */
{
va_list args;
va_start(args, format);
vaHtmlOpen(format, args);
}

void htmlClose()
/* Close down html format page. */
{
cartWebEnd();
}

void hTableStart()
/* For some reason BORDER=1 does not work in our web.c nested table scheme.
 * So use web.c's trick of using an enclosing table to provide a border.   */
{
puts("<!--outer table is for border purposes-->" "\n"
     "<TABLE BGCOLOR=\"#"HG_COL_BORDER"\" BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"1\"><TR><TD>");
puts("<TABLE BORDER=\"1\" BGCOLOR=\""HG_COL_INSIDE"\" CELLSPACING=\"0\">");
}

void hTableEnd()
/* Close out table started with hTableStart() */
{
puts("</TABLE>");
puts("</TR></TD></TABLE>");
}

/* --------------- Text Mode Helpers ----------------- */

static void textWarnHandler(char *format, va_list args)
/* Text mode error message handler. */
{
char *hLine =
"---------------------------------------------------------------------------\n";
if (format != NULL) {
    fflush(stdout);
    fprintf(stderr, "%s", hLine);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fprintf(stderr, "%s", hLine);
    }
}

static void textAbortHandler()
/* Text mode abort handler. */
{
exit(-1);
}

void textOpen()
/* Start up page in text format. (No need to close this). */
{
printf("Content-Type: text/plain\n\n");
pushWarnHandler(textWarnHandler);
pushAbortHandler(textAbortHandler);
}

/* --------- Utility functions --------------------- */

static struct region *getRegionsFullGenome()
/* Get a region list that covers all of each chromosome. */
{
struct slName *chrom, *chromList = hAllChromNames();
struct region *region, *regionList = NULL;
for (chrom = chromList; chrom != NULL; chrom = chrom->next)
    {
    AllocVar(region);
    region->chrom = chrom->name;
    slAddHead(&regionList, region);
    }
slReverse(&regionList);
slFreeList(&chromList);
return regionList;
}

struct region *getRegions(struct sqlConnection *conn)
/* Consult cart to get list of regions to work on. */
{
char *regionType = cartUsualString(cart, hgtaRegionType, "genome");
struct region *regionList = NULL, *region;
if (sameString(regionType, "genome"))
    {
    regionList = getRegionsFullGenome();
    }
else if (sameString(regionType, "range"))
    {
    char *range = cartString(cart, hgtaRange);
    regionList = AllocVar(region);
    if (!hgParseChromRange(range, &region->chrom, &region->start, &region->end))
        {
	errAbort("%s is not a chromosome range.  "
	         "Please go back and enter something like chrX:1000000-1100000 "
		 "in the range control.", range);
	}
    }
else
    errAbort("Sorry, don't understand %s type region yet", regionType);
return regionList;
}

char *connectingTableForTrack(struct trackDb *track)
/* Return table name to use with all.joiner for track. 
 * You can freeMem this when done. */
{
struct hTableInfo *hti = hFindTableInfo(NULL, track->tableName);
if (sameString(track->tableName, "mrna"))
    return cloneString("all_mrna");
else if (sameString(track->tableName, "intronEst"))
    return cloneString("all_est");
else if (sameString(track->tableName, "est"))
    return cloneString("all_est");
else 
    return cloneString(track->tableName);
}

char *chromTable(struct sqlConnection *conn, char *table)
/* Get chr1_table if it exists, otherwise table. */
{
char *chrom = hDefaultChrom();
if (sqlTableExists(conn, table))
    return cloneString(table);
else
    {
    char buf[256];
    safef(buf, sizeof(buf), "%s_%s", chrom, table);
    return cloneString(buf);
    }
}


void doTest(struct sqlConnection *conn)
/* Put up a page to see what happens. */
{
textOpen();
hPrintf("%s\n", cartUsualString(cart, "test", "n/a"));
hPrintf("All for now\n");
}


struct trackDb *findTrackInGroup(char *name, struct trackDb *trackList,
    struct grp *group)
/* Find named track that is in group (NULL for any group).
 * Return NULL if can't find it. */
{
struct trackDb *track;
for (track = trackList; track != NULL; track = track->next)
    {
    if (sameString(name, track->tableName) &&
       (group == NULL || sameString(group->name, track->grp)))
       return track;
    }
return NULL;
}

struct trackDb *findTrack(char *name, struct trackDb *trackList)
/* Find track, or return NULL if can't find it. */
{
return findTrackInGroup(name, trackList, NULL);
}

struct trackDb *findSelectedTrack(struct trackDb *trackList, 
    struct grp *group)
/* Find selected track - from CGI variable if possible, else
 * via various defaults. */
{
char *name = cartOptionalString(cart, hgtaTrack);
struct trackDb *track = NULL;
if (name != NULL)
    track = findTrackInGroup(name, trackList, group);
if (track == NULL)
    {
    if (group == NULL)
        track = trackList;
    else
	{
	for (track = trackList; track != NULL; track = track->next)
	    if (sameString(track->grp, group->name))
	         break;
	if (track == NULL)
	    internalErr();
	}
    }
return track;
}

void doFilterPage(struct sqlConnection *conn)
/* Respond to filter create/edit button */
{
htmlOpen("Table Browser Filter");
uglyf("Theoretically making filter.");
htmlClose();
}

void doOutPrimaryTable(struct trackDb *track, 
	struct sqlConnection *conn)
/* Dump out primary table. */
{
struct region *region, *regionList = NULL;
textOpen();
regionList = getRegions(conn);

for (region = regionList; region != NULL; region = region->next)
    {
    struct sqlResult *sr;
    char **row;
    int colOffset, colIx, colCount, lastCol;
    if (region->end == 0) /* Full chromosome. */
        {
	sr = hChromQuery(conn, track->tableName, region->chrom, NULL, &colOffset);
	}
    else 
        {
	sr = hRangeQuery(conn, track->tableName, region->chrom, 
		region->start, region->end, NULL, &colOffset);
	}
    colCount = sqlCountColumns(sr);
    lastCol = colCount - 1;

    /* First time through print column names. */
    if (region == regionList)
        {
	hPrintf("#");
	for (colIx = 0; colIx < lastCol; ++colIx)
	    hPrintf("%s\t", sqlFieldName(sr));
	hPrintf("%s\n", sqlFieldName(sr));
	}
    while ((row = sqlNextRow(sr)) != NULL)
	{
	for (colIx = 0; colIx < lastCol; ++colIx)
	    hPrintf("%s\t", row[colIx]);
	hPrintf("%s\n", row[lastCol]);
	}
    }
}

void doTopSubmit(struct sqlConnection *conn)
/* Respond to submit button on top level page.
 * This basically just dispatches based on output type. */
{
char *output = cartString(cart, hgtaOutputType);
char *trackName = cartString(cart, hgtaTrack);
struct trackDb *track = findTrack(trackName, fullTrackList);
if (track == NULL)
    errAbort("track %s doesn't exist in %s", trackName, database);
if (sameString(output, outPrimaryTable))
    doOutPrimaryTable(track, conn);
else if (sameString(output, outSchema))
    doTrackSchema(track, conn);
else
    errAbort("Don't know how to handle %s output yet", output);
}

void doIntersect(struct sqlConnection *conn)
/* Respond to intersection button. */
{
htmlOpen("Table Browser Intersect");
uglyf("Processing intersect button... too stupid to do anything real...");
htmlClose();
}

void dispatch(struct sqlConnection *conn)
/* Scan for 'Do' variables and dispatch to appropriate page-generator.
 * By default head to the main page. */
{
if (cartVarExists(cart, hgtaDoTest))
    doTest(conn);
else if (cartVarExists(cart, hgtaDoTopSubmit))
    doTopSubmit(conn);
else if (cartVarExists(cart, hgtaDoIntersect))
    doIntersect(conn);
else if (cartVarExists(cart, hgtaDoPasteIdentifiers))
    doPasteIdentifiers(conn);
else if (cartVarExists(cart, hgtaDoPastedIdentifiers))
    doPastedIdentifiers(conn);
else if (cartVarExists(cart, hgtaDoUploadIdentifiers))
    doUploadIdentifiers(conn);
else if (cartVarExists(cart, hgtaDoClearIdentifiers))
    doClearIdentifiers(conn);
else if (cartVarExists(cart, hgtaDoFilterPage))
    doFilterPage(conn);
else if (cartVarExists(cart, hgtaDoSchema))
    {
    doTableSchema( cartString(cart, hgtaDoSchemaDb), 
    	cartString(cart, hgtaDoSchema), conn);
    }
else if (cartVarExists(cart, hgtaDoValueHistogram))
    doValueHistogram(cartString(cart, hgtaDoValueHistogram));
else if (cartVarExists(cart, hgtaDoValueRange))
    doValueRange(cartString(cart, hgtaDoValueRange));
else if (cartVarExists(cart, hgtaDoMainPage))
    doMainPage(conn);
else	/* Default - put up initial page. */
    doMainPage(conn);
cartRemovePrefix(cart, hgtaDo);
}

char *excludeVars[] = {"Submit", "submit", NULL};

void hgTables()
/* hgTables - Get table data associated with tracks and intersect tracks. */
{
struct sqlConnection *conn = NULL;
oldVars = hashNew(10);

/* Sometimes we output HTML and sometimes plain text; let each outputter 
 * take care of headers instead of using a fixed cart*Shell(). */
cart = cartAndCookieNoContent(hUserCookie(), excludeVars, oldVars);

/* Set up global variables. */
getDbAndGenome(cart, &database, &genome);
hSetDb(database);
conn = hAllocConn();
fullTrackList = hTrackDb(NULL);
curTrack = findSelectedTrack(fullTrackList, NULL);

/* Go figure out what page to put up. */
dispatch(conn);

/* Save variables. */
cartCheckout(&cart);
}

int main(int argc, char *argv[])
/* Process command line. */
{
htmlPushEarlyHandlers(); /* Make errors legible during initialization. */
cgiSpoof(&argc, argv);
hgTables();
return 0;
}
