/* hgNear - gene family browser. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "jksql.h"
#include "htmshell.h"
#include "cart.h"
#include "hdb.h"
#include "hui.h"
#include "web.h"
#include "ra.h"
#include "kgAlias.h"
#include "findKGAlias.h"
#include "hgNear.h"

static char const rcsid[] = "$Id: hgNear.c,v 1.19 2003/06/24 20:16:32 kent Exp $";

char *excludeVars[] = { "submit", "Submit", confVarName, defaultConfName,
        idVarName, resetConfName, NULL }; 
/* The excludeVars are not saved to the cart. */

/* ---- Global variables. ---- */
struct cart *cart;	/* This holds cgi and other variables between clicks. */
char *database;		/* Name of genome database - hg15, mm3, or the like. */
char *organism;		/* Name of organism - mouse, human, etc. */
char *groupOn;		/* Current grouping strategy. */
int displayCount;	/* Number of items to display. */

struct genePos *curGeneId;	  /* Identity of current gene. */

/* ---- Some html helper routines. ---- */

void hvPrintf(char *format, va_list args)
/* Print out some html. */
{
vprintf(format, args);
}

void hPrintf(char *format, ...)
/* Print out some html. */
{
va_list(args);
va_start(args, format);
hvPrintf(format, args);
va_end(args);
}

void makeTitle(char *title, char *helpName)
/* Make title bar. */
{
hPrintf("<TABLE WIDTH=\"100%%\" BGCOLOR=\"#536ED3\" BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"2\"><TR>\n");
hPrintf("<TD ALIGN=LEFT><A HREF=\"/index.html\">%s</A></TD>", wrapWhiteFont("Home"));
hPrintf("<TD ALIGN=CENTER><FONT COLOR=\"#FFFFFF\" SIZE=5>%s</FONT></TD>", title);
hPrintf("<TD ALIGN=Right><A HREF=\"../goldenPath/help/%s\">%s</A></TD>", 
	helpName, wrapWhiteFont("Help"));
hPrintf("</TR></TABLE>");
}


/* ---- Some helper routines for column methods. ---- */

char *cellLookupVal(struct column *col, struct genePos *gp, 
	struct sqlConnection *conn)
/* Get a field in a table defined by col->table, col->keyField, col->valField. */
{
char query[512];
struct sqlResult *sr;
char **row;
char *res = NULL;
safef(query, sizeof(query), "select %s from %s where %s = '%s'",
	col->valField, col->table, col->keyField, gp->name);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    res = cloneString(row[0]);
sqlFreeResult(&sr);
return res;
}

void cellSimplePrint(struct column *col, struct genePos *gp, 
	struct sqlConnection *conn)
/* This just prints one field from table. */
{
char *s = col->cellVal(col, gp, conn);
if (s == NULL) 
    {
    hPrintf("<TD>n/a</TD>", s);
    }
else
    {
    hPrintf("<TD>%s</TD>", s);
    freeMem(s);
    }
}

void labelSimplePrint(struct column *col)
/* This just prints cell->shortLabel. */
{
hPrintf("<TH VALIGN=BOTTOM><B>%s</B></TH>", col->shortLabel);
}


static void cellSelfLinkPrint(struct column *col, struct genePos *gp,
	struct sqlConnection *conn)
/* Print self and hyperlink to make this the search term. */
{
char *s = col->cellVal(col, gp, conn);
if (s == NULL) 
    s = cloneString("n/a");
hPrintf("<TD><A HREF=\"../cgi-bin/hgNear?%s&near.search=%s\">%s</A></TD>",
	cartSidUrlString(cart), gp->name, s);
freeMem(s);
}

static boolean alwaysExists(struct column *col, struct sqlConnection *conn)
/* We don't exist ever. */
{
return TRUE;
}

boolean simpleTableExists(struct column *col, struct sqlConnection *conn)
/* This returns true if col->table exists. */
{
return sqlTableExists(conn, col->table);
}

static char *noVal(struct column *col, struct genePos *gp, struct sqlConnection *conn)
/* Return not-available value. */
{
return cloneString("n/a");
}

void columnDefaultMethods(struct column *col)
/* Set up default methods. */
{
col->exists = alwaysExists;
col->cellVal = noVal;
col->cellPrint = cellSimplePrint;
col->labelPrint = labelSimplePrint;
}

