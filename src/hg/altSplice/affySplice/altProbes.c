/* altProbes.c - Match probes to splicing events and do anaylysis. */
#include "common.h"
#include "bed.h"
#include "hash.h"
#include "options.h"
#include "chromKeeper.h"
#include "binRange.h"
#include "obscure.h"
#include "linefile.h"
#include "dystring.h"
#include "altGraphX.h"
#include "splice.h"
#include "hdb.h"
#include "dnaseq.h"
#include "dMatrix.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics_double.h>

struct altPath
/* Information about a path and associated bed probes. */
{
    struct altPath *next; /* Next in list. */
    int probeCount;        /* Number of probes on this path. */
    struct bed **beds;     /* Array of beds that are unique to this path. */
    int *bedTypes;         /* Are beds exons or splice junction probes. */
    double **expVals;      /* Expression values. */
    double **pVals;        /* Probability of expression. */
    double *avgExpVals;    /* Average value for each exp. */
    struct path *path;     /* Path that this altPath represents. */
    double score;               /* Score of being expressed overall. */
    double exonCorr;            /* Correlation of exon probe set with include junction if appropriate. */
    double exonSkipCorr;        /* Correlation of exon probe set with skip junction if appropriate. */
    double probCorr;            /* Correlation of exon probe set prob and include probs. */
    double probSkipCorr;        /* Correlation of exon probe set prob skip junction if appropriate. */
    double exonSjPercent;       /* Percentage of includes confirmed by exon probe set. */
    double sjExonPercent;       /* Percentage of exons confirmed by sj probe set. */
    int exonAgree;              /* Number of times exon agrees with matched splice junction */
    int exonDisagree;           /* Number of times exon disagrees with matched splice junction. */
    int sjAgree;                /* Number of times sj agrees with matched exon */
    int sjDisagree;             /* Number of times sj disagrees with matched exon. */
    int sjExp;                  /* Number of times sj expressed. */
    int exonExp;                /* Number of times exon expressed. */
};

struct altEvent 
/* Information about a particular alt-splicing event. Specifically a
 * collection of paths and the associated data. */
{
    struct altEvent *next;       /* Next in list. */
    struct splice *splice;       /* Splice event. */
    struct altPath *altPathList; /* Matching of paths to probe sets. */
    int altPathProbeCount;       /* Paths with associated probes. */
    int geneProbeCount;              /* Number of probes on this gene. */
    struct bed **geneBeds;           /* Array of beds that are unique to this gene. */
    double **geneExpVals;            /* Expression values. */
    double **genePVals;              /* Probability of expression. */
    double *avgExpVals;          /* Average value for each exp. */
    boolean isExpressed;         /* Is this altEvent Expressed. */
    boolean isAltExpressed;      /* Is this altEvent alt-expressed? */
};


static struct optionSpec optionSpecs[] = 
/* Our acceptable options to be called with. */
{
    {"help", OPTION_BOOLEAN},
    {"junctionProbes", OPTION_STRING},
    {"nonJunctProbes", OPTION_STRING},
    {"spliceFile", OPTION_STRING},
    {"probFile", OPTION_STRING},
    {"intensityFile", OPTION_STRING},
    {"db", OPTION_STRING},
    {"doPathRatios", OPTION_STRING},
    {"brainSpecific", OPTION_STRING},
    {"tissueExpThresh", OPTION_INT},
    {"otherTissExpThresh", OPTION_INT},
    {"useMaxProbeSet", OPTION_BOOLEAN},
    {"browser", OPTION_STRING},
    {"brainSpecificStrict", OPTION_BOOLEAN},
    {"tissueSpecific", OPTION_STRING},
    {NULL, 0}
};

static char *optionDescripts[] =
/* Description of our options for usage summary. */
{
    "Display this message.",
    "Bed format junction probe sets.",
    "Bed format non-junction (exon, etc) probe sets.",
    "Splice format file containing alternative splicing events.",
    "Data matrix with probability of transcription for each probe set.",
    "Data matrix with intensity estimates for each probe set.",
    "Database freeze of splices.",
    "Generate ratios of probe sets of splice events to each other.",
    "Output brain specific isoforms and dna.",
    "How many tissues have to be seen before called expressed [default=1].",
    "How many other tissues have to be seen before called expressed [default=1].",
    "Instead of using best correlated probe set use all probe sets.",
    "Browser to point at when creating web page.",
    "Require each non-brain tissue to be below absThresh.",
    "Output tissue specific counts for various events to this file."
};

struct hash *bedHash = NULL; /* Access bed probe sets by hash. */
double presThresh = .9;      /* Threshold for being called present. */
int tissueExpThresh = 1;     /* Number of tissues something must be seen in to 
				be considered expressed. */
double absThresh = .1;       /* Threshold for being called not present. */
boolean brainSpecificStrict = FALSE; /* Do we require that all non-brain tissues be below absThresh? */
char *browserName = NULL;    /* Name of browser that we are going to html link to. */
char *db = NULL;             /* Database version used here. */
boolean useMaxProbeSet;      /* Instead of using the best correlated probe set, use any 
				probe set. */
int otherTissueExpThres = 1; /* Number of other tissues something must be seen in to 
				be considered expressed, used */
int brainSpecificEventCount = 0;  /* How many brain specific events are there. */
int brainSpecificEventCassCount = 0;  /* How many brain specific events are there. */
int brainSpecificEventMutExCount = 0;  /* How many brain specific events are there. */
int brainSpecificPathCount = 0;  /* How many brain specific paths are there. */

FILE *brainSpDnaUpOut = NULL;  /* File for dna around first exon on
			      * brain specific isoforms. */

FILE *brainSpDnaUpDownOut = NULL;
FILE *brainSpDnaDownOut = NULL;
FILE *brainSpBedUpOut = NULL;  /* File for paths (in bed format) that
			      * are brain specific. */
FILE *brainSpBedDownOut = NULL;
FILE *brainSpPathBedOut = NULL;
FILE *brainSpTableHtmlOut = NULL; /* Html table for visualizing brain specific events. */
FILE *brainSpFrameHtmlOut = NULL; /* Html frame for visualizing brain specific events. */

static int *brainSpecificCounts;
FILE *tissueSpecificOut = NULL;    /* File to output tissue specific results to. */
boolean tissueSpecificStrict = FALSE; /* Do we require that all other tissues be below absThresh? */
static int **tissueSpecificCounts; /* Array of counts for different tissue specific splice
				      types per tissue. */
/* Keep track of how many of each event are occuring. */
static int alt5PrimeCount = 0;
static int alt3PrimeCount = 0;
static int altCassetteCount = 0;
static int altRetIntCount = 0;
static int altOtherCount = 0;
static int alt3PrimeSoftCount = 0;
static int alt5PrimeSoftCount = 0;
static int altMutExclusiveCount = 0;
static int altControlCount = 0;

