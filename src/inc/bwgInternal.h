/* bwgInternal - stuff to create and use bigWig files.  Generally you'll want to use the
 * simpler interfaces in the bigWig module instead.  This file is good reading though
 * if you want to extend the bigWig interface, or work with bigWig files directly
 * without going through the Kent library. */

#ifndef BIGWIGFILE_H
#define BIGWIGFILE_H

/* bigWig file structure:
 *     fixedWidthHeader
 *         magic# 		4 bytes
 *	   zoomLevels		4 bytes
 *         chromosomeTreeOffset	8 bytes
 *         fullDataOffset	8 bytes
 *	   fullIndexOffset	8 bytes
 *         reserved            32 bytes
 *     zoomHeaders		there are zoomLevels number of these
 *         reductionLevel	4 bytes
 *	   reserved		4 bytes
 *	   dataOffset		8 bytes
 *         indexOffset          8 bytes
 *     chromosome b+ tree       bPlusTree index
 *     full data
 *         sectionCount		4 bytes
 *         section data		section count sections, of three types
 *     full index               ciTree index
 *     zoom info             one of these for each zoom level
 *         zoom data
 *             zoomCount	4 bytes
 *             zoom data	there are zoomCount of these items
 *                 chromId	4 bytes
 *	           chromStart	4 bytes
 *                 chromEnd     4 bytes
 *                 validCount	4 bytes
 *                 minVal       4 bytes float 
 *                 maxVal       4 bytes float
 *                 sumData      4 bytes float
 *                 sumSquares   4 bytes float
 *         zoom index        	ciTree index
 */

struct bwgZoomLevel
/* A zoom level in bigWig file. */
    {
    struct bwgZoomLevel *next;		/* Next in list. */
    bits32 reductionLevel;		/* How many bases per item */
    bits32 reserved;			/* Zero for now. */
    bits64 dataOffset;			/* Offset of data for this level in file. */
    bits64 indexOffset;			/* Offset of index for this level in file. */
    };

struct bigWigFile 
/* An open bigWigFile */
    {
    struct bigWigFile *next;	/* Next in list. */
    char *fileName;		/* Name of file - for better error reporting. */
    FILE *f;			/* Open file handle. */
    bits32 typeSig;		/* bigBedSig or bigWigSig for now. */
    boolean isSwapped;		/* If TRUE need to byte swap everything. */
    struct bptFile *chromBpt;	/* Index of chromosomes. */
    bits32 zoomLevels;		/* Number of zoom levels. */
    bits64 chromTreeOffset;	/* Offset to chromosome index. */
    bits64 unzoomedDataOffset;	/* Start of unzoomed data. */
    bits64 unzoomedIndexOffset;	/* Start of unzoomed index. */
    struct cirTreeFile *unzoomedCir;	/* Unzoomed data index in memory - may be NULL. */
    struct bwgZoomLevel *levelList;	/* List of zoom levels. */
    };

enum bwgSectionType 
/* Code to indicate section type. */
    {
    bwgTypeBedGraph=1,
    bwgTypeVariableStep=2,
    bwgTypeFixedStep=3,
    };

struct bwgBedGraphItem
/* An bedGraph-type item in a bwgSection. */
    {
    struct bwgBedGraphItem *next;	/* Next in list. */
    bits32 start,end;		/* Range of chromosome covered. */
    float val;			/* Value. */
    };

struct bwgVariableStepItem
/* An variableStep type item in a bwgSection. */
    {
    struct bwgVariableStepItem *next;	/* Next in list. */
    bits32 start;		/* Start position in chromosome. */
    float val;			/* Value. */
    };

struct bwgFixedStepItem
/* An fixedStep type item in a bwgSection. */
    {
    struct bwgFixedStepItem *next;	/* Next in list. */
    float val;			/* Value. */
    };

union bwgItem
/* Union of item pointers for all possible section types. */
    {
    struct bwgBedGraphItem *bedGraph;
    struct bwgVariableStepItem *variableStep;
    struct bwgFixedStepItem *fixedStep;
    };

struct bwgSection
/* A section of a bigWig file - all on same chrom.  This is a somewhat fat data
 * structure used by the bigWig creation code.  See also bwgSection for the
 * structure returned by the bigWig reading code. */
    {
    struct bwgSection *next;		/* Next in list. */
    char *chrom;			/* Chromosome name. */
    bits32 start,end;			/* Range of chromosome covered. */
    enum bwgSectionType type;
    union bwgItem itemList;		/* List of items in this section. */
    bits32 itemStep;			/* Step within item if applicable. */
    bits32 itemSpan;			/* Item span if applicable. */
    bits16 itemCount;			/* Number of items in section. */
    bits32 chromId;			/* Unique small integer value for chromosome. */
    bits64 fileOffset;			/* Offset of section in file. */
    };