/* ---- Accession column ---- */

static char *accVal(struct column *col, struct genePos *gp, struct sqlConnection *conn)
/* Return clone of geneId */
{
return cloneString(gp->name);
}

void setupColumnAcc(struct column *col, char *parameters)
/* Set up a column that displays the geneId (accession) */
{
columnDefaultMethods(col);
col->cellVal = accVal;
}

/* ---- Number column ---- */

static char *numberVal(struct column *col, struct genePos *gp, 
	struct sqlConnection *conn)
/* Return incrementing number. */
{
static int ix = 0;
char buf[15];
++ix;
safef(buf, sizeof(buf), "%d", ix);
return cloneString(buf);
}

void setupColumnNum(struct column *col, char *parameters)
/* Set up column that displays index in displayed list. */
{
columnDefaultMethods(col);
col->cellVal = numberVal;
col->cellPrint = cellSelfLinkPrint;
}

/* ---- Simple table lookup type columns ---- */

void lookupTypeMethods(struct column *col, char *table, char *key, char *val)
/* Set up the methods for a simple lookup column. */
{
col->table = cloneString(table);
col->keyField = cloneString(key);
col->valField = cloneString(val);
col->exists = simpleTableExists;
col->cellVal = cellLookupVal;
}

void setupColumnLookup(struct column *col, char *parameters)
/* Set up column that just looks up one field in a table
 * keyed by the geneId. */
{
char *table = nextWord(&parameters);
char *keyField = nextWord(&parameters);
char *valField = nextWord(&parameters);
if (valField == NULL)
    errAbort("Not enough fields in type lookup for %s", col->name);
lookupTypeMethods(col, table, keyField, valField);
}

/* ---- Distance table type columns ---- */

static char *cellDistanceVal(struct column *col, struct genePos *gp, struct sqlConnection *conn)
/* Get a field in a table defined by col->table, col->keyField, col->valField. */
{
char query[512];
struct sqlResult *sr;
char **row;
char *res = NULL;
safef(query, sizeof(query), "select %s from %s where %s = '%s' and %s = '%s'",
	col->valField, col->table, col->keyField, gp->name, col->curGeneField, curGeneId->name);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    res = cloneString(row[0]);
sqlFreeResult(&sr);
return res;
}

void distanceTypeMethods(struct column *col, char *table, 
	char *curGene, char *otherGene, char *valField)
/* Set up a column that looks up a field in a distance matrix
 * type table such as the expression or homology tables. */
{
col->table = cloneString(table);
col->keyField = cloneString(otherGene);
col->valField = cloneString(valField);
col->curGeneField = cloneString(curGene);
col->exists = simpleTableExists;
col->cellVal = cellDistanceVal;
}

void setupColumnDistance(struct column *col, char *parameters)
/* Set up a column that looks up a field in a distance matrix
 * type table such as the expression or homology tables. */
{
char *table = nextWord(&parameters);
char *curGene = nextWord(&parameters);
char *otherGene = nextWord(&parameters);
char *valField = nextWord(&parameters);
if (valField == NULL)
    errAbort("Not enough fields in type distance for %s", col->name);
distanceTypeMethods(col, table, curGene, otherGene, valField);
}

/* ---- Page/Form Making stuff ---- */

void controlPanel()
/* Make control panel. */
{
hPrintf("<TABLE WIDTH=\"100%%\" BORDER=0 CELLSPACING=1 CELLPADDING=1>\n");
hPrintf("<TR><TD ALIGN=CENTER>");

/* Do configure button. */
    {
    cgiMakeButton(confVarName, "configure");
    hPrintf(" ");
    }

/* Do sort by drop-down */
groupOn = cartUsualString(cart, groupVarName, "expression");
    {
    static char *menu[] = {"expression", "homology", "position"};
    hPrintf("group by ");
    cgiMakeDropList(groupVarName, menu, ArraySize(menu), groupOn);
    }

/* Do items to display drop-down */
    {
    char *count = cartUsualString(cart, countVarName, "25");
    static char *menu[] = {"25", "50", "100", "200", "500", "1000"};
    hPrintf(" display ");
    cgiMakeDropList(countVarName, menu, ArraySize(menu), count);
    displayCount = atoi(count);
    }

/* Do search box. */
    {
    char *search = cartUsualString(cart, searchVarName, "");
    hPrintf(" search ");
    cgiMakeTextVar(searchVarName,  search, 25);
    }

/* Do go button. */
    {
    hPrintf(" ");
    cgiMakeButton("submit", "Go!");
    }

hPrintf("</TD></TR></TABLE>");
}

