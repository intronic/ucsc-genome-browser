/* wikiTrack - handle the wikiTrack section. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include "spDb.h"
#include "web.h"
#include "hgConfig.h"
#include "hgGene.h"
#include "htmlPage.h"
#include "hgColors.h"
#include "hdb.h"
#include "binRange.h"
#include "wikiLink.h"
#include "wikiTrack.h"

static char const rcsid[] = "$Id: wikiTrack.c,v 1.9 2007/06/28 16:46:56 hiram Exp $";

static char *hgGeneUrl()
{
static char retBuf[1024];
safef(retBuf, ArraySize(retBuf), "cgi-bin/hgGene?%s=1&%s",
	hggDoWikiTrack, cartSidUrlString(cart));
return retBuf;
}

static char *wikiTrackUserLoginUrl()
/* Return the URL for the wiki user login page. */
{
char *retEnc = encodedReturnUrl(hgGeneUrl);
char buf[2048];

safef(buf, sizeof(buf),
      "%s/index.php?title=Special:UserloginUCSC&returnto=%s",
      cfgOptionDefault(CFG_WIKI_URL, NULL), retEnc);
freez(&retEnc);
return(cloneString(buf));
}

static void offerLogin()
/* display login prompts to the wiki when user isn't already logged in */
{
char *wikiHost = wikiLinkHost();
char *loginUrl = wikiTrackUserLoginUrl();
printf("<P>Please login to add annotations to this UCSC gene.</P>\n");
printf("<P>The login page is handled by our "
       "<A HREF=\"http://%s/\" TARGET=_BLANK>wiki system</A>:\n", wikiHost);
printf("<A HREF=\"%s\"><B>click here to login.</B></A><BR>\n", loginUrl);
printf("The wiki also serves as a forum for users "
       "to share knowledge and ideas.\n</P>\n");
freeMem(loginUrl);
}

static struct bed *bedItem(char *chr, int start, int end, char *name)
{
struct bed *bb;
AllocVar(bb);
bb->chrom = chr; /* do not need to clone chr string, it is already a clone */
bb->chromStart = start;
bb->chromEnd = end;
bb->name = cloneString(name);
return bb;
}

static char *canonicalGene(struct sqlConnection *conn, char *id, char **protein)
/* given UCSC gene id, find canonical UCSC gene id and protein if asked for */
{
char *geneName;
struct sqlResult *sr;
char **row;
char query[1024];

safef(query, ArraySize(query), "SELECT e.transcript,e.protein FROM "
	"knownCanonical e, knownIsoforms j "
	"WHERE e.clusterId = j.clusterId AND j.transcript ='%s'", id);

sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row)
    {
    geneName = cloneString(row[0]);
    if (protein)
	*protein = cloneString(row[1]);
    }
else
    geneName = NULL;

sqlFreeResult(&sr);
return geneName;
}

static struct bed *geneCluster(struct sqlConnection *conn, char *geneSymbol,
	struct bed **returnBed)
/* simple cluster of all knownGenes with name geneSymbol
 *	any items overlapping are clustered together
 */
{
struct sqlResult *sr;
char **row;
struct bed *bed;
struct bed *bedList = NULL;
struct bed *clustered = NULL;
char query[1024];

if (! (sqlTableExists(conn, "knownGene") && sqlTableExists(conn, "kgXref")))
    {
    if (returnBed)
	*returnBed = NULL;
    return NULL;
    }

safef(query, ArraySize(query), "SELECT e.chrom,e.txStart,e.txEnd,e.alignID FROM knownGene e, kgXref j WHERE e.alignID = j.kgID AND j.geneSymbol ='%s' ORDER BY e.chrom,e.txStart", geneSymbol);

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    AllocVar(bed);
    bed->chrom = cloneString(row[0]);
    bed->chromStart = sqlUnsigned(row[1]);
    bed->chromEnd = sqlUnsigned(row[2]);
    bed->name = cloneString(row[3]);
    slAddHead(&bedList, bed);
    }
