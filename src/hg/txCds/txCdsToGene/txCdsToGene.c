/* txCdsToGene - Convert transcript bed and best cdsEvidence to genePred and 
 * protein sequence.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "localmem.h"
#include "dystring.h"
#include "options.h"
#include "fa.h"
#include "bed.h"
#include "dnautil.h"
#include "genePred.h"
#include "rangeTree.h"
#include "cdsEvidence.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "txCdsToGene - Convert transcript bed and best cdsEvidence to genePred and \n"
  "protein sequence.\n"
  "usage:\n"
  "   txCdsToGene in.bed in.fa in.tce out.gtf out.fa\n"
  "Where:\n"
  "   in.bed describes the genome position of transcripts, often from txWalk\n"
  "   in.fa contains the transcript sequence\n"
  "   in.tce is the best cdsEvidence (from txCdsPick) for the transcripts\n"
  "          For noncoding transcripts it need not have a line\n"
  "   out.gtf is the output gene predictions\n"
  "   out.fa is the output protein predictions\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

struct hash *cdsEvidenceReadAllIntoHash(char *fileName)
/* Return hash full of cdsEvidence keyed by transcript name. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct hash *hash = hashNew(18);
char *row[CDSEVIDENCE_NUM_COLS];
while (lineFileRowTab(lf, row))
    {
    struct cdsEvidence *cds = cdsEvidenceLoad(row);
    if (hashLookup(hash, cds->name))
        errAbort("%s duplicated in %s, perhaps you want to run txCdsPick?",
		cds->name, fileName);
    hashAdd(hash, cds->name, cds);
    }
lineFileClose(&lf);
return hash;
}

void flipExonList(struct range **pList, int regionSize)
/* Flip exon list to other strand */
{
struct range *exon;
for (exon = *pList; exon != NULL; exon = exon->next)
    reverseIntRange(&exon->start, &exon->end, regionSize);
slReverse(pList);
}

struct range *bedToExonList(struct bed *bed, struct lm *lm)
/* Convert bed to list of exons, allocated out of lm memory. 
 * These exons will be in coordinates relative to the bed start. */
{
struct range *exon, *exonList = NULL;
int i;
for (i=0; i<bed->blockCount; ++i)
    {
    lmAllocVar(lm, exon);
    exon->start = bed->chromStarts[i];
    exon->end = exon->start + bed->blockSizes[i];
    slAddHead(&exonList, exon);
    }
slReverse(&exonList);
return exonList;
}

