/* chopFaLines - Read in FA file with long lines and rewrite it with shorter lines. */
#include "common.h"
#include "dnaseq.h"
#include "fa.h"

static char const rcsid[] = "$Id: chopFaLines.c,v 1.2 2003/05/06 07:41:05 kate Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "chopFaLines - Read in FA file with long lines and rewrite it with shorter lines\n"
  "usage:\n"
  "   chopFaLines in.fa out.fa\n");
}

void chopFaLines(char *inName, char *outName)
/* chopFaLines - Read in FA file with long lines and rewrite it with shorter lines. */
{
FILE *in = mustOpen(inName, "r");
FILE *out = mustOpen(outName, "w");
char *commentLine;
struct dnaSeq *seq;

while (faReadNext(in, NULL, TRUE, &commentLine, &seq))
    {
    commentLine = trimSpaces(commentLine+1);
    uglyf(">%s\n", commentLine);
    mustWrite(uglyOut, seq->dna, 100);
    uglyf("\n");
    uglyAbort("All for now");
    faWriteNext(out, commentLine, seq->dna, seq->size);
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
if (argc != 3)
    usage();
chopFaLines(argv[1], argv[2]);
return 0;
}
