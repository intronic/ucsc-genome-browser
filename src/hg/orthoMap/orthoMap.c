/** orthoMap - Map cDNAs from one genome to another. 
<pre>
Using chains and nets to convert to find ortholgous regions, mrna, exons, bases.

Introduction:
-------------
Chains are alignments that have been chained together to form larger
alignments. Nets are filtering of the chains such that each nucleotide
is covered by at most one chain, which is the "best" chain for that
region. Nets also group the chains in a tree like structure to
organize them such that structures not usually present in standard
alignments, such as inversions and insertions are represented. This
document focuses on using the nets and chains to identify and map
orthologs from one genome to another. Together the nets and chains can
be thought of as nucleotide level syteny map of two genomes (or
versions of the same genome). 

Chains:
-------
A chain is an alignment which is made up of smaller alignments
"chained" (linked) together . Chains are different from standard
alignments in large inserts are tolerated by using a specialized gap
function and a notion of syteny is kept by keeping track of position
in chromosome position of the alignments. For example, assume that we
have aligned the mouse genome to the human genome and get something
like the following (where numbers indicate chromosomes):

Human chr1:   11111111111111111111111111111111111111111111111111111111111
mouse chroms: 555555  222222      2222222222222222 333   333   2222222222
                                      6666666
                                       444444

This would result in the following chains:
Human chr1:   11111111111111111111111111111111111111111111111111111111111
mouse chroms: 555555  222222------2222222222222222-------------2222222222
                                                   333---333
                                      6666666
                                       444444

Nets:
-----
Nets are generated by filtering chains such that each nucleotide is
covered by at most one chain and that chain is the "best" chain for
that region.  Nets also group the chains in a tree structure to
organize them such that structures not usually present in standard
alignments, such as inversions and insertions are represented.

For example, look at the chains from the above example:
Human chr1:   11111111111111111111111111111111111111111111111111111111111
mouse chroms: 555555  222222------2222222222222222-------------2222222222
                                                   333---333
                                      6666666
                                       444444

Visually the net of these chains would look like:
Human chr1:   11111111111111111111111111111111111111111111111111111111111
mouse chroms: 555555  222222------2222222222222222-------------2222222222
                                                   333---333

Where we have lost the chains from chromosomes 6 and 4 as they were
not as large as the chain from chromosme 2. Even if the alignment from
chromsome 4 was slightly better scoring locally than the chromosome 2
alignment, the chromsome 2 alignment is larger overall and more likely
to be correct. The motiviation for prefering the larger blocks of
alignments is that the mouse and human genomes had a common ancestor
and that the most parsimonious explanation of recombination should be
a good reconstruction.

In order to represent insertions and inversions in the alignments of
the two genomes nets have tree structure. The core datatype
representing a chain is a chainNet. A chainNet consists of a list of
cnFill (chain net fill) data types which are each parent node of a
tree representing the filtered chains for a given region of the larger
chainNet. Each cnFill node has a chain associated with it and can have
an arbitrary number of children.  Each child node is also of type
cnFill and their chains reside in the gaps of the chain associated
with the parent cnFill.

Working again from the example above we would have 1 chainNet
with two cnFill's in its chainNet->fill list.

cnFill 1:
     chain = 555555
     children = NULL

cnFill 2: 
     chain = 222222------2222222222222222-------------2222222222
     children = cnFill3
          cnFill 2.1:
               chain = 333---333
	       children = NULL

Mapping using nets and chains:
------------------------------

There are two phases to finding orthologous regions using the nets and chains.

1) Find the appropriate chain. This is done by traversing the
chainNet->fill list of cnFill structures to find the parent cnFill
that covers the region of interest. Once the correct cnFill tree has
been found it must be traveresed to find the chain aligned to the area
of interest (if there is one).

2) Once the chain is found, it is relatively straight forward to find
the corresponding region of the chain alignment.
</pre>
*/
#include "common.h"
#include "hdb.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "chain.h"
#include "axt.h"
#include "axtInfo.h"
#include "chainDb.h"
#include "chainNetDbLoad.h"
#include "altGraphX.h"
#include "psl.h"
#include "genePred.h"
#include "bed.h"
#include "rbTree.h"

static char const rcsid[] = "$Id: orthoMap.c,v 1.14 2005/01/10 00:42:08 kent Exp $";
static boolean doHappyDots;            /* output activity dots? */
static struct rbTree *netTree = NULL;  /* Global red-black tree to store cnfills in for quick searching. */
static char *workingChrom = NULL;      /* Chromosme we are working on. */

static struct optionSpec optionSpecs[] = 
/* Our acceptable options to be called with. */
{
    {"help", OPTION_BOOLEAN},
    {"db", OPTION_STRING},
    {"orthoDb", OPTION_STRING},
    {"chrom", OPTION_STRING},
    {"netTable", OPTION_STRING},
    {"netFile", OPTION_STRING},
    {"chainFile", OPTION_STRING},
    {"itemFile", OPTION_STRING},
    {"itemTable", OPTION_STRING},
    {"itemType", OPTION_STRING},
    {"outputFile", OPTION_STRING},
    {"selectedFile", OPTION_STRING},
    {"cdsErrorFile", OPTION_STRING},
    {NULL, 0}
};

