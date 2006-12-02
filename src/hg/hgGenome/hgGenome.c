/* hgGenome is a CGI script that produces a web page containing
 * a graphic with all chromosomes in genome, and a graph or two
 * on top of them. This module just contains the main routine,
 * the routine that dispatches to various page handlers, and shared
 * utility functions. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "htmshell.h"
#include "cart.h"
#include "hui.h"
#include "dbDb.h"
#include "ra.h"
#include "hdb.h"
#include "web.h"
#include "portable.h"
#include "hgColors.h"
#include "trackLayout.h"
#include "chromInfo.h"
#include "vGfx.h"
#include "genoLay.h"
#include "cytoBand.h"
#include "hCytoBand.h"
#include "errCatch.h"
#include "chromGraph.h"
#include "customTrack.h"
#include "hCommon.h"
#include "hPrint.h"
#include "jsHelper.h"
#include "hgGenome.h"

static char const rcsid[] = "$Id: hgGenome.c,v 1.48 2006/12/02 01:32:49 kent Exp $";

/* ---- Global variables. ---- */
struct cart *cart;	/* This holds cgi and other variables between clicks. */
struct hash *oldCart;	/* Old cart hash. */
char *database;		/* Name of genome database - hg15, mm3, or the like. */
char *genome;		/* Name of genome - mouse, human, etc. */
struct trackLayout tl;  /* Dimensions of things, fonts, etc. */
struct genoGraph *ggUserList;	/* List of user graphs */
struct genoGraph *ggDbList;	/* List of graphs in database. */
struct slRef *ggList; /* List of active genome graphs */
struct hash *ggHash;	  /* Hash of active genome graphs */
boolean withLabels;	/* Draw labels? */


void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgGenome - Full genome (as opposed to chromosome) view of data\n"
  "This is a cgi script, but it takes the following parameters:\n"
  "   db=<genome database>\n"
  "   hggt_table=on where table is name of chromGraph table\n"
  );
}

/* ---- Get list of graphs. ---- */

struct genoGraph *getUserGraphs()
/* Get list of all user graphs */
{
struct genoGraph *list = NULL, *gg;
struct customTrack *ct, *ctList = customTracksParseCart(cart, NULL, NULL);
for (ct = ctList; ct != NULL; ct = ct->next)
    {
    struct trackDb *tdb = ct->tdb;
    if (sameString(tdb->type, "chromGraph"))
        {
	AllocVar(gg);
	gg->name = tdb->tableName;
	gg->shortLabel = tdb->shortLabel;
	gg->longLabel = tdb->longLabel;
	gg->binFileName = trackDbRequiredSetting(tdb, "binaryFile");
	gg->settings = chromGraphSettingsGet(gg->name, NULL, tdb, cart);
	slAddHead(&list, gg);
	}
    }
slReverse(&list);
return list;
}

struct genoGraph *getDbGraphs(struct sqlConnection *conn)
/* Get graphs defined in database. */
{
struct genoGraph *list = NULL, *gg;
char *trackDbTable = hTrackDbName();
struct sqlConnection *conn2 = hAllocConn();
struct sqlResult *sr;
char **row;

/* Get initial information from metaChromGraph table */
if (sqlTableExists(conn, "metaChromGraph"))
    {
    sr = sqlGetResult(conn, "select name,binaryFile from metaChromGraph");
    while ((row = sqlNextRow(sr)) != NULL)
        {
	char *table = row[0], *binaryFile = row[1];

	AllocVar(gg);
	gg->name = gg->shortLabel = gg->longLabel = cloneString(table);
	gg->binFileName = cloneString(binaryFile);
	slAddHead(&list, gg);
	}
    sqlFreeResult(&sr);
    }

/* Where possible fill in additional info from trackDb.  Also
 * add db: prefix to name. */
for (gg = list; gg != NULL; gg = gg->next)
    {
    char query[512];
    char nameBuf[256];
    safef(query, sizeof(query), 
    	"select * from %s where tableName='%s'", trackDbTable, gg->name);
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
        {
	struct trackDb *tdb = trackDbLoad(row);
	struct chromGraphSettings *cgs = chromGraphSettingsGet(gg->name,
		conn2, tdb, cart);
	gg->shortLabel = tdb->shortLabel;
	gg->longLabel = tdb->longLabel;
	gg->settings = cgs;
	}
    else
        gg->settings = chromGraphSettingsGet(gg->name, NULL, NULL, NULL);
    sqlFreeResult(&sr);

    /* Add db: prefix to separate user and db name space. */
    safef(nameBuf, sizeof(nameBuf), "%s%s", hggDbTag, gg->name);
    gg->name = cloneString(nameBuf);
    }
hFreeConn(&conn2);
slReverse(&list);
return list;
}

