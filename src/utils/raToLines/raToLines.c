/* raToLines - output ra stanzas as single lines, with pipe separated fields */
#include "common.h"
#include "linefile.h"
#include "options.h"

static char const rcsid[] = "$Id: raToLines.c,v 1.1 2009/01/23 23:29:55 kate Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "raToLines - Output .ra file stanzas as single lines, with pipe-separated fields.\n\n"
  "usage:\n"
  "   raToLines in.ra out.txt\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void raToLines(char *inFile, char *outFile)
/* Output ra stanzas as lines */
{
struct lineFile *lf = lineFileOpen(inFile, TRUE);
FILE *of = fopen(outFile, "w");
char *line, *start;
boolean stanza = FALSE;

while (lineFileNext(lf, &line, NULL))
    {
    start = skipLeadingSpaces(line);
    if (*start == 0)
        {
        fputs("\n", of);
        stanza = FALSE;
        }
    else if (!stanza)
        {
        fputs(line, of);
        if (startsWith("#", start))
            fputc('\n', of);
        else
            stanza = TRUE;
        }
    else if (stanza)
        {
        fputs("|", of);
        fputs(line, of);
        }
    else
        {
        fputs(line, of);
        fputc('\n', of);
        }
    }
fflush(of);
fclose(of);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
raToLines(argv[1], argv[2]);
return 0;
}
