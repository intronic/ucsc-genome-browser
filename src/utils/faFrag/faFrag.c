/* faFrag - Extract a piece of DNA from a .fa file.. */
#include "common.h"
#include "dnaseq.h"
#include "fa.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "faFrag - Extract a piece of DNA from a .fa file.\n"
  "usage:\n"
  "   faFrag in.fa start end out.fa\n");
}

void faFrag(char *inName, int start, int end, char *outName)
/* faFrag - Extract a piece of DNA from a .fa file.. */
{
struct dnaSeq *seq;
FILE *f;
char name[512];

if (start >= end)
    usage();
seq = faReadAllDna(inName);
if (seq->next != NULL)
    warn("More than one sequence in %s, just using first\n", inName);
if (end > seq->size)
    {
    warn("%s only has %d bases, truncating", seq->name, seq->size);
    end = seq->size;
    if (start >= end)
        errAbort("Sorry, no sequence left after truncating");
    }
sprintf(name, "%s:%d-%d", seq->name, start, end);
faWrite(outName, name, seq->dna + start, end-start);
printf("Wrote %d bases to %s\n", end-start, outName);
}

int main(int argc, char *argv[])
/* Process command line. */
{
if (argc != 5)
    usage();
if (!isdigit(argv[2][0]) || !isdigit(argv[3][0]))
    usage();
faFrag(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4]);
return 0;
}
