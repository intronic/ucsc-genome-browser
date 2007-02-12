/* txOrtho - Produce list of shared edges between two transcription graphs in two species. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "memalloc.h"
#include "localmem.h"
#include "options.h"
#include "geneGraph.h"
#include "txGraph.h"
#include "binRange.h"
#include "chain.h"
#include "chainNet.h"
#include "rbTree.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "txOrtho - Produce list of shared edges between two transcription graphs in two species.\n"
  "usage:\n"
  "   txOrtho in.agx in.chain in.net ortho.agx out.edges\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

struct hash *netToBkHash(char *netFile)
/* Read net file into a hash full of binKeepers keyed by chromosome.
 * The binKeepers are full of nets. */
{
struct hash *netHash = hashNew(0);
struct lineFile *lf = lineFileOpen(netFile, TRUE);
struct chainNet *net, *netList = chainNetRead(lf);
for (net = netList; net != NULL; net = net->next)
    {
    if (hashLookup(netHash, net->name))
        errAbort("%s has multiple %s records", netFile, net->name);
    struct binKeeper *bk = binKeeperNew(0, net->size);
    hashAdd(netHash, net->name, bk);
    struct cnFill *fill;
    for(fill=net->fillList; fill != NULL; fill = fill->next)
	binKeeperAdd(bk, fill->tStart, fill->tStart+fill->tSize, fill);
    }
lineFileClose(&lf);
return netHash;                
}

struct hash *allChainsHash(char *fileName)
/* Hash all the chains in a given file by their ids. */
{
struct hash *chainHash = newHash(18);
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct chain *chain;
struct chain *chainList = NULL;
char chainId[20];
while ((chain = chainRead(lf)) != NULL)
    {
    safef(chainId, sizeof(chainId), "%x", chain->id);
    hashAddUnique(chainHash, chainId, chain);
    slAddHead(&chainList, chain);
    }
lineFileClose(&lf);
return chainHash;
}

struct chain *chainFromId(struct hash *chainHash, int id)
/* Given chain ID and hash, return chain. */
{
char chainId[20];
safef(chainId, sizeof(chainId), "%x", id);
return hashMustFindVal(chainHash, chainId);
}

struct minChromSize
/* Associate chromosome and size. */
    {
    char *chrom;	/* Chromosome name, Not alloced here */
    int minSize;		
    };

struct hash *txgChromMinSizeHash(struct txGraph *txgList)
/* Hash full of lower bounds on chromosome sizes, taken
 * from tEnd's in txgList. */
{
struct txGraph *txg;
struct hash *sizeHash = hashNew(16);
for (txg = txgList; txg != NULL; txg = txg->next)
    {
    struct minChromSize *chrom = hashFindVal(sizeHash, txg->tName);
    if (chrom == NULL)
        {
	lmAllocVar(sizeHash->lm, chrom);
	chrom->chrom = txg->tName;
	chrom->minSize = txg->tEnd;
	hashAdd(sizeHash, txg->tName, chrom);
	}
    else
        {
	chrom->minSize = max(chrom->minSize, txg->tEnd);
	}
    }
return sizeHash;
}

struct hash *txgIntoKeeperHash(struct txGraph *txgList)
/* Create a hash full of bin keepers (one for each chromosome or contig.
 * The binKeepers are full of txGraphs. */
{
struct hash *sizeHash = txgChromMinSizeHash(txgList);
struct hash *bkHash = hashNew(16);
struct txGraph *txg;
for (txg = txgList; txg != NULL; txg = txg->next)
    {
    struct binKeeper *bk = hashFindVal(bkHash, txg->tName);
    if (bk == NULL)
        {
	struct minChromSize *chrom = hashMustFindVal(sizeHash, txg->tName);
	verbose(3, "New binKeeper for %s\n", txg->tName);
	bk = binKeeperNew(0, chrom->minSize);
	hashAdd(bkHash, txg->tName, bk);
	}
    binKeeperAdd(bk, txg->tStart, txg->tEnd, txg);
    }
hashFree(&sizeHash);
return bkHash;
}

struct txGraph *agxForCoordinates(char *chrom, int chromStart, int chromEnd, char strand, 
				    struct hash *orthoChromHash)
