/* Handle details pages for wiggle tracks. */

#include "common.h"
#include "wiggle.h"
#include "cart.h"
#include "hgc.h"
#include "hCommon.h"
#include "hgColors.h"
#include "obscure.h"
#include "customTrack.h"

void genericWiggleClick(struct sqlConnection *conn, struct trackDb *tdb, 
	char *item, int start)
/* Display details for Wiggle data tracks.   conn is NULL for custom tracks */
{
char *chrom = cartString(cart, "c");
char table[64];
boolean hasBin;
unsigned span = 0;
struct wiggleDataStream *wDS = newWigDataStream();
unsigned long long valuesMatched = 0;
struct histoResult *histoGramResult;
float *valuesArray = NULL;
float *fptr = NULL;
size_t valueCount = 0;
struct customTrack *ct;
boolean isCustom = FALSE;

if (startsWith("ct_", tdb->tableName))
    {
    ct = lookupCt(tdb->tableName);
    if (!ct)
        {
        warn("<P>wiggleClick: can not find custom wiggle track '%s'</P>", tdb->tableName);
        return;
        }
    if (! ct->wiggle)
        {
        warn("<P>wiggleClick: called to do stats on a custom track that isn't wiggle data ?</P>");
        return;
        }
    safef(table,ArraySize(table), "%s", ct->wigFile);
    isCustom = TRUE;
    span = 0;	/*	cause all spans to be examined	*/
    }
else
    {
    hFindSplitTable(seqName, tdb->tableName, table, &hasBin);
    span = spanInUse(conn, table, chrom, winStart, winEnd, cart);
    }

/*	if for some reason we don't have a chrom and win positions, this
 *	should be run in a loop that does one chrom at a time.  In the
 *	case of hgc, there seems to be a chrom and a position.
 */
wDS->setSpanConstraint(wDS, span);
wDS->setChromConstraint(wDS, chrom);
wDS->setPositionConstraint(wDS, winStart, winEnd);

/*	We want to also fetch the actual data values so we can run a
 *	histogram function on them.  You can't fetch the data in the
 *	form of the data array since the span information is then lost.
 *	We have to do the ascii data list format, and prepare that to
 *	send to the histogram function.
 */
valuesMatched = wDS->getData(wDS, database, table,
			wigFetchStats | wigFetchAscii );

statsPreamble(wDS, chrom, winStart, winEnd, span, valuesMatched);

/*	output statistics table	*/
wDS->statsOut(wDS, "stdout", TRUE, TRUE);

/*	convert the ascii data listings to one giant float array 	*/
valuesArray = wDS->asciiToDataArray(wDS, valuesMatched, &valueCount);
fptr = valuesArray;

/*	histoGram() may return NULL if it doesn't work	*/

histoGramResult = histoGram(valuesArray, valueCount,
	    NAN, (unsigned) 0, NAN, (float) wDS->stats->lowerLimit,
		(float) (wDS->stats->lowerLimit + wDS->stats->dataRange),
		(struct histoResult *)NULL);

printHistoGram(histoGramResult);

freeHistoGram(&histoGramResult);
freeMem(valuesArray);
destroyWigDataStream(&wDS);
}