/* Keep track of how many of each event with probes */
static int alt5PrimeWProbeCount = 0;
static int alt3PrimeWProbeCount = 0;
static int altCassetteWProbeCount = 0;
static int altRetIntWProbeCount = 0;
static int altOtherWProbeCount = 0;
static int alt3PrimeSoftWProbeCount = 0;
static int alt5PrimeSoftWProbeCount = 0;
static int altMutExclusiveWProbeCount = 0;
static int altControlWProbeCount = 0;

/* Keep track of how many of each event expressed. */
static int alt5PrimeExpCount = 0;
static int alt3PrimeExpCount = 0;
static int altCassetteExpCount = 0;
static int altRetIntExpCount = 0;
static int altOtherExpCount = 0;
static int alt3PrimeSoftExpCount = 0;
static int alt5PrimeSoftExpCount = 0;
static int altMutExclusiveExpCount = 0;
static int altControlExpCount = 0;

/* Keep track of how many of each event alt-expressed */
static int alt5PrimeAltExpCount = 0;
static int alt3PrimeAltExpCount = 0;
static int altCassetteAltExpCount = 0;
static int altRetIntAltExpCount = 0;
static int altOtherAltExpCount = 0;
static int alt3PrimeSoftAltExpCount = 0;
static int alt5PrimeSoftAltExpCount = 0;
static int altMutExclusiveAltExpCount = 0;
static int altControlAltExpCount = 0;



void usage()
/** Print usage and quit. */
{
int i=0;
warn("altProbes - Match probes to splicing paths and analyze.\n"
     "options are:");
for(i=0; i<ArraySize(optionSpecs) -1; i++)
    fprintf(stderr, "  -%s -- %s\n", optionSpecs[i].name, optionDescripts[i]);
errAbort("\nusage:\n   ");
}

void logSpliceType(enum altSpliceType type)
/* Log the different types of splicing. */
{
switch (type) 
    {
    case alt5Prime:
	alt5PrimeCount++;
	break;
    case alt3Prime: 
	alt3PrimeCount++;
	break;
    case altCassette:
	altCassetteCount++;
	break;
    case altRetInt:
	altRetIntCount++;
	break;
    case altOther:
	altOtherCount++;
	break;
    case alt5PrimeSoft:
	alt5PrimeSoftCount++;
	break;
    case alt3PrimeSoft:
	alt3PrimeSoftCount++;
	break;
    case altMutExclusive:
	altMutExclusiveCount++;
	break;
    case altControl:
	altControlCount++;
	break;
    default:
	errAbort("logSpliceType() - Don't recognize type %d", type);
    }
}

void logSpliceTypeWProbe(enum altSpliceType type)
/* Log the different types of splicing. */
{
switch (type) 
    {
    case alt5Prime:
	alt5PrimeWProbeCount++;
	break;
    case alt3Prime: 
	alt3PrimeWProbeCount++;
	break;
    case altCassette:
	altCassetteWProbeCount++;
	break;
    case altRetInt:
	altRetIntWProbeCount++;
	break;
    case altOther:
	altOtherWProbeCount++;
	break;
    case alt5PrimeSoft:
	alt5PrimeSoftWProbeCount++;
	break;
    case alt3PrimeSoft:
	alt3PrimeSoftWProbeCount++;
	break;
    case altMutExclusive:
	altMutExclusiveWProbeCount++;
	break;
    case altControl:
	altControlWProbeCount++;
	break;
    default:
	errAbort("logSpliceType() - Don't recognize type %d", type);
    }
}

void logSpliceTypeExp(enum altSpliceType type)
/* Log the different types of splicing. */
{
switch (type) 
    {
    case alt5Prime:
	alt5PrimeExpCount++;
	break;
    case alt3Prime: 
	alt3PrimeExpCount++;
	break;
    case altCassette:
	altCassetteExpCount++;
	break;
    case altRetInt:
	altRetIntExpCount++;
	break;
    case altOther:
	altOtherExpCount++;
	break;
    case alt5PrimeSoft:
	alt5PrimeSoftExpCount++;
	break;
    case alt3PrimeSoft:
	alt3PrimeSoftExpCount++;
	break;
    case altMutExclusive:
	altMutExclusiveExpCount++;
	break;
    case altControl:
	altControlExpCount++;
	break;
    default:
	errAbort("logSpliceType() - Don't recognize type %d", type);
    }
}

void logSpliceTypeAltExp(enum altSpliceType type)
/* Log the different types of splicing. */
{
switch (type) 
    {
    case alt5Prime:
	alt5PrimeAltExpCount++;
	break;
    case alt3Prime: 
	alt3PrimeAltExpCount++;
	break;
    case altCassette:
	altCassetteAltExpCount++;
	break;
    case altRetInt:
	altRetIntAltExpCount++;
	break;
    case altOther:
	altOtherAltExpCount++;
	break;
    case alt5PrimeSoft:
	alt5PrimeSoftAltExpCount++;
	break;
    case alt3PrimeSoft:
	alt3PrimeSoftAltExpCount++;
	break;
    case altMutExclusive:
	altMutExclusiveAltExpCount++;
	break;
    case altControl:
	altControlAltExpCount++;
	break;
    default:
	errAbort("logSpliceType() - Don't recognize type %d", type);
    }
}

boolean isJunctionBed(struct bed *bed)
/* Return TRUE if bed looks like a junction probe. Specifically
   two blocks, each 15bp. FALSE otherwise. */
{
if(bed->blockCount == 2 && bed->blockSizes[0] == 15 && bed->blockSizes[1] == 15)
    return TRUE;
return FALSE;
}

boolean pathContainsIntron(struct splice *splice, struct path *path, char *chrom,
			   int chromStart, int chromEnd, char *strand)
/* Return TRUE if this path contains an intron (splice junction) that
   starts at chromStart and ends at chromEnd. */
{
int i = 0;
int *vPos = splice->vPositions;
int *verts = path->vertices;
if(differentString(splice->tName, chrom) || 
   differentString(splice->strand, strand))
    return FALSE;

/* Check the edges on the path. */
for(i = 0; i < path->vCount - 1; i++)
    {
    if(pathEdgeType(splice->vTypes, verts[i], verts[i+1]) == ggSJ)
	{
	if(chromStart == vPos[verts[i]] && chromEnd == vPos[verts[i+1]])
	    return TRUE;
	}
    }
return FALSE;
}

boolean pathContainsBlock(struct splice *splice, struct path *path, char *chrom,
			  int chromStart, int chromEnd, char *strand,
			  boolean allBases)