static char *optionDescripts[] = 
/* Description of our options for usage summary. */
{
    "Display this message.",
    "Database (i.e. hg15) that altInFile comes from.",
    "Database (i.e. mm3) to look for orthologous splicing in.",
    "Chromosme in db that we are working on.",
    "Datbase table containing net records, i.e. mouseNet.",
    "File table containing net records, i.e. chr16.net.",
    "File containing the chains. Usually I do this on a chromosome basis.",
    "File containing items to map.",
    "Table containing items to map.",
    "Type of item: {psl, bed, altGraphX, genePred}"
    "File to output mappings to.",
    "Records that are mapped are copied to this file",
    "Output records that are selected to this file.",
    "File for gene predictions with CDS errors."
};

void usage()
/** Print usage and quit. */
{
int i=0;
char *version = cloneString((char*)rcsid);
char *tmp = strstr(version, "orthoMap.c,v ");
if(tmp != NULL)
    version = tmp + 13;
tmp = strrchr(version, 'E');
if(tmp != NULL)
    (*tmp) = '\0';
warn("orthoMap - Map items from one organism to another. Must\n"
     "specify one type of item using the -itemFile or -itemTable\n"
     "flags. OrthoMap simply maps over the genomic coordinates discarding\n"
     "query inserts, mismatches, etc.\n"
     "   (version: %s)", version );
for(i=0; i<ArraySize(optionSpecs) -1; i++)
    fprintf(stderr, "  -%s -- %s\n", optionSpecs[i].name, optionDescripts[i]);
errAbort("\nusage:\n"
	 "   orthoMap -db=mm3 -orthoDb=hg15 -chrom=chr18 -itemFile=psls/chr18.mrna.psl \\\n"
	 "     -itemType=psl -netFile=nets/chr18.net -chainFile=chains/chr18.chain \\\n"
	 "     -outputFile=beds/chr18.mm3.hg15.mrna.bed");
}

int cnFillRangeCmp(void *va, void *vb)
/* Return -1 if a before b,  0 if a and b overlap,
 * and 1 if a after b. */
{
struct cnFill *a = va;
struct cnFill *b = vb;
if (a->tStart + a->tSize <= b->tStart)
    return -1;
else if (b->tStart + b->tSize <= a->tStart)
    return 1;
else
    return 0;
}

struct rbTree *rbTreeFromNetFile(char *fileName)
/* Build an rbTree from a net file */
{
struct rbTree *rbTree = rbTreeNew(cnFillRangeCmp);
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct chainNet *cn = chainNetRead(lf);
struct cnFill *fill = NULL;
for(fill=cn->fillList; fill != NULL; fill = fill->next)
    {
    rbTreeAdd(rbTree, fill);
    }
return rbTree;
}

struct hash *allChainsHash(char *fileName)
/* Create a hash of all the chains in a file by their id. */
{
struct hash *hash = newHash(0);
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct chain *chain;
char chainId[128];

while ((chain = chainRead(lf)) != NULL)
    {
    safef(chainId, sizeof(chainId), "%d", chain->id);
    hashAddUnique(hash, chainId, chain);
    }
lineFileClose(&lf);
return hash;
}

struct chain *chainFromId(int id)
/** Return a chain given the id. */
{
static struct hash *chainHash = NULL;
char key[128];
struct chain *chain = NULL;
if(id == 0)
    return NULL;
if(chainHash == NULL)
    {
    char *chainFile = optionVal("chainFile", NULL);
    if(chainFile == NULL)
	errAbort("orthoSplice::chainFromId() - Can't find file for 'chainFile' parameter");
    chainHash = allChainsHash(chainFile);
    }
safef(key, sizeof(key), "%d", id);
chain =  hashFindVal(chainHash, key);
if(chain == NULL && id != 0)
    warn("Chain not found for id: %d", id);
return chain;
}

void occassionalDot()
/* Write out a dot every 20 times this is called. */
{
static int dotMod = 100;
static int dot = 100;
if (doHappyDots && (--dot <= 0))
    {
    putc('.', stdout);
    fflush(stdout);
    dot = dotMod;
    }
}

struct chain *chainDbLoad(struct sqlConnection *conn, char *track,
			  char *chrom, int id)
/** Load chain. */
{
char table[64];
char query[256];
struct sqlResult *sr;
char **row;
int rowOffset;
struct chain *chain = NULL;

if (!hFindSplitTable(chrom, track, table, &rowOffset))
    errAbort("No %s track in database", track);
snprintf(query, sizeof(query), 
	 "select * from %s where id = %d", table, id);
sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row == NULL)
    errAbort("Can't find %d in %s", id, table);
chain = chainHeadLoad(row + rowOffset);
sqlFreeResult(&sr);
chainDbAddBlocks(chain, track, conn);
return chain;
}

int chainBlockCoverage(struct chain *chain, int start, int end,
		       int* blockStarts, int *blockSizes, int blockCount)
/* Calculate how many of the blocks are in a chain. */
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
	if(cBlock->tStart < blockStart && cBlock->tEnd > blockStart)
	    startFound = TRUE;
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
struct chain *subChain=NULL, *toFree=NULL;
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
    chain = chainFromId(fill->chainId);
    betterChain(chain, start, end, blockStarts, blockSizes, blockCount, bestChain, bestCover);
    for(gap = fill->children; gap != NULL; gap = gap->next)
	{
	chain = chainFromId(gap->chainId);
	betterChain(chain, start, end, blockStarts, blockSizes, blockCount, bestChain, bestCover);
	if(gap->children)
	    {
	    lookForBestChain(gap->children, start, end, 
				       blockStarts, blockSizes, blockCount,
				       bestChain, bestCover);
	    }
	}
    }
}

