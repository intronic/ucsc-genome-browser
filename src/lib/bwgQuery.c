/* bwQuery - implements the query side of bigWig.  See bwgInternal.h for definition of file
 * format. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "localmem.h"
#include "options.h"
#include "sig.h"
#include "sqlNum.h"
#include "bPlusTree.h"
#include "cirTree.h"
#include "bwgInternal.h"
#include "bigWig.h"

static char const rcsid[] = "$Id: bwgQuery.c,v 1.3 2009/01/28 23:11:33 kent Exp $";

void bptDumpCallback(void *context, void *key, int keySize, void *val, int valSize)
{
char *keyString = cloneStringZ(key, keySize);
bits32 *pVal = val;
printf("%s:%d:%u\n", keyString, valSize, *pVal);
freeMem(keyString);
}

struct bigWigFile *bigWigFileOpen(char *fileName)
/* Open up big wig file. */
{
struct bigWigFile *bwf;
AllocVar(bwf);
bwf->fileName = cloneString(fileName);
FILE *f = bwf->f = mustOpen(fileName, "rb");

/* Read magic number at head of file and use it to see if we are proper file type, and
 * see if we are byte-swapped. */
bits32 magic;
boolean isSwapped = bwf->isSwapped = FALSE;
mustReadOne(f, magic);
if (magic != bigWigSig)
    {
    magic = byteSwap32(magic);
    isSwapped = TRUE;
    if (magic != bigWigSig)
       errAbort("%s is not a bigWig file", fileName);
    }

/* Read rest of defined bits of header, byte swapping as needed. */
bwf->zoomLevels = readBits32(f, isSwapped);
bwf->chromTreeOffset = readBits64(f, isSwapped);
bwf->unzoomedDataOffset = readBits64(f, isSwapped);
bwf->unzoomedIndexOffset = readBits64(f, isSwapped);

/* Skip over reserved area. */
fseek(f, 32, SEEK_CUR);

/* Read zoom headers. */
int i;
struct bwgZoomLevel *level, *levelList = NULL;
for (i=0; i<bwf->zoomLevels; ++i)
    {
    AllocVar(level);
    level->reductionLevel = readBits32(f, isSwapped);
    level->reserved = readBits32(f, isSwapped);
    level->dataOffset = readBits64(f, isSwapped);
    level->indexOffset = readBits64(f, isSwapped);
    slAddHead(&levelList, level);
    }
slReverse(&levelList);
bwf->levelList = levelList;

#ifdef BADIDEA
/* Make up a zoom level for "unzoomed" */
AllocVar(level);
level->reductionLevel = 1;
level->dataOffset = bwf->unzoomedDataOffset;
level->indexOffset = bwf->unzoomedIndexOffset;
slAddHead(&levelList,  level);
#endif

/* Attach B+ tree of chromosome names and ids. */
bwf->chromBpt =  bptFileAttach(fileName, f);

return bwf;
}

void bigWigFileClose(struct bigWigFile **pBwf)
/* Close down a big wig file. */
{
struct bigWigFile *bwf = *pBwf;
if (bwf != NULL)
    {
    cirTreeFileDetach(&bwf->unzoomedCir);
    slFreeList(&bwf->levelList);
    bptFileDetach(&bwf->chromBpt);
    carefulClose(&bwf->f);
    freeMem(bwf->fileName);
    freez(pBwf);
    }
}

struct bwgSectionHead
/* A header from a bigWig file section */
    {
    bits32 chromId;	/* Chromosome short identifier. */
    bits32 start,end;	/* Range covered. */
    bits32 itemStep;	/* For some section types, the # of bases between items. */
    bits32 itemSpan;	/* For some section types, the # of bases in each item. */
    UBYTE type;		/* Type byte. */
    UBYTE reserved;	/* Always zero for now. */
    bits16 itemCount;	/* Number of items in block. */
    };