/* Return TRUE if this block is contained in the path, FALSE otherwise.
   If allBases then every base in chromStart-chromEnd must be covered
   by path. */
{
int i = 0;
int *vPos = splice->vPositions;
int *verts = path->vertices;
if(differentString(splice->tName, chrom) || 
   differentString(splice->strand, strand))
    return FALSE;

/* Check the first edge. */
if(path->upV != -1 && pathEdgeType(splice->vTypes, path->upV, 
				   verts[0]) == ggExon)
    {
    if(chromStart >= vPos[path->upV] && chromEnd <= vPos[verts[0]])
	return TRUE;
    else if(!allBases && rangeIntersection(chromStart, chromEnd, 
					   vPos[path->upV], vPos[verts[0]]) > 0)
	return TRUE;
    }

/* Check the last edge. */
if(path->downV != -1 && 
   pathEdgeType(splice->vTypes, verts[path->vCount - 1], path->downV) == ggExon)
    {
    if(chromStart >= vPos[verts[path->vCount - 1]] && 
       chromEnd <= vPos[path->downV])
	return TRUE;
    else if(!allBases && 
	    rangeIntersection(chromStart, chromEnd, 
			      vPos[verts[path->vCount - 1]], vPos[path->downV])  > 0)
	return TRUE;
    }

/* Check the edges on the path. */
for(i = 0; i < path->vCount - 1; i++)
    {
    if(pathEdgeType(splice->vTypes, verts[i], verts[i+1]) == ggExon)
	{
	if(chromStart >= vPos[verts[i]] && chromEnd <= vPos[verts[i+1]])
	    return TRUE;
	else if(!allBases && rangeIntersection(chromStart, chromEnd, 
					       vPos[verts[i]], vPos[verts[i+1]]) > 0)
	    return TRUE;
	}
    }
return FALSE;
}

boolean pathContainsBed(struct splice *splice, struct path *path, 
			struct bed *bed, boolean allBases, boolean intronsToo)
/* Return TRUE if this path contains this bed, FALSE otherwise. If
   allBases, the bed must be completely subsumed by the path. */
{
boolean containsBed = TRUE;
int i = 0;
assert(bed);
for(i = 0; i < bed->blockCount; i++)
    {
    int chromStart = bed->chromStart + bed->chromStarts[i];
    int chromEnd = bed->chromStart + bed->chromStarts[i] + bed->blockSizes[i];

    /* Check the exon. */
    containsBed &= pathContainsBlock(splice, path, bed->chrom, chromStart, 
				     chromEnd, bed->strand, allBases);
    

    /* Check the next intron. */
    if(intronsToo && i+1 != bed->blockCount) 
	containsBed &= pathContainsIntron(splice, path, bed->chrom, chromEnd,
					  bed->chromStarts[i+1]+chromStart, bed->strand);
    }
return containsBed;
}

void insertBedIntoPath(struct altEvent *altEvent, struct altPath *altPath,
		       struct bed *bed)
/* Add the cbed to this path. */
{
int pCount = 0;
assert(altPath);
pCount = altPath->probeCount;
ExpandArray(altPath->beds, pCount, pCount+1);
ExpandArray(altPath->bedTypes, pCount, pCount+1);
altPath->beds[altPath->probeCount++] = bed;
}

int findProbeSetsForEvents(struct altEvent *altEvent)
/* Look for all of the beds that overlap a splice and match
   them the ones that are uniq to a path. Returns the number
   of probes that map to this splice. */
{
struct binElement *be = NULL, *beList = NULL;
struct altPath *altPath = NULL, *altMatchPath = NULL;
struct bed *bed = NULL;

assert(altEvent);
struct splice *splice = altEvent->splice;
/* Load all the beds that span this range. */
beList = chromKeeperFind(splice->tName, splice->tStart, splice->tEnd);
for(be = beList; be != NULL; be = be->next)
    {
    bed = be->val;
    if(!isJunctionBed(bed))
	continue;
    altMatchPath = NULL; /* Indicates that we haven't found a match for this bed. */
    for(altPath = altEvent->altPathList; altPath != NULL; altPath = altPath->next)
	{
	if(pathContainsBed(splice, altPath->path, bed, TRUE, TRUE))
	    {
	    if(altMatchPath == NULL)
		altMatchPath = altPath; /* So far unique match. */
	    else
		{ /* Not a unique match. */
		altMatchPath = NULL;
		break;
		}
	    }
	}

    /* If we've uniquely found a path for a bed insert it. */
    if(altMatchPath != NULL)
	insertBedIntoPath(altEvent, altMatchPath, bed);
    }

/* Count up how many paths have probes. */
for(altPath = altEvent->altPathList; altPath != NULL; altPath = altPath->next)
    if(altPath->probeCount > 0)
	altEvent->altPathProbeCount++;
}		    

void bedLoadChromKeeper()
/* Load the beds's in the file into the chromKeeper. */
{
char *db = optionVal("db", NULL);
char *bedFile = optionVal("junctionProbes", NULL);
struct bed *bed = NULL, *bedList = NULL;
bedHash = newHash(12);
if(db == NULL)
    errAbort("Must specify a database when loading beds.");
if(bedFile == NULL)
    errAbort("Must specify a file for loading beds.");
chromKeeperInit(db);
bedList = bedLoadAll(bedFile);
for(bed = bedList; bed != NULL; bed = bed->next)
    {
    chromKeeperAdd(bed->chrom, bed->chromStart, bed->chromEnd, bed);
    hashAdd(bedHash, bed->name, bed);
    }
}

struct altEvent *altEventsFromSplices(struct splice *spliceList)
/* Create the altEvents and altPaths from the spliceList. */
{
struct splice *splice = NULL;
struct path *path = NULL;
struct altEvent *event = NULL, *eventList = NULL;
struct altPath *altPath = NULL, *altPathList = NULL;
for(splice = spliceList; splice != NULL; splice = splice->next)
    {
    AllocVar(event);
    event->splice = splice;
    for(path = splice->paths; path != NULL; path = path->next)
	{
	AllocVar(altPath);
	altPath->path = path;
	slAddHead(&event->altPathList, altPath);
	}
    slReverse(&event->altPathList);
    slAddHead(&eventList, event);
    }
slReverse(&eventList);
return eventList;
}

void fillInAltPathData(struct altPath *altPath, struct dMatrix *intenM,
		     struct dMatrix *probM)
/* Fill in the data for the probes that map to this
   altPath. */
{
int i = 0;
int pCount = altPath->probeCount;

if(pCount == 0)
    return;

AllocArray(altPath->expVals, pCount);
AllocArray(altPath->pVals, pCount);
AllocArray(altPath->avgExpVals, pCount);
for(i = 0; i < pCount; i++)
    {
    int index = hashIntValDefault(intenM->nameIndex, altPath->beds[i]->name, -1);
    if(index != -1)
	altPath->expVals[i] = intenM->matrix[index];
    index = hashIntValDefault(probM->nameIndex, altPath->beds[i]->name, -1);
    if(index != -1)
	altPath->pVals[i] = probM->matrix[index];
    }
}

char *altGeneNameForPSet(struct bed *bed)
/* Do some parsing of the bed name and assemble 
   a gene name. */
{
static char altGeneName[256]; /* static so memory char * can be returned. */
char buff[256];
char *mark = NULL;
char *extra = "EX";
*altGeneName = '\0';
safef(buff, sizeof(buff), "%s", bed->name);
mark = strchr(buff, '@');
if(mark != NULL)
    {
    *mark = '\0';

    /* If not looking for altSet skip the extra string. */
    if(stringIn("RC", bed->name))
	safef(altGeneName, sizeof(altGeneName), "%s%s_RC_a_at", buff, extra);
    else
	safef(altGeneName, sizeof(altGeneName), "%s%s_a_at", buff, extra);
    return altGeneName;
    }
return NULL;

}