void chainNetGetRange(char *db, char *netTable, char *chrom,
		      int start, int end, char *extraWhere, struct cnFill **fill,
		      struct chainNet **toFree)
/* Load up the appropriate chainNet list for this range. */
{
struct cnFill *searchHi=NULL, *searchLo = NULL;
struct slRef *refList = NULL, *ref = NULL;

(*fill) = NULL;
(*toFree) = NULL;
if(differentString(workingChrom, chrom))
    return;
if(netTree != NULL)
    {
    AllocVar(searchLo);
    searchLo->tStart = start;
    searchLo->tSize = 0;
    AllocVar(searchHi);
    searchHi->tStart = end;
    searchHi->tSize = 0;
    refList = rbTreeItemsInRange(netTree, searchLo, searchHi);
    for(ref = refList; ref != NULL; ref = ref->next)
	{
	slSafeAddHead(fill, ((struct slList *)ref->val));
	}
    slReverse(fill);
    freez(&searchHi);
    freez(&searchLo);
    (*toFree) = NULL;
    }
else
    {
    struct chainNet *net =chainNetLoadRange(db, netTable, chrom,
					   start, end, NULL);
    if(net != NULL)
	(*fill) = net->fillList;
    else
	(*fill) = NULL;
    (*toFree) = net;
    }
}

struct chain *chainForBlocks(struct sqlConnection *conn, char *db, char *netTable, 
			     char *chrom, int start, int end,
			     int *blockStarts, int *blockSizes, int blockCount)
