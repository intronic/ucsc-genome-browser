/* binRange Stuff to handle binning - which helps us restrict 
 * our attention to the parts of database that contain info
 * about a particular window on a chromosome. This scheme
 * will work without modification for chromosome sizes up
 * to half a gigaBase.  The finest sized bin is 128k (1<<17).
 * The next coarsest is 8x as big (1<<3).  There's a hierarchy
 * of bins with the chromosome itself being the final bin.
 * Features are put in the finest bin they'll fit in. */

#include "common.h"
#include "binRange.h"

static int binOffsets[] = {512+64+8+1, 64+8+1, 8+1, 1, 0};
#define _binFirstShift 17	/* How much to shift to get to finest bin. */
#define _binNextShift 3		/* How much to shift to get to next larger bin. */



int binLevels()
/* Return number of levels to bins. */
{
return ArraySize(binOffsets);
}

int binFirstShift()
/* Return amount to shift a number to get to finest bin. */
{
return _binFirstShift;
}

int binNextShift()
/* Return amount to shift a numbe to get to next coarser bin. */
{
return _binNextShift;
}

int binOffset(int level)
/* Return offset for bins of a given level. */
{
assert(level >= 0 && level < ArraySize(binOffsets));
return binOffsets[level];
}

int binFromRange(int start, int end)
/* Given start,end in chromosome coordinates assign it
 * a bin.   There's a bin for each 128k segment, for each
 * 1M segment, for each 8M segment, for each 64M segment,
 * and for each chromosome (which is assumed to be less than
 * 512M.)  A range goes into the smallest bin it will fit in. */
{
int startBin = start, endBin = end-1, i;
startBin >>= _binFirstShift;
endBin >>= _binFirstShift;
for (i=0; i<ArraySize(binOffsets); ++i)
    {
    if (startBin == endBin)
        return binOffsets[i] + startBin;
    startBin >>= _binNextShift;
    endBin >>= _binNextShift;
    }
errAbort("start %d, end %d out of range in findBin (max is 512M)", start, end);
return 0;
}


struct binKeeper *binKeeperNew(int minPos, int maxPos)
/* Create new binKeeper that can cover range. */
{
int maxBin;
struct binKeeper *bk;
if (minPos < 0 || maxPos < 0 || minPos > maxPos || maxPos > 512*1024*1024)
    errAbort("bad range %d,%d in binKeeperNew", minPos, maxPos);

maxBin = binFromRange(maxPos-1, maxPos);
AllocVar(bk);
bk->minPos = minPos;
bk->maxPos = maxPos;
bk->binCount = maxBin;
AllocArray(bk->binLists, maxBin);
return bk;
}

void binKeeperFree(struct binKeeper **pBk)
/* Free up a bin keeper. */
{
struct binKeeper *bk = *pBk;
if (bk != NULL)
    {
    int i;
    for (i=0; i<bk->binCount; ++i)
	slFreeList(&bk->binLists[i]);
    freeMem(bk->binLists);
    freez(pBk);
    }
}

void binKeeperAdd(struct binKeeper *bk, int start, int end, void *val)
/* Add item to binKeeper. */ 
{
int bin;
struct binElement *el;
if (start < bk->minPos || end > bk->maxPos || start > end)
    errAbort("(%d %d) out of range (%d %d) in binKeeperAdd", 
    	start, end, bk->minPos, bk->maxPos);
bin = binFromRange(start, end);
assert(bin < bk->binCount);
AllocVar(el);
el->start = start;
el->end = end;
el->val = val;
slAddHead(&bk->binLists[bin], el);
}

int binElementCmpStart(const void *va, const void *vb)
/* Compare to sort based on start. */
{
const struct binElement *a = *((struct binElement **)va);
const struct binElement *b = *((struct binElement **)vb);
return a->start - b->start;
}

struct binElement *binKeeperFind(struct binKeeper *bk, int start, int end)
/* Return a list of all items in binKeeper that intersect range.
 * Free this list with slFreeList. */
{
struct binElement *list = NULL, *newEl, *el;
int startBin = (start>>_binFirstShift), endBin = ((end-1)>>_binFirstShift);
int i,j;

for (i=0; i<ArraySize(binOffsets); ++i)
    {
    for (j=startBin; j<endBin; ++j)
        {
	for (el=bk->binLists[j]; el != NULL; el = el->next)
	    {
	    if (rangeIntersection(el->start, el->end, start, end))
	        {
		newEl = CloneVar(el);
		slAddHead(&list, newEl);
		}
	    }
	}
    startBin >>= _binNextShift;
    endBin >>= _binNextShift;
    }
return list;
}

struct binElement *binKeeperFindSorted(struct binKeeper *bk, int start, int end)
/* Like binKeeperFind, but sort list on start coordinates. */
{
struct binElement *list = binKeeperFind(bk, start, end);
slSort(&list, binElementCmpStart);
return list;
}