sqlFreeResult(&sr);
slSort(&bedList, bedCmpExtendedChr);

/* now cluster that list */
int start = BIGNUM;
int end = 0;
char *prevChr = NULL;
for (bed = bedList; bed; bed = bed->next)
    {
    int txStart = bed->chromStart;
    int txEnd = bed->chromEnd;
    if (prevChr)
	{
	boolean notOverlap = TRUE;
	if ((txEnd > start) && (txStart < end))
	    notOverlap = FALSE;
	if (notOverlap || differentWord(prevChr,bed->chrom))
	    {
	    struct bed *bb = bedItem(prevChr, start, end, geneSymbol);
	    slAddHead(&clustered, bb);
	    start = txStart;
	    end = txEnd;
	    prevChr = cloneString(bed->chrom);
	    }
	else
	    {
	    if (start > txStart)
		start = txStart;
	    if (end < txEnd)
		end = txEnd;
	    }
	if (differentWord(prevChr,bed->chrom))
	    {
		freeMem(prevChr);
		prevChr = cloneString(bed->chrom);
	    }
	}
    else
	{
	start = txStart;
	end = txEnd;
	prevChr = cloneString(bed->chrom);
	}
    }
struct bed *bb = bedItem(prevChr, start, end, geneSymbol);
slAddHead(&clustered, bb);
slSort(&clustered, bedCmpExtendedChr);
if (returnBed)
    *returnBed = bedList;
else
    bedFreeList(&bedList);
return clustered;
}	/*	static struct bed *geneCluster()	*/

static int addWikiTrackItem(char *db, char *chrom, int start, int end,
    char *name, int score, char *strand, char *owner, char *class,
	char *color, char *category, char *geneSymbol, char *wikiKey)
/* create new wikiTrack row with given parameters */
{
struct sqlConnection *conn = hConnectCentral();
struct wikiTrack *newItem;

AllocVar(newItem);
newItem->bin = binFromRange(start, end);
newItem->chrom = cloneString(chrom);
newItem->chromStart = start;
newItem->chromEnd = end;
newItem->name = cloneString(name);
newItem->score = score;
safef(newItem->strand, sizeof(newItem->strand), "%s", strand);
newItem->db = cloneString(db);
newItem->owner = cloneString(owner);
newItem->class = cloneString(class);
newItem->color = cloneString(color);
newItem->creationDate = cloneString("0");
newItem->lastModifiedDate = cloneString("0");
newItem->descriptionKey = cloneString("0");
newItem->id = 0;
newItem->geneSymbol = cloneString(geneSymbol);

wikiTrackSaveToDbEscaped(conn, newItem, WIKI_TRACK_TABLE, 1024);

int id = sqlLastAutoId(conn);
char descriptionKey[256];
/* when wikiKey is NULL, assign the default key of category:db-id,
 *	else, it is the proper key
 */
if (wikiKey)
    safef(descriptionKey,ArraySize(descriptionKey), "%s", wikiKey);
else
    safef(descriptionKey,ArraySize(descriptionKey),
	"%s:%s-%d", category, db, id);

wikiTrackFree(&newItem);

char query[1024];
safef(query, ArraySize(query), "UPDATE %s set creationDate=now(),lastModifiedDate=now(),descriptionKey='%s' WHERE id='%d'",
    WIKI_TRACK_TABLE, descriptionKey, id);

sqlUpdate(conn,query);
hDisconnectCentral(&conn);
return (id);
}

static struct wikiTrack *startNewItem(struct sqlConnection *conn,
    char *chrom, int itemStart, int itemEnd, char *name, char *strand)
