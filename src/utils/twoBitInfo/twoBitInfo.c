/* twoBitInfo - get information about sequences in a .2bit file. */
#include "common.h"
#include "options.h"
#include "twoBit.h"

static char const rcsid[] = "$Id: twoBitInfo.c,v 1.3 2005/03/24 13:26:52 baertsch Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "twoBitInfo - get information about sequences in a .2bit file\n"
  "usage:\n"
  "   twoBitInfo input.2bit output.tab\n"
  "options:\n"
  "\n"
  "Output file has the columns::\n"
  "   seqName size\n"
  "\n"
  "In the 2bit file is in the form path:seq or path:seq:start-end: or path:seq1,seq2,seqN...\n"
  "the information is returned only on the requested sequence (start-end\n"
  "is ignored).\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void twoBitInfo(char *inName, char *outName)
/* twoBitInfo - get information about sequences in a .2bit file. */
{
struct twoBitFile *tbf;
FILE *outFile;
char *seqName = NULL;

twoBitParseRange(inName, &inName, &seqName, NULL, NULL);
tbf = twoBitOpen(inName);
outFile = mustOpen(outName, "w");

if (seqName != NULL)
    {
    char *seqArray[1023];
    int i;
    int seqCount = chopString(seqName, ",", seqArray, ArraySize(seqArray));
    for (i = 0 ; i < seqCount ; i++)
        fprintf(outFile, "%s\t%d\n", seqArray[i], twoBitSeqSize(tbf, seqArray[i]));
    }
else
    {
    struct twoBitIndex *index;
    for (index = tbf->indexList; index != NULL; index = index->next)
        fprintf(outFile, "%s\t%d\n", index->name, twoBitSeqSize(tbf, index->name));
    }
twoBitClose(&tbf);
carefulClose(&outFile); 
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
twoBitInfo(argv[1], argv[2]);
return 0;
}
