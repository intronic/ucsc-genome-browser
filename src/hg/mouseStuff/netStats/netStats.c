/* netStats - Gather statistics on net. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "chainNet.h"
#include "localmem.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "netStats - Gather statistics on net\n"
  "usage:\n"
  "   netStats summary.out inNet(s)\n"
  "options:\n"
  "   -gap=gapFile\n"
  "   -fill=fillFile\n"
  "   -top=topFile\n"
  "   -nonSyn=topFile\n"
  "   -syn=synFile\n"
  "   -inv=invFile\n"
  "   -dupe=dupeFile\n"
  );
}

FILE *logFile;

void logIt(char *format, ...)
/* Record something to log and to standard out. */
{
va_list args;
va_start(args, format);
vprintf(format, args);
vfprintf(logFile, format, args);
va_end(args);
}

FILE *optionalFile(char *optionName)
/* Return open file or NULL depending if optional filename
 * exists. */
{
char *name = optionVal(optionName, NULL);
if (name == NULL)
    return NULL;
return mustOpen(name, "w");
}


void (*rtApply)(struct cnFill *fill, int level, FILE *optFile);
/* Function rTraverse below applies. */
FILE *rtOptFile;

void rTraverse(struct cnFill *fillList, int level)
/* Recursively traverse net. */
{
struct cnFill *fill;
for (fill = fillList; fill != NULL; fill = fill->next)
    {
    rtApply(fill, level, rtOptFile);
    if (fill->children)
        rTraverse(fill->children, level+1);
    }
}

void traverseNet(struct chainNet *netList, 
	FILE *optFile,
	void (*apply)(struct cnFill *fill, int level, FILE *optFile))
/* Traverse all nets and apply function. */
{
struct chainNet *net;
rtApply = apply;
rtOptFile = optFile;
for (net = netList; net != NULL; net = net->next)
    rTraverse(net->fillList, 0);
}


int depthMax;

void depthGather(struct cnFill *fill, int level, FILE *f)
/* Gather depth info. */
{
if (depthMax < level)
     depthMax = level;
}


double gapSizeT, gapSizeQ;
int gapCount;

void gapGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on gaps */
{
if (fill->chainId == 0)
    {
    gapCount += 1;
    gapSizeT += fill->tSize;
    gapSizeQ += fill->qSize;
    if (f)
	fprintf(f, "%d\t%d\n", fill->tSize, fill->qSize);
    }
}

struct lm *intLm;	/* Local memory for intLists . */

struct intList
/* A list of ints. */
    {
    struct intList *next;	/* Next in list. */
    int val;			/* Value. */
    };

int intListCmp(const void *va, const void *vb)
/* Compare to sort based on value. */
{
const struct intList *a = *((struct intList **)va);
const struct intList *b = *((struct intList **)vb);
return a->val - b->val;
}

struct fillStats
/* Information on fills. */
    {
    int count;
    struct intList *spanT, *spanQ;	/* Coverage with gaps. */
    struct intList *ali;		/* Coverage no gaps. */
    struct intList *qFar;		/* Total farness. */
    struct intList *qDup;		/* Total dupness. */
    };

struct fillStats fillStats;   /* Stats on all fills. */
struct fillStats topStats;    /* Stats on top fills. */
struct fillStats nonSynStats; /* Stats on nonSyn fills. */
struct fillStats invStats;    /* Stats on inv fills. */
struct fillStats synStats;    /* Stats on fills in same chrom same dir. */
struct fillStats dupeStats;   /* Stats on dupes. */

void addInt(struct intList **pList, int val)
/* Add value to int list. */
{
struct intList *el;
lmAllocVar(intLm, el);
el->val = val;
slAddHead(pList, el);
}

void addFillStats(struct fillStats *stats, struct cnFill *fill, FILE *f)
/* Add info from one fill to stats. */
{
stats->count += 1;
addInt(&stats->spanT, fill->tSize);
addInt(&stats->spanQ, fill->qSize);
addInt(&stats->ali, fill->ali);
if (fill->qDup > 0)
    addInt(&stats->qDup, fill->qDup);
if (fill->qFar > 0)
    addInt(&stats->qFar, fill->qFar);
if (f)
    fprintf(f, "%d\t%d\t%d\t%d\t%d\n", fill->tSize, fill->qSize, fill->ali,
    	fill->qDup, fill->qFar);
}

void fillGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on fill */
{
if (fill->chainId != 0)
    {
    addFillStats(&fillStats, fill, f);
    }
}

void topGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on top level fill */
{
if (fill->type != NULL && sameString(fill->type, "top"))
    {
    addFillStats(&topStats, fill, f);
    }
}

void nonSynGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on non-syntenic fill. */
{
if (fill->type != NULL && sameString(fill->type, "nonSyn"))
    {
    addFillStats(&nonSynStats, fill, f);
    }
}

void invGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on inv level fill */
{
if (fill->type != NULL && sameString(fill->type, "inv"))
    {
    addFillStats(&invStats, fill, f);
    }
}

void dupeGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on things mostly duplicated. */
{
if (fill->qDup > 0 && fill->qDup * 2 >= fill->ali)
    addFillStats(&dupeStats, fill, f);
}

void synGather(struct cnFill *fill, int level, FILE *f)
/* Gather stats on things mostly duplicated. */
{
if (fill->type != NULL && sameString(fill->type, "syn"))
    {
    addFillStats(&synStats, fill, f);
    }
}


void logListStats(struct intList **pList, int count)
/* Write out some stats to log file. */
{
int i, middle = count/2;
struct intList *el;
double total = 0;
int minVal = 0, medVal = 0, maxVal = 0;

if (*pList != NULL)
    {
    slSort(pList, intListCmp);
    minVal = (*pList)->val;
    for (i=0, el = *pList; el != NULL; el = el->next, ++i)
	{
	int val = el->val;
	total += val;
	maxVal = val;
	if (i == middle)
	    medVal = val;
	}
    logIt("ave %3.1f, min %d, max %d, median %d", 
	    total/count, minVal, maxVal, medVal);
    }
logIt("\n");
}

void logFillStats(char *name, struct fillStats *stats)
/* Write out info on stats */
{
logIt("%s count: %d\n", name, stats->count);
logIt("%s span T: ", name);
logListStats(&stats->spanT, stats->count);
logIt("%s span Q: ", name);
logListStats(&stats->spanQ, stats->count);
logIt("%s aligning: ", name);
logListStats(&stats->ali, stats->count);
if (stats->qDup != NULL)
    {
    logIt("%s ave qDup: ", name);
    logListStats(&stats->qDup, stats->count);
    }
if (stats->qFar != NULL)
    {
    logIt("%s ave qFar: ", name);
    logListStats(&stats->qFar, stats->count);
    }
}


void netStats(char *summaryFile, int inCount, char *inFiles[])
/* netStats - Gather statistics on net. */
{
int i;
int netCount = 0;
FILE *gapFile = optionalFile("gap");
FILE *fillFile = optionalFile("fill");
FILE *topFile = optionalFile("top");
FILE *nonSynFile = optionalFile("nonSyn");
FILE *invFile = optionalFile("inv");
FILE *synFile = optionalFile("syn");
FILE *dupeFile = optionalFile("dupe");

intLm = lmInit(0);
logFile = mustOpen(summaryFile, "w");
logIt("net files: %d\n", inCount);
for (i=0; i<inCount; ++i)
    {
    struct lineFile *lf = lineFileOpen(inFiles[i], TRUE);
    struct chainNet *net;
    while ((net = chainNetRead(lf)) != NULL)
	{
	printf("%s\n", net->name);
	++netCount;
	traverseNet(net, NULL, depthGather);
	traverseNet(net, gapFile, gapGather);
	traverseNet(net, fillFile, fillGather);
	traverseNet(net, topFile, topGather);
	traverseNet(net, nonSynFile, nonSynGather);
	traverseNet(net, synFile, synGather);
	traverseNet(net, invFile, invGather);
	traverseNet(net, dupeFile, dupeGather);
	chainNetFree(&net);
	}
    lineFileClose(&lf);
    }

logIt("net chromosomes: %d\n", netCount);
logIt("max depth: %d\n", depthMax);
logIt("gap count: %d\n",  gapCount);
logIt("gap average size T: %4.1f\n", gapSizeT/gapCount);
logIt("gap average size Q: %4.1f\n", gapSizeQ/gapCount);
logFillStats("fill", &fillStats);
logFillStats("top", &topStats);
logFillStats("nonSyn", &nonSynStats);
logFillStats("syn", &synStats);
logFillStats("inv", &invStats);
logFillStats("dupe", &dupeStats);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc < 3)
    usage();
netStats(argv[1], argc-2, argv+2);
return 0;
}