char *geneNameForPSet(struct bed *bed)
/* Do some parsing of the bed name and assemble 
   a gene name. */
{
static char geneName[256]; /* static so memory char * can be returned. */
char buff[256];
char *mark = NULL;
char *extra = "EX";
*geneName = '\0';
safef(buff, sizeof(buff), "%s", bed->name);
mark = strchr(buff, '@');
if(mark != NULL)
    {
    *mark = '\0';

    /* If not looking for altSet skip the extra string. */
    extra = "";
    if(stringIn("RC", bed->name))
	safef(geneName, sizeof(geneName), "%s%s_RC_a_at", buff, extra);
    else
	safef(geneName, sizeof(geneName), "%s%s_a_at", buff, extra);
    return geneName;
    }
return NULL;

}

void fillInGeneData(struct altEvent *altEvent, struct dMatrix *intenM,
		     struct dMatrix *probM)
/* Fill in the gene probe set data for this event. */
{
int i = 0;
struct altPath *altPath = NULL;

for(altPath = altEvent->altPathList; altPath != NULL; altPath = altPath->next)
    {
    if(altPath->probeCount > 0)
	{
	int index = -1;
	int altIndex = -1;
	char *geneName = NULL, *altGeneName = NULL;
	int geneCount = 0;
	/* Find the indexes of the possible gene sets. */
	geneName = geneNameForPSet(altPath->beds[0]);
	index = hashIntValDefault(intenM->nameIndex, geneName, -1);
	altGeneName = altGeneNameForPSet(altPath->beds[0]);
	altIndex = hashIntValDefault(intenM->nameIndex, altGeneName, -1);
	
	/* Count how many probes we found. */
	if(index != -1)
	    geneCount++;
	if(altIndex != -1)
	    geneCount++;

	if(geneCount > 0)
	    {
	    /* Allocate some memory. */
	    AllocArray(altEvent->geneBeds, geneCount);
	    AllocArray(altEvent->geneExpVals, geneCount);
	    AllocArray(altEvent->genePVals, geneCount);
	    AllocArray(altEvent->avgExpVals, geneCount);
	    altEvent->geneProbeCount = geneCount;

	    /* If we found the main name look it up. */
	    if(index != -1)
		{
		altEvent->geneBeds[0] = hashFindVal(bedHash, geneName);
		altEvent->geneExpVals[0] = intenM->matrix[index];
		index =  hashIntValDefault(probM->nameIndex, geneName, -1);
		if(index != -1)
		    altEvent->genePVals[0] = probM->matrix[index];
		}
	    /* Try the alternative gene probe set. */
	    if(altIndex != -1)
		{
		int offSet = geneCount;
		altEvent->geneBeds[offSet] = hashFindVal(bedHash, geneName);
		altEvent->geneExpVals[offSet] = intenM->matrix[index];
		index =  hashIntValDefault(probM->nameIndex, altGeneName, -1);
		if(index != -1)
		    altEvent->genePVals[offSet] = probM->matrix[index];
		}
	    }
	}
    }
}

void fillInEventData(struct altEvent *eventList, struct dMatrix *intenM,
		     struct dMatrix *probM)
/* Loop through the eventList and fill in the altPaths with
   data in intenM and probM. */
{
struct altEvent *event = NULL;
struct altPath *altPath = NULL;
for(event = eventList; event != NULL; event = event->next)
    {
    for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next)
	{
	fillInAltPathData(altPath, intenM, probM);
	}
    fillInGeneData(event, intenM, probM);
    }
}

boolean geneExpressed(struct altEvent *event, int tissueIx)
/* Return TRUE if any of the gene probe sets are expressed in
   this tissue. */
{
int geneIx = 0;
for(geneIx = 0; geneIx < event->geneProbeCount; geneIx++)
    {
    if(event->genePVals[geneIx] != NULL &&
       event->genePVals[geneIx][tissueIx] >= presThresh)
	return TRUE;
    }
return FALSE;
}

boolean altPathProbesExpressed(struct altEvent *event, struct altPath *altPath,
			       int probeIx, int tissueIx)
/* Return TRUE if the path is expressed, FALSE otherwise. */
{
boolean expressed = FALSE;
int i = 0;
if(useMaxProbeSet) 
    {
    for(i = 0; i < altPath->probeCount; i++)
	{
	if(altPath->pVals[i] && altPath->pVals[i][tissueIx] >= presThresh)
	    expressed = TRUE;
	}
    }
else 
    {
    if(altPath->pVals[probeIx] && altPath->pVals[probeIx][tissueIx] >= presThresh)
	expressed = TRUE;
    }
return expressed;
}

boolean tissueExpressed(struct altEvent *event, struct altPath *altPath, 
			int probeIx, int tissueIx)
/* Return TRUE if the probeIx in tissueIx is above minimum
   pVal, FALSE otherwise. */
{
int geneIx = 0;
if(!altPathProbesExpressed(event, altPath, probeIx, tissueIx))
    return FALSE;
if(event->genePVals && geneExpressed(event, tissueIx))
    return TRUE;
return FALSE;
}

boolean altPathProbesNotExpressed(struct altEvent *event, struct altPath *altPath,
			       int probeIx, int tissueIx)
/* Return TRUE if the path is notExpressed, FALSE otherwise. */
{
boolean notExpressed = FALSE;
int i = 0;
if(useMaxProbeSet) 
    {
    for(i = 0; i < altPath->probeCount; i++)
	{
	if(altPath->pVals[i] && altPath->pVals[i][tissueIx] <= absThresh)
	    notExpressed = TRUE;
	}
    }
else 
    {
    if(altPath->pVals[probeIx] && altPath->pVals[probeIx][tissueIx] <= absThresh)
	notExpressed = TRUE;
    }
return notExpressed;
}

boolean tissueNotExpressed(struct altEvent *event, struct altPath *altPath, 
			int probeIx, int tissueIx)
/* Return TRUE if the probeIx in tissueIx is above minimum
   pVal, FALSE otherwise. */
{
int geneIx = 0;
if(!altPathProbesNotExpressed(event, altPath, probeIx, tissueIx))
    return FALSE;
return TRUE;
}

double covariance(double *X, double *Y, int count)
/* Compute the covariance for two vectors. 
   cov(X,Y) = E[XY] - E[X]E[Y] 
   page 326 Sheldon Ross "A First Course in Probability" 1998
*/
{
double cov = gsl_stats_covariance(X, 1, Y, 1, count);
return cov;
}

double correlation(double *X, double *Y, int count)
/* Compute the correlation between X and Y 
   correlation(X,Y) = cov(X,Y)/ squareRt(var(X)var(Y))
   page 332 Sheldon Ross "A First Course in Probability" 1998
*/
{
double varX = gsl_stats_variance(X, 1, count);
double varY = gsl_stats_variance(Y, 1, count);
double covXY = gsl_stats_covariance(X, 1, Y, 1, count);

double correlation = covXY / sqrt(varX *varY);
return correlation;
}