void getGenoGraphs(struct sqlConnection *conn)
/* Set up ggList and ggHash with all available genome graphs */
{
struct genoGraph *userList = getUserGraphs();
struct genoGraph *dbList = getDbGraphs(conn);
struct genoGraph *gg;
struct slRef *ref, *refList = NULL;

/* Buld up ggList from user and db lists. */
for (gg = userList; gg != NULL; gg = gg->next)
    refAdd(&refList, gg);
for (gg = dbList; gg != NULL; gg = gg->next)
    refAdd(&refList, gg);
slReverse(&refList);
ggList = refList;

/* Buld up ggHash from ggList. */
ggHash = hashNew(0);
for (ref = ggList; ref != NULL; ref = ref->next)
    {
    gg = ref->val;
    hashAdd(ggHash, gg->name, gg);
    }
}

void fakeEmptyLabels(struct chromGraphSettings *cgs)
/* If no labels, fake them at 1/3 and 2/3 of the way through. */
{
if (cgs->linesAtCount == 0)
    {
    cgs->linesAtCount = 2;
    AllocArray(cgs->linesAt, 2);
    double range = cgs->maxVal - cgs->minVal;
    cgs->linesAt[0] = cgs->minVal + range/3.0;
    cgs->linesAt[1] = cgs->minVal + 2.0*range/3.0;
    if (range >= 3)
        {
	cgs->linesAt[0] = round(cgs->linesAt[0]);
	cgs->linesAt[1] = round(cgs->linesAt[1]);
	}
    }
}

void ggRefineUsed(struct genoGraph *gg)
/* Refine genoGraphs that are used. */
{
if (!gg->didRefine)
    {
    struct chromGraphSettings *cgs = gg->settings;
    gg->cgb = chromGraphBinOpen(gg->binFileName);
    if (cgs->minVal >= cgs->maxVal)
        {
	cgs->minVal = gg->cgb->minVal;
	cgs->maxVal = gg->cgb->maxVal;
	}
    fakeEmptyLabels(gg->settings);
    gg->didRefine = TRUE;
    }
}

int ggLabelWidth(struct genoGraph *gg, MgFont *font)
/* Return width used by graph labels. */
{
struct chromGraphSettings *cgs = gg->settings;
int i;
int minLabelWidth = 0, labelWidth;
char buf[24];
fakeEmptyLabels(cgs);
for (i=0; i<cgs->linesAtCount; ++i)
    {
    safef(buf, sizeof(buf), "%g ", cgs->linesAt[i]);
    labelWidth = mgFontStringWidth(font, buf);
    if (labelWidth > minLabelWidth)
        minLabelWidth = labelWidth;
    }
return minLabelWidth;
}

/* Routines to fetch cart variables. */

double getThreshold()
/* Return user-set threshold */
{
return cartUsualDouble(cart, hggThreshold, defaultThreshold);
}

boolean getYellowMissing()
/* Return draw background in yellow for missing data flag. */
{
return cartUsualBoolean(cart, hggYellowMissing, FALSE);
}