/* create the database item to get a new one started */
{
char *userName = NULL;
int score = 0;
int id = 0;
char *description = descriptionString(curGeneId, conn);
char *aliases = aliasString(curGeneId, conn);
struct dyString *extraHeader = dyStringNew(0);
char *protein = NULL;
char *canonical = canonicalGene(conn, curGeneId, &protein);

if (canonical)
    dyStringPrintf(extraHeader,
	"[http://%s/cgi-bin/hgGene?db=%s&hgg_gene=%s canonical gene details]&nbsp;''%s''<BR>\n",
	    cfgOptionDefault(CFG_WIKI_BROWSER, DEFAULT_BROWSER), database,
	    canonical, name);
else
    dyStringPrintf(extraHeader,
	"[http://%s/cgi-bin/hgGene?db=%s&hgg_gene=%s gene details]&nbsp;''%s''<BR>\n",
	    cfgOptionDefault(CFG_WIKI_BROWSER, DEFAULT_BROWSER), database,
	    curGeneId, name);
if (protein)
    dyStringPrintf(extraHeader,
	"[http://%s/cgi-bin/pbTracks?db=%s&proteinID=%s "
	    "protein details]&nbsp;''%s''<BR>\n",
		cfgOptionDefault(CFG_WIKI_BROWSER, DEFAULT_BROWSER), database,
		    protein, protein);
dyStringPrintf(extraHeader, "%s", description);
if (aliases)
    {
    dyStringPrintf(extraHeader, "%s\n", aliases);
    freeMem(aliases);
    }

dyStringPrintf(extraHeader, "\n<HR>\n");

if (! wikiTrackEnabled(&userName))
    errAbort("create new wiki item: wiki track not enabled");
if (NULL == userName)
    errAbort("create new wiki item: user not logged in ?");

id = addWikiTrackItem(database, chrom, itemStart, itemEnd, name,
    score, strand, userName, GENE_CLASS, "#000000",
	"UCSCGeneAnnotation", name, NULL);

char wikiItemId[64];
safef(wikiItemId,ArraySize(wikiItemId),"%d", id);
struct wikiTrack *item = findWikiItemId(wikiItemId);

addDescription(item, userName, chrom, itemStart, itemEnd, cart, database,
	extraHeader->string);
dyStringFree(&extraHeader);
return(item);
}

static void addComments(struct sqlConnection *conn, struct wikiTrack **item,
    char *userName, struct bed *clusterList)
{
if (*item)
    {
    addDescription(*item, userName, curGeneChrom,
	curGeneStart, curGeneEnd, cart, database, NULL);
    }
else
    {
    struct bed *el = clusterList;
    *item = startNewItem(conn, el->chrom, el->chromStart, el->chromEnd,
	el->name, "+");
    el = el->next;
    for ( ; el; el = el->next)
	{
	(void) addWikiTrackItem(database, el->chrom, el->chromStart,
	    el->chromEnd, el->name, 0, "+", userName, GENE_CLASS, "#000000",
	    "UCSCGeneAnnotation", el->name, (*item)->descriptionKey);
	}
    }
}