int determineBestProbe(struct altEvent *event, struct altPath *altPath,
		       struct dMatrix *intenM, struct dMatrix *probM,
		       int pathIx)
/* Find the probe with the best correlation to the gene probe sets. */
{
double corr = 0, bestCorr = -2;
int bestIx = -1;
int bedIx = 0, geneIx = 0;

/* Check the easy case. */
if(altPath->probeCount == 1)
    return 0;

/* Loop through and find the probe set with the best 
   correlation to the gene sets. */
for(bedIx = 0; bedIx < altPath->probeCount; bedIx++)
    {
    if(altPath->expVals[bedIx] == NULL)
	continue;
    for(geneIx = 0; geneIx < event->geneProbeCount; geneIx++)
	{
	if(event->geneExpVals[geneIx] == NULL)
	    continue;
	corr = correlation(altPath->expVals[bedIx],
				  event->geneExpVals[geneIx], intenM->colCount);
	if(corr >= bestCorr)
	    {
	    bestIx = bedIx;
	    bestCorr = corr;
	    }
	}
    }
return bestIx;
}

boolean altPathExpressed(struct altEvent *event, struct altPath *altPath,
		      struct dMatrix *intenM, struct dMatrix *probM,
		      int **expressed, int **notExpressed, int pathIx)
/* Fill in the expression matrix for this path. */
{
int bestProbeIx = 0;
int tissueIx = 0;
int total = 0;

/* Quick check to see if there are any 
   probes at all. */
if(altPath->probeCount == 0)
    return FALSE;

bestProbeIx = determineBestProbe(event, altPath, intenM, probM, pathIx);
/* Loop through the tissues to see if they are expressed. */
if(bestProbeIx == -1)
    return FALSE;

for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
    {
    if(tissueExpressed(event, altPath, bestProbeIx, tissueIx))
	{
	expressed[pathIx][tissueIx]++;
	total++;
	}
    else if(tissueNotExpressed(event, altPath, bestProbeIx, tissueIx))
	{
	notExpressed[pathIx][tissueIx]++;
	}
    }
return total > 0;
}

boolean brainSpecific(struct altEvent *event, int pathIx, 
		      int **expressed, int **notExpressed, 
		      int pathCount, struct dMatrix *probM)
/* Output the event if it is alt-expressed and brain
   specific. */
{
int tissueIx = 0;
int brainTissues = 0;
int otherTissues = 0;
int geneBrainTissues = 0;
int geneOtherTissues = 0;
int brainIx = 0;
static char *brainTissuesStrings[] = {"cerebellum","cerebral_hemisphere","cortex","hind_brain",
				      "medial_eminence","olfactory_bulb","pinealgland","thalamus"};
static int isBrain[256];
static boolean initDone = FALSE;

/* Setup array of what is brain and what isn't. */
if(!initDone) 
    {
    if(ArraySize(isBrain) < probM->colCount)
	errAbort("Can only handle up to %d columns, %d is too many", 
		 ArraySize(isBrain), probM->colCount);
    /* Set everthing to FALSE. */
    for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
	isBrain[tissueIx] = FALSE;

    /* Set brain tissue to TRUE. */
    for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
	for(brainIx = 0; brainIx < ArraySize(brainTissuesStrings); brainIx++)
	    if(sameWord(brainTissuesStrings[brainIx], probM->colNames[tissueIx]))
		isBrain[tissueIx] = TRUE;

    initDone = TRUE;
    }
    
for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
    {
    if(expressed[pathIx] != NULL &&
       expressed[pathIx][tissueIx])
	{
	if(isBrain[tissueIx])
	    brainTissues++;
	else
	    otherTissues++;
	}
    else if(brainSpecificStrict &&
	    notExpressed[pathIx] != NULL &&
	    !notExpressed[pathIx][tissueIx])
	{
	if(!isBrain[tissueIx])
	    otherTissues++;
	}
    if(geneExpressed(event, tissueIx))
	{
	if(isBrain[tissueIx])
	    geneBrainTissues++;
	else
	    geneOtherTissues++;
	}
    }

if(otherTissues == 0 && 
   brainTissues >= tissueExpThresh &&
   geneOtherTissues >= tissueExpThresh)
    return TRUE;
return FALSE;
}

boolean tissueSpecific(struct altEvent *event, int pathIx, int targetTissue,
		      int **expressed, int **notExpressed, 
		      int pathCount, struct dMatrix *probM)
/* Output the event if it is alt-expressed and brain
   specific. */
{
int tissueIx = 0;
int specificTissues = 0;
int otherTissues = 0;
int geneSpecificTissues = 0;
int geneOtherTissues = 0;
    
for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
    {
    if(expressed[pathIx] != NULL &&
       expressed[pathIx][tissueIx])
	{
	if(tissueIx == targetTissue)
	    specificTissues++;
	else
	    otherTissues++;
	}
    else if(tissueSpecificStrict &&
	    notExpressed[pathIx] != NULL &&
	    !notExpressed[pathIx][tissueIx])
	{
	if(tissueIx != targetTissue)
	    otherTissues++;
	}
    if(geneExpressed(event, tissueIx))
	{
	if(tissueIx == targetTissue)
	    geneSpecificTissues++;
	else
	    geneOtherTissues++;
	}
    }

if(otherTissues == 0 && 
   specificTissues >= tissueExpThresh &&
   geneOtherTissues >= tissueExpThresh)
    return TRUE;
return FALSE;
}

void fillInSequences(struct altEvent *event, struct path *path, 
		     struct dnaSeq **upSeq, struct dnaSeq **downSeq,
		     struct bed **upSeqBed, struct bed **downSeqBed)
