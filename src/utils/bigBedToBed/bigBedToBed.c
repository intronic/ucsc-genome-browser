/* bigBedToBed - Convert from bigBed to ascii bed format.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "localmem.h"
#include "bigBed.h"

static char const rcsid[] = "$Id: bigBedToBed.c,v 1.3 2009/02/02 06:05:52 kent Exp $";

char *clChrom = NULL;
int clStart = -1;
int clEnd = -1;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "bigBedToBed - Convert from bigBed to ascii bed format.\n"
  "usage:\n"
  "   bigBedToBed input.bb output.bed\n"
  "options:\n"
  "   -chrom=chr1 - if set restrict output to given chromosome\n"
  "   -start=N - if set, restrict output to only that over start\n"
  "   -end=N - if set, restict output to only that under end\n"
  );
}

static struct optionSpec options[] = {
   {"chrom", OPTION_STRING},
   {"start", OPTION_INT},
   {"end", OPTION_INT},
   {NULL, 0},
};

void bigBedToBed(char *inFile, char *outFile)
/* bigBedToBed - Convert from bigBed to ascii bed format.. */
{
struct bbiFile *bwf = bigBedFileOpen(inFile);
FILE *f = mustOpen(outFile, "w");
struct bbiChromInfo *chrom, *chromList = bbiChromList(bwf);
for (chrom = chromList; chrom != NULL; chrom = chrom->next)
    {
    if (clChrom != NULL && !sameString(clChrom, chrom->name))
        continue;
    char *chromName = chrom->name;
    struct lm *lm = lmInit(0);
    int start = 0, end = chrom->size;
    if (clStart > 0)
        start = clStart;
    if (clEnd > 0)
        end = clEnd;
    struct bigBedInterval *interval, *intervalList = bigBedIntervalQuery(bwf, chromName, 
    	start, end, lm);
    for (interval = intervalList; interval != NULL; interval = interval->next)
	fprintf(f, "%s\t%u\t%u\t%s\n", chromName, interval->start, interval->end, interval->rest);
    lmCleanup(&lm);
    }
bbiChromInfoFreeList(&chromList);
carefulClose(&f);
bbiFileClose(&bwf);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
clChrom = optionVal("chrom", clChrom);
clStart = optionInt("start", clStart);
clEnd = optionInt("end", clEnd);
if (argc != 3)
    usage();
bigBedToBed(argv[1], argv[2]);
return 0;
}