void bwgSectionHeadRead(struct bigWigFile *bwf, struct bwgSectionHead *head)
/* Read section header. */
{
FILE *f = bwf->f;
boolean isSwapped = bwf->isSwapped;
head->chromId = readBits32(f, isSwapped);
head->start = readBits32(f, isSwapped);
head->end = readBits32(f, isSwapped);
head->itemStep = readBits32(f, isSwapped);
head->itemSpan = readBits32(f, isSwapped);
head->type = getc(f);
head->reserved = getc(f);
head->itemCount = readBits16(f, isSwapped);
}

#ifdef DEBUG
void bigWigBlockDump(struct bigWigFile *bwf, char *chrom, FILE *out)
/* Print out info on block starting at current position. */
{
boolean isSwapped = bwf->isSwapped;
FILE *f = bwf->f;
struct bwgSectionHead head;
bwgSectionHeadRead(bwf, &head);
bits16 i;
float val;

switch (head.type)
    {
    case bwgTypeBedGraph:
	{
	fprintf(out, "#bedGraph section %s:%u-%u\n",  chrom, head.start, head.end);
	for (i=0; i<head.itemCount; ++i)
	    {
	    bits32 start = readBits32(f, isSwapped);
	    bits32 end = readBits32(f, isSwapped);
	    mustReadOne(f, val);
	    fprintf(out, "%s\t%u\t%u\t%g\n", chrom, start, end, val);
	    }
	break;
	}
    case bwgTypeVariableStep:
	{
	fprintf(out, "variableStep chrom=%s span=%u\n", chrom, head.itemSpan);
	for (i=0; i<head.itemCount; ++i)
	    {
	    bits32 start = readBits32(f, isSwapped);
	    mustReadOne(f, val);
	    fprintf(out, "%u\t%g\n", start+1, val);
	    }
	break;
	}
    case bwgTypeFixedStep:
	{
	fprintf(out, "fixedStep chrom=%s start=%u step=%u span=%u\n", 
		chrom, head.start, head.itemStep, head.itemSpan);
	for (i=0; i<head.itemCount; ++i)
	    {
	    mustReadOne(f, val);
	    fprintf(out, "%g\n", val);
	    }
	break;
	}
    default:
        internalErr();
	break;
    }
}
#endif /* DEBUG */

struct fileOffsetSize *bigWigOverlappingBlocks(struct bigWigFile *bwf, struct cirTreeFile *ctf,
	char *chrom, bits32 start, bits32 end)
/* Fetch list of file blocks that contain items overlapping chromosome range. */
{
struct bwgChromIdSize idSize;
if (!bptFileFind(bwf->chromBpt, chrom, strlen(chrom), &idSize, sizeof(idSize)))
    return NULL;
return cirTreeFindOverlappingBlocks(ctf, idSize.chromId, start, end);
}

void chromNameCallback(void *context, void *key, int keySize, void *val, int valSize)
/* Callback that captures chromInfo from bPlusTree. */
{
struct bigWigChromInfo *info;
struct bwgChromIdSize *idSize = val;
assert(valSize == sizeof(*idSize));
AllocVar(info);
info->name = cloneStringZ(key, keySize);
info->id = idSize->chromId;
info->size = idSize->chromSize;
struct bigWigChromInfo **pList = context;
slAddHead(pList, info);
}

struct bigWigChromInfo *bigWigChromList(struct bigWigFile *bwf)
/* Return list of chromosomes. */
{
struct bigWigChromInfo *list = NULL;
bptFileTraverse(bwf->chromBpt, &list, chromNameCallback);
slReverse(&list);
return list;
}

bits32 bigWigChromSize(struct bigWigFile *bwf, char *chrom)
/* Return chromosome size, or 0 if no such chromosome in file. */
{
struct bwgChromIdSize idSize;
if (!bptFileFind(bwf->chromBpt, chrom, strlen(chrom), &idSize, sizeof(idSize)))
    return 0;
return idSize.chromSize;
}

