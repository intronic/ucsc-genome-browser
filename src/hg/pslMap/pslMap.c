/* pslMap - map PSLs alignments to new targets using alignments of the old
 * target to the new target. */
#include "common.h"
#include "pslTransMap.h"
#include "options.h"
#include "linefile.h"
#include "binRange.h"
#include "chromBins.h"
#include "dystring.h"
#include "psl.h"
#include "dnautil.h"
#include "chain.h"

static char const rcsid[] = "$Id: pslMap.c,v 1.17 2009/07/10 18:11:56 markd Exp $";

/* command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"suffix", OPTION_STRING},
    {"keepTranslated", OPTION_BOOLEAN},
    {"chainMapFile", OPTION_BOOLEAN},
    {"swapMap", OPTION_BOOLEAN},
    {"swapIn", OPTION_BOOLEAN},
    {"mapInfo", OPTION_STRING},
    {"mappingPsls", OPTION_STRING},
    {"simplifyMappingIds", OPTION_BOOLEAN},
    {NULL, 0}
};

/* Values parsed from command line */
static char* suffix = NULL;
static unsigned mapOpts = pslTransMapNoOpts;
static boolean chainMapFile = FALSE;
static boolean swapMap = FALSE;
static boolean swapIn = FALSE;
static boolean simplifyMappingIds = FALSE;
static char* mapInfoFile = NULL;
static char* mappingPslFile = NULL;

static char *mapInfoHdr =
    "#srcQName\t" "srcQStart\t" "srcQEnd\t" "srcQSize\t"
    "srcTName\t" "srcTStart\t" "srcTEnd\t"
    "srcStrand\t" "srcAligned\t"
    "mappingQName\t" "mappingQStart\t" "mappingQEnd\t"
    "mappingTName\t" "mappingTStart\t" "mappingTEnd\t"
    "mappingStrand\t" "mappingId\t"
    "mappedQName\t" "mappedQStart\t" "mappedQEnd\t"
    "mappedTName\t" "mappedTStart\t" "mappedTEnd\t"
    "mappedStrand\t"
    "mappedAligned\t" "qStartTrunc\t" "qEndTrunc\n";

static void usage(char *msg)
/* usage msg and exit */
{
/* message got huge, so it's in a generate file */
static char *usageMsg =
#include "usage.msg"
    ;
errAbort("%s\n%s", msg, usageMsg);
}

struct mapAln
/* Mapping alignment, psl plus additional information. */
{
    struct psl *psl;  /* psl, maybe created from a chain */
    int id;           /* chain id, or psl file row  */
};

static struct mapAln *mapAlnNew(struct psl *psl, int id)
/* construct a new mapAln object */
{
struct mapAln *mapAln;
AllocVar(mapAln);
mapAln->psl = psl;
mapAln->id = id;
return mapAln;
}

static char *getMappingId(char *id, struct dyString **idBufPtr)
/* get the mapping id, optionally simplified.  idBuf will be
 * allocated if NULL and is used to store the result */
{
if (!simplifyMappingIds)
    return id;
if (*idBufPtr == NULL)
    *idBufPtr = dyStringNew(128);
struct dyString *idBuf = *idBufPtr;
dyStringClear(idBuf);
dyStringAppend(idBuf, id);
char *p = strrchr(idBuf->string, '-');
if (p != NULL)
    *p = '\0';
p = strrchr(idBuf->string, '.');
if (p != NULL)
    *p = '\0';
idBuf->stringSize = strlen(idBuf->string);
return idBuf->string;
}

static struct mapAln *chainToPsl(struct chain *ch)
/* convert a chain to a psl, ignoring match counts, etc */
{
struct psl *psl;
struct cBlock *cBlk;
int iBlk;
int qStart = ch->qStart, qEnd = ch->qEnd;
char strand[2];
strand[0] = ch->qStrand;
strand[1] = '\0';
if (ch->qStrand == '-')
    reverseIntRange(&qStart, &qEnd, ch->qSize);

psl = pslNew(ch->qName, ch->qSize, qStart, qEnd,
             ch->tName, ch->tSize, ch->tStart, ch->tEnd,
             strand, slCount(ch->blockList), 0);
for (cBlk = ch->blockList, iBlk = 0; cBlk != NULL; cBlk = cBlk->next, iBlk++)
    {
    psl->blockSizes[iBlk] = (cBlk->tEnd - cBlk->tStart);
    psl->qStarts[iBlk] = cBlk->qStart;
    psl->tStarts[iBlk] = cBlk->tStart;
    psl->match += psl->blockSizes[iBlk];
    }
psl->blockCount = iBlk;
if (swapMap)
    pslSwap(psl, FALSE);
return mapAlnNew(psl, ch->id);
}

