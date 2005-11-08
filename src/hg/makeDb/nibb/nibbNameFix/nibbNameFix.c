/* nibbNameFix - Regularize format of NIBB sequence names. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "portable.h"
#include "dnaseq.h"
#include "fa.h"
#include "psl.h"

static char const rcsid[] = "$Id: nibbNameFix.c,v 1.1 2005/11/08 19:26:36 kent Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "nibbNameFix - Regularize format of NIBB sequence names\n"
  "usage:\n"
  "   nibbNameFix old.fa old.psl picDir new.fa new.psl estToImage\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void fixName(char *name)
/* Convert NIBB format name into something a bit more regular. */
{
if (startsWith("gi|", name))
    {
    static char *pat = "|gb|";
    static int patSize = 4;
    char *s = stringIn(pat, name);
    if (s != NULL)
        {
	char *e;
	s += 4;
	e = s + strlen(s) - 1;
	if (*e == '|')
	    *e = 0;
	strcpy(name, "nibb");
	strcpy(name+4, s);
	}
    }
else if (startsWith("Contig", name))
    {
    strcpy(name, "nibb");
    strcpy(name+4, name+6);
    }
}


void nibbNameFix(char *oldFa, char *oldPsl, 
	char *newFa, char *newPsl)
/* nibbNameFix - Regularize format of NIBB sequence names. */
{
FILE *fFa = mustOpen(newFa, "w");
FILE *fPsl = mustOpen( newPsl, "w");
struct lineFile *lfFa = lineFileOpen(oldFa, TRUE);
struct lineFile *lfPsl = pslFileOpen(oldPsl);
struct dnaSeq seq;
struct psl *psl;
int imageProbes = 0;
int faCount = 0;

ZeroVar(&seq);
while (faSpeedReadNext(lfFa, &seq.dna, &seq.size, &seq.name))
    {
    fixName(seq.name);
    faWriteNext(fFa, seq.name, seq.dna, seq.size);
    ++faCount;
    }
carefulClose(&fFa);
verbose(1, "Wrote %d to %s\n", faCount, newFa);

while ((psl = pslNext(lfPsl)) != NULL)
    {
    fixName(psl->qName);
    pslTabOut(psl, fPsl);
    pslFree(&psl);
    }
verbose(1, "Wrote %s\n", newPsl);

carefulClose(&fPsl);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();
nibbNameFix(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
