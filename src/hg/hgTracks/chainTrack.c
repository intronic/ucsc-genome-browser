/* chainTrack - stuff to load and display chain type tracks in
 * browser.   Chains are typically from cross-species genomic
 * alignments. */

#include "common.h"
#include "hash.h"
#include "localmem.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "hgTracks.h"
#include "chainBlock.h"
#include "chainLink.h"
#include "chainDb.h"

static char const rcsid[] = "$Id: chainTrack.c,v 1.16 2003/07/15 17:52:25 braney Exp $";


static void doQuery(struct sqlConnection *conn, char *fullName, 
			struct lm *lm, struct hash *hash, 
			int start, int end, char * chainId)
/* doQuery- check the database for chain elements between
 * 	start and end.  Use the passed hash to resolve chain
 * 	id's and place the elements into the right
 * 	linkedFeatures structure
 */
{
struct sqlResult *sr = NULL;
char **row;
struct linkedFeatures *lf;
struct simpleFeature *sf;
struct dyString *query = newDyString(1024);

if (chainId == NULL)
    dyStringPrintf(query, 
	"select chainId,tStart,tEnd,qStart from %sLink where ",fullName);
else
    dyStringPrintf(query, 
	"select chainId, tStart,tEnd,qStart from %sLink where chainId=%s and ",fullName,chainId);
hAddBinToQuery(start, end, query);
dyStringPrintf(query, "tStart<%u and tEnd>%u", end, start);
sr = sqlGetResult(conn, query->string);

/* Loop through making up simple features and adding them
 * to the corresponding linkedFeature. */
while ((row = sqlNextRow(sr)) != NULL)
    {
    lf = hashFindVal(hash, row[0]);
    if (lf != NULL)
	{
	lmAllocVar(lm, sf);
	sf->start = sqlUnsigned(row[1]);
	sf->end = sqlUnsigned(row[2]);
	sf->grayIx = sqlUnsigned(row[3]); /* actually qStart */
	slAddHead(&lf->components, sf);
	}
    }
sqlFreeResult(&sr);
dyStringFree(&query);
}

static void chainDraw(struct track *tg, int seqStart, int seqEnd,
        struct vGfx *vg, int xOff, int yOff, int width, 
        MgFont *font, Color color, enum trackVisibility vis)
/* Draw chained features. This loads up the simple features from 
 * the chainLink table, calls linkedFeaturesDraw, and then
 * frees the simple features again. */
{
struct linkedFeatures *lf;
struct simpleFeature *sf;
struct lm *lm;
struct hash *hash;	/* Hash of chain ids. */
struct sqlConnection *conn;
double scale = ((double)(winEnd - winStart))/width;
char fullName[64];
int start, end, extra;
struct simpleFeature *components, *lastSf;
int maxOverLeft = 0, maxOverRight = 0;
int overLeft, overRight;

if (tg->items == NULL)		/*Exit Early if nothing to do */
    return;

lm = lmInit(1024*4);
hash = newHash(0);
conn = hAllocConn();

/* Make up a hash of all linked features keyed by
 * id, which is held in the extras field.  To
 * avoid burning memory on full chromosome views
 * we'll just make a single simple feature and
 * exclude from hash chains less than three pixels wide, 
 * since these would always appear solid. */
for (lf = tg->items; lf != NULL; lf = lf->next)
    {
    double pixelWidth = (lf->end - lf->start) / scale;
    if (pixelWidth >= 2.5)
	{
	hashAdd(hash, lf->extra, lf);
	overRight = lf->end - seqEnd;
	if (overRight > maxOverRight)
	    maxOverRight = overRight;
	overLeft = seqStart - lf->start ;
	if (overLeft > maxOverLeft)
	    maxOverLeft = overLeft;
	}
    else
	{
	lmAllocVar(lm, sf);
	sf->start = lf->start;
	sf->end = lf->end;
	sf->grayIx = lf->grayIx;
	lf->components = sf;
	}
    }

/* if some chains are bigger than 3 pixels */
if (hash->size)
    {
    /* Make up range query. */
    sprintf(fullName, "%s_%s", chromName, tg->mapName);
    if (!hTableExistsDb(hGetDb(), fullName))
	strcpy(fullName, tg->mapName);

    /* in dense mode we don't draw the lines 
     * so we don't need items off the screen 
     */
    if (vis == tvDense)
	doQuery(conn, fullName, lm,  hash, seqStart, seqEnd, NULL);
    else
	{
	/* if chains extend beyond edge of window we need to get 
	 * elements that are off the screen
	 * in both directions so we know whether to draw
	 * one or two lines to the edge of the screen.
	 */
#define STARTSLOP	10000
	extra = (STARTSLOP < maxOverLeft) ? STARTSLOP : maxOverLeft;
	start = seqStart - extra;
	extra = (STARTSLOP < maxOverRight) ? STARTSLOP : maxOverRight;
	end = seqEnd + extra;
	doQuery(conn, fullName, lm,  hash, start, end, NULL);

	for (lf = tg->items; lf != NULL; lf = lf->next)
	    {
	    if (lf->components != NULL)
		slSort(&lf->components, linkedFeaturesCmpStart);
	    extra = (STARTSLOP < maxOverRight)?STARTSLOP:maxOverRight;
	    end = seqEnd + extra;
	    while (lf->end > end )
		{
		for(lastSf=sf=lf->components;sf;lastSf=sf,sf=sf->next)
		    ;

		/* get out if we have an element off right side */
		if ( (lastSf != NULL) &&(lastSf->end > seqEnd))
		    break;

		extra *= 10;
		start = end;
		end = start + extra;
		doQuery(conn, fullName, lm,  hash, start, end, lf->extra);
		if (lf->components != NULL)
		    slSort(&lf->components, linkedFeaturesCmpStart);
		}

	    /* we know we have a least one component off right
	     * now look for one off left
	     */
	    extra = (STARTSLOP < maxOverLeft) ? STARTSLOP:maxOverLeft;
	    start = seqStart - extra; 
	    while((lf->start < start) && 
		(lf->components->start > seqStart))
		{
		extra *= 10;
		end = start;
		start = end - extra;
		doQuery(conn, fullName, lm,  hash, start, end, lf->extra);
		slSort(&lf->components, linkedFeaturesCmpStart);
		}
	    }
	}
    }
linkedFeaturesDraw(tg, seqStart, seqEnd, vg, xOff, yOff, width,
	font, color, vis);
/* Cleanup time. */
for (lf = tg->items; lf != NULL; lf = lf->next)
    lf->components = NULL;

lmCleanup(&lm);
freeHash(&hash);
hFreeConn(&conn);
}

