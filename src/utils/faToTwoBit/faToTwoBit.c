/* faToTwoBit - Convert DNA from fasta to 2bit format. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "dnaseq.h"
#include "dnautil.h"
#include "fa.h"
#include "twoBit.h"

static char const rcsid[] = "$Id: faToTwoBit.c,v 1.9 2007/04/24 18:37:46 hiram Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "faToTwoBit - Convert DNA from fasta to 2bit format\n"
  "usage:\n"
  "   faToTwoBit in.fa [in2.fa in3.fa ...] out.2bit\n"
  "options:\n"
  "   -noMask       - Ignore lower-case masking in fa file.\n"
  "   -stripVersion - Strip off version number after . for genbank accessions.\n"
  "   -ignoreDups   - only convert first sequence if there are duplicates\n"
  );
}

boolean noMask = FALSE;
boolean stripVersion = FALSE;
boolean ignoreDups = FALSE;

static struct optionSpec options[] = {
   {"noMask", OPTION_BOOLEAN},
   {"stripVersion", OPTION_BOOLEAN},
   {"ignoreDups", OPTION_BOOLEAN},
   {NULL, 0},
};

static void unknownToN(char *s, int size)
/* Convert non ACGT characters to N. */
{
char c;
int i;
for (i=0; i<size; ++i)
    {
    c = s[i];
    if (ntChars[(int)c] == 0)
        {
	if (isupper(c))
	    s[i] = 'N';
	else
	    s[i] = 'n';
	}
    }
}

	    
void faToTwoBit(char *inFiles[], int inFileCount, char *outFile)
/* Convert inFiles in fasta format to outfile in 2 bit 
 * format. */
{
struct twoBit *twoBitList = NULL, *twoBit;
int i;
struct hash *uniqHash = newHash(18);
FILE *f;

for (i=0; i<inFileCount; ++i)
    {
    char *fileName = inFiles[i];
    struct lineFile *lf = lineFileOpen(fileName, TRUE);
    struct dnaSeq seq;
    ZeroVar(&seq);
    while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
        {
	if (seq.size == 0)
	    {
	    warn("Skipping item %s which has no sequence.\n",seq.name);
	    continue;
	    }
	    
        /* strip off version number */
        if (stripVersion)
            {
            char *sp = NULL;
            sp = strchr(seq.name,'.');
            if (sp != NULL)
                *sp = '\0';
            }

        if (hashLookup(uniqHash, seq.name))
            {
            if (!ignoreDups)
                errAbort("Duplicate sequence name %s", seq.name);
            else
                continue;
            }
	hashAdd(uniqHash, seq.name, NULL);
	if (noMask)
	    faToDna(seq.dna, seq.size);
	else
	    unknownToN(seq.dna, seq.size);
	twoBit = twoBitFromDnaSeq(&seq, !noMask);
	slAddHead(&twoBitList, twoBit);
	}
    lineFileClose(&lf);
    }
slReverse(&twoBitList);
f = mustOpen(outFile, "wb");
twoBitWriteHeader(twoBitList, f);
for (twoBit = twoBitList; twoBit != NULL; twoBit = twoBit->next)
    {
    twoBitWriteOne(twoBit, f);
    }
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc < 3)
    usage();
noMask = optionExists("noMask");
stripVersion = optionExists("stripVersion");
ignoreDups = optionExists("ignoreDups");
dnaUtilOpen();
faToTwoBit(argv+1, argc-2, argv[argc-1]);
return 0;
}