/* Fill in the sequences from the path. 200 from intron, 5bp from
 exon. */
{
int firstSplice = -1;
int secondSplice = -1;
int i = 0;
struct splice *splice = event->splice;
int *vPos = splice->vPositions;
unsigned char*vTypes = splice->vTypes;
int vC = path->vCount;
int *verts = path->vertices;

/* Want first exon in order of transcription. */
if(sameString(splice->strand,"+"))
    {
    for(i = 0; i < vC -1; i++)
	if(pathEdgeType(vTypes, verts[i], verts[i+1]) == ggExon)
	    {
	    firstSplice = vPos[verts[i]];
	    secondSplice = vPos[verts[i+1]];
	    }
    }
else
    {
    for(i = vC-1; i > 0; i--)
	if(pathEdgeType(vTypes, verts[i-1], verts[i]) == ggExon)
	    {
	    firstSplice = vPos[verts[i-1]];
	    secondSplice = vPos[verts[i]];
	    }
    }

if(firstSplice < 0 || secondSplice < 0)
    errAbort("Problem with event %s at %s:%d-%d doesn't have an exon",
	     splice->name, splice->tName, splice->tStart, splice->tEnd);

AllocVar(*upSeqBed);
AllocVar(*downSeqBed);
/* Construct the beds. */
(*upSeqBed)->name = cloneString(splice->name);
(*upSeqBed)->chrom = cloneString(splice->tName);
safef((*upSeqBed)->strand, sizeof((*upSeqBed)->strand), "%s", splice->strand);
(*upSeqBed)->chromStart = firstSplice - 200;
(*upSeqBed)->chromEnd = firstSplice + 5;
(*upSeqBed)->score = splice->type;

(*downSeqBed)->name = cloneString(splice->name);
(*downSeqBed)->chrom = cloneString(splice->tName);
safef((*downSeqBed)->strand, sizeof((*downSeqBed)->strand), "%s", splice->strand);
(*downSeqBed)->chromStart = secondSplice - 5;
(*downSeqBed)->chromEnd = secondSplice + 200;
(*downSeqBed)->score = splice->type;

/* Get the sequences. */
*upSeq = hSeqForBed((*upSeqBed));
*downSeq = hSeqForBed((*downSeqBed));

/* If on negative strand swap up and down. */
if(sameString(splice->strand, "-"))
    {
    struct dnaSeq *tmpSeq = NULL;
    struct bed *tmpBed = NULL;
    /* Swap beds. */
    tmpBed = *upSeqBed;
    *upSeqBed = *downSeqBed;
    *downSeqBed = tmpBed;
    /* Swap sequences. */
    tmpSeq = *upSeq;
    *upSeq = *downSeq;
    *downSeq = tmpSeq;
    }
}

char * nameForType(int type)
/* Log the different types of splicing. */
{
switch (type) 
    {
    case alt5Prime:
	return "alt5Prime";
	break;
    case alt3Prime: 
	return "alt3Prime";
	break;
    case altCassette:
	return "altCassette";
	break;
    case altRetInt:
	return "altRetInt";
	break;
    case altOther:
	return "altOther";
	break;
    case altControl:
	return "altControl";
	break;
    case altMutExclusive:
	return "altMutEx";
	break;
    case alt5PrimeSoft:
	return "altTxStart";
	break;
    case alt3PrimeSoft:
	return "altTxEnd";
	break;
    case altIdentity:
	return "altIdentity";
	break;
    default:
	errAbort("nameForType() - Don't recognize type %d", type);
    }
return "error";
}

void makeJunctMdbGenericLink(struct splice *js, struct dyString *buff, char *name)
{
int offSet = 100;
int i = 0;
dyStringClear(buff);
dyStringPrintf(buff, "<a target=\"plots\" href=\"http://mdb1-sugnet.cse.ucsc.edu/cgi-bin/mdbSpliceGraph?mdbsg.calledSelf=on&coordString=%s:%c:%s:%d-%d&mdbsg.cs=%d&mdbsg.ce=%d&mdbsg.expName=%s&mdbsg.probeSummary=on&mdbsg.toScale=on\">", 
	       "mm2", js->strand[0], js->tName, js->tStart, js->tEnd, 
	       js->tStart - offSet, js->tEnd + offSet, "AffyMouseSplice1-02-2004");
dyStringPrintf(buff, "%s", name);
dyStringPrintf(buff, "</a> ");
}

void printLinks(struct splice *splice)
/* Loop through and print each splicing event to web page. */
{
struct splice *s = NULL;
struct path *lastPath = NULL;
int diff = -1;
struct dyString *buff = NULL;
if(splice->paths == NULL || splice->type == altControl)
    return;
lastPath = slLastEl(splice->paths);
if(slCount(splice->paths) == 2)
    diff = abs(splice->paths->bpCount - splice->paths->next->bpCount);
else
    diff = lastPath->bpCount;
buff = newDyString(256);
fprintf(brainSpTableHtmlOut, "<tr><td><a target=\"browser\" "
	"href=\"http://%s/cgi-bin/hgTracks?db=%s&position=%s:%d-%d\">", browserName,
	db, splice->tName, splice->tStart-100, splice->tEnd+100);
fprintf(brainSpTableHtmlOut,"%s </a>\n", splice->name);
makeJunctMdbGenericLink(splice, buff, "[p]");
fprintf(brainSpTableHtmlOut, "%s", buff->string);
fprintf(brainSpTableHtmlOut, " </td>");
fprintf(brainSpTableHtmlOut,"<td>%s</td>", nameForType(splice->type));
fprintf(brainSpTableHtmlOut,"<td>%d</td></tr>\n", diff);
dyStringFree(&buff);
}

void outputBrainSpecificEvents(struct altEvent *event, int **expressed, int **notExpressed,
			       int pathCount, struct dMatrix *probM)
/* Output the event if it is alt-expressed and brain
   specific. */
{
struct dnaSeq *upSeq = NULL;
struct dnaSeq *downSeq = NULL;
struct bed *pathBed = NULL;
struct bed *upSeqBed = NULL;
struct bed *downSeqBed = NULL;
int pathIx = 0;
struct altPath *altPath = NULL;
boolean brainSpecificEvent = FALSE;
struct splice *splice = event->splice;
for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next, pathIx++)
    {
    if(!brainSpecific(event, pathIx, expressed, notExpressed, pathCount, probM) || 
       (splice->type != altCassette && splice->type != altMutExclusive && 
	splice->type != alt5Prime && splice->type != alt3Prime &&
	splice->type != altOther)) //	splice->type != alt3PrimeSoft &&
	{
	continue;
	}
 
    /* check to make sure this path includes some sequence. */
    pathBed = pathToBed(altPath->path, event->splice, -1, -1, FALSE);
    if(pathBed == NULL)
	continue;
    brainSpecificPathCount++;
    brainSpecificEvent = TRUE;
    fillInSequences(event, slLastEl(event->splice->paths),
		    &upSeq, &downSeq, &upSeqBed, &downSeqBed);

    
    /* Write out the data. */
    faWriteNext(brainSpDnaUpOut, upSeq->name, upSeq->dna, upSeq->size);
    bedTabOutN(upSeqBed, 6, brainSpBedUpOut);
    faWriteNext(brainSpDnaDownOut, downSeq->name, downSeq->dna, downSeq->size);
    bedTabOutN(downSeqBed, 6, brainSpBedDownOut);
    bedTabOutN(pathBed, 12, brainSpPathBedOut);
    bedFree(&upSeqBed);
    bedFree(&downSeqBed);
    dnaSeqFree(&upSeq);
    dnaSeqFree(&downSeq);
    }

if(brainSpecificEvent == TRUE)
    {
    brainSpecificEventCount++;
    printLinks(splice);
    switch(splice->type)
	{
	case altCassette :
	    brainSpecificCounts[altCassette]++;
	    break;
	case altMutExclusive :
	    brainSpecificCounts[altMutExclusive]++;
	    break;
	case alt5Prime :
	    brainSpecificCounts[alt5Prime]++;
	    break;
	case alt3Prime :
	    brainSpecificCounts[alt3Prime]++;
	    break;
	case alt3PrimeSoft :
	    brainSpecificCounts[alt3PrimeSoft]++;
	    break;
	case altOther :
	    brainSpecificCounts[altOther]++;
	    break;
	}
    }
}