struct bed *breakUpBedAtCdsBreaks(struct cdsEvidence *cds, struct bed *bed)
/* Create a new broken-up that excludes part of gene between CDS breaks.  
 * Deals with transcript to genome coordinate mapping including negative
 * strand.  Be afraid, be very afraid! */
{
/* Create range tree covering all breaks.  The coordinates here
 * are transcript coordinates. */
struct rbTree *gapTree = rangeTreeNew();
int bedSize = bed->chromEnd - bed->chromStart;
struct lm *lm = gapTree->lm;	/* Convenient place to allocate memory. */
int i, lastCds = cds->cdsCount-1;
for (i=0; i<lastCds; ++i)
    {
    int gapStart = cds->cdsStarts[i] + cds->cdsSizes[i];
    int gapEnd = cds->cdsStarts[i+1];
    rangeTreeAdd(gapTree, gapStart, gapEnd);
    }

/* Get list of exons in bed, flipped to reverse strand if need be. */
struct range *exon, *exonList = bedToExonList(bed, lm);
if (bed->strand[0] == '-')
    flipExonList(&exonList, bedSize);

/* Go through exon list, mapping each exon to transcript
 * coordinates. Check if exon needs breaking up, and if
 * so do so, as we copy it to new list. */
/* Copy exons to new list, breaking them up if need be. */
struct range *newList = NULL, *nextExon, *newExon;
int txStartPos = 0, txEndPos;
for (exon = exonList; exon != NULL; exon = nextExon)
    {
    txEndPos = txStartPos + exon->end - exon->start;
    nextExon = exon->next;
    struct range *gapList = rangeTreeAllOverlapping(gapTree, txStartPos, txEndPos);
    if (gapList != NULL)
        {
	verbose(3, "Splitting exon because of CDS gap\n");
	uglyf("Splitting exon %d-%d at txPos %d-%d because of gaps\n",
		exon->start, exon->end, txStartPos, txEndPos);

	/* Make up exons from current position up to next gap.  This is a little
	 * complicated by possibly the gap starting before the exon. */
	int exonStart = exon->start;
	int txStart = txStartPos;
	struct range *gap;
	for (gap = gapList; gap != NULL; gap = gap->next)
	    {
	    int txEnd = gap->start;
	    int gapSize = rangeIntersection(gap->start, gap->end, exon->start, exon->end);
	    int exonSize = txEnd - txStart;
	    if (exonSize > 0)
		{
		lmAllocVar(lm, newExon);
		newExon->start = exonStart;
		newExon->end = exonStart + exonSize;
		slAddHead(&newList, newExon);
		}
	    else /* This case happens if gap starts before exon */
	        {
		exonSize = 0;
		}

	    /* Update current position in both transcript and genome space. */
	    exonStart += exonSize + gapSize;
	    txStart += exonSize + gapSize;
	    }

	/* Make up final exon from last gap to end, at least if we don't end in a gap. */
	if (exonStart < exon->end)
	    {
	    lmAllocVar(lm, newExon);
	    newExon->start = exonStart;
	    newExon->end = exon->end;
	    slAddHead(&newList, newExon);
	    }
	}
    else
        {
	/* Easy case where we don't intersect any gaps. */
	slAddHead(&newList, exon);
	}
    txStartPos= txEndPos;
    }
slReverse(&newList);

/* Flip exons back to forward strand if need be */
if (bed->strand[0] == '-')
    flipExonList(&newList, bedSize);

/* Convert exons to bed12 */
struct bed *newBed;
AllocVar(newBed);
newBed->chrom = cloneString(bed->chrom);
newBed->chromStart = newList->start + bed->chromStart;
newBed->chromEnd = newList->end + bed->chromStart;
newBed->name  = cloneString(bed->name);
newBed->score = bed->score;
newBed->strand[0] = bed->strand[0];
newBed->blockCount = slCount(newList);
AllocArray(newBed->blockSizes,  newBed->blockCount);
AllocArray(newBed->chromStarts,  newBed->blockCount);
for (exon = newList, i=0; exon != NULL; exon = exon->next, i++)
    {
    newBed->chromStarts[i] = exon->start;
    newBed->blockSizes[i] = exon->end - exon->start;
    newBed->chromEnd = exon->end + bed->chromStart;
    }

/* Clean up and go home. */
rbTreeFree(&gapTree);
return newBed;
}

void outputGtf(struct cdsEvidence *cds, struct bed *bed, char *geneName, FILE *f)
/* Write out bed in gtf format.  This is complicated mostly by the reverse 
 * strand. */
{
/* Get exons and CDS in bed-relative coordinates. */
struct lm *lm = lmInit(0);
struct range *exon, *exonList = bedToExonList(bed, lm);
int bedSize = bed->chromEnd - bed->chromStart;
int cdsStart = -1, cdsEnd = 0;
char *source = "noncoding";
uglyf("outputGtf %s %d-%d\n", bed->name, bed->chromStart, bed->chromEnd);

if (cds != NULL)
    {
    cdsStart = cds->start;
    cdsEnd = cds->end;
    source = cds->source;
    uglyf("cds %d-%d (in transcript coordinates)\n", cdsStart, cdsEnd);
    }

/* If on the strand of darkness flip coordinates */
if (bed->strand[0] == '-')
    flipExonList(&exonList, bedSize);

/* Loop though and output exons and coding regions. */
int cdsPos = 0;	/* Track position within CDS */
int txPos = 0;	/* Track position within transcript. */
for (exon = exonList; exon != NULL; exon = exon->next)
    {
    int exonStart = exon->start;
    int exonEnd = exon->end;
    int exonSize = exonEnd - exonStart;
    uglyf("exon %d-%d (%d)\n", exonStart, exonEnd, exonSize);
    if (bed->strand[0] == '-')
        reverseIntRange(&exonStart, &exonEnd, bedSize);
    fprintf(f, "%s\t%s\texon\t%d\t%d\t.\t%s\t.\t",
    	bed->chrom, source, exonStart + 1 + bed->chromStart, 
	exonEnd + bed->chromStart, bed->strand);
    fprintf(f, "gene_id \"%s\"; transcript_id \"%s\";\n", geneName, bed->name);
    if (cds != NULL)
	{
	int txStart = txPos;
	int txEnd = txStart + exonSize;
	int txCdsStart = max(txStart, cdsStart);
	int txCdsEnd = min(txEnd, cdsEnd);
	int txCdsSize = txCdsEnd - txCdsStart;
	uglyf("tx %d %d (%d), start/end of cds overlap %d %d (%d)\n", txStart, txEnd, txEnd-txStart, txCdsStart, txCdsEnd, txCdsSize);
	if (txCdsSize > 0)
	    {
	    int offsetInExon = txCdsStart - txStart;
	    int frame = cdsPos%3;
	    int start = exon->start + offsetInExon;
	    int end = start + txCdsSize;
	    uglyf("  cds %d %d frame %d\n", start, end, frame);
	    if (bed->strand[0] == '-')
		reverseIntRange(&start, &end, bedSize);
	    fprintf(f, "%s\t%s\tCDS\t%d\t%d\t.\t%s\t%d\t",
		bed->chrom, source, start + 1 + bed->chromStart, end + bed->chromStart, bed->strand, frame);
	    fprintf(f, "gene_id \"%s\"; transcript_id \"%s\";\n", geneName, bed->name);
	    cdsPos += txCdsSize;
	    }
	}
    txPos += exonSize;
    }

lmCleanup(&lm);
}

