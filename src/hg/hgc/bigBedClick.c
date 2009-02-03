/* Handle details pages for wiggle tracks. */

#include "common.h"
#include "wiggle.h"
#include "cart.h"
#include "hgc.h"
#include "hCommon.h"
#include "hgColors.h"
#include "bigBed.h"

static char const rcsid[] = "$Id: bigBedClick.c,v 1.1 2009/02/03 21:37:40 kent Exp $";


void genericBigBedClick(struct sqlConnection *conn, struct trackDb *tdb, 
		     char *item, int start, int bedSize)
/* Handle click in generic bigBed track. */
{
char *chrom = cartString(cart, "c");

char query[256];
safef(query, sizeof(query), "select fileName from %s", tdb->tableName);
char *fileName = sqlQuickString(conn, query);
if (fileName == NULL)
    errAbort("Missing fileName in %s table", tdb->tableName);

/* Open BigWig file and get interval list. */
struct bbiFile *bbi = bigBedFileOpen(fileName);
struct lm *lm = lmInit(0);
struct bigBedInterval *bbList = bigBedIntervalQuery(bbi, chrom, winStart, winEnd, lm);
bbiFileClose(&bbi);

/* Find particular item in list - matching start, and item if possible. */
struct bigBedInterval *bbMatch = NULL, *bb;
for (bb = bbList; bb != NULL; bb = bb->next)
    {
    if (bb->start == start)
        {
	if (bedSize > 3)
	    {
	    char *name = cloneFirstWordInLine(bb->rest);
	    boolean match = sameString(name, item);
	    freez(&name);
	    if (match)
	        {
		bbMatch = bb;
		break;
		}
	    }
	else
	    {
	    bbMatch = bb;
	    break;
	    }
	}
    }

if (bbMatch != NULL)
    {
    char *fields[bedSize];
    char startBuf[16], endBuf[16];
    char *rest = cloneString(bbMatch->rest);
    int bbFieldCount = bigBedIntervalToRow(bbMatch, chrom, startBuf, endBuf, fields, bedSize); 
    if (bbFieldCount != bedSize)
        {
	errAbort("Disagreement between trackDb field count (%d) and %s fieldCount (%d)", 
		bedSize, fileName, bbFieldCount);
	}
    struct bed *bed = bedLoadN(fields, bedSize);
    bedPrintPos(bed, bedSize, tdb);
    printf("Full bed record:<BR><PRE><TT>%s\t%u\t%u\t%s\n</TT></PRE>\n",
	    chrom, bb->start, bb->end, rest);
    }
else
    {
    printf("No item %s starting at %d\n", emptyForNull(item), start);
    }

lmCleanup(&lm);
}