/* Get list of graphs that cover a particular region. */
{
struct binElement *beList = NULL, *be = NULL;
struct txGraph *agx = NULL, *agxList = NULL;
struct binKeeper *bk = hashFindVal(orthoChromHash, chrom);
if (bk != NULL)
    {
    beList = binKeeperFind(bk, chromStart, chromEnd);
    for(be = beList; be != NULL; be = be->next)
	{
	agx = be->val;
	if(agx->strand[0] == strand)
	    slSafeAddHead(&agxList, agx);
	}
    slReverse(&agxList);
    slFreeList(&beList);
    }
return agxList;
}

void qChainRangePlusStrand(struct chain *chain, int *retQs, int *retQe)
/* Return range of bases covered by chain on q side on the plus
 * strand. */
{
if (chain == NULL)
    errAbort("Can't find range in null query chain.");
if (chain->qStrand == '-')
    {
    *retQs = chain->qSize - chain->qEnd;
    *retQe = chain->qSize - chain->qStart;
    }
else
    {
    *retQs = chain->qStart;
    *retQe = chain->qEnd;
    }
}

void loadOrthoAgxList(struct txGraph *ag, struct chain *chain, struct hash *orthoGraphHash,
				   boolean *revRet, struct txGraph **orthoAgListRet)
/** Return the txGraph records in the orhtologous position on the other genome
    as defined by ag and chain. */
{
int qs = 0, qe = 0;
struct txGraph *orthoAgList = NULL; 
struct chain *subChain = NULL, *toFree = NULL;
boolean reverse = FALSE;
char *strand = NULL;
if(chain != NULL) 
    {
    /* First find the orthologous splicing graph. */
    chainSubsetOnT(chain, ag->tStart, ag->tEnd, &subChain, &toFree);    
    if(subChain != NULL)
	{
	qChainRangePlusStrand(subChain, &qs, &qe);
	if ((subChain->qStrand == '-'))
	    reverse = TRUE;
	if(reverse)
	    { 
	    if(ag->strand[0] == '+')
		strand = "-";
	    else
		strand = "+";
	    }
	else
	    strand = ag->strand;
	orthoAgList = agxForCoordinates(subChain->qName, qs, qe, strand[0], orthoGraphHash);
	chainFreeList(&toFree);
	}
    }
*revRet = reverse;
*orthoAgListRet = orthoAgList;
}
#ifdef SOON 
#endif /* SOON */

int chainBlockCoverage(struct chain *chain, int start, int end,
		       int* blockStarts, int *blockSizes, int blockCount)
/* Calculate how many of the blocks are covered at both block begin and
 * end by a chain. */
{
struct cBlock *cBlock = NULL, *boxInList=NULL;
int blocksCovered = 0;
int i=0;

/* Find the part of the chain of interest to us. */
for(cBlock = chain->blockList; cBlock != NULL; cBlock = cBlock->next)
    {
    if(cBlock->tEnd >= start)
	{
	boxInList = cBlock;
	break;
	}
    }
/* Check to see how many of our exons the boxInList contains covers. 
   For each block check to see if the blockStart and blockEnd are 
   found in the boxInList. */
for(i=0; i<blockCount; i++)
    {
    boolean startFound = FALSE;
    int blockStart = blockStarts[i];
    int blockEnd = blockStarts[i] + blockSizes[i];
    for(cBlock = boxInList; cBlock != NULL && cBlock->tStart < end; cBlock = cBlock->next)
	{
	//    CCCCCC  CCCCC       CCCCCC    CCC  CCCC
	//     BBB   BBBB    BBB BBBBBBBB        BBBB
	//     yes    no     no     no      no   yes?
	// JK - if (cBlock->tStart <= blockStart && cBlock->tEnd >= blockStart) ?
	if(cBlock->tStart < blockStart && cBlock->tEnd > blockStart)
	    startFound = TRUE;
	// JK - if (startFound && cBlock->tStart <= blockEnd && cBlock->tEnd >= blockEnd) ?
	if(startFound && cBlock->tStart < blockEnd && cBlock->tEnd > blockEnd)
	    {
	    blocksCovered++;
	    break;
	    }
	}
    }
return blocksCovered;
}

