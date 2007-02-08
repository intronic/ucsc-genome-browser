/* txBedToTxg - Create a transcription graph out of a bed-12 file.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "bed.h"
#include "ggMrnaAli.h"
#include "geneGraph.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "txBedToTxg - Create a transcription graph out of a bed-12 file.\n"
  "usage:\n"
  "   txBedToTxg in.bed out.txg\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

struct ggMrnaAli *bedToMa(struct bed *bed)
/* convert psl to format the clusterer likes. */
{
struct ggMrnaAli *ma;
int i;
int blockCount = bed->blockCount;
struct ggMrnaBlock *blocks, *block;
char *strand = bed->strand;
int orientation = (strand[0] == '-' ? -1 : 1); 

AllocVar(ma);
ma->orientation = orientation;
ma->qName = cloneString(bed->name);
ma->qStart = 0;
ma->qEnd = bed->chromEnd - bed->chromStart;
ma->baseCount = ma->qEnd;
ma->milliScore = bed->score;
snprintf(ma->strand, sizeof(ma->strand), "%s", strand);
ma->hasIntrons = (blockCount > 1);
ma->tName = cloneString(bed->chrom);
ma->tStart = bed->chromStart;
ma->tEnd = bed->chromEnd;
ma->blockCount = blockCount;
ma->blocks = AllocArray(blocks, blockCount);

for (i = 0; i<blockCount; ++i)
    {
    int bSize = bed->blockSizes[i];
    int tStart = bed->chromStarts[i] + bed->chromStart;
    int qStart = tStart;
    block = blocks+i;
    block->qStart = qStart;
    block->qEnd = qStart + bSize;
    block->tStart = tStart;
    block->tEnd = tStart + bSize;
    }
return ma;
}

struct ggMrnaAli *bedListToGgMrnaAliList(struct bed *bedList)
/* Copy list of beds to list of ggMrnaAli. */
{
struct bed *bed;
struct ggMrnaAli *maList = NULL;
for (bed = bedList; bed != NULL; bed = bed->next)
    {
    struct ggMrnaAli *ma = bedToMa(bed);
    slAddHead(&maList, ma);
    }
slReverse(&maList);
return maList;
}


void txBedToTxg(char *inBed, char *outGraph)
/* txBedToTxg - Create a transcription graph out of a bed-12 file.. */
{
struct bed *bedList = bedLoadAll(inBed);
FILE *f = mustOpen(outGraph, "w");
verbose(1, "Loaded %d beds from %s\n", slCount(bedList), inBed);
struct ggMrnaAli *maList = bedListToGgMrnaAliList(bedList);
verbose(1, "Created %d ma's\n", slCount(maList));
struct ggMrnaInput *ci = ggMrnaInputFromAlignments(maList, NULL);
struct ggMrnaCluster *mc, *mcList = ggClusterMrna(ci);
verbose(1, "Clustered ok\n");
for (mc = mcList; mc != NULL; mc = mc->next)
    {
    struct geneGraph *gg = ggGraphConsensusCluster(mc, ci, NULL, FALSE);
    struct altGraphX *ag = ggToAltGraphX(gg);
    if (ag != NULL)
        {
	static int id=0;
	char name[16];
	safef(name, sizeof(name), "a%d", ++id);
	freez(&ag->name);
	ag->name = name;
	altGraphXTabOut(ag, f);
	ag->name = NULL;
	}
    }
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
txBedToTxg(argv[1], argv[2]);
return 0;
}
