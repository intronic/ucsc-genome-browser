/* nibSize - print out nib sizes */
#include "common.h"
#include "nib.h"
#include "options.h"

static char const rcsid[] = "$Id: nibSize.c,v 1.1 2004/02/10 22:21:49 markd Exp $";

static struct optionSpec optionSpecs[] = {
    {NULL, 0}
};
void usage()
/* Explain usage and exit. */
{
errAbort(
  "nibSize - print size of nibs\n"
  "usage:\n"
  "   nibSize nib1 [...]\n"
  );
}

void printNibSize(char* nibFile)
/* print the size of a nib */
{
FILE* fh;
int size;
char name[128];

nibOpenVerify(nibFile, &fh, &size);
splitPath(nibFile, NULL, name, NULL);
printf("%s\t%s\t%d\n", nibFile, name, size);

carefulClose(&fh);
}

void nibSize(int numNibs, char** nibFiles)
/* nibSize - print nib sizes */
{
int i;
for (i = 0; i < numNibs; i++)
    printNibSize(nibFiles[i]);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc < 2)
    usage();
nibSize(argc-1, argv+1);
return 0;
}