void incrementCountByType(int *array, int type)
{
switch(type)
    {
    case altCassette :
	array[altCassette]++;
	break;
    case altMutExclusive :
	array[altMutExclusive]++;
	break;
    case alt5Prime :
	array[alt5Prime]++;
	break;
    case alt3Prime :
	array[alt3Prime]++;
	break;
    case alt3PrimeSoft :
	array[alt3PrimeSoft]++;
	break;
    case altOther :
	array[altOther]++;
	break;
    }
}

void outputTissueSpecificEvents(struct altEvent *event, int **expressed, int **notExpressed,
			       int pathCount, struct dMatrix *probM)
/* Output the event if it is alt-expressed and tissue specific
   specific. */
{
int tissueIx = 0;
struct altPath *altPath = NULL;
struct splice *splice = event->splice;
for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
    {
    boolean tissueSpecificEvent = FALSE;
    int pathIx = 0;
    for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next, pathIx++)
	{
	if(tissueSpecific(event, pathIx, tissueIx, expressed, notExpressed, pathCount, probM))
	    {
	    incrementCountByType(tissueSpecificCounts[tissueIx], splice->type);
	    break;
	    }
	}
    }
}

   

void doEventAnalysis(struct altEvent *event, struct dMatrix *intenM,
		     struct dMatrix *probM)
/* Analyze a given event. */
{
int i = 0;
int **expressed = NULL;
int **notExpressed = NULL;
struct altPath *altPath = NULL;
int pathCount = slCount(event->altPathList);
int pathExpCount = 0;
int pathIx = 0;
int withProbes = 0;

/* Allocate a 2D array for expression. */
AllocArray(expressed, pathCount);
AllocArray(notExpressed, pathCount);
for(i = 0; i < pathCount; i++)
    {
    AllocArray(expressed[i], probM->colCount);
    AllocArray(notExpressed[i], probM->colCount);
    }

/* Fill in the array. */
for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next)
    {
    if(altPath->probeCount > 0)
	withProbes++;
    if(altPathExpressed(event, altPath, intenM, probM, 
			expressed, notExpressed, pathIx))
	pathExpCount++;
    pathIx++;
    }


/* Determine our expression, alternate or otherwise. */
if(pathExpCount >= tissueExpThresh && withProbes > 1)
    event->isExpressed = TRUE;
if(pathExpCount >= tissueExpThresh * 2 && withProbes > 1)
    event->isAltExpressed = TRUE;

if(brainSpBedUpOut != NULL && event->isExpressed == TRUE)
    outputBrainSpecificEvents(event, expressed, notExpressed, pathCount, probM);

if(tissueSpecificOut != NULL && event->isExpressed == TRUE)
    outputTissueSpecificEvents(event, expressed, notExpressed, pathCount, probM);

/* Cleanup some memory. */
for(i = 0; i < pathCount; i++)
    {
    freez(&expressed[i]);
    freez(&notExpressed[i]);
    }
freez(&notExpressed);
freez(&expressed);
}

void doAnalysis(struct altEvent *eventList, struct dMatrix *intenM,
		struct dMatrix *probM)
/* How many of the alt events are expressed. */
{
struct altEvent *event = NULL;
int i = 0;
struct altPath *altPath = NULL;
int expressed = 0, altExpressed = 0;
for(event = eventList; event != NULL; event = event->next)
    {
    doEventAnalysis(event, intenM, probM);
    if(event->isExpressed)
	logSpliceTypeExp(event->splice->type);
    if(event->isAltExpressed)
	logSpliceTypeAltExp(event->splice->type);
    }
}

double calcPercent(double numerator, double denominator)
/* Calculate a percent checking for zero. */
{
if(denominator != 0)
    return (100.0 * numerator / denominator);
return 0;
}

void reportTissueSpecificCounts(char **tissueNames, int tissueCount)
/* Writ out a matrix of tissues and their counts. */
{
int i = 0, j = 0;

assert(tissueSpecificOut);

fprintf(tissueSpecificOut, "\taltCassette\taltMutExclusive\talt5Prime\talt3Prime\taltOther\n");
for(i = 0; i < tissueCount; i++)
    {
    fprintf(tissueSpecificOut, "%22s\t%4d\t%4d\t%4d\t%4d\t%4d\n", tissueNames[i],
	    tissueSpecificCounts[i][altCassette], tissueSpecificCounts[i][altMutExclusive],
	    tissueSpecificCounts[i][alt5Prime],tissueSpecificCounts[i][alt3Prime],
	    tissueSpecificCounts[i][altOther]);
    }
}

void reportBrainSpCounts()
/* Print brain specific results. */
{
if(brainSpDnaUpOut != NULL)
    {
    fprintf(stderr, "%d Brain Specific Events. %d Brain Specific paths\n", 
	    brainSpecificEventCount, brainSpecificPathCount);
    fprintf(stderr, "%d Cassette, %d Mutally Exclusive.\n", 
	    brainSpecificCounts[altCassette], brainSpecificCounts[altMutExclusive]);
    fprintf(stderr, "%d Alt 3', %d Alt 5'.\n", 
	    brainSpecificCounts[alt3Prime], brainSpecificCounts[alt5Prime]);
    fprintf(stderr, "%d Alt Txn End, %d Alt Other.\n", 
	    brainSpecificCounts[alt3PrimeSoft], brainSpecificCounts[altOther]);
    
    }
}

void reportEventCounts()
/* Print some stats about splices and probe with counts. */
{
fprintf(stderr, "+----------------------+-------+-----------+-----------+------+---------+\n");
fprintf(stderr, "| Alt Event.           | Count | w/ Probes | Expressed | Alt. | Percent |\n");
fprintf(stderr, "+----------------------+-------+-----------+-----------+------+---------+\n");
fprintf(stderr, "| alt 5'               | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n", 
	alt5PrimeCount, alt5PrimeWProbeCount, alt5PrimeExpCount, alt5PrimeAltExpCount,
	calcPercent(alt5PrimeAltExpCount, alt5PrimeExpCount));
fprintf(stderr, "| alt 3'               | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n", 
	alt3PrimeCount, alt3PrimeWProbeCount, alt3PrimeExpCount, alt3PrimeAltExpCount,
	calcPercent(alt3PrimeAltExpCount, alt3PrimeExpCount));
fprintf(stderr, "| alt Cass             | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	altCassetteCount, altCassetteWProbeCount, altCassetteExpCount, altCassetteAltExpCount,
	calcPercent(altCassetteAltExpCount, altCassetteExpCount));
fprintf(stderr, "| alt Ret. Int.        | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	altRetIntCount, altRetIntWProbeCount, altRetIntExpCount, altRetIntAltExpCount,
	calcPercent(altRetIntAltExpCount, altRetIntExpCount));
fprintf(stderr, "| alt Mutual Exclusive | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	altMutExclusiveCount, altMutExclusiveWProbeCount, altMutExclusiveExpCount, altMutExclusiveAltExpCount,
	calcPercent(altMutExclusiveAltExpCount, altMutExclusiveExpCount));
fprintf(stderr, "| alt Txn Start        | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	alt5PrimeSoftCount, alt5PrimeWProbeCount, alt5PrimeSoftExpCount, alt5PrimeSoftAltExpCount,
	calcPercent(alt5PrimeSoftAltExpCount, alt5PrimeSoftExpCount));
fprintf(stderr, "| alt Txn End          | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	alt3PrimeSoftCount, alt3PrimeWProbeCount, alt3PrimeSoftExpCount, alt3PrimeSoftAltExpCount,
	calcPercent(alt3PrimeSoftAltExpCount, alt3PrimeSoftExpCount));
fprintf(stderr, "| alt Other            | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	altOtherCount, altOtherWProbeCount, altOtherExpCount, altOtherAltExpCount,
	calcPercent(altOtherAltExpCount, altOtherExpCount));
fprintf(stderr, "| alt Control          | %5d |      %4d |      %4d |  %3d |  %5.1f%% |\n",
	altControlCount, altControlWProbeCount, altControlExpCount, altControlAltExpCount,
	calcPercent(altControlAltExpCount, altControlExpCount));
fprintf(stderr, "+----------------------+-------+-----------+-----------+------+---------+\n");
}