int graphHeight()
/* Return height of graph. */
{
return cartUsualIntClipped(cart, hggGraphHeight, 27, 5, 200);
}

int regionPad()
/* Number of bases to pad regions by. */
{
return cartUsualIntClipped(cart, hggRegionPad, hggRegionPadDefault, 0, 10000000);
}

int graphsPerLine()
/* Return number of graphs to draw per line. */
{
return cartUsualIntClipped(cart, hggGraphsPerLine,
	defaultGraphsPerLine, minGraphsPerLine, maxGraphsPerLine);
}

int linesOfGraphs()
/* Return number of lines of graphs */
{
return cartUsualIntClipped(cart, hggLinesOfGraphs,
	defaultLinesOfGraphs, minLinesOfGraphs, maxLinesOfGraphs);
}

char *graphVarName(int row, int col)
/* Get graph data source variable for given row and column.  Returns
 * static buffer. */
{
static char buf[32];
safef(buf, sizeof(buf), "%s_%d_%d", hggGraphPrefix, row+1, col+1);
return buf;
}

char *graphColorVarName(int row, int col)
/* Get graph color variable for givein row and column. Returns
 * static buffer. */
{
static char buf[32];
safef(buf, sizeof(buf), "%s_%d_%d", hggGraphColorPrefix, row+1, col+1);
return buf;
}

char *graphSourceAt(int row, int col)
/* Return graph source at given row/column, NULL if none. */
{
char *varName = graphVarName(row, col);
char *source = NULL;
if (cartVarExists(cart, varName))
    {
    source = skipLeadingSpaces(cartString(cart, varName));
    if (source[0] == 0)
        source = NULL;
    }
return source;
}

struct genoGraph *ggAt(int row, int col)
/* Return genoGraph at given position or NULL */
{
char *source = graphSourceAt(row, col);
if (source != NULL)
     return hashFindVal(ggHash, source);
return NULL;
}

struct genoGraph *ggFirstVisible()
/* Return first visible graph, or NULL if none. */
{
int graphRows = linesOfGraphs();
int graphCols = graphsPerLine();
struct genoGraph *gg = NULL;
int i,j;
for (i=0; i<graphRows; ++i)
   for (j=0; j<graphCols; ++j)
       if ((gg = ggAt(i,j)) != NULL)
           return gg;
return NULL;
}

struct slRef *ggAllVisible()
/* Return list of references to all visible graphs */
{
struct slRef *list = NULL;
int graphRows = linesOfGraphs();
int graphCols = graphsPerLine();
struct genoGraph *gg = NULL;
int i,j;
for (i=0; i<graphRows; ++i)
   for (j=0; j<graphCols; ++j)
       if ((gg = ggAt(i,j)) != NULL)
           refAddUnique(&list, gg);
slReverse(&list);
return list;
}

struct genoLay *ggLayout(struct sqlConnection *conn, 
	int graphRows, int graphCols)
/* Figure out how to lay out image. */
{
int i,j;
struct genoLayChrom *chromList;
int oneRowHeight;
int minLeftLabelWidth = 0, minRightLabelWidth = 0;

/* Figure out basic dimensions of image. */
trackLayoutInit(&tl, cart);
tl.picWidth = cartUsualInt(cart, hggImageWidth, hgDefaultPixWidth);

/* Refine all graphs actually used, and calculate label
 * widths if need be. */
for (i=0; i<graphRows; ++i)
    {
    for (j=0; j<graphCols; ++j)
	{
	char *source = graphSourceAt(i,j);
	if (source != NULL)
	    {
	    struct genoGraph *gg = hashFindVal(ggHash, source);
	    if (gg != NULL)
		{
		ggRefineUsed(gg);
		if (withLabels)
		    {
		    int labelWidth;
		    labelWidth = ggLabelWidth(gg, tl.font);
		    if (j == 0 && labelWidth > minLeftLabelWidth)
			minLeftLabelWidth = labelWidth;
		    if (j == 1 && labelWidth > minRightLabelWidth)
			minRightLabelWidth = labelWidth;
		    }
		}
	    }
	}
    }

/* Get list of chromosomes and lay them out. */
chromList = genoLayDbChroms(conn, FALSE);
oneRowHeight = graphHeight()+betweenRowPad;
return genoLayNew(chromList, tl.font, tl.picWidth, graphRows*oneRowHeight,
	minLeftLabelWidth, minRightLabelWidth);
}

