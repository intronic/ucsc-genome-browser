#include "common.h"
#include "hdb.h"
#include "bed.h"
#include "dnaseq.h"
#include "fa.h"
#include "dnautil.h"

void usage()
{
errAbort("mgcFastaForBed - Take a bed file and return a fasta file\n"
	 "with exons uppercase and introns lowercase.\n"
	 "usage:\n"
	 "   mgcFastaForBed db bedName fastaOutName\n");
}

struct dnaSeq *seqFromBed(struct bed *bed)
/* Allocate a dnaSeq and uppercase the parts that are covered by the bed. 
 Free with dnaSeqFree().*/
{
struct dnaSeq *seq = hChromSeq(bed->chrom, bed->chromStart, bed->chromEnd);
int i=0,j=0;
for(i=0;i<bed->blockCount; i++)
    {
    int start = bed->chromStarts[i];
    int end = bed->chromStarts[i] + bed->blockSizes[i];
    for(j=start; j<end; j++)
	seq->dna[j] = toupper(seq->dna[j]);
    }
if(sameWord(bed->strand, "-"))
    reverseComplement(seq->dna, seq->size);
return seq;
}

void mgcFastaForBeds(struct bed *bedList, FILE *out)
/* Convert beds to sequence and output. */
{
struct bed *bed = NULL;
warn("Converting beds");
for(bed = bedList; bed != NULL; bed = bed->next)
    {
    struct dnaSeq *seq = seqFromBed(bed);
    faWriteNext(out, bed->name, seq->dna, seq->size);
    dnaSeqFree(&seq);
    }
}

int main(int argc, char *argv[])
{
char *db = NULL;
struct bed *bedList = NULL;
FILE *out = NULL;
if(argc != 4)
    usage();
hSetDb(argv[1]);
warn("Loading Beds.");
bedList = bedLoadAll(argv[2]);
out = mustOpen(argv[3], "w");
dnaUtilOpen();
mgcFastaForBeds(bedList, out);
warn("Done");
return 0;
}
