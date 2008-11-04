/* bfastMafFix - Fix bfast's broken MAFs.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "obscure.h"
#include "sqlNum.h"
#include "maf.h"

static char const rcsid[] = "$Id: bfastMafFix.c,v 1.1 2008/11/04 20:21:37 kent Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "bfastMafFix - Fix bfast's broken MAFs.\n"
  "usage:\n"
  "   bfastMafFix bfast.maf chrom.sizes fixed.maf\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

struct mafAli *processLineA(char *line, int lineIx, char **retContig)
/* Process a maf line that starts with an 'a' and start building a mafAli around it. */
{
struct hash *varHash = hashVarLine(line, lineIx);
*retContig = hashMustFindVal(varHash, "contig-name");
char *scoreString = hashMustFindVal(varHash, "score");
struct mafAli *maf;
AllocVar(maf);
maf->score = atof(scoreString);
return maf;
}

void addComponent(struct mafAli *maf, char **pContig, struct hash *chromSizeHash,
	struct lineFile *lf, char *line)
/* Add component to maf file. */
{
/* Chop into space delimited fields. */
char *row[7];
int fieldCount = chopByWhite(line, row, ArraySize(row));
if (fieldCount != 6)	/* Already skipped the s word. */
    errAbort("Expecting %d fields got %d line %d of %s", 
    	7, fieldCount+1, lf->lineIx, lf->fileName);

struct mafComp *mc;
AllocVar(mc);
if (maf->components == NULL)
    {
    mc->src = *pContig;
    pContig = NULL;
    if (mc->src == NULL)
        errAbort("No contig-name line %d of %s", lf->lineIx-1, lf->fileName);
    mc->srcSize = hashIntVal(chromSizeHash, mc->src);
    mc->strand = '+';
    mc->start = sqlUnsigned(row[1]);
    mc->size = sqlUnsigned(row[2]);
    mc->text = cloneString(row[5]);
    maf->textSize = strlen(mc->text);
    maf->components = mc;
    }
else
    {
    if (maf->components->next != NULL)
        errAbort("Got three s lines line %d of %s, expected just 2", lf->lineIx, lf->fileName);
    mc->src = cloneString(row[0]+1);
    mc->srcSize = sqlUnsigned(row[4]);
    mc->strand = row[3][0];
    mc->start = 0;
    mc->size = mc->srcSize;
    mc->text = cloneString(row[5]);
    if (strlen(mc->text) != maf->textSize)
       errAbort("text size mismatch between components %d vs %d line %d of %s",
       	maf->textSize, (int)strlen(mc->text), lf->lineIx, lf->fileName);
    maf->components->next = mc;
    }
}

void bfastMafFix(char *input, char *chromSizes, char *output)
/* bfastMafFix - Fix bfast's broken MAFs.. */
{
struct lineFile *lf = lineFileOpen(input, TRUE);
struct hash *chromSizeHash = hashNameIntFile(chromSizes);
FILE *f = mustOpen(output, "w");
mafWriteStart(f, "bfastFixed");
char *line;
struct mafAli *maf = NULL;
struct mafComp *mc = NULL;
char *contig;

while (lineFileNext(lf, &line, NULL))
    {
    line = skipLeadingSpaces(line);
    char c = line[0];
    line += 2;
    switch (c)
        {
	case 0:
	   mafWrite(f, maf);
	   mafAliFree(&maf);
	   break;
	case 'a':
	   maf = processLineA(line, lf->lineIx, &contig);
	   break;
	case 's':
	   addComponent(maf, &contig, chromSizeHash, lf, line);
	   break;
	default:
	   break;
	}
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
bfastMafFix(argv[1], argv[2], argv[3]);
return 0;
}