void bigWigChromInfoFree(struct bigWigChromInfo **pInfo)
/* Free up one chromInfo */
{
struct bigWigChromInfo *info = *pInfo;
if (info != NULL)
    {
    freeMem(info->name);
    freez(pInfo);
    }
}

void bigWigChromInfoFreeList(struct bigWigChromInfo **pList)
/* Free a list of dynamically allocated bigWigChromInfo's */
{
struct bigWigChromInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bigWigChromInfoFree(&el);
    }
*pList = NULL;
}

static void bwgAttachUnzoomedCir(struct bigWigFile *bwf)
/* Make sure unzoomed cir is attached. */
{
if (bwf->unzoomedCir == NULL)
    {
    fseek(bwf->f, bwf->unzoomedIndexOffset, SEEK_SET);
    bwf->unzoomedCir = cirTreeFileAttach(bwf->fileName, bwf->f);
    }
}

struct bigWigInterval *bigWigIntervalQuery(struct bigWigFile *bwf, char *chrom, int start, int end,
	struct lm *lm)
/* Get data for interval.  Return list allocated out of lm. */
{
bwgAttachUnzoomedCir(bwf);
struct bigWigInterval *el, *list = NULL;
struct fileOffsetSize *blockList = bigWigOverlappingBlocks(bwf, bwf->unzoomedCir, 
	chrom, start, end);
struct fileOffsetSize *block;
FILE *f = bwf->f;
boolean isSwapped = bwf->isSwapped;
float val;
int i;
for (block = blockList; block != NULL; block = block->next)
    {
    struct bwgSectionHead head;
    fseek(f, block->offset, SEEK_SET);
    bwgSectionHeadRead(bwf, &head);
    switch (head.type)
	{
	case bwgTypeBedGraph:
	    {
	    for (i=0; i<head.itemCount; ++i)
		{
		bits32 s = readBits32(f, isSwapped);
		bits32 e = readBits32(f, isSwapped);
		mustReadOne(f, val);
		if (s < start) s = start;
		if (e > end) e = end;
		if (s < e)
		    {
		    lmAllocVar(lm, el);
		    el->start = s;
		    el->end = e;
		    el->val = val;
		    slAddHead(&list, el);
		    }
		}
	    break;
	    }
	case bwgTypeVariableStep:
	    {
	    for (i=0; i<head.itemCount; ++i)
		{
		bits32 s = readBits32(f, isSwapped);
		bits32 e = s + head.itemSpan;
		mustReadOne(f, val);
		if (s < start) s = start;
		if (e > end) e = end;
		if (s < e)
		    {
		    lmAllocVar(lm, el);
		    el->start = s;
		    el->end = e;
		    el->val = val;
		    slAddHead(&list, el);
		    }
		}
	    break;
	    }
	case bwgTypeFixedStep:
	    {
	    mustReadOne(f, val);
	    bits32 s = head.start;
	    bits32 e = s + head.itemSpan;
	    for (i=0; i<head.itemCount; ++i)
		{
		mustReadOne(f, val);
		if (s < start) s = start;
		if (e > end) e = end;
		if (s < e)
		    {
		    lmAllocVar(lm, el);
		    el->start = s;
		    el->end = e;
		    el->val = val;
		    slAddHead(&list, el);
		    }
		s += head.itemStep;
		e += head.itemStep;
		}
	    break;
	    }
	default:
	    internalErr();
	    break;
	}
    }
slFreeList(&blockList);
slReverse(&list);
return list;
}

struct bigWigInterval *bigWigChromData(struct bigWigFile *bwf, char *chrom, struct lm *lm)
/* Get all data for a chromosome. The returned list will be allocated in lm. */
{
return bigWigIntervalQuery(bwf, chrom, 0, bigWigChromSize(bwf, chrom), lm);
}