/** Load the chain in the database for this position from the net track. */
{
char query[256];
struct sqlResult *sr;
char **row;
struct cnFill *fill=NULL;
struct cnFill *gap=NULL;
struct chain *subChain=NULL, *toFree=NULL;
struct chain *chain = NULL;
struct chainNet *net = NULL;
int bestOverlap = 0;
chainNetGetRange(db, netTable, chrom,
		 start, end, NULL, &fill, &net);
if(fill != NULL)
    lookForBestChain(fill, start, end, 
		     blockStarts, blockSizes, blockCount, 
		     &chain, &bestOverlap);
chainNetFreeList(&net);
return chain;
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

void fillInBed(struct chain *chain, struct bed *bed, struct bed **pBed)
/** Fill in an orthoBed structure with initial information for bed. */
{
struct bed *orthoBed = NULL;
int qs, qe;
AllocVar(orthoBed);
orthoBed->chrom = cloneString(chain->qName);
orthoBed->name = cloneString(bed->name);
orthoBed->score = bed->score;
AllocArray(orthoBed->chromStarts, bed->blockCount);
AllocArray(orthoBed->blockSizes, bed->blockCount);
if (orthoBed->score < 0) orthoBed->score = 0;
if(chain->qStrand == '+')
    strncpy(orthoBed->strand,  bed->strand, sizeof(orthoBed->strand));
else
    {
    if(bed->strand[0] == '+')
	strncpy(orthoBed->strand,  "-", sizeof(orthoBed->strand));
    else if(bed->strand[0] == '-')
	strncpy(orthoBed->strand,  "+", sizeof(orthoBed->strand));
    else
	errAbort("Don't recognize strand %s from bed %s", bed->strand, bed->name);
    }
*pBed = orthoBed;
}

void addExonToBedFromBlock(struct chain *chain, struct bed *bed, int blockStart, int blockEnd)
/** Convert block to block in orthologous genome for bed using chain. */
{
struct chain *subChain=NULL, *toFree=NULL;
int qs, qe;
chainSubsetOnT(chain, blockStart,blockEnd , &subChain, &toFree);    
if(subChain == NULL)
    return;
qChainRangePlusStrand(subChain, &qs, &qe);
bed->chromStarts[bed->blockCount] = qs - bed->chromStart;
bed->blockSizes[bed->blockCount] = abs(qe-qs);
bed->blockCount++;
chainFree(&toFree);
}


struct bed *orthoBedFromBed(struct sqlConnection *conn, char *db, char *orthoDb,
			    char *netTable, struct bed *bed)
/** Produce a bed on the orthologous genome from the original bed. */
{
struct bed *orthoBed = NULL;
int i;
struct chain *chain = NULL;
int diff = 0;

if(bed->blockCount == 0)
    {
    /* Spoof bed to have an exon. */
    AllocArray(bed->chromStarts, 1);
    AllocArray(bed->blockSizes, 1);
    bed->blockCount = 1;
    bed->blockSizes[0] = bed->chromEnd - bed->chromStart;
    if(bed->strand[0] != '+' && bed->strand[0] != '-')
	safef(bed->strand, sizeof(bed->strand), "+");
    }

/* Here we go. */
for(i=0; i<bed->blockCount; i++)
    bed->chromStarts[i] += bed->chromStart;
chain = chainForBlocks(conn, db, netTable, 
		       bed->chrom, bed->chromStart, bed->chromEnd,
		       bed->chromStarts, bed->blockSizes, bed->blockCount);
if(chain == NULL) 
    {
    for(i=0; i<bed->blockCount; i++)
	bed->chromStarts[i] += bed->chromStart;
    return NULL;
    }
/* Found chain, continue mapping. */
fillInBed(chain, bed, &orthoBed);
if(chain->qStrand == '+')
    {
    for(i=0; i<bed->blockCount; i++)
	addExonToBedFromBlock(chain, orthoBed, 
			      bed->chromStarts[i], bed->chromStarts[i]+ bed->blockSizes[i] );
    }
else
    {
    for(i=bed->blockCount-1; i>=0; i--)
	addExonToBedFromBlock(chain, orthoBed,
			      bed->chromStarts[i], bed->chromStarts[i]+ bed->blockSizes[i] );
    }
/* Find the range of our mapped bed. */
orthoBed->chromStart = BIGNUM;
orthoBed->chromEnd = 0;
for(i=0; i<orthoBed->blockCount; i++)
    {
    orthoBed->chromStart = orthoBed->thickStart = min(orthoBed->chromStart, orthoBed->chromStarts[i]);
    orthoBed->chromEnd = orthoBed->thickEnd = max(orthoBed->chromEnd, orthoBed->chromStarts[i]+orthoBed->blockSizes[i]);
    }
/* Set the chromstarts etc. */
for(i=0; i<orthoBed->blockCount; i++)
    orthoBed->chromStarts[i] -= orthoBed->chromStart;
for(i=0; i<bed->blockCount; i++)
    bed->chromStarts[i] -= bed->chromStart;
return orthoBed;	
}

struct bed *orthoBedFromPsl(struct sqlConnection *conn, char *db, char *orthoDb,
			    char *netTable, struct psl *psl)
/** Produce a bed on the orthologous genome from the original psl. */
{
struct bed *bed = NULL, *orthoBed = NULL;
int i;
bed = bedFromPsl(psl);
orthoBed = orthoBedFromBed(conn, db, orthoDb, netTable, bed);
bedFree(&bed);
return orthoBed;
}

struct psl *loadPslFromTable(struct sqlConnection *conn, char *table,
			     char *chrom, int chromStart, int chromEnd)
/** Load all of the psls between chromstart and chromEnd */
{
struct sqlResult *sr = NULL;
char **row = NULL;
int rowOffset = -100;
struct psl *pslList = NULL;
struct psl *psl = NULL;
int i=0;
sr = hRangeQuery(conn, table, chrom, chromStart, chromEnd, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    psl = pslLoad(row+rowOffset);
    slSafeAddHead(&pslList, psl);
    }
sqlFreeResult(&sr);
slReverse(&pslList);
return pslList;
}

struct bed *loadBedFromTable(struct sqlConnection *conn, char *table,
			     char *chrom, int chromStart, int chromEnd)
/** Load all of the beds between chromstart and chromEnd */
{
struct sqlResult *sr = NULL;
char **row = NULL;
int rowOffset = -100;
struct bed *bedList = NULL;
struct bed *bed = NULL;
int i=0;
sr = hRangeQuery(conn, table, chrom, chromStart, chromEnd, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    bed = bedLoad(row+rowOffset);
    slSafeAddHead(&bedList, bed);
    }
sqlFreeResult(&sr);
slReverse(&bedList);
return bedList;
}

void altGraphXReverse(struct altGraphX *ag)
/* Change the orientation of an altGraphX record. */
{
int vCount = ag->vertexCount;
int *vPos = ag->vPositions;
int *tmp = NULL;
unsigned char *vTypes = ag->vTypes;
int i;
for(i=0; i<vCount; i++)
    {
    if(ag->vTypes[i] == ggHardEnd)
	ag->vTypes[i] = ggHardStart;
    else if(ag->vTypes[i] ==ggHardStart)
	ag->vTypes[i] = ggHardEnd;
    else if(ag->vTypes[i] == ggSoftEnd)
	ag->vTypes[i] = ggSoftStart;
    else if(ag->vTypes[i] == ggSoftStart)
	ag->vTypes[i] = ggSoftEnd;
    }
tmp = ag->edgeEnds;
ag->edgeEnds = ag->edgeStarts;
ag->edgeStarts = tmp;
if(sameString(ag->strand, "+"))
    snprintf(ag->strand, sizeof(ag->strand), "%s", "-");
else if(sameString(ag->strand, "-"))
    snprintf(ag->strand, sizeof(ag->strand), "%s", "+");
else
    errAbort("orthoMap::altGraphXReverse() - Don't recognize strand: %s", ag->strand);
}

struct altGraphX *mapAltGraphX(struct altGraphX *ag, struct sqlConnection *conn,
			       char *db, char *netTable )
/* Map one altGraphX record. Return NULL if can't find. This function
 is getting a bit long but it isn't easy to do...*/
{
struct altGraphX *agNew = NULL;
struct chain *chain = NULL;
struct chain *workingChain = NULL, *workingChainFree = NULL;
struct chain *subChain = NULL, *toFree = NULL;
int i,j,k;
int edgeCountNew =0;
int vCountNew=0;
bool reverse = FALSE;
int *starts = NULL, *sizes = NULL;
int blockCount =0;

/* Find the best chain (one that overlaps the most exons. */
AllocArray(starts, ag->edgeCount);
AllocArray(sizes, ag->edgeCount);
for(i=0; i<ag->edgeCount; i++)
    {
    if(getSpliceEdgeType(ag, i) == ggExon)
	{
	starts[blockCount] = ag->vPositions[ag->edgeStarts[i]];
	sizes[blockCount] = ag->vPositions[ag->edgeEnds[i]] - ag->vPositions[ag->edgeStarts[i]];
	blockCount++;
	}
    }
chain = chainForBlocks(conn, db, netTable, ag->tName, ag->tStart, ag->tEnd,
		       starts, sizes, blockCount);
freez(&starts);
freez(&sizes);

if(chain == NULL)
    return NULL;
/* Make a smaller chain to work on... */
chainSubsetOnT(chain, ag->tStart-1, ag->tEnd+1, &workingChain, &workingChainFree);
if(workingChain == NULL)
    return NULL;
if ((chain->qStrand == '-'))
    reverse = TRUE;
agNew = altGraphXClone(ag);
freez(&agNew->tName);
agNew->tName = cloneString(chain->qName);
/* Map vertex positions using chain. */
for(i = 0; i < agNew->vertexCount; i++)
    {
    struct cBlock *bi = NULL;
    int targetPos = agNew->vPositions[i];
    struct chain *subChain=NULL, *toFree=NULL;
    agNew->vPositions[i] = -1;
    chainSubsetOnT(workingChain, targetPos , targetPos, &subChain, &toFree);    
    if(subChain != NULL)
	{
	int qs, qe;
	qChainRangePlusStrand(subChain, &qs, &qe);
	agNew->vPositions[i] = qs;
	}
    chainFree(&toFree);
    }
/* Prune out edges not found. */

/* Set up to remember how many edges we have and our start and stop. */
edgeCountNew = agNew->edgeCount;
vCountNew = agNew->vertexCount;
agNew->tStart = BIGNUM;
agNew->tEnd = 0;
for(i=0; i<agNew->vertexCount && i>= 0; i++)
    {
    struct evidence *ev = NULL;
    if(agNew->vPositions[i] == -1)
	{
	/* Adjust positions, overwriting one that isn't found. */
	vCountNew--;
	for(j=i; j<agNew->vertexCount-1; j++)
	    {
	    agNew->vPositions[j] = agNew->vPositions[j+1];
	    agNew->vTypes[j] = agNew->vTypes[j+1];
	    }
	/* Remove edges associated with this vertex. */
	for(j=0; j<agNew->edgeCount && j>=0; j++)
	    {
	    if(agNew->edgeStarts[j] == i || agNew->edgeEnds[j] == i)
		{
		edgeCountNew--;
		/* Remove evidence. */
		ev = slElementFromIx(agNew->evidence, j);
		slRemoveEl(&agNew->evidence, ev);
		for(k=j; k<agNew->edgeCount -1; k++)
		    {
		    agNew->edgeStarts[k] = agNew->edgeStarts[k+1];
		    agNew->edgeEnds[k] = agNew->edgeEnds[k+1];
		    agNew->edgeTypes[k] = agNew->edgeTypes[k+1];
		    }
		j--;
		agNew->edgeCount--;
		}
	    }
	/* Subtract off one vertex from all the others. */
	for(j=0; j<agNew->edgeCount; j++)
	    {
	    if(agNew->edgeStarts[j] > i)
		agNew->edgeStarts[j]--; 
	    if(agNew->edgeEnds[j] > i)
		agNew->edgeEnds[j]--; 
	    }
	i--;
	agNew->vertexCount--;
	}
    /* Else if vertex found set agNew start and ends. */
    else
	{
	agNew->tStart = min(agNew->vPositions[i], agNew->tStart);
	agNew->tEnd = max(agNew->vPositions[i], agNew->tEnd);
	}
    }
/* Not going to worry about mRNAs that aren't used anymore. Leave them in
   for now. */
agNew->vertexCount = vCountNew;
agNew->edgeCount = edgeCountNew;
if(agNew->vertexCount == 0 || agNew->edgeCount == 0)
    {
    altGraphXFree(&agNew);
    return NULL;
    }
for(i=0; i<agNew->edgeCount; i++)
    {
    if(agNew->edgeStarts[i] >= agNew->vertexCount ||
       agNew->edgeEnds[i] >= agNew->vertexCount)
	{
	warn("For %s vertexes occur at %d when in reality there are only %d vertices.",
	     agNew->name, max(agNew->edgeStarts[i], agNew->edgeEnds[i]), agNew->vertexCount);
	}
    }
/* If it is on the other strand reverse it. */
if(reverse)
    {
    altGraphXReverse(agNew);
    }
chainFree(&workingChainFree);
return agNew;
}

void mapAltGraphXFile(struct sqlConnection *conn, char *db, char *orthoDb, char *chrom,
		      char *netTable, char *altGraphXFileName, char *altGraphXTableName,
		      FILE *agxOut, FILE *selectedOut, int *foundCount, int *notFoundCount)
/* Map over altGraphX Structures from one organism to
another. Basically create a mapping for the vertices and then reverse
them if on '-' strand.*/
{
int count =0;
struct bed *bed = NULL;
struct altGraphX *agList = NULL, *ag = NULL, *agNew = NULL;

if(altGraphXFileName != NULL)
    {
    warn("Loading altGraphX Records from file %s.", altGraphXFileName);
    agList = altGraphXLoadAll(altGraphXFileName);
    }
else if(altGraphXTableName != NULL)
    {
    char query[256];
    warn("Reading altGraphX Records from table %s.", altGraphXTableName);
    safef(query, sizeof(query), "select * from %s where tName like '%s'", altGraphXTableName, chrom);
    agList = altGraphXLoadByQuery(conn, query);
    }
else
    errAbort("orthoMap::mapAlGraphXFile() - Need a table name or file name to load altGraphX records");
warn("Mapping altGraphX records.");
for(ag = agList; ag != NULL; ag = ag->next)
    {
    if(differentString(ag->tName, chrom))
	continue;
    occassionalDot();
    agNew = mapAltGraphX(ag, conn, db, netTable);
    if(agNew == NULL)
	(*notFoundCount)++;
    else
	{
	(*foundCount)++;
	altGraphXTabOut(agNew, agxOut);
	altGraphXFree(&agNew);
        if (selectedOut != NULL)
            altGraphXTabOut(ag, selectedOut);
	}
    count++;
    }	
}

void doAltGraphXs(struct sqlConnection *conn, char *db, char *orthoDb, char *chrom,
		  char *netTable, char *altGraphXFileName, char *altGraphXTableName,
		  char *agxOutName, char *selectedFileName, int *foundCount, int *notFoundCount)
/* Map over altGraphX Records. */
{
FILE *agxOut = NULL;
FILE *selectedOut = NULL;
if(agxOutName == NULL)
    errAbort("Must specify altGraphXOut if specifying altGraphXFile. Use -help for help");
agxOut = mustOpen(agxOutName, "w");
if (selectedFileName != NULL)
    selectedOut = mustOpen(selectedFileName, "w");
mapAltGraphXFile(conn, db, orthoDb, chrom, netTable, altGraphXFileName, altGraphXTableName, 
		 agxOut, selectedOut, foundCount, notFoundCount);
carefulClose(&selectedOut);
carefulClose(&agxOut);
}

void doPsls(struct sqlConnection *conn, char *db, char *orthoDb, char *chrom, 
	    char *netTable, char *pslFileName, char *pslTableName,
	    char *outBedName, char *selectedFileName, 
            int *foundCount, int *notFoundCount)
/* Map over psls. */
{
FILE *bedOut = NULL;
FILE *selectedOut = NULL;
struct bed *bed = NULL;
struct psl *psl=NULL, *pslList = NULL;
/* Load psls. */
warn("Loading psls.");
if(pslFileName)
    pslList=pslLoadAll(pslFileName);
else
    pslList=loadPslFromTable(conn, pslTableName, chrom, 0, BIGNUM);
/* Convert psls. */
warn("Converting psls.");
assert(outBedName);
bedOut = mustOpen(outBedName, "w");
if (selectedFileName != NULL)
    selectedOut = mustOpen(selectedFileName, "w");
for(psl = pslList; psl != NULL; psl = psl->next)
    {
    if(differentString(psl->tName, chrom))
	continue;
    occassionalDot();
    bed = orthoBedFromPsl(conn, db, orthoDb, netTable, psl);
    if(bed != NULL && bed->blockCount > 0)
	{
	(*foundCount)++;
	bedTabOutN(bed, 12, bedOut);
        if (selectedOut != NULL)
            pslTabOut(psl, selectedOut);
	}
    else
	(*notFoundCount)++;
    bedFree(&bed);
    }
carefulClose(&selectedOut);
carefulClose(&bedOut);
}

void fillInGene(struct chain *chain, struct genePred *gene, struct genePred **pGene)
/** Fill in syntenic gene structure with initial information for gene. */
{
FILE *cdsErrorFp;
struct genePred *synGene = NULL;
int qs, qe;
struct chain *subChain=NULL, *toFree=NULL;
AllocVar(synGene);
chainSubsetOnT(chain, gene->txStart, gene->txEnd , &subChain, &toFree);    
if(subChain == NULL)
    {
    *pGene= NULL;
    return;
    }
qChainRangePlusStrand(subChain, &qs, &qe);
synGene->chrom = cloneString(subChain->qName);
synGene->name = cloneString(gene->name);
synGene->txStart = qs;
synGene->txEnd = qe;
AllocArray(synGene->exonStarts, gene->exonCount);
AllocArray(synGene->exonEnds, gene->exonCount);
if(chain->qStrand == '+')
    strncpy(synGene->strand,  gene->strand, sizeof(synGene->strand));
else
    {
    if(gene->strand[0] == '+')
	strncpy(synGene->strand,  "-", sizeof(synGene->strand));
    else if(gene->strand[0] == '-')
	strncpy(synGene->strand,  "+", sizeof(synGene->strand));
    else
	errAbort("Don't recognize strand %s from gene %s", gene->strand, gene->name);
    }
chainFree(&toFree);
chainSubsetOnT(chain, gene->cdsStart, gene->cdsEnd , &subChain, &toFree);    
if(subChain == NULL )
    {
    if(optionExists("cdsErrorFile"))
        {
        cdsErrorFp = fopen( optionVal("cdsErrorFile",NULL), "a" );
        fprintf( cdsErrorFp, "%s\t%s\t%u\t%u\t%u\t%u\t%s\t%d\n", gene->name, gene->chrom, gene->txStart, 
		 gene->txEnd, gene->cdsStart, gene->cdsEnd, gene->strand, gene->exonCount );
        fclose(cdsErrorFp);
        }
    *pGene = NULL;
    genePredFree(&synGene); 
    return;
    }
qChainRangePlusStrand(subChain, &qs, &qe);
synGene->cdsStart = qs;
synGene->cdsEnd = qe;
chainFree(&toFree);
*pGene = synGene;
}

void addExonToGene(struct chain *chain, struct genePred *gene, struct genePred *synGene, int block)
/** Converte block in genePred to block in orthologous genome for synGene using chain. */
{
struct chain *subChain=NULL, *toFree=NULL;
int qs, qe;
int end = gene->exonEnds[block];
chainSubsetOnT(chain, gene->exonStarts[block], end , &subChain, &toFree);    
if(subChain == NULL)
    return;
qChainRangePlusStrand(subChain, &qs, &qe);
synGene->exonStarts[synGene->exonCount] = qs; //- synGene->txStart;
synGene->exonEnds[synGene->exonCount] = qe;
synGene->exonCount++;
chainFree(&toFree);
}

struct genePred *orthoBedFromGene(struct sqlConnection *conn, char *db, char *orthoDb,
			    char *netTable, struct genePred *gene)
/** Produce a genePred on the orthologous genome from the original gene. */
{
struct genePred *synGene= NULL;
int i;
unsigned *blockSizes;
struct chain *chain = NULL;
int diff = 0;

AllocArray(blockSizes, gene->exonCount);
for(i=0; i<gene->exonCount; i++)
    blockSizes[i] = gene->exonEnds[i] - gene->exonStarts[i];

chain = chainForBlocks(conn, db, netTable, 
		       gene->chrom, gene->txStart, gene->txEnd,
		       gene->exonStarts, blockSizes, gene->exonCount);
if(chain == NULL)
    return NULL;
fillInGene(chain, gene, &synGene);
if(synGene == NULL)
    return NULL;
if(chain->qStrand == '+')
    {
    for(i=0; i<gene->exonCount; i++)
	{
	addExonToGene(chain, gene, synGene, i);
	}
    }
else
    {
    for(i=gene->exonCount-1; i>=0; i--)
	{
	addExonToGene(chain, gene, synGene, i);
	}
    }
if(synGene->exonCount > 0 && synGene->exonStarts[0] != 0)
    diff = synGene->exonStarts[0];
return synGene;
}

struct genePred *loadGeneFromTable(struct sqlConnection *conn, char *table,
			     char *chrom, int chromStart, int chromEnd)
/** Load all of the genes between chromstart and chromEnd */
{
struct sqlResult *sr = NULL;
char **row = NULL;
int rowOffset = -100;
struct genePred *geneList = NULL;
struct genePred *gene = NULL;
int i=0;
sr = hRangeQuery(conn, table, chrom, chromStart, chromEnd, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    gene = genePredLoad(row+rowOffset);
    slSafeAddHead(&geneList, gene);
    }
sqlFreeResult(&sr);
slReverse(&geneList);
return geneList;
}

void doGenePreds(struct sqlConnection *conn, char *db, char *orthoDb, char *chrom, 
	    char *netTable, char *geneFileName, char *geneTableName,
	    char *outBedName, char *selectedFileName, int *foundCount, int *notFoundCount)	
/* Map over genePreds. */
{
FILE *bedOut = NULL;
FILE *selectedOut = NULL;
FILE *cdsErrorFp = NULL;
struct genePred *gene = NULL, *geneList = NULL;
struct bed *bed = NULL;

//init output files
if(optionExists("cdsErrorFile"))
{
    cdsErrorFp = fopen( optionVal("cdsErrorFile", NULL), "w" );
    fprintf( cdsErrorFp, "#name\tchrom\ttxStart\ttxEnd\tcdsStart\tcdsEnd\tstrand\texonCount\n" );
    fclose(cdsErrorFp);
}

warn("Loading Gene Predictions.");
assert(outBedName);
if(geneFileName)
    geneList=genePredLoadAll(geneFileName);
else
    geneList=loadGeneFromTable(conn, geneTableName, chrom, 0, BIGNUM);
/* Convert genePreds. */
warn("Converting genes.");
bedOut = mustOpen(outBedName, "w");
if (selectedFileName != NULL)
    selectedOut = mustOpen(selectedFileName, "w");
for(gene = geneList; gene != NULL; gene = gene->next)
    {
    struct genePred *synGene = NULL;
    if(differentString(gene->chrom, chrom))
	continue;
    synGene = orthoBedFromGene(conn, db, orthoDb, netTable, gene);
    occassionalDot();
    if(synGene != NULL && synGene->exonCount > 0)
	{
	(*foundCount)++;
	genePredTabOut(synGene, bedOut);
        if (selectedOut != NULL)
            genePredTabOut(gene, selectedOut);
	}
    else
	(*notFoundCount)++;
    genePredFree(&synGene);
    }
carefulClose(&selectedOut);
 carefulClose(&bedOut);
}

void doBeds(struct sqlConnection *conn, char *db, char *orthoDb, char *chrom, 
	    char *netTable, char *bedFileName, char *bedTableName,
	    char *outBedName, char *selectedFileName, int *foundCount, int *notFoundCount)	
/* Map over beds. */
{
FILE *bedOut = NULL;
FILE *selectedOut = NULL;
struct bed *bed=NULL, *bedList = NULL, *orthoBed=NULL;
/* Load beds. */
warn("Loading beds.");
if(bedFileName)
    bedList=bedLoadAll(bedFileName);
else
    bedList=loadBedFromTable(conn, bedTableName, chrom, 0, BIGNUM);
/* Convert beds. */
warn("Converting beds.");
assert(outBedName);
bedOut = mustOpen(outBedName, "w");
if (selectedFileName != NULL)
    selectedOut = mustOpen(selectedFileName, "w");
for(bed = bedList; bed != NULL; bed = bed->next)
    {
    if(differentString(bed->chrom, chrom))
	continue;
    occassionalDot();
    orthoBed = orthoBedFromBed(conn, db, orthoDb, netTable, bed);
    if(orthoBed != NULL && orthoBed->blockCount > 0)
	{
	(*foundCount)++;
	bedTabOutN(orthoBed, 12, bedOut);
        if (selectedOut != NULL)
            bedTabOutN(bed, 12, selectedOut);
	}
    else
	(*notFoundCount)++;
    bedFree(&orthoBed);
    }
bedFreeList(&bedList);
carefulClose(&selectedOut);
carefulClose(&bedOut);
}

void orthoMap()
/** Top level function. Load up the items and transform them to beds. */
{
/* All of the possible input files and tables. */
char *itemTableName = NULL;
char *itemFileName = NULL;
char *itemType = NULL;
char *outputFileName = NULL;
char *selectedFileName = NULL;

/* Parameters common to all mappings. */
char *db = NULL;
char *orthoDb = NULL;
char *chrom = NULL;
char *netTable = NULL;
char *netFile = NULL;

boolean inputOk = TRUE;
boolean needConnection = TRUE;
struct bed *bed = NULL, *bedList = NULL;
struct sqlConnection *conn = NULL;
FILE *bedOut = NULL;
int foundCount=0, notFoundCount=0;

/* Load up our parameters. This is getting a little much to 
   support all the different types we have. */
netFile=optionVal("netFile", NULL);
itemFileName = optionVal("itemFile", NULL);
itemTableName = optionVal("itemTable", NULL);
itemType = optionVal("itemType", NULL);
outputFileName = optionVal("outputFile", NULL);
selectedFileName = optionVal("selectedFile", NULL);
netTable = optionVal("netTable", NULL);
db = optionVal("db", NULL);
orthoDb = optionVal("orthoDb", NULL);
chrom = optionVal("chrom", NULL);

/* Check over our user input. */
if(orthoDb == NULL || db == NULL)
    {
    inputOk = FALSE;
    warn("Must set both db and orthoDb.");
    }
if((netTable == NULL && netFile == NULL) || chrom == NULL)
    {
    inputOk = FALSE;
    warn("Must specify netFile or table and chromsome.");
    }
if(outputFileName == NULL || itemType == NULL)
    {
    inputOk = FALSE;
    warn("Must specify output file and itemType.");
    }
if(itemTableName == NULL)
    { 
    if(netTable == NULL)
	needConnection = FALSE;
    if(itemFileName == NULL)
	{
	inputOk = FALSE;
	warn("Must specify an item table or file using itemFile or itemTable flags.");
	}
    }
if(inputOk == FALSE)
    errAbort("Fatal errors. Try using with -help to see usage");

/* Set up the database and load nets. */
hSetDb(db);
hSetDb2(orthoDb);
workingChrom = chrom;
if(netFile != NULL)
    {
    warn("Loading net info from file %s", netFile);
    netTree = rbTreeFromNetFile(netFile);
    }
if(needConnection)
    conn = hAllocConn();

/* Do some converting. */
if(sameWord("altGraphX", itemType))
    {
    doAltGraphXs(conn, db, orthoDb, chrom, netTable, itemFileName, itemTableName,
		 outputFileName, selectedFileName,&foundCount, &notFoundCount);
    }
else if(sameWord("psl", itemType))
    {
    doPsls(conn, db, orthoDb, chrom, netTable, itemFileName, itemTableName,
	   outputFileName, selectedFileName, &foundCount, &notFoundCount);
    }
else if(sameWord("genePred", itemType))
    {
    doGenePreds(conn, db, orthoDb, chrom, netTable, itemFileName, itemTableName,
		outputFileName, selectedFileName,&foundCount, &notFoundCount);
    }
else if(sameWord("bed", itemType))
    {
    doBeds(conn, db, orthoDb, chrom, netTable, itemFileName, itemTableName,
	   outputFileName, selectedFileName,&foundCount, &notFoundCount);
    }
else
    errAbort("orthoMap::orthoMap() - Don't recognize itemType: %s", itemType);

warn("\n%d of %d found (%.1f%%) when moving from genome %s to genome %s", 
     foundCount, notFoundCount+foundCount, (float)foundCount/(foundCount+notFoundCount)*100, db, orthoDb); 
warn("Done");
}

int main(int argc, char *argv[])
{
if(argc == 1)
    usage();
doHappyDots = isatty(1);  /* stdout */
optionInit(&argc, argv, optionSpecs);
if(optionExists("help"))
    usage();
orthoMap();
return 0;
}