void doWikiTrack(struct sqlConnection *conn)
/* display wiki track business */
{
char *userName = NULL;
struct wikiTrack *item = findWikiItemByGeneSymbol(database, curGeneName);
char title[1024];
struct bed *bedList = NULL;
struct bed *clusterList = geneCluster(conn, curGeneName, &bedList);
boolean editOK = FALSE;

safef(title,ArraySize(title), "UCSC gene annotations: %s", curGeneName);
cartWebStart(cart, title);

/* safety check, both of these lists should be non-zero */
int locusLocationCount = slCount(clusterList);
int rawListCount = slCount(bedList);
if ((0 == rawListCount) || (0 == locusLocationCount))
    {
    hPrintf("<EM>(Feature under development, not available for "
	"all genome browsers yet)</EM><BR>\n");
    hPrintf("hgGene.doWikiTrack: can not find any genes "
	"called %s<BR>\n",curGeneName);
    cartWebEnd();
    return;
    }

/* we already know the wiki track is enabled since we are here,
 *	now calling this just to see if user is logged into the wiki
 */
if(!wikiTrackEnabled(&userName))
    errAbort("hgGene.doWikiTrack: called when wiki track is not enabled");
if (isNotEmpty(userName) && emailVerified())
    editOK = TRUE;

if (editOK && cartVarExists(cart, hggDoWikiAddComment))
    addComments(conn, &item, userName, clusterList);
else
    cartRemove(cart, NEW_ITEM_COMMENT);

if (NULL != item)
    {
    displayComments(item);
    hPrintf("\n<HR>\n");
    }
else
    {
    char *protein;
    char *canonical = canonicalGene(conn, curGeneId, &protein);
    hPrintf("<em>(no annotations for this gene at this time)</em><B>%s %s</B><BR>\n<HR>\n", canonical, protein);
    }

if (isEmpty(userName))
    offerLogin();
else if (emailVerified())  /* prints message when not verified */
    {
    hPrintf("<FORM ID=\"hgg_wikiAddComment\" NAME=\"hgg_wikiAddComment\" "
	"METHOD=\"POST\" ACTION=\"../cgi-bin/hgGene\">\n\n");
    cartSaveSession(cart);
    cgiMakeHiddenVar(hggDoWikiTrack, "1");
    cgiMakeHiddenVar(hggDoWikiAddComment, "1");
    cgiMakeHiddenVar("db", database);
    cgiMakeHiddenVar("hgg_gene", curGeneId);
    webPrintLinkTableStart();
    /* first row is a title line */
    char label[256];
    safef(label, ArraySize(label), "'%s' adding comments to gene %s\n",
	userName, curGeneName);
    webPrintWideLabelCell(label, 2);
    webPrintLinkTableNewRow();
    /* second row is initial comment/description text entry */
    webPrintWideCellStart(2, HG_COL_TABLE);
    cgiMakeTextArea(NEW_ITEM_COMMENT, ADD_ITEM_COMMENT_DEFAULT, 3, 40);
    webPrintLinkCellEnd();
    webPrintLinkTableNewRow();
    hPrintf("<TD BGCOLOR=\"#%s\" ALIGN=\"CENTER\" VALIGN=\"TOP\">",
	    HG_COL_TABLE);
    cgiMakeButton("submit", "add comments");
    hPrintf("\n</FORM>\n");
    webPrintLinkCellEnd();
    hPrintf("<TD BGCOLOR=\"#%s\" ALIGN=\"CENTER\" VALIGN=\"TOP\">",
	    HG_COL_TABLE);
    webPrintLinkCellEnd();
    webPrintLinkTableEnd();

    if (NULL != item)
	{
	char *url = cfgOptionDefault(CFG_WIKI_URL, NULL);
	hPrintf("For extensive edits, it may be more convenient to edit the ");
	hPrintf("wiki article <A HREF=\"%s/index.php/%s\" TARGET=_blank>%s</A> "
	   "for this item's description", url, item->descriptionKey,
		item->descriptionKey);
	}
    }

createPageHelp("wikiTrackGeneAnnotationHelp");

hPrintf("<HR>\n");

if ((1 == locusLocationCount) && (1 == rawListCount))
    {
    hPrintf("<B>There is a single location for gene %s (%s)</B><BR>\n",
	    curGeneName, curGeneId);
    }
else
    {
    if (1 == locusLocationCount)
	hPrintf("<B>There is a single locus location for gene %s:</B><BR>\n",
	    curGeneName);
    else
	hPrintf("<B>There are %d locus locations for gene %s:</B><BR>\n",
	    locusLocationCount, curGeneName);

    struct bed *el;
    for (el = clusterList; el; el = el->next)
	hPrintf("%s:%d-%d<BR>\n", el->chrom, el->chromStart, el->chromEnd);
    hPrintf("<B>From %d separate UCSC gene IDs:</B><BR>\n", rawListCount);
    for (el = bedList; el; el = el->next)
	{
	hPrintf("%s", el->name);
	if (el->next)
	    hPrintf(", ");
	}
    hPrintf("<BR>\n");
    }

cartWebEnd();

bedFreeList(&bedList);
bedFreeList(&clusterList);
}
