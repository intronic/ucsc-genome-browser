#include "common.h"
#include "errabort.h"
#include "linefile.h"
#include "hdb.h"
#include "hash.h"
#include "bed.h"
#include "psl.h"
#include "options.h"
#include "phyloTree.h"
#include "element.h"
#include "dystring.h"
#include "math.h"

static char const rcsid[] = "$Id: gapDist.c,v 1.2 2007/10/16 19:13:09 braney Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "gapDist - read in gaps and output distance matrix\n"
  "usage:\n"
  "   gapDist in.gap out.dist\n"
  "arguments:\n"
  "   in.gap         list of gaps\n"
  "   out.dist       distance matrix\n"
  "options:\n"
  "   -chanceSwap=percent   chance that a row will be replaced by another row\n"
  );
}

static struct optionSpec options[] = {
   {"chanceSwap", OPTION_INT},
   {NULL, 0},
};

char *bigWords[10*10*1024];

int chanceSwap = 0;

char **getNameAndNumber(char *inGapName, int *number)
{
struct lineFile *lf = lineFileOpen(inGapName, TRUE);
int wordsRead;
int numRows;
char **species;
char buffer[4096];
int ii;

wordsRead = lineFileChopNext(lf, bigWords, sizeof(bigWords)/sizeof(char *));

if (wordsRead != 2)
    errAbort("expected two words first line of %s\n",inGapName);

if (bigWords[0][0] != '>')
    errAbort("expected '>' first line of %s\n",inGapName);

numRows = atoi(bigWords[1]);
*number = numRows;

species = needMem(numRows * sizeof(char *));

for(ii=0; ii < numRows; ii++)
    {
    wordsRead = lineFileChopNext(lf, bigWords, sizeof(bigWords)/sizeof(char *)) ;
    
    safef(buffer, sizeof buffer, "%s.%d",bigWords[0],ii);
    species[ii] = cloneString(buffer);
    }

return species;
}

void gapDist(char *inGapName, char *outDistName)
{
int number;
char **species = getNameAndNumber(inGapName, &number);
struct lineFile *lf = lineFileOpen(inGapName, TRUE);
int wordsRead;
boolean needAtom = TRUE;
int count = 0;
int *dists;
int *distMatrix;

dists = needMem(number * sizeof(int));
distMatrix = needMem(number * number * sizeof(int));

while( (wordsRead = lineFileChopNext(lf, bigWords, sizeof(bigWords)/sizeof(char *)) ))
    {
    if (wordsRead == 0)
	continue;

    if (needAtom)
	{
	if (bigWords[0][0] != '>')
	    errAbort("expecting '>' on line %d\n",lf->lineIx);
	needAtom = FALSE;
	count = 0;
	}
    else
	{
	if (bigWords[0][0] == '>')
	    errAbort("not expecting '>' on line %d\n",lf->lineIx);
	
	dists[count++] = atoi(bigWords[1]);

	if (count == number)
	    {
	    int ii, jj;

	    needAtom = TRUE;
	    for(ii=0; ii < number; ii++)
		for(jj=0; jj < number; jj++)
		    {
		    int chance = rand() % 100;
		    boolean beDiff = chance <  chanceSwap;

		    if (beDiff || (dists[ii] != dists[jj]))
			distMatrix[ii*number+jj]++;
		    }
	    }
	}
    }

int ii, jj;
FILE *f = mustOpen(outDistName, "w");

/*
for(jj=0; jj < number; jj++)
    {
    double byRand = 1 + 0.3 * ((double)rand() / RAND_MAX) ;
    //printf("byRand %g\n",byRand);

    for(ii=0; ii < number; ii++)
	{
	distMatrix[ii*number+jj] *= byRand;
	distMatrix[jj*number+ii] *= byRand;
	}
    }
    */

fprintf(f, "%d\n",number);
for(ii=0; ii < number; ii++)
    {
    fprintf(f, "%s ",species[ii]);
    for(jj=0; jj < number; jj++)
	{
	fprintf(f, "%d ", distMatrix[ii*number+jj]);
	/*
	if (distMatrix[ii*number+jj] != 0)
	    fprintf(f, "%d ",max - distMatrix[ii*number+jj]);
	else
	    fprintf(f, "%d ",0);
	    */
	}
    fprintf(f, "\n");
    }

}

int main(int argc, char *argv[])
/* Process command line. */
{
srand(getpid());
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
chanceSwap = optionInt("chanceSwap", 0);

gapDist(argv[1],argv[2]);
return 0;
}