void initTissueSpecific(int tissueCount)
/* Open up the files for tissue specific isoforms and dna. */
{
char *name = optionVal("tissueSpecific", NULL);
int i = 0;
AllocArray(tissueSpecificCounts, tissueCount);
for(i = 0; i < tissueCount; i++)
    {
    AllocArray(tissueSpecificCounts[i], 20);
    }
tissueSpecificOut = mustOpen(name, "w");
}

void initBrainSpecific()
/* Open up the files for brain specific isoforms and dna. */
{
char *prefix = optionVal("brainSpecific", NULL);
struct dyString *file = newDyString(strlen(prefix)+10);
brainSpecificStrict = optionExists("brainSpecificStrict");
AllocArray(brainSpecificCounts, altMutExclusive + 1);

dyStringClear(file);
dyStringPrintf(file, "%s.path.bed", prefix);
brainSpPathBedOut = mustOpen(file->string, "w");

dyStringClear(file);
dyStringPrintf(file, "%s.up.fa", prefix);
brainSpDnaUpOut = mustOpen(file->string, "w");

dyStringClear(file);
dyStringPrintf(file, "%s.down.fa", prefix);
brainSpDnaDownOut = mustOpen(file->string, "w");

dyStringClear(file);
dyStringPrintf(file, "%s.up.bed", prefix);
brainSpBedUpOut = mustOpen(file->string, "w");

dyStringClear(file);
dyStringPrintf(file, "%s.down.bed", prefix);
brainSpBedDownOut = mustOpen(file->string, "w");

dyStringClear(file);
dyStringPrintf(file, "%s.table.html", prefix);
brainSpTableHtmlOut = mustOpen(file->string, "w");
fprintf(brainSpTableHtmlOut, "<html>\n<body bgcolor=\"#FFF9D2\"><b>Alt-Splice List</b>\n"
	"<table border=1><tr><th>Name</th><th>Type</th><th>Size</th></tr>\n");

dyStringClear(file);
dyStringPrintf(file, "%s.frame.html", prefix);
brainSpFrameHtmlOut = mustOpen(file->string, "w");
fprintf(brainSpFrameHtmlOut, "<html><head><title>Brain Specific Events.</title></head>\n"
	"<frameset cols=\"30%,70%\">\n"
	"   <frame name=\"_list\" src=\"./%s%s\">\n"
	"   <frameset rows=\"50%,50%\">\n"
	"      <frame name=\"browser\" src=\"http://%s/cgi-bin/hgTracks?db=%s\">\n"
	"      <frame name=\"plots\" src=\"http://%s/cgi-bin/hgTracks?db=%s\">\n"
	"   </frameset>\n"
	"</frameset>\n"
	"</html>\n", prefix, ".table.html", 
	browserName, db, browserName, db );
carefulClose(&brainSpFrameHtmlOut);

dyStringFree(&file);
}

void altProbes()
/* Top level function to map probes to paths and analyze. */
{
struct splice *spliceList = NULL;
struct altEvent *eventList = NULL, *event = NULL;
struct dMatrix *intenM = NULL, *probM = NULL;
char *intenIn = optionVal("intensityFile", NULL);
char *probIn = optionVal("probFile", NULL);
char *splicesIn = optionVal("spliceFile", NULL);
if(splicesIn == NULL)
    errAbort("Must specify a apliceFile");

warn("Loading splices from %s", splicesIn);
spliceList = spliceLoadAll(splicesIn);
eventList = altEventsFromSplices(spliceList);
warn("Loading beds.");
bedLoadChromKeeper();
dotForUserInit(max(slCount(eventList)/20, 1));
for(event = eventList; event != NULL; event = event->next)
    {
    dotForUser();
    findProbeSetsForEvents(event);
    logSpliceType(event->splice->type);
    if(event->altPathProbeCount >= 2)
	logSpliceTypeWProbe(event->splice->type);
    }
warn("");
warn("Reading Data Matrixes.");
if(probIn == NULL || intenIn == NULL)
    errAbort("Must specify intensityFile and probeFile");
intenM = dMatrixLoad(intenIn);
probM = dMatrixLoad(probIn);
if(optionExists("tissueSpecific"))
    initTissueSpecific(probM->colCount);
fillInEventData(eventList, intenM, probM);
doAnalysis(eventList, intenM, probM);
reportEventCounts();
reportBrainSpCounts();
reportTissueSpecificCounts(probM->colNames, probM->colCount);
if(brainSpTableHtmlOut)
    fprintf(brainSpTableHtmlOut, "</table></body></html>\n");
carefulClose(&brainSpTableHtmlOut);
carefulClose(&brainSpDnaUpOut);
carefulClose(&brainSpDnaDownOut);
carefulClose(&brainSpBedUpOut);
carefulClose(&brainSpBedDownOut);
carefulClose(&brainSpPathBedOut);
}

void setOptions()
/* Set up some options. */
{
tissueExpThresh = optionInt("tissueExpThresh", 1);
otherTissueExpThres = optionInt("otherTissueExpThres", 1);
useMaxProbeSet = optionExists("useMaxProbeSet");
db = optionVal("db", NULL);
browserName = optionVal("browser", "hgwdev-sugnet.cse.ucsc.edu");
if(useMaxProbeSet)
    warn("Using max value from all probe sets in a path.");
/* Set up the datbase. */
db = optionVal("db", NULL);
if(db != NULL)
    hSetDb(db);
else
    errAbort("Must specify database.");

if(optionVal("brainSpecific", NULL) != NULL)
    initBrainSpecific();
}

int main(int argc, char *argv[])
/* Everybody's favorite function... */
{
if(argc == 1)
    usage();
optionInit(&argc, argv, optionSpecs);
if(optionExists("help"))
    usage();
setOptions();
altProbes();
return 0;
}
