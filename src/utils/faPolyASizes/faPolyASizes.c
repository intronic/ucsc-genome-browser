/* faPolyASizes - get poly-A tail sizes*/
#include "common.h"
#include "linefile.h"
#include "dnautil.h"
#include "options.h"
#include "fa.h"

static char const rcsid[] = "$Id: faPolyASizes.c,v 1.1 2005/02/04 01:48:16 markd Exp $";

/* command line options */
static struct optionSpec optionSpecs[] =
{
    {NULL, 0}
};

void usage()
/* Explain usage and exit. */
{
errAbort(
  "faPolyASizes - get poly A sizes\n"
  "usage:\n"
  "   faPolyASizes in.fa out.tab\n"
  "\n"
  "output file has four columns:\n"
  "   id seqSize tailPolyASize headPolyTSize\n"
  "\n"
  "options:\n"
  );
}

void faPolyASizes(char *inFile, char *outFile)
/* faPolyASizes - get poly-A tail sizes*/
{
DNA *seq;
int size;
char *name;
struct lineFile *lf = lineFileOpen(inFile, TRUE);
FILE *f = mustOpen(outFile, "w");

while (faSomeSpeedReadNext(lf, &seq, &size, &name, FALSE))
    fprintf(f, "%s\t%d\t%d\t%d\n", name, size,
            maskTailPolyA(seq, size), maskHeadPolyT(seq, size));
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc != 3 )
    usage();
faPolyASizes(argv[1], argv[2]);
return 0;
}