static void addPadToBed3(struct bed3 *bedList, int atStart, int atEnd)
/* Add padding to bed3 list */
{
struct bed3 *bed;
for (bed = bedList; bed != NULL; bed = bed->next)
    {
    bed->chromStart += atStart;
    bed->chromEnd += atEnd;
    }
}

struct bed3 *regionsOverThreshold(struct genoGraph *gg)
/* Get list of regions over threshold */
{
/* Get list of regions. */
if (gg == NULL)
    errAbort("Please go back and select a graph.");
double threshold = getThreshold();
struct bed3 *bedList = chromGraphBinToBed3(gg->binFileName, threshold);
if (bedList == NULL)
    errAbort("No regions over %g, please go back and set a lower threshold",
    	threshold);
int pad = regionPad();
addPadToBed3(bedList, -pad, pad);
return bedList;
}

void dispatchPage()
/* Look at command variables in cart and figure out which
 * page to draw. */
{
struct sqlConnection *conn = hAllocConn();
if (cartVarExists(cart, hggConfigure))
    {
    configurePage();
    }
else if (cartVarExists(cart, hggUpload))
    {
    uploadPage();
    }
else if (cartVarExists(cart, hggSubmitUpload2))
    {
    submitUpload2(conn);
    }
else if (cartVarExists(cart, hggCorrelate))
    {
    correlatePage(conn);
    }
else if (cartVarExists(cart, hggBrowse))
    {
    browseRegions(conn);
    }
else if (cartVarExists(cart, hggSort))
    {
    sortGenes(conn);
    }
else
    {
    /* Default case - start fancy web page. */
    mainPage(conn);
    }
cartRemovePrefix(cart, hggDo);
}

void hggDoUsualHttp()
/* Wrap html page dispatcher with code that writes out
 * HTTP header and write cart back to database. */
{
cartWriteCookie(cart, hUserCookie());
printf("Content-Type:text/html\r\n\r\n");

/* Dispatch other pages, that actually want to write HTML. */
cartWarnCatcher(dispatchPage, cart, cartEarlyWarningHandler);
cartCheckout(&cart);
}


void dispatchLocation()
/* When this is called no output has been written at all.  We
 * look at command variables in cart and figure out if we just
 * are going write an HTTP location line, which happens when we
 * want to invoke say the genome browser or gene sorter without 
 * another intermediate page.  If we need to do more than that
 * then we call hggDoUsualHttp. */
{
struct sqlConnection *conn = NULL;
getDbAndGenome(cart, &database, &genome);
hSetDb(database);
cartSetString(cart, "db", database); /* Some custom tracks code needs this */
withLabels = cartUsualBoolean(cart, hggLabels, TRUE);
conn = hAllocConn();
getGenoGraphs(conn);

/* Handle cases that just want a HTTP Location line: */
if (cartVarExists(cart, hggClickX))
    {
    clickOnImage(conn);
    return;
    }

hFreeConn(&conn);

/* For other cases we want to print out some of the usual HTTP
 * lines including content-type */
hggDoUsualHttp();
}

char *excludeVars[] = {"Submit", "submit", NULL};

int main(int argc, char *argv[])
/* Process command line. */
{
htmlPushEarlyHandlers();
cgiSpoof(&argc, argv);
// htmlSetStyle(htmlStyleUndecoratedLink);
if (argc != 1)
    usage();
oldCart = hashNew(12);
cart = cartForSession(hUserCookie(), excludeVars, oldCart);
dispatchLocation();
return 0;
}
