/* pslCheck - validate PSL files. */
#include "common.h"
#include "options.h"
#include "portable.h"
#include "psl.h"

static char const rcsid[] = "$Id: pslCheck.c,v 1.3 2003/11/19 18:20:07 markd Exp $";

/* global count of errors */
int errCount = 0;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "pslCheck - validate PSL files\n"
  "usage:\n"
  "   pslCheck file(s)\n"
  "options:\n"
  "\n");
}

void checkPslFile(char *fileName)
/* Check one .psl file */
{
char pslDesc[PATH_LEN+64];
struct lineFile *lf = pslFileOpen(fileName);
struct psl *psl;

while ((psl = pslNext(lf)) != NULL)
    {
    safef(pslDesc, sizeof(pslDesc), "%s:%u", fileName, lf->lineIx);
    errCount += pslCheck(pslDesc, stderr, psl);
    pslFree(&psl);
    }
lineFileClose(&lf);
}

void checkPsls(int fileCount, char *fileNames[])
/* checkPsls - check files. */
{
int i;

for (i=0; i<fileCount; ++i)
    checkPslFile(fileNames[i]);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc < 2)
    usage();
checkPsls(argc-1, argv+1);
return ((errCount == 0) ? 0 : 1);
}
