/* ave - Compute average and basic stats. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"

static char const rcsid[] = "$Id: ave.c,v 1.6 2005/01/25 00:45:04 kent Exp $";

int col = 1;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "ave - Compute average and basic stats\n"
  "usage:\n"
  "   ave file\n"
  "options:\n"
  "   -col=N Which column to use.  Default 1\n"
  );
}

int cmpDouble(const void *va, const void *vb)
/* Compare two slNames. */
{
const double *a = va;
const double *b = vb;
double diff = *a - *b;
if (diff < 0)
   return -1;
else if (diff > 0)
   return 1;
else
   return 0;
}

void showStats(double *array, int count)
/* Compute stats on sorted array */
{
double val, minVal = 9.9E999, maxVal = -9.9E999;
double total = 0, average;
int i;
double oneVar, totalVar = 0;

for (i=0; i<count; ++i)
    {
    val = array[i];
    if (minVal > val) minVal = val;
    if (maxVal < val) maxVal = val;
    total += val;
    }
average = total/count;

printf("median %f\n", array[count/2]);
printf("average %f\n", average);
printf("min %f\n", minVal);
printf("max %f\n", maxVal);
printf("count %d\n", count);
printf("total %f\n", total);

for (i=0; i<count; ++i)
    {
    val = array[i];
    oneVar = (average-val);
    totalVar += oneVar*oneVar;
    }
printf("standard deviation %f\n", sqrt(totalVar/count));
}

void ave(char *fileName)
/* ave - Compute average and basic stats. */
{
int count = 0, alloc = 1024;
double *array;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *words[128], *word;
int wordCount;
int wordIx = col-1;
int num;

AllocArray(array, alloc);
while ((wordCount = lineFileChop(lf, words)) > 0)
    {
    if (count >= alloc)
        {
	alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    word = words[wordIx];
    if (word[0] == '-' || isdigit(word[0]))
        {
	array[count++] = atof(word);
	}
    }
if (count == 0)
    errAbort("No numerical data column %d of %s", col, fileName);
qsort(array, count, sizeof(array[0]), cmpDouble);
showStats(array, count);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 2)
    usage();
col = optionInt("col", col);
ave(argv[1]);
return 0;
}
