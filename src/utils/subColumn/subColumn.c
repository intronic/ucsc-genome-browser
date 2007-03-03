/* subColumn - Substitute one column in a tab-separated file.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "dystring.h"
#include "obscure.h"

static char const rcsid[] = "$Id: subColumn.c,v 1.2 2007/03/03 05:09:00 kent Exp $";

boolean isList = FALSE;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "subColumn - Substitute one column in a tab-separated file.\n"
  "usage:\n"
  "   subColumn column in.tab sub.tab out.tab\n"
  "Where:\n"
  "    column is the column number (starting with 1)\n"
  "    in.tab is a tab-separated file\n"
  "    sub.tab is a where first column is old values, second new\n"
  "    out.tab is the substituted output\n"
  "options:\n"
  "   -list - Column is a comma-separated list.  Substitute all elements in list\n"
  );
}

static struct optionSpec options[] = {
   {"list", OPTION_BOOLEAN},
   {NULL, 0},
};

char *subCommaList(struct hash *subHash, char *in)
/* In is a comma-separated list.  Return the list with
 * substitution done.  This routine not reentrant. */
{
static struct dyString *dy;
if (dy == NULL)
    dy = dyStringNew(0);
else
    dyStringClear(dy);
while (in != NULL && in[0] != 0)
    {
    char *e = strchr(in, ',');
    if (e != NULL)
        *e++ = 0;
    char *s = hashMustFindVal(subHash, in);
    dyStringPrintf(dy, "%s,", s);
    in = e;
    }
return dy->string;
}

void subColumn(char *asciiColumn, char *inFile, char *subFile, char *outFile)
/* subColumn - Substitute one column in a tab-separated file.. */
{
struct hash *subHash = hashTwoColumnFile(subFile);
int column = atoi(asciiColumn);
if (column == 0)
    usage();
else
    column -= 1;
char *row[1024*4];
struct lineFile *lf = lineFileOpen(inFile, TRUE);
FILE *f = mustOpen(outFile, "w");
int rowCount;
while ((rowCount = lineFileChopNextTab(lf, row, ArraySize(row))) > 0)
    {
    if (rowCount == ArraySize(row))
        errAbort("Too many columns (%d) line %d of  %s.", rowCount, lf->lineIx, lf->fileName);
    if (column >= rowCount)
        errAbort("Not enough columns (%d) line %d of  %s.", rowCount, lf->lineIx, lf->fileName);
    int i;
    for (i=0; i<rowCount; ++i)
	{
	char *s = row[i];
	if (i == column)
	    {
	    if (isList)
	        {
		s = subCommaList(subHash, s);
		}
	    else
		{
		char *sub = hashFindVal(subHash, s);
		if (sub == NULL)
		    errAbort("%s not in %s line %d of %s", s, subFile, lf->lineIx, lf->fileName);
		s = sub;
		}
	    }
	fputs(s, f);
	if (i == rowCount-1)
	    fputc('\n', f);
	else
	    fputc('\t', f);
	}
    }
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();
isList = optionExists("list");
subColumn(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