void chainLoadItems(struct track *tg)
/* Load up all of the chains from correct table into tg->items 
 * item list.  At this stage to conserve memory for other tracks
 * we don't load the links into the components list until draw time. */
{
char *track = tg->mapName;
struct chain chain;
int rowOffset;
char **row;
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr = NULL;
struct linkedFeatures *list = NULL, *lf;
int qs;
char optionChr[128]; /* Option -  chromosome filter */
char *optionChrStr;
char extraWhere[128] ;

snprintf( optionChr, sizeof(optionChr), "%s.chromFilter", tg->mapName);
optionChrStr = cartUsualString(cart, optionChr, "All");
if (startsWith("chr",optionChrStr)) 
    {
    snprintf(extraWhere, sizeof(extraWhere), "qName = \"%s\"",optionChrStr);
    sr = hRangeQuery(conn, track, chromName, winStart, winEnd, extraWhere, &rowOffset);
    }
else
    {
    snprintf(extraWhere, sizeof(extraWhere), " ");
    sr = hRangeQuery(conn, track, chromName, winStart, winEnd, NULL, &rowOffset);
    }
while ((row = sqlNextRow(sr)) != NULL)
    {
    char buf[16];
    chainHeadStaticLoad(row + rowOffset, &chain);
    AllocVar(lf);
    lf->start = lf->tallStart = chain.tStart;
    lf->end = lf->tallEnd = chain.tEnd;
    lf->grayIx = maxShade;
    lf->filterColor = -1;
    lf->score = chain.score;
    if (chain.qStrand == '-')
	{
	lf->orientation = -1;
        qs = chain.qSize - chain.qEnd;
	}
    else
        {
	lf->orientation = 1;
	qs = chain.qStart;
	}
    snprintf(lf->name, sizeof(lf->name), "%s %c %dk", 
    	chain.qName, chain.qStrand, qs/1000);
    snprintf(lf->popUp, sizeof(lf->name), "%s %c start %d size %d",
    	chain.qName, chain.qStrand, qs, chain.qEnd - chain.qStart);
    snprintf(buf, sizeof(buf), "%d", chain.id);
    lf->extra = cloneString(buf);
    slAddHead(&list, lf);
    }

/* Make sure this is sorted if in full mode. */
if (tg->visibility != tvDense)
    slSort(&list, linkedFeaturesCmpStart);
else
    slReverse(&list);
tg->items = list;

/* Clean up. */
sqlFreeResult(&sr);
hFreeConn(&conn);
}

void chainMethods(struct track *tg)
/* Fill in custom parts of alignment chains. */
{
char option[128]; /* Option -  rainbow chromosome color */
char optionChr[128]; /* Option -  chromosome filter */
char *optionChrStr; 
char *optionStr ;
linkedFeaturesMethods(tg);
snprintf(option, sizeof(option), "%s.color", tg->mapName);
optionStr = cartUsualString(cart, option, "on");
tg->itemColor = lfChromColor;
tg->loadItems = chainLoadItems;
tg->drawItems = chainDraw;
tg->mapItemName = lfMapNameFromExtra;
tg->subType = lfSubChain;
}