boolean betterChain(struct chain *chain, int start, int end,
		    int* blockStarts, int *blockSizes, int blockCount,
		    struct chain **bestChain, int *bestCover)
/* Return TRUE if chain is a better fit than bestChain. If TRUE
   fill in bestChain and bestCover. */
{
int blocksCovered = 0;
boolean better = FALSE;
/* Check for easy case. */
if(chain == NULL || chain->tStart > end || chain->tStart + chain->tSize < start)
    return FALSE;
blocksCovered = chainBlockCoverage(chain, start, end, blockStarts, blockSizes, blockCount);
if(blocksCovered > (*bestCover))
    {
    *bestChain = chain;
    *bestCover = blocksCovered;
    better = TRUE;
    }
return better;
}

void lookForBestChain(struct cnFill *list, int start, int end,
		      int* blockStarts, int *blockSizes, int blockCount,
		      struct hash *chainHash, 
		      struct chain **bestChain, int *bestCover)
/* Recursively look for the best chain. Best is defined as the chain
   that covers the most number of blocks found in starts and sizes. This
   will be stored in bestChain and number of blocks that it covers will
   be stored in bestCover. */
{
struct cnFill *fill=NULL;
struct cnFill *gap=NULL;

struct chain *chain = NULL;
for(fill = list; fill != NULL; fill = fill->next)
    {
    chain = chainFromId(chainHash, fill->chainId);
    betterChain(chain, start, end, blockStarts, blockSizes, blockCount, bestChain, bestCover);
    for(gap = fill->children; gap != NULL; gap = gap->next)
	{
	if(gap->children)
	    {
	    lookForBestChain(gap->children, start, end, 
				       blockStarts, blockSizes, blockCount,
				       chainHash, bestChain, bestCover);
	    }
	}
    }
}

struct cnFill *netFillAt(char *chrom, int start, int end, struct hash *netHash)
/* Get list of highest level fill for net at given position. */
{
struct cnFill *fillList = NULL, *fill;
struct binKeeper *bk = hashFindVal(netHash, chrom);
if (bk != NULL)
    {
    struct binElement *beList = NULL, *be = NULL;
    beList = binKeeperFind(bk, start, end);
    for (be = beList; be != NULL; be = be->next)
        {
	fill = be->val;
	slAddHead(&fillList, fill);
	}
    slFreeList(&beList);
    slReverse(&fillList);
    }
return fillList;
}

struct txGraph *orthoGraphsViaChain(struct txGraph *inGraph, struct chain *chain, 
	struct hash *orthoGraphHash)
/* Get a list of orthologous graphs in another species using chain to map.
 * The orthologous graphs are simply those that overlap at the exon level on the
 * same strand. */
{
boolean reverse = FALSE;
struct txGraph *orthoGraphList = NULL;
loadOrthoAgxList(inGraph, chain, orthoGraphHash, &reverse, &orthoGraphList);
return orthoGraphList;
}

struct chain *bestChainForGraph(struct txGraph *inGraph, struct hash *chainHash,
	struct hash *netHash)
/* Find chain that has most block-level overlap with exons in graph. */
{
/* Get appropriate part of nets. */
struct cnFill *fillList = netFillAt(inGraph->tName, 
	inGraph->tStart, inGraph->tEnd, netHash);
if (fillList == NULL)
    return NULL;

/* Create arrays of exon starts and sizes. */
int blockCount = 0;
int *starts, *sizes;
AllocArray(starts, inGraph->edgeCount);
AllocArray(sizes, inGraph->edgeCount);
struct txEdge *edge;
for (edge = inGraph->edges; edge != NULL; edge = edge->next)
    {
    if (edge->type == ggExon)
        {
	int start = starts[blockCount] = inGraph->vertices[edge->startIx].position;
	sizes[blockCount] = inGraph->vertices[edge->endIx].position - start;
	++blockCount;
	}
    }

/* Find chain that most overlaps blocks. */
struct chain *chain = NULL;
int bestOverlap = 0;
lookForBestChain(fillList, inGraph->tStart, inGraph->tEnd, starts, sizes, blockCount,
	chainHash, &chain, &bestOverlap);

freeMem(starts);
freeMem(sizes);
return chain;
}


