/* pslUniq - strip out all but first record found */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "psl.h"
#include "obscure.h"

static char const rcsid[] = "$Id: pslUniq.c,v 1.4 2005/08/17 20:56:24 hiram Exp $";

int numAllow = 1;

void usage()
/* Explain usage and exit. */
{
errAbort("usage: pslUniq in.psl out.psl\n"
"   -numAllow=N  how many of each identifier to keep (default 1)\n"
);
}

void pslUniq( char *pslName, char *outName, int numAllow)
{
struct psl *psl;
struct hash *pslHash = newHash(0);
struct hashEl *hel;
FILE *out = mustOpen(outName, "w");
struct lineFile *pslF = pslFileOpen(pslName);

while ((struct psl *)NULL != (psl = pslNext(pslF)))
    {
    if ( (hel = hashLookup(pslHash, psl->qName)) == NULL)
	{
	hashAdd(pslHash, psl->qName, intToPt(1));
	pslTabOut(psl, out); 
	}
    else
	{
	hel->val++;
	if (ptToInt(hel->val) <= numAllow)
	    pslTabOut(psl, out); 
	}

    }
lineFileClose(&pslF);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
numAllow = optionInt("numAllow", 1);
if (argc != 3)
    usage();
pslUniq(argv[1], argv[2], numAllow);
return 0;
}