struct bwgSectionOnDisk
/* The part of bwgSection that ends up on disk - in the same order as written on disk. */
    {
    bits32 chromId;			/* Unique small integer value for chromosome. */
    bits32 start,end;			/* Range of chromosome covered. */
    bits32 itemStep;			/* Step within item if applicable. */
    bits32 itemSpan;			/* Item span if applicable. */
    UBYTE type;				/* Item type */
    UBYTE reserved;			/* Reserved for future use. */
    bits16 itemCount;			/* Number of items in section. */
    };

struct bwgSection *bwgParseWig(char *fileName, int maxSectionSize, struct lm *lm);
/* Parse out ascii wig file - allocating memory in lm. */

struct bwgSummary
/* A summary type item. */
    {
    struct bwgSummary *next;
    bits32 chromId;		/* ID of associated chromosome. */
    bits32 start,end;		/* Range of chromosome covered. */
    bits32 validCount;		/* Count of (bases) with actual data. */
    float minVal;		/* Minimum value of items */
    float maxVal;		/* Maximum value of items */
    float sumData;		/* sum of values for each base. */
    float sumSquares;		/* sum of squares for each base. */
    bits64 fileOffset;		/* Offset of summary in file. */
    };

struct bwgSummaryOnDisk
/* The part of the summary that ends up on disk - in the same order written to disk. */
    {
    bits32 chromId;		/* ID of associated chromosome. */
    bits32 start,end;		/* Range of chromosome covered. */
    bits32 validCount;		/* Count of (bases) with actual data. */
    float minVal;		/* Minimum value of items */
    float maxVal;		/* Maximum value of items */
    float sumData;		/* sum of values for each base. */
    float sumSquares;		/* sum of squares for each base. */
    };

struct bwgChromIdSize
/* We store an id/size pair in chromBpt bPlusTree */
    {
    bits32 chromId;	/* Chromosome ID */
    bits32 chromSize;	/* Chromosome Size */
    };

struct bigWigChromInfo;		/* Declared in bigWig.h. */

struct bwgZoomLevel *bwgBestZoom(struct bwgZoomLevel *levelList, int desiredReduction);
/* Return zoom level that is the closest one that is less than or equal to 
 * desiredReduction. */

void bwgDumpSummary(struct bwgSummary *sum, FILE *f);
/* Write out summary info to file. */

struct hash *bwgChromSizesFromFile(char *fileName);
/* Read two column file into hash keyed by chrom. (Here mostly for use
 * with bwgMakeChromInfo.) */

void bigWigChromInfoKey(const void *va, char *keyBuf);
/* Get key field out of bigWigChromInfo. */

void *bigWigChromInfoVal(const void *va);
/* Get val field out of bigWigChromInfo. */

int bwgAverageResolution(struct bwgSection *sectionList);
/* Return the average resolution seen in sectionList. */

void bwgAttachUnzoomedCir(struct bigWigFile *bwf);
/* Make sure unzoomed cir is attached. */

void bwgAddRangeToSummary(bits32 chromId, bits32 chromSize, bits32 start, bits32 end, 
	float val, int reduction, struct bwgSummary **pOutList);
/* Add chromosome range to summary - putting it onto top of list if possible, otherwise
 * expanding list. */

bits64 bwgTotalSummarySize(struct bwgSummary *list);
/* Return size on disk of all summaries. */

struct bwgSummary *bwgReduceSectionList(struct bwgSection *sectionList, 
	struct bigWigChromInfo *chromInfoArray, int reduction);
/* Reduce section by given amount. */

struct bwgSummary *bwgReduceSummaryList(struct bwgSummary *inList, 
	struct bigWigChromInfo *chromInfoArray, int reduction);
/* Reduce summary list to another summary list. */

bits64 bwgWriteSummaryAndIndex(struct bwgSummary *summaryList, 
	int blockSize, int itemsPerSlot, FILE *f);
/* Write out summary and index to summary, returning start position of
 * summary index. */

#define bwgSummaryFreeList slFreeList

struct fileOffsetSize *bigWigOverlappingBlocks(struct bigWigFile *bwf, struct cirTreeFile *ctf,
	char *chrom, bits32 start, bits32 end);
 

struct bigWigFile *bigWigFileOpen(char *fileName);
/* Open up a big wig file. */

void bigWigFileClose(struct bigWigFile **pBwf);
/* Close down a big wig file. */


#endif /* BIGWIGFILE_H */