struct txGraph *findOrthoGraphs(struct txGraph *inGraph, struct chain *chain,
	struct hash *orthoGraphHash)
/* Find list of orthologous graphs if any.  Beware the side effect of tweaking
 * some of the fill->next pointers at the highest level of the net. It's expensive
 * to avoid the side effect, and it doesn't bother subsequent calls to this function. */
{
struct txGraph *orthoGraphList = NULL;
if (chain != NULL)
    {
    verbose(3, "Best chain for %s is %s:%d-%d %c %s:%d-%d\n", inGraph->name,
    	chain->tName, chain->tStart, chain->tEnd, chain->qStrand, 
	chain->qName, chain->qStart, chain->qEnd);
    orthoGraphList = orthoGraphsViaChain(inGraph, chain, orthoGraphHash);
    }
else
    {
    verbose(3, "Couldn't find best chain for %s\n", inGraph->name);
    }
return orthoGraphList;
}

int chainBasesInBlocks(struct chain *chain)
/* Return total number of bases in chain blocks. */
{
int total = 0;
struct cBlock *block;
for (block = chain->blockList; block != NULL; block = block->next)
    total += block->tEnd - block->tStart;
return total;
}


boolean edgeMap(int start, int end, struct chain *chain,
	int *retStart, int *retEnd, boolean *retRev,
	boolean *retStartExact, boolean *retEndExact,
	int *retCoverage)
/* Map edge through chain. Return FALSE if no map. */
{
struct chain *subChain = NULL, *toFree = NULL;
chainSubsetOnT(chain, start, end, &subChain, &toFree);
if (!subChain)
    return FALSE;
*retRev = FALSE;
*retStartExact = *retEndExact = FALSE;
qChainRangePlusStrand(subChain, retStart, retEnd);
if (start == subChain->tStart)
    *retStartExact = TRUE;
if (end == subChain->tEnd)
    *retEndExact = TRUE;
if ((subChain->qStrand == '-'))
    *retRev = TRUE;
*retCoverage = chainBasesInBlocks(subChain);
chainFree(&toFree);
return TRUE;
}

void writeOverlappingEdges(
	enum ggEdgeType edgeType, struct txGraph *inGraph,
	int inStart, int start, enum ggVertexType startType, boolean startMappedExact, 
	int inEnd, int end, enum ggVertexType endType, boolean endMappedExact, 
	struct txGraph *graph, boolean orthoRev, FILE *f)
/* Write edges of graph that overlap correctly to f */
{
if (startType == ggSoftStart || startType == ggSoftEnd || startMappedExact)
    {
    if (endType == ggSoftStart || endType == ggSoftEnd || endMappedExact)
	{
	struct txEdge *edge;
	for (edge = graph->edges; edge != NULL; edge = edge->next)
	    {
	    if (edge->type == edgeType)
		{
		int oStartIx = edge->startIx;
		int oStart = graph->vertices[oStartIx].position;
		// enum ggVertexType oStartType = graph->vTypes[oStartIx];
		int oEndIx = edge->endIx;
		int oEnd = graph->vertices[oEndIx].position;
		// enum ggVertexType oEndType = graph->vTypes[oEndIx];
		int overlap = rangeIntersection(start, end, oStart, oEnd);
		if (overlap > 0)
		    {
		    enum ggVertexType rStartType = startType, rEndType = endType;
		    if (orthoRev)
		        {
			rStartType = endType;
			rEndType = startType;
			}
		    if (rStartType == ggSoftStart || rStartType == ggSoftEnd || start == oStart)
			{
			if (rEndType == ggSoftStart || rEndType == ggSoftEnd || end == oEnd)
			    {
			    fprintf(f, "%s\t%s\t%d\t%d\t%s\t%d\t%d\t%s\t%s\t%d\t%d\n", 
				inGraph->name,
				(edgeType == ggExon ? "exon" : "intron"),
				startType, endType, inGraph->tName, inStart, inEnd,
				graph->name, graph->tName, start, end);
			    }
			}
		    }
		}
	    }
	}
    }
}