static struct chromBins* loadMapChains(char *chainFile)
/* read a chain file, convert to mapAln object and chromBins by query locations. */
{
struct chromBins* mapAlns = chromBinsNew((chromBinsFreeFunc*)pslFree);
struct chain *ch;
struct lineFile *chLf = lineFileOpen(chainFile, TRUE);
while ((ch = chainRead(chLf)) != NULL)
    {
    struct mapAln *mapAln = chainToPsl(ch);
    chromBinsAdd(mapAlns, mapAln->psl->qName, mapAln->psl->qStart, mapAln->psl->qEnd, mapAln);
    chainFree(&ch);
    }
lineFileClose(&chLf);
return mapAlns;
}

static struct chromBins* loadMapPsls(char *pslFile)
/* read a psl file and chromBins by query, linking multiple PSLs for the
 * same query.*/
{
struct dyString* idBuf = NULL;
struct chromBins* mapAlns = chromBinsNew((chromBinsFreeFunc*)pslFree);
int id = 0;
struct psl* psl;
struct lineFile *pslLf = pslFileOpen(pslFile);
while ((psl = pslNext(pslLf)) != NULL)
    {
    if (swapMap)
        pslSwap(psl, FALSE);
    chromBinsAdd(mapAlns, getMappingId(psl->qName, &idBuf), psl->qStart, psl->qEnd,
                 mapAlnNew(psl, id));
    id++;
    }
lineFileClose(&pslLf);
dyStringFree(&idBuf);
return mapAlns;
}

static int pslAlignedBases(struct psl *psl)
/* get number of aligned bases in a psl.  Need because we can't set the stats
 * in the mapped psl */
{
int i, cnt = 0;
for (i = 0; i < psl->blockCount; i++)
    cnt += psl->blockSizes[i];
return cnt;
}

static void writeMapInfo(FILE* fh, struct psl *inPsl, struct mapAln *mapAln, 
                         struct psl* mappedPsl)
/* write mapInfo row */
{
/* srcQName srcQStart srcQEnd srcQSize */
fprintf(fh, "%s\t%d\t%d\t%d\t",
        inPsl->qName, inPsl->qStart, inPsl->qEnd, inPsl->qSize);
/* srcTName srcTStart srcTEnd srcStrand srcAligned */
fprintf(fh, "%s\t%d\t%d\t%s\t%d\t",
        inPsl->tName, inPsl->tStart, inPsl->tEnd, inPsl->strand,
        pslAlignedBases(inPsl));
/* mappingQName mappingQStart mappingQEnd */
fprintf(fh, "%s\t%d\t%d\t",
        mapAln->psl->qName, mapAln->psl->qStart, mapAln->psl->qEnd);
/* mappingTName mappingTStart mappingTEnd mappingStrand mappingId */
fprintf(fh, "%s\t%d\t%d\t%s\t%d\t",
        mapAln->psl->tName, mapAln->psl->tStart, mapAln->psl->tEnd,
        mapAln->psl->strand, mapAln->id);
/* mappedQName mappedQStart mappedQEnd mappedTName mappedTStart mappedTEnd mappedStrand */
fprintf(fh, "%s\t%d\t%d\t%s\t%d\t%d\t%s\t",
        mappedPsl->qName, mappedPsl->qStart, mappedPsl->qEnd, 
        mappedPsl->tName, mappedPsl->tStart, mappedPsl->tEnd,
        mappedPsl->strand);
/* mappedAligned qStartTrunc qEndTrunc */
fprintf(fh, "%d\t%d\t%d\n",
        pslAlignedBases(mappedPsl), 
        (mappedPsl->qStart-inPsl->qStart), (inPsl->qEnd-mappedPsl->qEnd));
}

