/* bigWigTrack - stuff to handle loading and display of bigWig type tracks in browser.   */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "hgTracks.h"
#include "localmem.h"
#include "wigCommon.h"
#include "bbiFile.h"
#include "bigWig.h"
#include "errCatch.h"
#include "container.h"
#include "bigWarn.h"

struct preDrawContainer *bigWigLoadPreDraw(struct track *tg, int seqStart, int seqEnd, int width)
/* Do bits that load the predraw buffer tg->preDrawContainer. */
{
/* Just need to do this once... */
if (tg->preDrawContainer != NULL)
    return tg->preDrawContainer;

struct preDrawContainer *preDrawList = NULL;
/* protect against temporary network error */
struct errCatch *errCatch = errCatchNew();
if (errCatchStart(errCatch))
    {
    /* Allocate predraw area. */

    /* Get summary info from bigWig */
    int summarySize = width;
    struct bbiSummaryElement *summary;
    AllocArray(summary, summarySize);

    struct bbiFile *bbiFile ;
    for(bbiFile = tg->bbiFile; bbiFile ; bbiFile = bbiFile->next)
	{
	struct preDrawContainer *pre = initPreDrawContainer(width);
	slAddHead(&preDrawList, pre);

	if (bigWigSummaryArrayExtended(bbiFile, chromName, winStart, winEnd, summarySize, summary))
	    {
	    /* Convert format to predraw */
	    int i;
	    int preDrawZero = pre->preDrawZero;
	    struct preDrawElement *preDraw = pre->preDraw;
	    for (i=0; i<summarySize; ++i)
		{
		struct preDrawElement *pe = &preDraw[i + preDrawZero];
		struct bbiSummaryElement *be = &summary[i];
		pe->count = be->validCount;
		pe->min = be->minVal;
		pe->max = be->maxVal;
		pe->sumData = be->sumData;
		pe->sumSquares = be->sumSquares;
		}
	    }
	}
    freeMem(summary);
    }
errCatchEnd(errCatch);
if (errCatch->gotError)
    {
    tg->networkErrMsg = cloneString(errCatch->message->string);
    }
errCatchFree(&errCatch);
tg->preDrawContainer = preDrawList;
return preDrawList;
}

static void bigWigDrawItems(struct track *tg, int seqStart, int seqEnd,
	struct hvGfx *hvg, int xOff, int yOff, int width,
	MgFont *font, Color color, enum trackVisibility vis)
{
boolean canCloseHere = FALSE;
struct preDrawContainer *pre = tg->customPt; // populated by bigWigLoadItems
if (! pre)	// custom tracks could not do this at their load time
    {
    pre = bigWigLoadPreDraw(tg, seqStart, seqEnd, width);
    canCloseHere = TRUE;
    }

if (tg->networkErrMsg == NULL)
    {
    /* Call actual graphing routine. */
    wigDrawPredraw(tg, seqStart, seqEnd, hvg, xOff, yOff, width, font, color, vis,
		   pre, pre->preDrawZero, pre->preDrawSize, 
		   &tg->graphUpperLimit, &tg->graphLowerLimit);
    if (canCloseHere)
	bigWigFileClose(&tg->bbiFile);
    }
else
    bigDrawWarning(tg, seqStart, seqEnd, hvg, xOff, yOff, width, font, color, vis);
}

static void bigWigOpenCatch(struct track *tg, char *fileName)
/* Try to open big wig file, store error in track struct */
{
/* protect against temporary network error */
struct errCatch *errCatch = errCatchNew();
if (errCatchStart(errCatch))
    {
    struct bbiFile *bbiFile = bigWigFileOpen(fileName);
    slAddHead(&tg->bbiFile, bbiFile);
    }
errCatchEnd(errCatch);
if (errCatch->gotError)
    {
    tg->networkErrMsg = cloneString(errCatch->message->string);
    tg->drawItems = bigDrawWarning;
    if (!sameOk(parentContainerType(tg), "multiWig"))
	tg->totalHeight = bigWarnTotalHeight;
    }
errCatchFree(&errCatch);
}


static void bigWigLoadItems(struct track *tg)
/* Fill up tg->items with bedGraphItems derived from a bigWig file */
{
char *extTableString = trackDbSetting(tg->tdb, "extTable");

if (tg->bbiFile == NULL)
    {
    /* Figure out bigWig file name. */
    struct sqlConnection *conn = hAllocConnTrack(database, tg->tdb);
    char *fileName = bbiNameFromSettingOrTable(tg->tdb, conn, tg->table);
    bigWigOpenCatch(tg, fileName);
    // if there's an extra table, read this one in too
    if (extTableString != NULL)
	{
	fileName = bbiNameFromSettingOrTable(tg->tdb, conn, extTableString);
        bigWigOpenCatch(tg, fileName);
	}
    hFreeConn(&conn);
    tg->customPt = (void *)bigWigLoadPreDraw(tg, winStart, winEnd, insideWidth);
    bigWigFileClose(&tg->bbiFile);
    }
}

void bigWigMethods(struct track *track, struct trackDb *tdb, 
	int wordCount, char *words[])
/* Set up bigWig methods. */
{
bedGraphMethods(track, tdb, wordCount, words);
track->loadItems = bigWigLoadItems;
track->drawItems = bigWigDrawItems;
track->loadPreDraw = bigWigLoadPreDraw;
}