void writeCommonEdges(struct txGraph *inGraph, 
	struct chain *chain, struct txGraph *orthoGraph, FILE *f)
{
struct txEdge *edge;
for (edge = inGraph->edges; edge != NULL; edge = edge->next)
    {
    enum ggEdgeType edgeType = edge->type;
    /* Load up end info on exon in other organism. */
    int inStartIx = edge->startIx;
    int inEndIx = edge->endIx;
    int inStart = inGraph->vertices[inStartIx].position;
    int inEnd  = inGraph->vertices[inEndIx].position;
    enum ggVertexType inStartType = inGraph->vertices[inStartIx].type;
    enum ggVertexType inEndType = inGraph->vertices[inEndIx].type;

    int orthoStart, orthoEnd, orthoCoverage;
    boolean orthoRev, orthoStartExact, orthoEndExact;
    if (edgeMap(inStart, inEnd, chain,  &orthoStart, &orthoEnd,
	    &orthoRev, &orthoStartExact, &orthoEndExact, &orthoCoverage))
	{
	writeOverlappingEdges(edgeType, inGraph,
		inStart, orthoStart, inStartType, orthoStartExact, 
		inEnd, orthoEnd, inEndType, orthoEndExact, orthoGraph, orthoRev, f);
	}
    }
}

void writeOrthoEdges(struct txGraph *inGraph, struct hash *chainHash,
	struct hash *netHash, struct hash *orthoGraphHash, FILE *f)
/* Look for orthologous edges, and write any we find. */
{
struct chain *chain = bestChainForGraph(inGraph, chainHash, netHash);
struct txGraph *orthoGraphList = 
	findOrthoGraphs(inGraph, chain, orthoGraphHash);
if (orthoGraphList != NULL)
    {
    struct txGraph *orthoGraph;
    for (orthoGraph = orthoGraphList; orthoGraph != NULL; orthoGraph = orthoGraph->next)
        {
	writeCommonEdges(inGraph, chain, orthoGraph, f);
	}
    verbose(3, "Graph %s maps to %d orthologous graph starting with %s\n", 
    	inGraph->name, slCount(orthoGraphList), orthoGraphList->name);
    }
else
    verbose(4, "No orthologous graph for %s\n", inGraph->name);
}

void txOrtho(char *inAgx, char *inChain, char *inNet, char *orthoAgx, char *outEdges)
/* txOrtho - Produce list of shared edges between two transcription graphs in two species. */
{
/* Load up input and create output file */
struct txGraph *inGraphList = txGraphLoadAll(inAgx);
verbose(1, "Loaded %d input graphs in %s\n", slCount(inGraphList), inAgx);
struct hash *chainHash = allChainsHash(inChain);
verbose(1, "Read %d chains from %s\n", chainHash->elCount, inChain);
struct hash *netHash = netToBkHash(inNet);
verbose(1, "Read %d nets from %s\n", netHash->elCount, inNet);
struct txGraph *orthoGraphList = txGraphLoadAll(orthoAgx);
verbose(1, "Loaded %d ortho graphs in %s\n", slCount(orthoGraphList), orthoAgx);
struct hash *orthoGraphHash = txgIntoKeeperHash(orthoGraphList);
verbose(1, "%d ortho chromosomes/scaffolds\n", orthoGraphHash->elCount);
FILE *f = mustOpen(outEdges, "w");

/* Loop through inGraphList. */
struct txGraph *inGraph;
for (inGraph = inGraphList; inGraph != NULL; inGraph = inGraph->next)
    {
    verbose(2, "Processing %s %s:%d-%d strand %s\n", 
    	inGraph->name, inGraph->tName, inGraph->tStart, inGraph->tEnd,
	inGraph->strand);
    writeOrthoEdges(inGraph, chainHash, netHash, orthoGraphHash, f);
    }
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
pushCarefulMemHandler(1000000000);
optionInit(&argc, argv, options);
if (argc != 6)
    usage();
txOrtho(argv[1], argv[2], argv[3], argv[4], argv[5]);
return 0;
}