struct genePos *findGeneId()
/* Find out gene ID from search term. */
{
char *search = cartUsualString(cart, searchVarName, "");
char query[256];
struct sqlResult *sr;
char **row;
struct genePos *gpList = NULL, *gp;

search = trimSpaces(search);
if (sameString(search, ""))
    search = NULL;
if (search != NULL)
    {
    struct sqlConnection *conn = hAllocConn();
    safef(query, sizeof(query), 
    	"select name,chrom,txStart,txEnd from knownGene where name = '%s'", search);
    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
        {
	AllocVar(gp);
	gp->name = cloneString(row[0]);
	gp->chrom = cloneString(row[1]);
	gp->start = sqlUnsigned(row[2]);
	gp->end = sqlUnsigned(row[3]);
	slAddHead(&gpList, gp);
	}
    sqlFreeResult(&sr);
    hFreeConn(&conn);
    }
slReverse(&gpList);
return gpList;
}

struct genePos *genePosSimple(char *name)
/* Return a simple gene pos - with no chrom info. */
{
struct genePos *gp;
AllocVar(gp);
gp->name = cloneString(name);
return gp;
}

struct genePos *genePosFull(char *name, char *chrom, int start, int end)
/* Return full gene position */
{
struct genePos *gp = genePosSimple(name);
gp->chrom = cloneString(chrom);
gp->start = start;
gp->end = end;
return gp;
}

static struct genePos *neighborhoodList(struct sqlConnection *conn, char *query, 
	int maxCount)
/* Get list of up to maxCount from query. */
{
struct sqlResult *sr;
char **row;
struct genePos *list = NULL, *name;
struct hash *dupeHash = newHash(0);
int count = 0;
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (!hashLookup(dupeHash, row[0]))
	{
	hashAdd(dupeHash, row[0], NULL);
	name = genePosSimple(row[0]);
	slAddHead(&list, name);
	if (++count >= maxCount)
	    break;
	}
    }
freeHash(&dupeHash);
sqlFreeResult(&sr);
slReverse(&list);
return list;
}

struct genePos *getExpressionNeighbors(struct sqlConnection *conn)
/* Get expression neighborhood. */
{
struct dyString *query = dyStringNew(1024);
struct genePos *list;

/* Look for matchers.  Look for a few more than they ask for to
 * account for dupes. */
dyStringPrintf(query, 
	"select target from knownExpDistance where query='%s'", 
	curGeneId->name);
dyStringPrintf(query, " order by distance limit %d", displayCount);
list = neighborhoodList(conn, query->string, displayCount);
dyStringFree(&query);
return list;
}

struct genePos *getHomologyNeighbors(struct sqlConnection *conn)
/* Get homology neighborhood. */
{
struct dyString *query = dyStringNew(1024);
struct genePos *list;

/* Look for matchers.  Look for a few more than they ask for to
 * account for dupes. */
dyStringPrintf(query, 
	"select target from knownBlastTab where query='%s'", 
	curGeneId->name);
dyStringPrintf(query, " order by bitScore desc limit %d", (int)(displayCount*1.5));
list = neighborhoodList(conn, query->string, displayCount);
dyStringFree(&query);
return list;
}


struct genePos *getGenomicNeighbors(struct sqlConnection *conn, struct genePos *curGp)
/* Get neighbors in genome. */
{
struct genePos *gpList = NULL, *gp, *next;
char query[256];
struct sqlResult *sr;
char **row;
int i, ix = 0, chosenIx = -1;
int startIx, endIx, listSize;
int geneCount = 0;
struct genePos *geneList = NULL, *gene;

/* Get list of all genes in chromosome */
safef(query, sizeof(query), 
	"select name,txStart,txEnd from knownGene where chrom='%s'", curGp->chrom);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    gp = genePosFull(row[0], curGp->chrom, 
    	sqlUnsigned(row[1]), sqlUnsigned(row[2]));
    slAddHead(&gpList, gp);
    if (curGp->start == gp->start && curGp->end == gp->end 
    	&& sameString(curGp->name, gp->name) )
        chosenIx = ix;
    ++ix;
    ++geneCount;
    }
