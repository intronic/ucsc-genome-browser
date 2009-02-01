/* bigBed - interface to binary file with bed-style values (that is a bunch of
 * possibly overlapping regions.
 *
 * This shares a lot with the bigWig module.  In particular the query side of
 * the interface is implemented in bwgQuery.c in parallel to the bigWig query
 * implementation.  This file just contains the file creation stuff.  Beware
 * that this does have to keep in sync with the bigWig file creation stuff.
 * Except for the initial 4-byte magic number, and the unzoomed bed data,
 * bigWig and bigBed files are the same. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "localmem.h"
#include "obscure.h"
#include "rangeTree.h"
#include "cirTree.h"
#include "bPlusTree.h"
#include "bwgInternal.h"
#include "sig.h"
#include "bigWig.h"
#include "bigBed.h"

struct ppBed
/* A partially parsed out bed record plus some extra fields. */
    {
    struct ppBed *next;	/* Next in list. */
    char *chrom;		/* Chromosome name (not allocated here) */
    bits32 start, end;		/* Range inside chromosome - half open zero based. */
    char *rest;			/* The rest of the bed. */
    bits64 fileOffset;		/* File offset. */
    bits32 chromId;		/* Chromosome ID. */
    };

static int ppBedCmp(const void *va, const void *vb)
/* Compare to sort based on chrom,start,end. */
{
const struct ppBed *a = *((struct ppBed **)va);
const struct ppBed *b = *((struct ppBed **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    {
    dif = a->start - b->start;
    if (dif == 0)
        dif = a->end - b->end;
    }
return dif;
}

static struct cirTreeRange ppBedFetchKey(const void *va, void *context)
/* Fetch ppBed key for r-tree */
{
struct cirTreeRange res;
const struct ppBed *a = *((struct ppBed **)va);
res.chromIx = a->chromId;
res.start = a->start;
res.end = a->end;
return res;
}

static bits64 ppBedFetchOffset(const void *va, void *context)
/* Fetch ppBed file offset for r-tree */
{
const struct ppBed *a = *((struct ppBed **)va);
return a->fileOffset;
}


static struct ppBed *ppBedLoadAll(char *fileName, struct hash *chromHash, struct lm *lm, 
	bits64 *retDiskSize)
/* Read bed file and return it as list of ppBeds. The whole thing will
 * be allocated in the passed in lm - don't ppBedFreeList or slFree
 * list! */
{
struct ppBed *pbList = NULL, *pb;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line;
bits64 diskSize = 0;
while (lineFileNextReal(lf, &line))
    {
    int i;
    char *words[3];
    for (i=0; i<3; ++i)
	words[i] = nextWord(&line);
    lmAllocVar(lm, pb);
    char *chrom = words[0];
    struct hashEl *hel = hashLookup(chromHash, chrom);
    if (hel == NULL)
        errAbort("%s is not in chrom.sizes line %d of %s", chrom, lf->lineIx, lf->fileName);
    pb->chrom = hel->name;
    pb->start = lineFileNeedNum(lf, words, 1);
    pb->end = lineFileNeedNum(lf, words, 2);
    int len = 0;
    line = skipLeadingSpaces(line);
    if (line != NULL)
	{
        len = strlen(line);
	pb->rest = lmCloneString(lm, skipLeadingSpaces(line));
	}
    diskSize += len + 1 + 3*sizeof(bits32);
    slAddHead(&pbList, pb);
    }
slReverse(&pbList);
*retDiskSize = diskSize;
return pbList;
}

static double ppBedAverageSize(struct ppBed *pbList)
/* Return size of average bed in list. */
{
struct ppBed *pb;
double total = 0.0;
long count = 0;
for (pb = pbList; pb != NULL; pb = pb->next)
    {
    total += pb->end - pb->start;
    count += 1;
    }
return total/count;
}

static void makeChromInfo(struct ppBed *pbList, struct hash *chromSizeHash,
	int *retChromCount, struct bigWigChromInfo **retChromArray,
	int *retMaxChromNameSize)
/* Fill in chromId field in pbList.  Return array of chromosome name/ids. 
 * The chromSizeHash is keyed by name, and has int values. */
{
/* Build up list of unique chromosome names. */
struct ppBed *pb;
char *chromName = "";
int chromCount = 0;
int maxChromNameSize = 0;
struct slRef *uniq, *uniqList = NULL;
for (pb = pbList; pb != NULL; pb = pb->next)
    {
    if (!sameString(pb->chrom, chromName))
        {
	chromName = pb->chrom;
	refAdd(&uniqList, chromName);
	++chromCount;
	int len = strlen(chromName);
	if (len > maxChromNameSize)
	    maxChromNameSize = len;
	}
    pb->chromId = chromCount-1;
    }
slReverse(&uniqList);

/* Allocate and fill in results array. */
struct bigWigChromInfo *chromArray;
AllocArray(chromArray, chromCount);
int i;
for (i = 0, uniq = uniqList; i < chromCount; ++i, uniq = uniq->next)
    {
    chromArray[i].name = uniq->val;
    chromArray[i].id = i;
    chromArray[i].size = hashIntVal(chromSizeHash, uniq->val);
    }

/* Clean up, set return values and go home. */
slFreeList(&uniqList);
*retChromCount = chromCount;
*retChromArray = chromArray;
*retMaxChromNameSize = maxChromNameSize;
}

static struct bwgSummary *summaryOnDepth(struct ppBed *pbList, struct bigWigChromInfo *chromInfoArray, 
	int reduction)
/* Produce a summary based on depth of coverage. */
{
struct bwgSummary *outList = NULL;
struct ppBed *chromStart, *chromEnd;

for (chromStart = pbList; chromStart != NULL; chromStart = chromEnd)
    {
    /* Figure out where end of data on this chromosome is. */
    bits32 chromId = chromStart->chromId;
    for (chromEnd = chromStart; chromEnd != NULL; chromEnd = chromEnd->next)
        if (chromEnd->chromId != chromId)
	    break;

    /* Get a range tree */
    struct rbTree *chromRanges = rangeTreeNew();
    struct ppBed *pb;
    for (pb = chromStart; pb != chromEnd; pb = pb->next)
	rangeTreeAddToCoverageDepth(chromRanges, pb->start, pb->end);

    struct range *range, *rangeList = rangeTreeList(chromRanges);
    verbose(3, "Got %d ranges on %d\n", slCount(rangeList), chromId);
    for (range = rangeList; range != NULL; range = range->next)
        {
	bwgAddRangeToSummary(chromId, chromInfoArray[chromId].size, range->start, range->end,
		ptToInt(range->val), reduction, &outList);
	}
    }
slReverse(&outList);
return outList;
}


void bigBedFileCreate(
	char *inName, 	  /* Input file in a tabular bed format <chrom><start><end> + whatever. */
	char *chromSizes, /* Two column tab-separated file: <chromosome> <size>. */
	int blockSize,	  /* Number of items to bundle in r-tree.  1024 is good. */
	int itemsPerSlot, /* Number of items in lowest level of tree.  64 is good. */
	char *outName)    /* BigBed output file name. */
/* Convert tab-separated bed file to binary indexed, zoomed bigBed version. */
{
/* Declare a bunch of variables. */
int i;
bits32 reserved32 = 0;
bits64 reserved64 = 0;
bits64 dataOffset = 0, dataOffsetPos;
bits64 indexOffset = 0, indexOffsetPos;
bits64 chromTreeOffset = 0, chromTreeOffsetPos;
bits32 sig = bigBedSig;
struct bwgSummary *reduceSummaries[10];
bits32 reductionAmounts[10];
bits64 reductionDataOffsetPos[10];
bits64 reductionDataOffsets[10];
bits64 reductionIndexOffsets[10];

/* Load in chromosome sizes. */
struct hash *chromHash = bwgChromSizesFromFile(chromSizes);
verbose(1, "Read %d chromosomes and sizes from %s\n",  chromHash->elCount, chromSizes);

/* Load and sort input file. */
bits64 fullSize;
struct ppBed *pb, *pbList = ppBedLoadAll(inName, chromHash, chromHash->lm, &fullSize);
bits64 pbCount = slCount(pbList);
verbose(1, "Read %llu items from %s\n", pbCount, inName);
slSort(&pbList, ppBedCmp);

/* Make chromosomes. */
int chromCount, maxChromNameSize;
struct bigWigChromInfo *chromInfoArray;
makeChromInfo(pbList, chromHash, &chromCount, &chromInfoArray, &maxChromNameSize);
verbose(1, "%d of %d chromosomes used (%4.2f%%)\n", chromCount, chromHash->elCount, 
	100.0*chromCount/chromHash->elCount);

/* Calculate first meaningful reduction size and make first reduction. */
bits32 summaryCount = 0;
int initialReduction = ppBedAverageSize(pbList)*10;
verbose(2, "averageBedSize=%d\n", initialReduction/10);
bits64 lastSummarySize = 0, summarySize;
struct bwgSummary *summaryList, *firstSummaryList;
for (;;)
    {
    summaryList = summaryOnDepth(pbList, chromInfoArray, initialReduction);
    summarySize = bwgTotalSummarySize(summaryList);
    if (summarySize >= fullSize && summarySize != lastSummarySize)
        {
	initialReduction *= 2;
	bwgSummaryFreeList(&summaryList);
	lastSummarySize = summarySize;
	}
    else
        break;
    }
summaryCount = 1;
reduceSummaries[0] = firstSummaryList = summaryList;
reductionAmounts[0] = initialReduction;
verbose(1, "Initial zoom reduction x%d data size %4.2f%%\n", 
	initialReduction, 100.0 * summarySize/fullSize);

/* Now calculate up to 10 levels of further summary. */
bits64 reduction = initialReduction;
for (i=0; i<ArraySize(reduceSummaries)-1; i++)
    {
    reduction *= 10;
    if (reduction > 1000000000)
        break;
    summaryList = bwgReduceSummaryList(reduceSummaries[summaryCount-1], chromInfoArray, 
    	reduction);
    summarySize = bwgTotalSummarySize(summaryList);
    if (summarySize != lastSummarySize)
        {
 	reduceSummaries[summaryCount] = summaryList;
	reductionAmounts[summaryCount] = reduction;
	++summaryCount;
	}
    int summaryItemCount = slCount(summaryList);
    if (summaryItemCount <= chromCount)
        break;
    }


/* Open output file and write out fixed size header, including some dummy values for
 * offsets we'll fill in later. */
FILE *f = mustOpen(outName, "wb");
writeOne(f, sig);
writeOne(f, summaryCount);
chromTreeOffsetPos = ftell(f);
writeOne(f, chromTreeOffset);
dataOffsetPos = ftell(f);
writeOne(f, dataOffset);
indexOffsetPos = ftell(f);
writeOne(f, indexOffset);
for (i=0; i<8; ++i)
    writeOne(f, reserved32);

/* Write summary headers */
for (i=0; i<summaryCount; ++i)
    {
    writeOne(f, reductionAmounts[i]);
    writeOne(f, reserved32);
    reductionDataOffsetPos[i] = ftell(f);
    writeOne(f, reserved64);	// Fill in with data offset later
    writeOne(f, reserved64);	// Fill in with index offset later
    }

/* Write chromosome bPlusTree */
chromTreeOffset = ftell(f);
int chromBlockSize = min(blockSize, chromCount);
bptFileBulkIndexToOpenFile(chromInfoArray, sizeof(chromInfoArray[0]), chromCount, chromBlockSize,
    bigWigChromInfoKey, maxChromNameSize, bigWigChromInfoVal, 
    sizeof(chromInfoArray[0].id) + sizeof(chromInfoArray[0].size), 
    f);

/* Write the main data. */
dataOffset = ftell(f);
writeOne(f, pbCount);
for (pb = pbList; pb != NULL; pb = pb->next)
    {
    pb->fileOffset = ftell(f);
    writeOne(f, pb->chromId);
    writeOne(f, pb->start);
    writeOne(f, pb->end);
    if (pb->rest != NULL)
	{
	int len = 0;
        len = strlen(pb->rest);
	mustWrite(f, pb->rest, len);
	}
    putc(0, f);
    }

/* Write the main index. */
indexOffset = ftell(f);
struct ppBed **pbArray;
AllocArray(pbArray, pbCount);
for (pb = pbList, i=0; pb != NULL; pb = pb->next, ++i)
    pbArray[i] = pb;
cirTreeFileBulkIndexToOpenFile(pbArray, sizeof(pbArray[0]), pbCount,
    blockSize, itemsPerSlot, NULL, ppBedFetchKey, ppBedFetchOffset, 
    indexOffset, f);
freez(&pbArray);

/* Write out summary sections. */
verbose(2, "bigBedFileCreate writing %d summaries\n", summaryCount);
for (i=0; i<summaryCount; ++i)
    {
    reductionDataOffsets[i] = ftell(f);
    reductionIndexOffsets[i] = bwgWriteSummaryAndIndex(reduceSummaries[i], blockSize, itemsPerSlot, f);
    }

/* Go back and fill in offsets properly in header. */
fseek(f, dataOffsetPos, SEEK_SET);
writeOne(f, dataOffset);
fseek(f, indexOffsetPos, SEEK_SET);
writeOne(f, indexOffset);
fseek(f, chromTreeOffsetPos, SEEK_SET);
writeOne(f, chromTreeOffset);

/* Also fill in offsets in zoom headers. */
for (i=0; i<summaryCount; ++i)
    {
    fseek(f, reductionDataOffsetPos[i], SEEK_SET);
    writeOne(f, reductionDataOffsets[i]);
    writeOne(f, reductionIndexOffsets[i]);
    }

carefulClose(&f);
freez(&chromInfoArray);
}