void outputProtein(struct cdsEvidence *cds, struct dnaSeq *txSeq, FILE *f)
/* Translate txSeq to protein guided by cds, and output to file.
 * The implementation is a little complicated by checking for internal
 * stop codons and other error conditions. */
{
struct dyString *dy = dyStringNew(4*1024);
int blockIx;
for (blockIx=0; blockIx<cds->cdsCount; ++blockIx)
    {
    DNA *dna = txSeq->dna + cds->cdsStarts[blockIx];
    int rnaSize = cds->cdsSizes[blockIx];
    if (rnaSize%3 != 0)
        {
	errAbort("size of block (%d) not multiple of 3 in %s",
	    rnaSize, cds->name);
	}
    int aaSize = rnaSize/3;
    int i;
    for (i=0; i<aaSize; ++i)
        {
	AA aa = lookupCodon(dna);
	if (aa == 0) aa = '*';
	dyStringAppendC(dy, aa);
	dna += 3;
	}
    }
int lastCharIx = dy->stringSize-1;
if (dy->string[lastCharIx] == '*')
    {
    dy->string[lastCharIx] = 0;
    dy->stringSize = lastCharIx;
    }
char *prematureStop = strchr(dy->string, '*');
if (prematureStop != NULL)
    {
    errAbort("Stop codons in CDS at position %d for %s", 
    	(int)(prematureStop - dy->string), cds->name);
    }
faWriteNext(f, cds->name, dy->string, dy->stringSize);
dyStringFree(&dy);
}

void txCdsToGene(char *txBed, char *txFa, char *txCds, char *outGtf, char *outFa)
/* txCdsToGene - Convert transcript bed and best cdsEvidence to genePred and 
 * protein sequence. */
{
struct hash *txSeqHash = faReadAllIntoHash(txFa, dnaLower);
verbose(2, "Read %d transcript sequences from %s\n", txSeqHash->elCount, txFa);
struct hash *cdsHash = cdsEvidenceReadAllIntoHash(txCds);
verbose(2, "Read %d cdsEvidence from %s\n", cdsHash->elCount, txCds);
struct lineFile *lf = lineFileOpen(txBed, TRUE);
FILE *fGtf = mustOpen(outGtf, "w");
FILE *fFa = mustOpen(outFa, "w");
char *row[12];
while (lineFileRow(lf, row))
    {
    struct bed *bed = bedLoad12(row);
    verbose(2, "processing %s\n", bed->name);
    struct cdsEvidence *cds = hashFindVal(cdsHash, bed->name);
    struct dnaSeq *txSeq = hashFindVal(txSeqHash, bed->name);
    if (txSeq == NULL)
        errAbort("%s is in %s but not %s", bed->name, txBed, txFa);
    if (cds != NULL)
	{
        outputProtein(cds, txSeq, fFa);
	if (cds->cdsCount > 1)
	    {
	    struct bed *newBed = breakUpBedAtCdsBreaks(cds, bed);
	    bedFree(&bed);
	    bed = newBed;
	    }
	}
    char *geneName = cloneString(bed->name);
    chopSuffix(geneName);
    chopSuffix(geneName);
    outputGtf(cds, bed, geneName, fGtf);
    freez(&geneName);
    bedFree(&bed);
    }
lineFileClose(&lf);
carefulClose(&fFa);
carefulClose(&fGtf);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
dnaUtilOpen();
if (argc != 6)
    usage();
txCdsToGene(argv[1], argv[2], argv[3], argv[4], argv[5]);
return 0;
}