sqlFreeResult(&sr);
slReverse(&gpList);

if (chosenIx < 0)
    errAbort("%s is not mapped", curGp->name);

/* Figure out start and ending index of genes we want. */
startIx = chosenIx - displayCount/2;
endIx = startIx + displayCount;
if (startIx < 0) startIx = 0;
if (endIx > geneCount) endIx = geneCount;
listSize = endIx - startIx;

gp = slElementFromIx(gpList, startIx);
for (i=0; i<listSize; ++i, gp=next)
    {
    next = gp->next;
    slAddHead(&geneList, gp);
    }
slReverse(&geneList);

return geneList;
}


struct genePos *getPositionNeighbors(struct sqlConnection *conn)
/* Get genes in genomic neighborhood. */
{
return getGenomicNeighbors(conn, curGeneId);
}

struct genePos *getNeighbors(struct sqlConnection *conn)
/* Return gene neighbors. */
{
if (sameString(groupOn, "expression"))
    return getExpressionNeighbors(conn);
else if (sameString(groupOn, "homology"))
    return getHomologyNeighbors(conn);
else if (sameString(groupOn, "position"))
    return getPositionNeighbors(conn);
else
    {
    errAbort("Unknown sort value %s", groupOn);
    return NULL;
    }
}

int columnCmpPriority(const void *va, const void *vb)
/* Compare to sort columns based on priority. */
{
const struct column *a = *((struct column **)va);
const struct column *b = *((struct column **)vb);
float dif = a->priority - b->priority;
if (dif < 0)
    return -1;
else if (dif > 0)
    return 1;
else
    return 0;
}

static void refinePriorities(struct column *colList)
/* Consult colOrderVar if it exists to reorder priorities. */
{
char *orig = cartOptionalString(cart, colOrderVar);
if (orig != NULL)
    {
    char *dupe = cloneString(orig);
    char *s = dupe;
    char *name, *val;
    struct hash *colHash = hashColumns(colList);
    struct column *col;
    while ((name = nextWord(&s)) != NULL)
        {
	if ((val = nextWord(&s)) == NULL || !isdigit(val[0]))
	    {
	    warn("Bad priority list: %s", orig);
	    cartRemove(cart, colOrderVar);
	    break;
	    }
	col = hashFindVal(colHash, name);
	if (col != NULL)
	    col->priority = atof(val);
	}
    hashFree(&colHash);
    freez(&dupe);
    }
}

void refineVisibility(struct column *colList)
/* Consult cart to set on/off visibility. */
{
char varName[128], *val;
struct column *col;

for (col = colList; col != NULL; col = col->next)
    {
    safef(varName, sizeof(varName), "near.col.%s", col->name);
    val = cartOptionalString(cart, varName);
    if (val != NULL)
	col->on = sameString(val, "on");
    }
}

char *mustFindInRaHash(struct lineFile *lf, struct hash *raHash, char *name)
/* Look up in ra hash or die trying. */
{
char *val = hashFindVal(raHash, name);
if (val == NULL)
    errAbort("Missing required %s field in record ending line %d of %s",
    	name, lf->lineIx, lf->fileName);
return val;
}

void setupColumnType(struct column *col)
/* Set up methods and column-specific variables based on
 * track type. */
{
char *dupe = cloneString(col->type);	
char *s = dupe;
char *type = nextWord(&s);

if (type == NULL)
    warn("Missing type value for column %s", col->name);
if (sameString(type, "num"))
    setupColumnNum(col, s);
else if (sameString(type, "lookup"))
    setupColumnLookup(col, s);
else if (sameString(type, "acc"))
    setupColumnAcc(col, s);
else if (sameString(type, "distance"))
    setupColumnDistance(col, s);
else if (sameString(type, "knownPos"))
    setupColumnKnownPos(col, s);
else if (sameString(type, "lookupKnown"))
    setupColumnLookupKnown(col, s);
else if (sameString(type, "expRatio"))
    setupColumnExpRatio(col, s);
else
    errAbort("Unrecognized type %s for %s", col->type, col->name);
freez(&dupe);
}