static void addQNameSuffix(struct psl *psl)
/* add the suffix to the mapped psl qName */
{
char *oldQName = psl->qName;
char *newQName = needMem(strlen(psl->qName)+strlen(suffix)+1);
strcpy(newQName, oldQName);
strcat(newQName, suffix);
psl->qName = newQName;
freeMem(oldQName);
}

static void mapPslPair(struct psl *inPsl, struct mapAln *mapAln,
                       FILE* outPslFh, FILE *mapInfoFh, FILE *mappingPslFh)
/* map one pair of query and target PSL */
{
struct psl* mappedPsl;
if (inPsl->tSize != mapAln->psl->qSize)
    errAbort("Error: inPsl %s tSize (%d) != mapping alignment %s qSize (%d) (perhaps you need to specify -swapMap?)\n",
             inPsl->tName, inPsl->tSize, mapAln->psl->qName, mapAln->psl->qSize);

mappedPsl = pslTransMap(mapOpts, inPsl, mapAln->psl);

/* only output if blocks were actually mapped */
if (mappedPsl != NULL)
    {
    if (suffix != NULL)
        addQNameSuffix(mappedPsl);
    pslTabOut(mappedPsl, outPslFh);
    if (mapInfoFh != NULL)
        writeMapInfo(mapInfoFh, inPsl, mapAln, mappedPsl);
    if (mappingPslFh != NULL)
        pslTabOut(mapAln->psl, mappingPslFh);
    }
pslFree(&mappedPsl);
}

static void mapQueryPsl(struct psl* inPsl, struct chromBins *mapAlns,
                        FILE* outPslFh, FILE *mapInfoFh, FILE *mappingPslFh)
/* map a query psl to all targets  */
{
static struct dyString *idBuf = NULL;
struct binElement *overMapAlns
    = chromBinsFind(mapAlns, getMappingId(inPsl->tName, &idBuf), inPsl->tStart, inPsl->tEnd);
struct binElement *overMapAln;
for (overMapAln = overMapAlns; overMapAln != NULL; overMapAln = overMapAln->next)
    mapPslPair(inPsl, (struct mapAln *)overMapAln->val, outPslFh, mapInfoFh, mappingPslFh);
slFreeList(&overMapAlns);
}

static void pslMap(char* inPslFile, char *mapFile, char *outPslFile)
/* project inPsl query through mapFile query to mapFile target */
{
struct chromBins *mapAlns;
struct psl* inPsl;
struct lineFile* inPslLf = pslFileOpen(inPslFile);
FILE *outPslFh, *mapInfoFh = NULL, *mappingPslFh = NULL;

if (chainMapFile)
    mapAlns = loadMapChains(mapFile);
else
    mapAlns = loadMapPsls(mapFile);

outPslFh = mustOpen(outPslFile, "w");
if (mapInfoFile != NULL)
    {
    mapInfoFh = mustOpen(mapInfoFile, "w");
    fputs(mapInfoHdr, mapInfoFh);
    }
if (mappingPslFile != NULL)
    mappingPslFh = mustOpen(mappingPslFile, "w");
while ((inPsl = pslNext(inPslLf)) != NULL)
    {
    if (swapIn)
        pslSwap(inPsl, FALSE);
    mapQueryPsl(inPsl, mapAlns, outPslFh, mapInfoFh, mappingPslFh);
    pslFree(&inPsl);
    }
carefulClose(&mappingPslFh);
carefulClose(&mapInfoFh);
carefulClose(&outPslFh);
lineFileClose(&inPslLf);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc != 4)
    usage("Error: wrong number of arguments");
suffix = optionVal("suffix", NULL);
if (optionExists("keepTranslated"))
    mapOpts |= pslTransMapKeepTrans;
chainMapFile = optionExists("chainMapFile");
swapMap = optionExists("swapMap");
swapIn = optionExists("swapIn");
simplifyMappingIds = optionExists("simplifyMappingIds");
mapInfoFile = optionVal("mapInfo", NULL);
mappingPslFile = optionVal("mappingPsls", NULL);
pslMap(argv[1], argv[2], argv[3]);

return 0;
}
/*
 * Local Variables:
 * c-file-style: "jkent-c"
 * End:
 */