struct column *getColumns(struct sqlConnection *conn)
/* Return list of columns for big table. */
{
char *raName = "columnDb.ra";
struct lineFile *lf = lineFileOpen(raName, TRUE);
struct column *col, *colList = NULL;
struct hash *raHash;

while ((raHash = raNextRecord(lf)) != NULL)
    {
    AllocVar(col);
    col->name = mustFindInRaHash(lf, raHash, "name");
    col->shortLabel = mustFindInRaHash(lf, raHash, "shortLabel");
    col->longLabel = mustFindInRaHash(lf, raHash, "longLabel");
    col->priority = atof(mustFindInRaHash(lf, raHash, "priority"));
    col->on = sameString(mustFindInRaHash(lf, raHash, "visibility"), "on");
    col->type = mustFindInRaHash(lf, raHash, "type");
    col->settings = raHash;
    columnDefaultMethods(col);
    setupColumnType(col);
    if (col->exists(col, conn))
	slAddHead(&colList, col);
    }
lineFileClose(&lf);
refinePriorities(colList);
refineVisibility(colList);
slSort(&colList, columnCmpPriority);
return colList;
}

struct hash *hashColumns(struct column *colList)
/* Return a hash of columns keyed by name. */
{
struct column *col;
struct hash *hash = hashNew(8);
for (col = colList; col != NULL; col = col->next)
    {
    if (hashLookup(hash, col->name))
        warn("duplicate %s in column list", col->name);
    hashAdd(hash, col->name, col);
    }
return hash;
}

void bigTable()
/* Put up great big table. */
{
struct sqlConnection *conn = hAllocConn();
struct genePos *geneList = getNeighbors(conn), *gene;
struct column *colList = getColumns(conn), *col;

hPrintf("<TABLE BORDER=1 CELLSPACING=1 CELLPADDING=1>\n");

/* Print label row. */
hPrintf("<TR BGCOLOR=\"#E0E0FF\">");
for (col = colList; col != NULL; col = col->next)
    {
    char *colName = col->shortLabel;
    if (col->on)
	{
	col->labelPrint(col);
	}
    }
hPrintf("</TR>\n");

/* Print other rows. */
for (gene = geneList; gene != NULL; gene = gene->next)
    {
    if (sameString(gene->name, curGeneId->name))
        hPrintf("<TR BGCOLOR=\"#D0FFD0\">");
    else
        hPrintf("<TR>");
    for (col = colList; col != NULL; 
    		col = col->next)
        {
	if (col->on)
	    {
	    if (col->cellPrint == NULL)
		hPrintf("<TD></TD>");
	    else
		col->cellPrint(col,gene,conn);
	    }
	}
    hPrintf("</TR>");
    }

hPrintf("</TABLE>");
hFreeConn(&conn);
}

void doMain(struct genePos *gp)
/* The main page. */
{
char buf[128];
safef(buf, sizeof(buf), "UCSC %s Gene Family Browser", organism);
makeTitle(buf, "hgNear.html");
hPrintf("<FORM ACTION=\"../cgi-bin/hgNear\" METHOD=GET>\n");
controlPanel();
if (gp != NULL)
    {
    curGeneId = gp;
    bigTable();
    }
}

void doSearch()
/* Search.  If result is unambiguous call doMain, otherwise
 * put up a page of choices. */
{
doMain(findGeneId());
}

void doMiddle(struct cart *theCart)
/* Write the middle parts of the HTML page. 
 * This routine sets up some globals and then
 * dispatches to the appropriate page-maker. */
{
char *var = NULL;
cart = theCart;
getDbAndGenome(cart, &database, &organism);
hSetDb(database);
if (cgiVarExists(confVarName))
    doConfigure(NULL);
else if ((var = cartFindFirstLike(cart, "near.up.*")) != NULL)
    doConfigure(var);
else if ((var = cartFindFirstLike(cart, "near.down.*")) != NULL)
    doConfigure(var);
else if (cgiVarExists(defaultConfName))
    {
    doDefaultConfigure();
    }
else
    {
    doSearch(cartOptionalString(cart, searchVarName));
    }
cartRemoveLike(cart, "near.up.*");
cartRemoveLike(cart, "near.down.*");
cartSaveSession(cart);
hPrintf("</FORM>");
}

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgNear - gene family browser - a cgi script\n"
  "usage:\n"
  "   hgNear\n"
  );
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
cartHtmlShell("Gene Family v1", doMiddle, hUserCookie(), excludeVars, NULL);
return 0;
}
