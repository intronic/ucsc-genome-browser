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
#include "liftOver.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_cdf.h>
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
    int *motifUpCounts;         /* Number of motifs counted upstream for each motif. */
    int *motifDownCounts;        /* Number of motifs counted downstream for each motif. */
    int *motifInsideCounts;        /* Number of motifs counted inside for each motif. */
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
    double flipScore;            /* Number of times on version - number of times other / sum of both. */
    double percentUltra;           /* Percentage of bases that overlap very conserved elements. */
    boolean isExpressed;         /* Is this altEvent Expressed. */
    boolean isAltExpressed;      /* Is this altEvent alt-expressed? */
};

struct bindSite
/* Binding sites for a protein. */
{
    struct bindSite *next; /* Next in list. */
    char **motifs;     /* List of valid motifs. */
    int motifCount;    /* Number of moitfs to look for. */
    char *rnaBinder;   /* Rna binding protein associated with these sites. */
};


struct unitTest
{
    struct unitTest *next; /* Next in list. */
    boolean (*test)(struct unitTest *test);     /* Function to call for test. */
    char *description;     /* Description of tests. */
    char *errorMsg;        /* Error message returned by test. */
};

/* Create a funtion pointer. */
typedef boolean (*testFunction)(struct unitTest *test);

struct unitTest *tests = NULL;

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
    {"brainTissues", OPTION_STRING},
    {"combinePSets", OPTION_BOOLEAN},
    {"useExonBeds", OPTION_BOOLEAN},
    {"skipMotifControls", OPTION_BOOLEAN},
    {"selectedPValues", OPTION_STRING},
    {"selectedPValFile", OPTION_STRING},
    {"outputRatioStats", OPTION_STRING},
    {"psToRefGene", OPTION_STRING},
    {"splicePVals", OPTION_STRING},
    {"cassetteBeds", OPTION_STRING},
    {"newDb", OPTION_STRING},
    {"flipScoreSort", OPTION_BOOLEAN},
    {"doTests", OPTION_BOOLEAN},
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
    "Output tissue specific counts for various events to this file.",
    "Comma separated list of tissues to consider brain.",
    "Combine the p-values for a path using Fisher's method.",
    "Use exon beds, not just splice junction beds.",
    "Skip looking at the motifs in the controls, can be quite slow.",
    "File containing list of AFFY G###### ids and gene names.",
    "File to output pvalue vectors for selected genes to.",
    "Output the ratio for altCassette, alt5, alt3, and altMutExclusive to file.",
    "File with mapping from probe set to gene name.",
    "File with pvalues for specific splices of interest.",
    "Prefix for files to output not-expressed, expressed not alt, and alt cassette beds.",
    "Genome to try and lift coordinates to.",
    "Sort brain specific genes by flip score.",
    "Do some unit tests."
};


struct hash *liftOverHash = NULL; /* Hash holding chains for lifting. */
struct hash *bedHash = NULL; /* Access bed probe sets by hash. */
double presThresh = .9;      /* Threshold for being called present. */
int tissueExpThresh = 1;     /* Number of tissues something must be seen in to 
				be considered expressed. */
double absThresh = .1;       /* Threshold for being called not present. */
boolean brainSpecificStrict = FALSE; /* Do we require that all non-brain tissues be below absThresh? */
char *browserName = NULL;    /* Name of browser that we are going to html link to. */
char *db = NULL;             /* Database version used here. */
char *newDb = NULL;             /* Database version lifting to. */
boolean useMaxProbeSet;      /* Instead of using the best correlated probe set, use any 
				probe set. */
boolean useComboProbes = FALSE; /* Combine probe sets on a given path using fisher's method. */
boolean useExonBedsToo = FALSE; /* Use exon beds too, not just splice junction beds. */
int intronsConserved = 0;
int intronsConsCounted = 0;
struct slRef *brainSpEvents = NULL; /* List of brain specific events. */
FILE *ratioStatOut = NULL;  /* File to output a matrix of ratios of skip/includes. */
FILE *ratioSkipIntenOut = NULL; /* Intensity file for skip paths. */
FILE *ratioIncIntenOut = NULL; /* Intensity file for skip paths. */
FILE *ratioGeneIntenOut = NULL; /* Intensity file for gene probe set. */
FILE *ratioProbOut = NULL; /* Probability gene is expressed. */
FILE *incProbOut = NULL;   /* Include probability values. */
FILE *skipProbOut = NULL;   /* Include probability values. */

FILE *pathProbabilitiesOut = NULL; /* File to print out path probability vectors. */
FILE *pathExpressionOut = NULL; /* File to print out path probability vectors. */
FILE *pathProbabilitiesBedOut = NULL; /* File to print out corresponding beds for paths. */

double pvalThresh = .01; //.05 / 1263; /* Threshold at which we believe pval is real. */
struct hash *splicePValsHash = NULL; /* Hash of pvals of interest. */
struct hash *outputPValHash = NULL; /* Hash of selected names. */
struct hash *psToRefGeneHash = NULL; /* Hash of probe set to affy genes. */
int otherTissueExpThres = 1; /* Number of other tissues something must be seen in to 
				be considered expressed, used */
struct bindSite *bindSiteList = NULL; /* List of rna binding sites to look for. */
struct bindSite **bindSiteArray = NULL; /* array of rna binding sites to look for. */
int bindSiteCount = 0;             /* Number of binding sites to loop through. */
int cassetteExonCount = 0;         /* Number of cassette exons examined. */
int *cassetteUpMotifs = NULL;      /* Counts of motifs upstream cassette exons. */
int *cassetteDownMotifs = NULL;    /* Counts of moitfs downstream cassette exons. */
int *cassetteInsideMotifs = NULL;    /* Counts of moitfs downstream cassette exons. */
int controlExonCount = 0;         /* Number of control exons examined. */
int *controlUpMotifs = NULL;      /* Counts of motifs upstream cassette exons. */
int *controlDownMotifs = NULL;    /* Counts of moitfs downstream cassette exons. */
int *controlInsideMotifs = NULL;  /* Counts of moitfs inside cassette exons. */
boolean skipMotifControls = FALSE; /* Skip looking for motifs in controls. */
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
int *brainSpecificMotifUpCounts = NULL;
int *brainSpecificMotifDownCounts = NULL;
int *brainSpecificMotifInsideCounts = NULL;
FILE *expressedCassetteBedOut = NULL; /* Output beds for casssette exons that are expressed. */
FILE *altCassetteBedOut = NULL;       /* Output beds for cassettte exons that are alt. expressed. */
FILE *notExpressedCassetteBedOut = NULL; /* Output beds for cassette exons that don't look expressed. */

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

struct splice *ndr2CassTest()
/* Create a splice for use in testsing. */
{
char *string = cloneString("chr14	43771400	43772080	chr14.7822-8.26	2	-	9319	38	43765895,43766921,43767127,43767179,43767304,43767340,43767563,43767611,43767860,43767912,43768199,43768244,43768682,43768786,43768944,43769001,43769182,43769269,43769488,43769549,43770672,43770735,43770929,43771050,43771294,43771356,43771400,43771404,43771700,43771742,43772080,43772161,43773937,43774090,43774093,43774180,43774538,43774609,	0,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,1,3,2,1,3,1,3,1,1,2,2,1,2,	2	{\"chr14\",43771400,43772080,0,2,2,{26,30,},24,31,0,},{\"chr14\",43771400,43772080,0,4,4,{26,28,29,30,},24,31,42,},");
struct splice *splice = NULL;
char *words[100];
chopByWhite(string, words, sizeof(words));
splice = spliceLoad(words);
}

struct bed *ndr2BedTest()
/* Create beds used in testing. */
{
char *inc1 = "chr14	43771385	43771715	G6912708@J918653_RC@j_at	0	-	43771385	43771715	0	2	15,15,	0,315,";
char *skip = "chr14	43771385	43772095	G6912708@J918654_RC@j_at	0	-	43771385	43772095	0	2	15,15,	0,695,";
char *inc2 = "chr14	43771727	43772095	G6912708@J918655_RC@j_at	0	-	43771727	43772095	0	2	15,15,	0,353,";
char *gene = "chr14	43765981	43772151	G6912708_RC_a_at	0	-	43765981	43772151	0	9	25,25,25,25,25,25,25,25,43,	0,149,1602,3222,3262,3516,5000,5392,6127,";
struct bed *bedList = NULL, *bed = NULL;
char *words[12];

chopByWhite(cloneString(inc1), words, ArraySize(words));
bed = bedLoadN(words,12);
slAddHead(&bedList, bed);

chopByWhite(cloneString(skip), words, ArraySize(words));
bed = bedLoadN(words,12);
slAddHead(&bedList, bed);

chopByWhite(cloneString(inc2), words, ArraySize(words));
bed = bedLoadN(words,12);
slAddHead(&bedList, bed);

chopByWhite(cloneString(gene), words, ArraySize(words));
bed = bedLoadN(words,12);
slAddHead(&bedList, bed);

slReverse(&bedList);
return bedList;
}

struct dMatrix *ndr2BedTestMat(boolean probability)
/* Create a fake matrix of probabilities. If probability is TRUE fill
 in a probability matrix, otherwise fill in an intensity matrix. */
{
struct dMatrix *probs = NULL;
static char *colNames[] = {"cerebellum", "cortex", "heart", "skeletal"};
static char *rowNames[] = {"G6912708@J918653_RC@j_at", "G6912708@J918654_RC@j_at", 
		    "G6912708@J918655_RC@j_at", "G6912708_RC_a_at"};
double probM[4][4] = {{.8, 1, 0, .5},{0, 0, 0, 1}, {.8, 0, 0, .5}, {1, 0, .5, 1}};
double intenM[4][4] = {{1, 1, 1, 2.5}, {1, 1, 1, 1}, {1, 1, 0, 1.5}, {1, 1, 1, .5}};
double **matrix = NULL;
int count = 0;
AllocVar(probs);
AllocArray(matrix, ArraySize(rowNames));
probs->nameIndex = newHash(3);

for(count = 0; count < ArraySize(colNames); count++)
    {
    if(probability)
	matrix[count] = CloneArray(probM[count], ArraySize(probM[count])); 
    else
	matrix[count] = CloneArray(intenM[count], ArraySize(intenM[count])); 
    hashAddInt(probs->nameIndex, rowNames[count], count); 
    }

probs->colNames = colNames;
probs->colCount = ArraySize(colNames);
probs->rowNames = rowNames;
probs->rowCount = ArraySize(rowNames);
probs->matrix = matrix;
return probs;
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

void initSplicePVals()
/* Load up the pvals of interest generated in a separate file. */
{
struct lineFile *lf = NULL;
char *fileName = optionVal("splicePVals", NULL);
char *words[2];
struct slDouble *sd = NULL;
assert(fileName);
splicePValsHash = newHash(10);
lf = lineFileOpen(fileName, TRUE);
while(lineFileNextRow(lf, words, ArraySize(words)))
    {
    char *mark = NULL;
    if((mark = strchr(words[0], ':')) != NULL)
	*mark = '\0';
    hashAdd(splicePValsHash, words[0], slDoubleNew(sqlDouble((words[1]))));
    }
lineFileClose(&lf);
}

double pvalForSplice(char *name)
/* Return a pval as specified in the splicePVal file or 2 if not there. */
{
struct slDouble *d = NULL;
if(splicePValsHash == NULL)
    errAbort("Need to specify -splicePVal file if using pvalForSplice");
assert(name);
d = hashFindVal(splicePValsHash, name);
if(d == NULL)
    return 2.0;
return d->val;
}

void initPsToRefGene()
/* Load up a file that converts from affy psName to gene names. */
{
struct lineFile *lf = NULL;
char *fileName = optionVal("psToRefGene", NULL);
char *words[2];
assert(fileName);
psToRefGeneHash = newHash(10);
lf = lineFileOpen(fileName, TRUE);
while(lineFileNextRow(lf, words, ArraySize(words)))
    {
    hashAdd(psToRefGeneHash, words[1], cloneString(words[0]));
    }
lineFileClose(&lf);
}

char *refSeqForPSet(char *pSet)
/* Look up the refseq name for a probe set. */
{
static boolean warned = FALSE;
char *gene = NULL;
if(psToRefGeneHash == NULL)
    {
    if(warned != TRUE)
	warn("Can't convert psName to refSeq name, have to specify psToRefGene flag.");
    warned = TRUE;
    return pSet;
    }
gene = hashFindVal(psToRefGeneHash, pSet);
if(gene == NULL)
    return pSet;
return gene;
}

void initBindSiteList()
/* Create a little hand curated knowledge about binding sites. */
{
struct bindSite *bsList = NULL, *bs = NULL;
int i = 0;

/* Nova-1 */
AllocVar(bs);
bs->motifCount = 8;
AllocArray(bs->motifs, bs->motifCount);
/* Expand TCATY_3 to all 8 possibilities */
bs->motifs[i++] = cloneString("TCATCTCATCTCATC");
bs->motifs[i++] = cloneString("TCATCTCATCTCATT");
bs->motifs[i++] = cloneString("TCATCTCATTTCATT");
bs->motifs[i++] = cloneString("TCATCTCATTTCATC");
bs->motifs[i++] = cloneString("TCATTTCATCTCATC");
bs->motifs[i++] = cloneString("TCATTTCATCTCATT");
bs->motifs[i++] = cloneString("TCATTTCATTTCATT");
bs->motifs[i++] = cloneString("TCATTTCATTTCATC");
bs->rnaBinder = cloneString("Nova-1");
slAddHead(&bsList, bs);

/* Fox-1 */
AllocVar(bs);
bs->motifCount = 1;
AllocArray(bs->motifs, bs->motifCount);
bs->motifs[0] = cloneString("GCATG");
bs->rnaBinder = cloneString("Fox-1");
slAddHead(&bsList, bs);

/* PTB */
AllocVar(bs);
bs->motifCount = 1;
AllocArray(bs->motifs, bs->motifCount);
bs->motifs[0] = cloneString("CTCTCT");
bs->rnaBinder = cloneString("PTB/nPTB");
slAddHead(&bsList, bs);

/* hnRNPs */
AllocVar(bs);
bs->motifCount = 1;
AllocArray(bs->motifs, bs->motifCount);
bs->motifs[0] = cloneString("GGGG");
bs->rnaBinder = cloneString("hnRNP-H/F");
slAddHead(&bsList, bs);

/* hnRNPs */
AllocVar(bs);
bs->motifCount = 1;
AllocArray(bs->motifs, bs->motifCount);
bs->motifs[0] = cloneString("ACTGAA");
bs->rnaBinder = cloneString("ChuckFav");
slAddHead(&bsList, bs);

bindSiteList = bsList;
slReverse(&bsList);
AllocArray(bindSiteArray, slCount(bsList));

for(bs = bsList; bs != NULL; bs = bs->next)
    bindSiteArray[bindSiteCount++] = bs;
AllocArray(cassetteUpMotifs, bindSiteCount);
AllocArray(cassetteDownMotifs, bindSiteCount);
AllocArray(cassetteInsideMotifs, bindSiteCount);
AllocArray(controlUpMotifs, bindSiteCount);
AllocArray(controlDownMotifs, bindSiteCount);
AllocArray(controlInsideMotifs, bindSiteCount);
}

boolean isJunctionBed(struct bed *bed)
/* Return TRUE if bed looks like a junction probe. Specifically
   two blocks, each 15bp. FALSE otherwise. */
{
if(bed->blockCount == 2 && bed->blockSizes[0] == 15 && bed->blockSizes[1] == 15)
    return TRUE;
return FALSE;
}

boolean validVertex(struct splice *splice, int v1, int v2)
/* Check to see if a given vertex is the sink or source of the
   graph and should be ignored. */
{
return (v1 >= 0 && v1 < splice->vCount && v2 >= 0 && v2 < splice->vCount);
}

enum ggEdgeType pathEdgeTypeValid(struct splice *s, int v1, int v2)
{
assert(validVertex(s, v1,v2));
return pathEdgeType(s->vTypes, v1, v2);
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
    if(validVertex(splice, verts[i], verts[i+1]) &&
       pathEdgeTypeValid(splice, verts[i], verts[i+1]) == ggSJ)
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
if(validVertex(splice, path->upV, verts[0]) &&
   pathEdgeTypeValid(splice, path->upV,verts[0]) == ggExon)
    {
    if(chromStart >= vPos[path->upV] && chromEnd <= vPos[verts[0]])
	return TRUE;
    else if(!allBases && rangeIntersection(chromStart, chromEnd, 
					   vPos[path->upV], vPos[verts[0]]) > 0)
	return TRUE;
    }

/* Check the last edge. */
if(validVertex(splice, verts[path->vCount -1], path->downV) &&
   pathEdgeTypeValid(splice, verts[path->vCount - 1], path->downV) == ggExon)
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
    if(validVertex(splice, verts[i], verts[i+1]) &&
       pathEdgeTypeValid(splice, verts[i], verts[i+1]) == ggExon)
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
			struct bed *bed,  boolean intronsToo)
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
				     chromEnd, bed->strand, TRUE);
    if(containsBed == FALSE)
	break;

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
    if(isJunctionBed(bed) || (useExonBedsToo && strchr(bed->name, '@') != NULL))
	{
	altMatchPath = NULL; /* Indicates that we haven't found a match for this bed. */
	for(altPath = altEvent->altPathList; altPath != NULL; altPath = altPath->next)
	    {
	    /* If it is a junction pass the intronsToo flag == TRUE, otherwise
	       set to false. */
	    if((isJunctionBed(bed) && pathContainsBed(splice, altPath->path, bed, TRUE)) ||
	       (!isJunctionBed(bed) && pathContainsBed(splice, altPath->path, bed, FALSE)))
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
	}
    else
	continue;
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
		int offSet = geneCount - 1;
		altEvent->geneBeds[offSet] = hashFindVal(bedHash, geneName);
		altEvent->geneExpVals[offSet] = intenM->matrix[altIndex];
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

double geneExpression(struct altEvent *event, int tissueIx)
/* Return maximum pval for all the gene sets. */
{
double maxInten = -1;
int geneIx = 0;
for(geneIx = 0; geneIx < event->geneProbeCount; geneIx++)
    {
    if(event->geneExpVals[geneIx] != NULL &&
       (event->geneExpVals[geneIx][tissueIx] >= maxInten))
	maxInten = event->geneExpVals[geneIx][tissueIx];
    }
return maxInten;
}


double genePVal(struct altEvent *event, int tissueIx)
/* Return maximum pval for all the gene sets. */
{
double maxPval = -1;
int geneIx = 0;
for(geneIx = 0; geneIx < event->geneProbeCount; geneIx++)
    {
    if(event->genePVals[geneIx] != NULL &&
       (event->genePVals[geneIx][tissueIx] >= maxPval))
	maxPval = event->genePVals[geneIx][tissueIx];
    }
return maxPval;
}


boolean geneExpressed(struct altEvent *event, int tissueIx)
/* Return TRUE if any of the gene probe sets are expressed in
   this tissue. */
{
if(genePVal(event, tissueIx) >= presThresh)
    return TRUE;
return FALSE;
}

void printVars(double *preLog, double *log)
{
printf("Pre-Log %.20f, Log() %.20f\n", preLog, log);
}

double pvalAltPathCombExpressed(struct altPath *altPath, int tissueIx)
/* Combine pvals that path expressed using fisher's method and return
   overall prob that expressed. */
{
double probProduct = 0;
int probCount = 0;
int i = 0;
double combination = 0;
double x = 0;
for(i = 0; i < altPath->probeCount; i++)
    {
    if(altPath->pVals[i])
	{
	/* Check for log 0, make it just log very small number. */
	if(altPath->pVals[i][tissueIx] == 0)
	    x = log(.00000001);
	else
	    x = log(altPath->pVals[i][tissueIx]);
	probProduct += x;
	probCount++;
	}
    }
/* If no probes return 0. */
if(probCount == 0) 
    return 0;
combination = gsl_cdf_chisq_P(-2.0*probProduct,2.0*probCount); 
return combination;
}

boolean altPathProbesCombExpressed(struct altEvent *event, struct altPath *altPath,
			       int probeIx, int tissueIx)
/* Return TRUE if the path is expressed, FALSE otherwise. */
{
double combination = 1;
assert(altPath);
combination = pvalAltPathCombExpressed(altPath, tissueIx);
if(combination <= 1 - presThresh)
    return TRUE;
return FALSE;
}

boolean altPathProbesExpressed(struct altEvent *event, struct altPath *altPath,
			       int probeIx, int tissueIx, double *expression)
/* Return TRUE if the path is expressed, FALSE otherwise. */
{
int i = 0;
int probeCount = 0;
double **expVals = altPath->expVals;
boolean expressed = FALSE;
if(useMaxProbeSet) 
    {
    for(i = 0; i < altPath->probeCount; i++)
	{
	if(altPath->pVals[i] && altPath->pVals[i][tissueIx] >= presThresh)
	    {
	    expressed = TRUE;
	    *expression = max(*expression, expVals[i][tissueIx]);
	    }	    
	}
    }
else if(useComboProbes)
    {
    if(altPathProbesCombExpressed(event, altPath, probeIx, tissueIx))
	expressed = TRUE;
    if(expVals[i] != NULL)
	{
	for(i = 0; i < altPath->probeCount; i++)
	    {
	    *expression += expVals[i][tissueIx];
	    probeCount++;
	    }
	}
    if(probeCount > 0)
	*expression = *expression / probeCount;
    }
else
    {
    if(altPath->pVals[probeIx] && altPath->pVals[probeIx][tissueIx] >= presThresh)
	{
	expressed = TRUE;
	*expression = altPath->expVals[probeIx][tissueIx];
	}
    }
return expressed;
}

boolean tissueExpressed(struct altEvent *event, struct altPath *altPath, 
			int probeIx, int tissueIx, double *expression)
/* Return TRUE if the probeIx in tissueIx is above minimum
   pVal, FALSE otherwise. */
{
int geneIx = 0;
if(!altPathProbesExpressed(event, altPath, probeIx, tissueIx, expression))
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

void readSelectedList(char *fileName)
/* Read in a list of gene identifiers that we want to 
   output. */
{
struct lineFile *lf = NULL;
struct hash *hash = newHash(12);
char *words[2];
int wordCount = ArraySize(words);
assert(fileName);
lf = lineFileOpen(fileName, TRUE);
while(lineFileChopNext(lf, words, wordCount)) 
    {
    hashAdd(hash, words[0], cloneString(words[1]));
    }
lineFileClose(&lf);
outputPValHash = hash;
}

char *isSelectedProbeSet(struct altEvent *event, struct altPath *altPath)
/* Return gene name if this probe set is select for outputing
   probability vector else return NULL. */
{
char *mark;
char buff[256];
struct altPath *path = NULL;
int i = 0; 
for(i = 0; i < altPath->probeCount; i++)
    {
    if(altPath->beds[i] != NULL)
	{
	char *mark = strchr(altPath->beds[i]->name, '@');
	if(mark != NULL)
	    {
	    char *geneName = NULL;
	    *mark = '\0';
	    if((geneName = hashFindVal(outputPValHash, altPath->beds[i]->name)) != NULL)
		{
		*mark = '@';
		return geneName;
		}
	    }
	}
    }
return NULL;
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

void outputPathProbabilities(struct altEvent* event, struct altPath *altPath,
			     struct dMatrix *intenM, struct dMatrix *probM,
			     int** expressed, int **notExpressed, double **expression,
			     int pathIx)
/* Print out a thresholded probablilty vector for clustering and visualization
   in treeview. */
{
int i = 0;
FILE *out = pathProbabilitiesOut;
FILE *eOut = pathExpressionOut;
struct splice *splice = event->splice;
struct path *path = altPath->path;
char *geneName = NULL;
assert(out);
if((geneName = isSelectedProbeSet(event, altPath)) != NULL)
    {
    char *psName = "Error";
    assert(altPath->beds[0] != NULL);
    psName = altPath->beds[0]->name;
    fprintf(out, "%s\t%s.%s.%s.%d", psName, geneName, nameForType(splice->type), psName, path->bpCount);
    fprintf(eOut, "%s\t%s.%s.%s.%d", psName, geneName, nameForType(splice->type), psName, path->bpCount);
    for(i = 0; i < probM->colCount; i++)
	{
	fprintf(eOut, "\t%f", expression[pathIx][i]);
	if(expressed[pathIx][i])
	    fprintf(out, "\t2");
	else if(notExpressed[pathIx][i])
	    fprintf(out, "\t-2");
	else
	    fprintf(out, "\t0");
	}
    fprintf(out, "\n");
    fprintf(eOut, "\n");
    }
}

boolean altPathExpressed(struct altEvent *event, struct altPath *altPath,
			 struct dMatrix *intenM, struct dMatrix *probM,
			 int **expressed, int **notExpressed, double **expression,
			 int pathIx)
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
    double e = -1;
    if(tissueExpressed(event, altPath, bestProbeIx, tissueIx, &e))
	{
	expressed[pathIx][tissueIx]++;
	total++;
	}
    else if(tissueNotExpressed(event, altPath, bestProbeIx, tissueIx))
	{
	notExpressed[pathIx][tissueIx]++;
	}
    expression[pathIx][tissueIx] = e;
    }
return total > 0;
}

double expressionAverage(struct altPath *altPath, int tissueIx)
/* Return the average expression for all probe sets matched
   to this path in this tissue. */
{
double pathExp = 0.0;
int pCount = 0.0;
int probeIx = 0;
for(probeIx = 0; probeIx < altPath->probeCount; probeIx++)
    {
    if(altPath->expVals[probeIx])
	{
	pathExp += altPath->expVals[probeIx][tissueIx];
	pCount++;
	}
    }
if(pCount == 0)
    return -1;
pathExp = pathExp / pCount;
return pathExp;
}

void calculateFlipScore(struct altEvent *event, struct dMatrix *probM, boolean *isBrain)

/* Calculate |(#times skip > include) - (#times skip < include)|/totalTissues */
{
struct altPath *skip = NULL, *include = NULL;
int tissueIx = 0;
int brainCount = 0;
int notBrainCount = 0;
int tissueCount = 0;
assert(event);
event->flipScore = 0;
skip = event->altPathList;
include = event->altPathList->next;
assert(probM);
assert(probM->colCount > 0);
for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
    {
    if(geneExpressed(event, tissueIx))
	{
	double skipI = expressionAverage(skip, tissueIx);
	double incI = expressionAverage(include, tissueIx);
	if(incI >= skipI)
	    {
	    if(isBrain[tissueIx])
		brainCount++;
	    else 
		notBrainCount++;
	    }
	else 
	    {
	    if(isBrain[tissueIx])
		brainCount--;
	    else 
		notBrainCount--;
	    }
	tissueCount++;
	}
    }
/* If no data, then set score to worst val: 1 */
if(tissueCount == 0)
    event->flipScore = 0;
else
    {
    double brainScore = 0;
    double nonBrainScore = 0;
    brainScore = (double) brainCount / tissueCount;
    nonBrainScore = (double) notBrainCount / tissueCount;
    event->flipScore = fabs(brainScore - nonBrainScore);
    }
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
static int tissueCount = 0;
static char *brainTissuesToChop = NULL;
static char *brainTissuesStrings[256];
static int isBrain[256];
static boolean initDone = FALSE;

/* Setup array of what is brain and what isn't. */
if(!initDone) 
    {
    brainTissuesToChop = cloneString(optionVal("brainTissues","cerebellum,cerebral_hemisphere,cortex,hind_brain,medial_eminence,olfactory_bulb,pinealgland,thalamus"));
    if(ArraySize(isBrain) < probM->colCount)
	errAbort("Can only handle up to %d columns, %d is too many", 
		 ArraySize(isBrain), probM->colCount);
    /* Set everthing to FALSE. */
    for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
	isBrain[tissueIx] = FALSE;

    /* Set brain tissue to TRUE. */
    tissueCount = chopByChar(brainTissuesToChop, ',', brainTissuesStrings, ArraySize(brainTissuesStrings));
    for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
	for(brainIx = 0; brainIx < tissueCount; brainIx++)
	    if(stringIn(brainTissuesStrings[brainIx], probM->colNames[tissueIx]))
		isBrain[tissueIx] = TRUE;

    initDone = TRUE;
    }

if(event->splice->type == altCassette)
    calculateFlipScore(event, probM, isBrain);

/* If we have pvals specified by the user use them instead. */
if(splicePValsHash != NULL)
    {
    double d = pvalForSplice(event->splice->name);
    if(d >=0 && d <= 1 && d <= pvalThresh)
	return TRUE;
    else
	return FALSE;
    }
   
/* Otherwise use the routines here. */
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
int exonOffset = 0;
int intronOffset = 100;
/* Want first exon in order of transcription. */
if(sameString(splice->strand,"+"))
    {
    for(i = 0; i < vC -1; i++)
	if(validVertex(splice, verts[i], verts[i+1]) &&
	   pathEdgeTypeValid(splice, verts[i], verts[i+1]) == ggExon)
	    {
	    firstSplice = vPos[verts[i]];
	    secondSplice = vPos[verts[i+1]];
	    break;
	    }
    }
else
    {
    for(i = vC-1; i > 0; i--)
	if(validVertex(splice, verts[i-1], verts[i]) &&
	   pathEdgeTypeValid(splice, verts[i-1], verts[i]) == ggExon)
	    {
	    firstSplice = vPos[verts[i-1]];
	    secondSplice = vPos[verts[i]];
	    break;
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
(*upSeqBed)->chromStart = firstSplice - intronOffset;
(*upSeqBed)->chromEnd = firstSplice + exonOffset;
(*upSeqBed)->score = splice->type;

(*downSeqBed)->name = cloneString(splice->name);
(*downSeqBed)->chrom = cloneString(splice->tName);
safef((*downSeqBed)->strand, sizeof((*downSeqBed)->strand), "%s", splice->strand);
(*downSeqBed)->chromStart = secondSplice - exonOffset;
(*downSeqBed)->chromEnd = secondSplice + intronOffset;
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

struct bed *phastConForRegion(char *chrom, int chromStart, int chromEnd)
{
struct sqlConnection *conn = hAllocConn2();
struct sqlResult *sr = NULL;
struct bed *conBed = NULL, *conBedList = NULL;
int rowOffset = 0;
char **row = NULL;
sr = hRangeQuery(conn, "phastConsElements90", chrom, chromStart, chromEnd,
		 NULL, &rowOffset);
while((row = sqlNextRow(sr)) != NULL)
    {
    conBed = bedLoad6(row+rowOffset);
    slAddHead(&conBedList, conBed);
    }
sqlFreeResult(&sr);
hFreeConn2(&conn);
slReverse(&conBedList);
return conBedList;
}

double percentIntronBasesOverlappingCons(char *chrom, int chromStart, int chromEnd)
/* Percentage of intron bases that overlap a phastCons element. */
{
int intronOffset = 100, overlap = 0;
double percent = 0;
if(hTableExists2("phastConsElements90"))
    {
    struct bed *upstream = phastConForRegion(chrom, chromStart-intronOffset, chromStart);
    struct bed *downstream = phastConForRegion(chrom, chromEnd, chromEnd + intronOffset);
    struct bed *bed = NULL;
    for(bed = upstream; bed != NULL; bed = bed->next)
	{
	overlap += positiveRangeIntersection(chromStart-intronOffset, chromStart,
					     bed->chromStart, bed->chromEnd);
	}
    for(bed = downstream; bed != NULL; bed = bed->next)
	{
	overlap += positiveRangeIntersection(chromEnd, chromEnd+intronOffset,
					     bed->chromStart, bed->chromEnd);
	}
    percent = (double)overlap / (2 * intronOffset);
    bedFreeList(&upstream);
    bedFreeList(&downstream);
    }
return percent;
}

void printLinks(struct altEvent *event, struct bed *pathBed)
/* Loop through and print each splicing event to web page. */
{
struct splice *s = NULL;
struct path *lastPath = NULL;
struct splice *splice = event->splice;
char *chrom = NULL;
char strand;
int chromStart = 0,  chromEnd = 0;
int diff = -1;
char *useDb = db;
char *newDb = optionVal("newDb", NULL);
boolean overlapsPhasCons = FALSE;
double overlapPercent = 0;
struct dyString *buff = NULL;
if(splice->paths == NULL || splice->type == altControl)
    return;
lastPath = slLastEl(splice->paths);
if(slCount(splice->paths) == 2)
    diff = abs(splice->paths->bpCount - splice->paths->next->bpCount);
else
    diff = lastPath->bpCount;
chrom = cloneString(pathBed->chrom);
chromStart = pathBed->chromStart;
chromEnd = pathBed->chromEnd;
strand = pathBed->strand[0];

if(newDb != NULL)
    {
    if(liftOverCoords(&chrom, &chromStart, &chromEnd, &strand))
	useDb = newDb;
    }


/* overlapPercent = percentIntronBasesOverlappingCons(chrom, chromStart,chromEnd); */
buff = newDyString(256);
fprintf(brainSpTableHtmlOut, "<tr><td>");
if(overlapsPhasCons)
    fprintf(brainSpTableHtmlOut, "<font color=red> * </font>");
fprintf(brainSpTableHtmlOut, "<a target=\"browser\" "
	"href=\"http://%s/cgi-bin/hgTracks?db=%s&position=%s:%d-%d&hgt.motifs=GGGG%%2CCTCTCT%%2CGCATG\",>", browserName,
	useDb, chrom, chromStart-100, chromEnd+100);
fprintf(brainSpTableHtmlOut,"%s </a><font size=-1>%s</font>\n", 
	refSeqForPSet(event->altPathList->beds[0]->name), useDb);

fprintf(brainSpTableHtmlOut, "<a target=\"browser\" "
	"href=\"http://%s/cgi-bin/hgTracks?db=%s&position=%s:%d-%d&complement=%d&hgt.motifs=GGGG%%2CCTCTCT%%2CGCATG\">[u]</a>", 
	browserName, useDb, chrom, chromStart-95, chromStart+5,
	strand == '-' ? 1 : 0);
fprintf(brainSpTableHtmlOut, "<a target=\"browser\" "
	"href=\"http://%s/cgi-bin/hgTracks?db=%s&position=%s:%d-%d&complement=%d&hgt.motifs=GGGG%%2CCTCTCT%%2CGCATG\">[d]</a>", 
	browserName, useDb, chrom, chromEnd-5, chromEnd+95,
	strand == '-' ? 1 : 0);
fprintf(brainSpTableHtmlOut, "<a target=\"plots\" href=\"./brainDiffPlots.9/%s:%s:%s:%s:%s:%s:%d:%d.jpg\">[f]</a>", 
	splice->name, refSeqForPSet(event->altPathList->beds[0]->name), 
	nameForType(splice->type), event->altPathList->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
makeJunctMdbGenericLink(splice, buff, "[p]");
fprintf(brainSpTableHtmlOut, "%s", buff->string);
fprintf(brainSpTableHtmlOut, "(%5.2f%%) ", 100.0 * event->percentUltra);
if(splice->type == altCassette)
    {
    int i = 0;
    int *u = event->altPathList->next->motifUpCounts;
    int *d = event->altPathList->next->motifDownCounts;
    for(i = 0; i < bindSiteCount; i++)
	fprintf(brainSpTableHtmlOut, "%d ", u[i] + d[i]);
    }
fprintf(brainSpTableHtmlOut, " </td>");
// fprintf(brainSpTableHtmlOut,"<td>%s</td>", nameForType(splice->type));
if(optionExists("flipScoreSort"))
    fprintf(brainSpTableHtmlOut,"<td>%.4f</td></tr>\n", event->flipScore);
else
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
struct slRef *brainEvent = NULL;
boolean lifted = FALSE;
int i = 0;
int pathIx = 0;
struct altPath *altPath = NULL;
boolean brainSpecificEvent = FALSE;
struct splice *splice = event->splice;

for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next, pathIx++)
    {
    char *chrom = NULL;
    int chromStart = 0;
    int chromEnd = 0;
    char strand;
    if(!brainSpecific(event, pathIx, expressed, notExpressed, pathCount, probM) || (splice->type != altCassette))
/*        (splice->type != altCassette && splice->type != altMutExclusive &&  */
/* 	splice->type != alt5Prime && splice->type != alt3Prime && */
/* 	splice->type != altOther)) //	splice->type != alt3PrimeSoft && */
	{
	continue;
	}
    
    /* check to make sure this path includes some sequence. */
    pathBed = pathToBed(altPath->path, event->splice, -1, -1, FALSE);
    if(pathBed == NULL)
	continue;

    /* Add event to our list for later sorting, etc. */
    AllocVar(brainEvent);
    brainEvent->val = event;
    slAddHead(&brainSpEvents, brainEvent);

/*     /\* Calculate the flip score for now. *\/ */
/*     if(onlyTwoPaths(event)) */
/* 	calculateFlipScore(event, probM); */
    
    chrom = cloneString(pathBed->chrom);
    chromStart = pathBed->chromStart;
    chromEnd = pathBed->chromEnd;
    strand = pathBed->strand[0];
    
    if(newDb != NULL)
	{
	lifted = liftOverCoords(&chrom, &chromStart, &chromEnd, &strand);
	}
    event->percentUltra = percentIntronBasesOverlappingCons(chrom, chromStart,chromEnd);
    if(event->percentUltra >= .20)
	intronsConserved++;
    intronsConsCounted++;
	
    /* Count up some motifs. */
    if(splice->type == altCassette && altPath->path->bpCount > 0)
	{
	for(i = 0; i < bindSiteCount; i++)
	    {
	    brainSpecificMotifUpCounts[i] += altPath->motifUpCounts[i];
	    brainSpecificMotifDownCounts[i] += altPath->motifDownCounts[i];
	    brainSpecificMotifInsideCounts[i] += altPath->motifInsideCounts[i];
	    }
	}
	
    brainSpecificPathCount++;
    brainSpecificEvent = TRUE;
    fillInSequences(event, slLastEl(event->splice->paths),
		    &upSeq, &downSeq, &upSeqBed, &downSeqBed);

    /* Write out the data. */
    if(event->percentUltra >= .20)
	{
	faWriteNext(brainSpDnaUpOut, upSeq->name, upSeq->dna, upSeq->size);
	bedTabOutN(upSeqBed, 6, brainSpBedUpOut);
	faWriteNext(brainSpDnaDownOut, downSeq->name, downSeq->dna, downSeq->size);
	bedTabOutN(downSeqBed, 6, brainSpBedDownOut);
	bedTabOutN(pathBed, 6, brainSpPathBedOut);
	}
    bedFree(&upSeqBed);
    bedFree(&downSeqBed);
    dnaSeqFree(&upSeq);
    dnaSeqFree(&downSeq);
/*     freez(&chrom); */
    }

if(brainSpecificEvent == TRUE)
    {
    brainSpecificEventCount++;

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
/* Output the event if it is tissue specific
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
	if(altPath->path->bpCount > 0 && 
	   tissueSpecific(event, pathIx, tissueIx, expressed, notExpressed, pathCount, probM))
	    {
	    incrementCountByType(tissueSpecificCounts[tissueIx], splice->type);
	    break;
	    }
	}
    }
}


boolean onlyTwoPaths(struct altEvent *event)
/* Return TRUE if event is altCassette, alt3Prime, alt5Prime,
 * altMutEx. FALSE otherwise. */
{
int type = event->splice->type;
if(type == altCassette || type == alt3Prime || type == alt5Prime || type == altMutExclusive)
    return TRUE;
return FALSE;
}



double expressionRatio(struct altEvent *event, struct altPath *skipPath, 
		       struct altPath *incPath, int tissueIx)
/* Calculate the expression ratio between the two paths. -1 if not
 * expressed.*/
{
double skipExp = 0.0, incExp = 0.0;
int probeIx = 0;
double ratio = 0.0;
/* If not expressed, output -1. */
if(!geneExpressed(event, tissueIx))
    ratio = -1;
else
    {
    /* Calculate average expression for inc and skip paths. */
    skipExp = expressionAverage(skipPath, tissueIx);
    incExp =  expressionAverage(incPath, tissueIx);
    if(skipExp == -1 || incExp == -1)
	return -1;

    /* Caclulate the ratio. */
    if(incExp <= 0)
	incExp = .001;
    ratio = skipExp / incExp;
    }
return ratio;
}

void outputRatioStats(struct altEvent *event, struct dMatrix *probM, 
		      struct dMatrix *intenM)
/* Calculate the ratio of skip to include for tissues that are
   considered expressed. For tissues where event isn't expressed
   output -1 as all ratios will be positive this makes a good empty
   data place holder. */
{
struct splice *splice = event->splice;
struct altPath *skipPath = NULL, *incPath = NULL;
int tissueIx = 0;

double tmpThresh = presThresh;

/* Few sanity checks. */
assert(slCount(event->altPathList) == 2);
assert(ratioStatOut != NULL);
assert(intenM->colCount = probM->colCount);

skipPath = event->altPathList;
incPath = event->altPathList->next;

/* If there is no data don't output anything. */
if(skipPath->probeCount == 0 || incPath->probeCount == 0)
    return;

/* Going to include everything that we think might
   have a chance of being expressed. */
presThresh = absThresh;
fprintf(ratioStatOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
fprintf(ratioSkipIntenOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
fprintf(ratioIncIntenOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
fprintf(ratioGeneIntenOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
fprintf(ratioProbOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
fprintf(incProbOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);
fprintf(skipProbOut, "%s:%s:%s:%s:%s:%s:%d:%d", 
	splice->name, refSeqForPSet(skipPath->beds[0]->name), nameForType(splice->type), skipPath->beds[0]->name,
	splice->strand, splice->tName, splice->tStart, splice->tEnd);

/* Loop through tissues caclulating ratio. */
for(tissueIx = 0; tissueIx < probM->colCount; tissueIx++)
    {
    double ratio = expressionRatio(event, skipPath, incPath, tissueIx);
    fprintf(ratioStatOut, "\t%.6f", ratio);
    fprintf(ratioSkipIntenOut, "\t%.6f", expressionAverage(skipPath, tissueIx));
    fprintf(ratioIncIntenOut, "\t%.6f", expressionAverage(incPath, tissueIx));
    fprintf(ratioGeneIntenOut, "\t%.6f", geneExpression(event, tissueIx));
    fprintf(ratioProbOut, "\t%.6f", genePVal(event, tissueIx));
    fprintf(skipProbOut, "\t%.6f", pvalAltPathCombExpressed(skipPath, tissueIx));
    fprintf(incProbOut, "\t%.6f", pvalAltPathCombExpressed(incPath, tissueIx));
    }
fprintf(ratioStatOut, "\n");
fprintf(ratioSkipIntenOut, "\n");
fprintf(ratioIncIntenOut, "\n");
fprintf(ratioGeneIntenOut, "\n");
fprintf(ratioProbOut, "\n");
fprintf(incProbOut, "\n");
fprintf(skipProbOut, "\n");
presThresh = tmpThresh;
}

void doEventAnalysis(struct altEvent *event, struct dMatrix *intenM,
		     struct dMatrix *probM)
/* Analyze a given event. */
{
int i = 0;
int **expressed = NULL;
int **notExpressed = NULL;
double **expression = NULL;
struct altPath *altPath = NULL;
int pathCount = slCount(event->altPathList);
int pathExpCount = 0;
int pathIx = 0;
int withProbes = 0;

/* Allocate a 2D array for expression. */
AllocArray(expressed, pathCount);
AllocArray(notExpressed, pathCount);
AllocArray(expression, pathCount);
for(i = 0; i < pathCount; i++)
    {
    AllocArray(expressed[i], probM->colCount);
    AllocArray(notExpressed[i], probM->colCount);
    AllocArray(expression[i], intenM->colCount);
    }

/* Fill in the array. */
for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next)
    {
    if(altPath->probeCount > 0)
	withProbes++;
    if(altPathExpressed(event, altPath, intenM, probM, 
			expressed, notExpressed, expression, pathIx))
	pathExpCount++;
    
    /* Output the probabilities. */
    if(pathProbabilitiesOut != NULL && event->splice->type != altOther)
	{
	outputPathProbabilities(event, altPath, intenM, probM, 
				expressed, notExpressed, expression, pathIx);
	}
    pathIx++;
    }

/* Determine our expression, alternate or otherwise. */
if(pathExpCount >= tissueExpThresh && withProbes > 1)
    event->isExpressed = TRUE;
if(pathExpCount >= tissueExpThresh * 2 && withProbes > 1)
    event->isAltExpressed = TRUE;

/* If we are outputting the beds for cassettes do it now. */
if(altCassetteBedOut && event->splice->type == altCassette && event->altPathProbeCount >= 2)
    {
    struct bed *outBed = pathToBed(event->altPathList->next->path, event->splice, -1, -1, FALSE);
    if(event->isAltExpressed)
	bedTabOutN(outBed, 6, altCassetteBedOut);
    else if(event->isExpressed)
	bedTabOutN(outBed, 6, expressedCassetteBedOut);
    else 
	bedTabOutN(outBed, 6, notExpressedCassetteBedOut);
    bedFree(&outBed);
    }

/* If we are outputting a ratio statistic do it now. */
if(onlyTwoPaths(event) && ratioStatOut)
    {
    outputRatioStats(event, probM, intenM);
    }

/* Get a background count of motifs in the cassettes. */
if(event->isAltExpressed && event->splice->type == altCassette)
    {
    for(i = 0; i < bindSiteCount; i++)
	{
	cassetteUpMotifs[i] += event->altPathList->next->motifUpCounts[i];
	cassetteDownMotifs[i] += event->altPathList->next->motifDownCounts[i];
	cassetteInsideMotifs[i] += event->altPathList->next->motifInsideCounts[i];
	}
    cassetteExonCount++;
    }

if(brainSpBedUpOut != NULL)
    outputBrainSpecificEvents(event, expressed, notExpressed, pathCount, probM);

if(tissueSpecificOut != NULL && event->isExpressed == TRUE)
    outputTissueSpecificEvents(event, expressed, notExpressed, pathCount, probM);

/* Cleanup some memory. */
for(i = 0; i < pathCount; i++)
    {
    freez(&expressed[i]);
    freez(&notExpressed[i]);
    freez(&expression[i]);
    }
freez(&notExpressed);
freez(&expressed);
freez(&expression);
}

int stringOccurance(char *needle, char *haystack)
/* How many times is needle in haystack? */
{
int count = 0;
char *rest = haystack;
touppers(needle);
touppers(haystack);
while((rest = stringIn(needle, rest)) != NULL)
    {
    count++;
    rest++;
    }
return count;
}

int countMotifs(char *seq, struct bindSite *bs)
/* Count how many times this binding site appears in this
   sequence. */
{
int i = 0, j = 0;
int count = 0;
touppers(seq);
for(i = 0; i < bs->motifCount; i++)
    {
    count += stringOccurance(bs->motifs[i],seq);
    }
return count;
}

void doMotifCassetteAnalysis(struct altEvent *event)
/* Look to see how many motifs are found around this event. */
{
struct dnaSeq *upStream = NULL, *downStream = NULL, *tmp = NULL, *inside = NULL;
struct altPath *altPath = event->altPathList->next;
struct path *path = altPath->path;
int *vPos = event->splice->vPositions;
int *v = path->vertices;
int chromStart = vPos[v[1]];
int chromEnd = vPos[v[2]];
int motifWin = optionInt("motifWin", 200);
int i = 0;
if(event->splice->strand[0] == '-')
    {
    upStream = hChromSeq(path->tName, chromEnd, chromEnd+motifWin);
    reverseComplement(upStream->dna, upStream->size);
    downStream = hChromSeq(path->tName, chromStart-motifWin, chromStart);
    reverseComplement(downStream->dna, downStream->size);
    inside = hChromSeq(path->tName, chromStart, chromEnd);
    reverseComplement(inside->dna, inside->size);
    }
else
    {
    upStream = hChromSeq(path->tName, chromStart-motifWin, chromStart);
    downStream = hChromSeq(path->tName, chromEnd, chromEnd+motifWin);
    inside = hChromSeq(path->tName, chromStart, chromEnd);
    }
AllocArray(altPath->motifUpCounts, bindSiteCount);
AllocArray(altPath->motifDownCounts, bindSiteCount);
AllocArray(altPath->motifInsideCounts, bindSiteCount);
for(i = 0; i < bindSiteCount; i++)
    {
    altPath->motifUpCounts[i] += countMotifs(upStream->dna, bindSiteArray[i]);
    altPath->motifDownCounts[i] += countMotifs(downStream->dna, bindSiteArray[i]);
    altPath->motifInsideCounts[i] += countMotifs(inside->dna, bindSiteArray[i]);
    }
dnaSeqFree(&upStream);
dnaSeqFree(&downStream);
dnaSeqFree(&inside);
}

void doMotifControlAnalysis(struct altEvent *event)
/* Look to see how many motifs are found around this event. */
{
struct dnaSeq *upStream = NULL, *downStream = NULL, *tmp = NULL, *inside = NULL;
struct altPath *altPath = event->altPathList;
struct splice *splice = event->splice;
struct path *path = altPath->path;
struct bed *bed = NULL;
int *vPos = event->splice->vPositions;
int *v = path->vertices;
int chromStart = 0;
int chromEnd = 0;
int motifWin = optionInt("motifWin", 200);
int i = 0;
int blockIx;
bed = pathToBed(path, splice, -1, -1, FALSE);
if(bed == NULL)
    return;
for(blockIx = 0; blockIx < bed->blockCount; blockIx++)
    {
    struct bed *outBed = NULL;

    /* Print out a bed of this exon. */
    AllocVar(outBed);
    outBed->chromStart = chromStart = bed->chromStart + bed->chromStarts[blockIx];
    outBed->chromEnd = chromEnd = chromStart + bed->blockSizes[blockIx];
    safef(outBed->strand,sizeof(outBed->strand),"%s", bed->strand);
    outBed->score = bed->score;
    outBed->chrom = cloneString(bed->chrom);
    outBed->name = cloneString(bed->name);
    bedTabOutN(outBed, 6, brainSpPathBedOut);
    bedFree(&outBed);

    /* Get the sequences and do motif analysis. */
    if(event->splice->strand[0] == '-')
	{
	upStream = hChromSeq(path->tName, chromEnd, chromEnd+motifWin);
	reverseComplement(upStream->dna, upStream->size);
	downStream = hChromSeq(path->tName, chromStart-motifWin, chromStart);
	reverseComplement(downStream->dna, downStream->size);
	inside = hChromSeq(path->tName, chromStart, chromEnd);
	reverseComplement(inside->dna, inside->size);
	}
    else
	{
	upStream = hChromSeq(path->tName, chromStart-motifWin, chromStart);
	downStream = hChromSeq(path->tName, chromEnd, chromEnd+motifWin);
	inside = hChromSeq(path->tName, chromStart, chromEnd);
	}
    touppers(upStream->dna);
    touppers(downStream->dna);
    touppers(inside->dna);
    if(altPath->motifUpCounts == NULL)
	{
	AllocArray(altPath->motifUpCounts, bindSiteCount);
	AllocArray(altPath->motifDownCounts, bindSiteCount);
	AllocArray(altPath->motifInsideCounts, bindSiteCount);
	}
    for(i = 0; i < bindSiteCount; i++)
	{
	int upStreamCount = countMotifs(upStream->dna, bindSiteArray[i]);
	int downStreamCount = countMotifs(downStream->dna, bindSiteArray[i]);
	int insideCount = countMotifs(inside->dna, bindSiteArray[i]);
	altPath->motifUpCounts[i] += upStreamCount;
	controlUpMotifs[i] += upStreamCount;
	altPath->motifDownCounts[i] += downStreamCount;
	controlDownMotifs[i] += downStreamCount;
	altPath->motifInsideCounts[i] += insideCount;
	controlInsideMotifs[i] += insideCount;
	}
    controlExonCount++;
    }
dnaSeqFree(&upStream);
dnaSeqFree(&downStream);
dnaSeqFree(&inside);
}

int flipScoreCmp(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct slRef *a = *((struct slRef**)va);
const struct slRef *b = *((struct slRef **)vb);
struct altEvent *aE = a->val;
struct altEvent *bE = b->val;
if( aE->flipScore > bE->flipScore)
    return -1;
else if(aE->flipScore < bE->flipScore)
    return 1;
return 0;
}

int percentUltraCmp(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct slRef *a = *((struct slRef**)va);
const struct slRef *b = *((struct slRef **)vb);
struct altEvent *aE = a->val;
struct altEvent *bE = b->val;
if(aE->percentUltra < bE->percentUltra)
    return 1;
else if(aE->percentUltra > bE->percentUltra)
    return -1;
return 0;
}

void doAnalysis(struct altEvent *eventList, struct dMatrix *intenM,
		struct dMatrix *probM)
/* How many of the alt events are expressed. */
{
struct altEvent *event = NULL;
int i = 0;
struct altPath *altPath = NULL;
int expressed = 0, altExpressed = 0;
struct slRef *ref = NULL;
for(event = eventList; event != NULL; event = event->next)
    {
    if(event->splice->type == altCassette && bindSiteCount > 0)
	doMotifCassetteAnalysis(event);
    else if(event->splice->type == altControl && bindSiteCount > 0 && 
	    !skipMotifControls)
	doMotifControlAnalysis(event);
    doEventAnalysis(event, intenM, probM);
    
    if(event->isExpressed)
	logSpliceTypeExp(event->splice->type);
    if(event->isAltExpressed)
	logSpliceTypeAltExp(event->splice->type);
    }
if(brainSpBedUpOut != NULL)
    {
    if(optionExists("flipScoreSort"))
	slSort(&brainSpEvents, flipScoreCmp);
    else
	slSort(&brainSpEvents, percentUltraCmp);
    for(ref = brainSpEvents; ref != NULL; ref = ref->next)
	{
	struct altEvent *bEvent = ref->val;
	struct altPath *bAltPath = bEvent->altPathList->next;
	struct bed *pathBed = pathToBed(bAltPath->path, bEvent->splice, -1, -1, FALSE);
	printLinks(bEvent, pathBed);
	}
    }
}

double calcPercent(double numerator, double denominator)
/* Calculate a percent checking for zero. */
{
if(denominator != 0)
    return (100.0 * numerator / denominator);
return 0;
}

double cph(double numerator, double denominator)
/* Calculate a ratio checking for zero. */
{
if(denominator != 0)
    return (numerator / denominator);
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
    fprintf(tissueSpecificOut, "%-22s\t%4d\t%4d\t%4d\t%4d\t%4d\n", tissueNames[i],
	    tissueSpecificCounts[i][altCassette], tissueSpecificCounts[i][altMutExclusive],
	    tissueSpecificCounts[i][alt5Prime],tissueSpecificCounts[i][alt3Prime],
	    tissueSpecificCounts[i][altOther]);
    }
}

void reportMotifCounts(int *up, int *down, int *in, int total)
{
int *u = up;
int *d = down;
int *i = in;
int t = total;
struct bindSite **b = bindSiteArray;
assert(up);
assert(down);
fprintf(stderr, "+----------+------------+--------------+--------------+---------------+--------------+\n");
fprintf(stderr, "| Position | %10s | %12s | %12s | %13s | %12s |\n", 
	b[0]->rnaBinder, b[1]->rnaBinder, b[2]->rnaBinder, b[3]->rnaBinder, b[4]->rnaBinder);
fprintf(stderr, "+----------+------------+--------------+--------------+---------------+--------------+\n");
fprintf(stderr, "| %-8s | %3d (%4.2f) | %5d (%4.2f) | %5d (%4.2f) | %6d (%4.2f) | %5d (%4.2f) |\n",
	"UpStream", u[0], cph(u[0],t), u[1], cph(u[1],t), u[2], cph(u[2],t), u[3], cph(u[3],t), u[4],cph(u[4],t));
fprintf(stderr, "| %-8s | %3d (%4.2f) | %5d (%4.2f) | %5d (%4.2f) | %6d (%4.2f) | %5d (%4.2f) |\n",
	"DnStream", d[0], cph(d[0],t), d[1], cph(d[1],t), d[2], cph(d[2],t), d[3], cph(d[3],t), d[4],cph(d[4],t));
fprintf(stderr, "| %-8s | %3d (%4.2f) | %5d (%4.2f) | %5d (%4.2f) | %6d (%4.2f) | %5d (%4.2f) |\n",
	"Inside  ", i[0], cph(i[0],t), i[1], cph(i[1],t), i[2], cph(i[2],t), i[3], cph(i[3],t), i[4],cph(i[4],t));
fprintf(stderr, "+----------+------------+--------------+--------------+---------------+--------------+\n");
}


void reportBrainSpCounts()
/* Print brain specific results. */
{
if(brainSpDnaUpOut != NULL)
    {
    int *bC = brainSpecificCounts;
    char *tissue = optionVal("brainTissues","Brain");
    fprintf(stderr, "%d %s Specific Events. %d %s Specific paths\n", 
	     brainSpecificEventCount, tissue, brainSpecificPathCount, tissue);
    fprintf(stderr, "+----------+---------+-------+-------+-------+-------+\n");
    fprintf(stderr, "| Cassette | MutExcl | Alt3' | Alt5' | TxEnd | Other |\n");
    fprintf(stderr, "+----------+---------+-------+-------+-------+-------+\n");
    fprintf(stderr, "| %8d | %7d | %5d | %5d | %5d | %5d |\n",
	    bC[altCassette], bC[altMutExclusive], bC[alt3Prime], bC[alt5Prime],
	    bC[alt3PrimeSoft], bC[altOther]);
    fprintf(stderr, "+----------+---------+-------+-------+-------+-------+\n");
    if(bindSiteCount != 0)
	{
	int *u = brainSpecificMotifUpCounts;
	int *d = brainSpecificMotifDownCounts;
	int *i = brainSpecificMotifInsideCounts;
	struct bindSite **b = bindSiteArray;
	fprintf(stderr,"Cassette binding sites:\n");
	reportMotifCounts(u,d,i,bC[altCassette]);
	}
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

if(bindSiteCount != 0)
    {
    int *u = cassetteUpMotifs;
    int *d = cassetteDownMotifs;
    int *i = cassetteInsideMotifs;
    struct bindSite **b = bindSiteArray;
    fprintf(stderr,"Cassette binding sites in %d exons:\n", cassetteExonCount);
    reportMotifCounts(u,d,i,cassetteExonCount);
    }

if(bindSiteCount != 0)
    {
    int *u = controlUpMotifs;
    int *d = controlDownMotifs;
    int *i = controlInsideMotifs;
    struct bindSite **b = bindSiteArray;
    fprintf(stderr,"Control binding sites in %d exons:\n", controlExonCount);
    reportMotifCounts(u,d,i,controlExonCount);
    }
}

void initLiftOver()
/* Initialize the liftOver process. */
{
char *from = optionVal("db", NULL);
char *to = optionVal("newDb", NULL);
char *chainFile = NULL;

assert(from);
hSetDb2(to);
if(to == NULL)
    errAbort("Must specify a -newDb to do a lift.");
chainFile = liftOverChainFile(from, to);
if(chainFile == NULL)
    errAbort("Couldn't load a lift chainFile for %s->%s", from, to);
liftOverHash = newHash(12);
readLiftOverMap(chainFile, liftOverHash);
}

boolean liftOverCoords(char **chrom, int *chromStart, int *chromEnd, char *strand)
/* Return TRUE and new values if values passed in can be lifted. */
{
char *newChrom = NULL;
int newStart = 0, newEnd = 0;
char newStrand;
char *msg = NULL;
boolean success = FALSE;
msg = liftOverRemapRange(liftOverHash, .95, *chrom, *chromStart, *chromEnd, *strand,
			 1.0, &newChrom, &newStart, &newEnd, &newStrand);
if(msg == NULL)
    {
    freez(&(*chrom));
    *chrom = newChrom;
    *chromStart = newStart;
    *chromEnd = newEnd;
    *strand = newStrand;
    success = TRUE;
    }
return success;
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
char *useDb = newDb;
brainSpecificStrict = optionExists("brainSpecificStrict");

AllocArray(brainSpecificCounts, altMutExclusive + 1);

if(bindSiteCount > 0)
    {
    AllocArray(brainSpecificMotifUpCounts, bindSiteCount);
    AllocArray(brainSpecificMotifDownCounts, bindSiteCount);
    AllocArray(brainSpecificMotifInsideCounts, bindSiteCount);
    }

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
	"<br>Motif Counts: Nova-1, Fox-1, PTB/nPTB, hnRNP-H/F<br>"
	"<table border=1><tr><th>Name</th><th>Size</th></tr>\n");

dyStringClear(file);
dyStringPrintf(file, "%s.frame.html", prefix);
brainSpFrameHtmlOut = mustOpen(file->string, "w");
fprintf(brainSpFrameHtmlOut, "<html><head><title>%s Specific Events.</title></head>\n"
	"<frameset cols=\"30%,70%\">\n"
	"   <frame name=\"_list\" src=\"./%s%s\">\n"
	"   <frameset rows=\"50%,50%\">\n"
	"      <frame name=\"browser\" src=\"http://%s/cgi-bin/hgTracks?db=%s&hgt.motifs=GCATG,CTCTCT,GGGG\">\n"
	"      <frame name=\"plots\" src=\"http://%s/cgi-bin/hgTracks?db=%s\">\n"
	"   </frameset>\n"
	"</frameset>\n"
	"</html>\n", optionVal("brainTissues","Brain"), prefix, ".table.html", 
	browserName, useDb, browserName, useDb );
carefulClose(&brainSpFrameHtmlOut);

dyStringFree(&file);
}

void printCdtHeader(FILE *out, struct dMatrix *dM)
{
int i = 0;
fprintf(out, "YORF\tNAME");
for(i = 0; i < dM->colCount; i++)
    {
    fprintf(out, "\t%s", dM->colNames[i]);
    }
fprintf(out, "\n");
}

void initRatioStats(struct dMatrix *intenM)
/* Setupt a file to output ratio statistics to. */
{
int i = 0;
char *outputRatioStats = optionVal("outputRatioStats", NULL);
struct dyString *buff = NULL;
assert(outputRatioStats);
assert(intenM);
buff = newDyString(strlen(outputRatioStats)+30);
ratioStatOut = mustOpen(outputRatioStats, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.skip.intensity", outputRatioStats);
ratioSkipIntenOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.inc.intensity", outputRatioStats);
ratioIncIntenOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.gene.prob", outputRatioStats);
ratioProbOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.gene.intensity", outputRatioStats);
ratioGeneIntenOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.inc.prob", outputRatioStats);
incProbOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.skip.prob", outputRatioStats);
skipProbOut = mustOpen(buff->string, "w");

for(i = 0; i < intenM->colCount - 1; i++)
    {
    fprintf(ratioStatOut, "%s\t", intenM->colNames[i]);
    fprintf(ratioSkipIntenOut, "%s\t", intenM->colNames[i]);
    fprintf(ratioIncIntenOut, "%s\t", intenM->colNames[i]);
    fprintf(ratioGeneIntenOut, "%s\t", intenM->colNames[i]);
    fprintf(ratioProbOut, "%s\t", intenM->colNames[i]);
    fprintf(skipProbOut, "%s\t", intenM->colNames[i]);
    fprintf(incProbOut, "%s\t", intenM->colNames[i]);
    }
fprintf(ratioStatOut, "%s\n", intenM->colNames[i]);
fprintf(ratioSkipIntenOut, "%s\n", intenM->colNames[i]);
fprintf(ratioIncIntenOut, "%s\n", intenM->colNames[i]);
fprintf(ratioGeneIntenOut, "%s\n", intenM->colNames[i]);
fprintf(ratioProbOut, "%s\n", intenM->colNames[i]);
fprintf(skipProbOut, "%s\n", intenM->colNames[i]);
fprintf(incProbOut, "%s\n", intenM->colNames[i]);
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

/* If we're doing path probabilities print some headers. */
if(pathProbabilitiesOut != NULL)
    {
    printCdtHeader(pathProbabilitiesOut, probM);
    printCdtHeader(pathExpressionOut, intenM);
    }

if(optionExists("tissueSpecific"))
    initTissueSpecific(probM->colCount);

if(optionExists("outputRatioStats"))
   initRatioStats(intenM);

warn("Filling in event data.");
fillInEventData(eventList, intenM, probM);
warn("Doing analysis");
doAnalysis(eventList, intenM, probM);
reportEventCounts();
reportBrainSpCounts();
if(intronsConsCounted > 0)
    warn("%d of %d introns are heavily conserved %.2f%%", 
	 intronsConserved, intronsConsCounted, (100.0 * intronsConserved)/intronsConsCounted); 
if(tissueSpecificOut)
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

boolean unitTestFunct(struct unitTest *test, boolean passed, 
			 struct dyString *error, char *errorMsg)
/* Wrapper around printing things to the error. */
{
assert(error);
if(!passed)
    dyStringPrintf(error, "%s, ", errorMsg);
return passed;
}


boolean testStringOccurance(struct unitTest *test)
/* Test how many times a motif is counted. */
{
int count = 0;

if(stringOccurance(cloneString("ggGG"), cloneString("nnnnnggggg")) != 2)
    return FALSE;

if(stringOccurance(cloneString("ggGG"), cloneString("nnnnngggggnnnn")) != 2)
    return FALSE;

if(stringOccurance(cloneString("ggGG"), cloneString("gggggnnnn")) != 2)
    return FALSE;

if(stringOccurance(cloneString("gcATG"), cloneString("GcatGcattg")) != 1)
    return FALSE;

return TRUE;
}

boolean testCdfChiSqP(struct unitTest *test)
/* Test the gsl results for the chisq function. Gold values from R.*/
{
double result = 0;
char buff[256];

/* Run the function for som test values, print to 
   a buffer to get the precision / rounding right. */
result = gsl_cdf_chisq_P(-2*log(.9),2);
safef(buff, sizeof(buff), "%.7f", result);
if(differentString(buff, "0.1000000"))
    return FALSE;

result = gsl_cdf_chisq_P(-2*log(.1),2);
safef(buff, sizeof(buff), "%.7f", result);
if(differentString(buff, "0.9000000"))
    return FALSE;

result = gsl_cdf_chisq_P(-2*(log(.1)+log(.9)+log(.5)),6);
safef(buff, sizeof(buff), "%.7f", result);
if(differentString(buff, "0.5990734"))
    return FALSE;

result = gsl_cdf_chisq_P(-2*(log(.8)+log(.8)),4);
safef(buff, sizeof(buff), "%.8f", result);
if(differentString(buff, "0.07437625"))
    return FALSE;
return TRUE;
}

boolean testPathContains(struct unitTest *test)
/* Test the path contains block functionality. */
{
struct splice *ndr2 = ndr2CassTest();
struct path *skipPath = NULL, *incPath = NULL;
struct dyString *error = newDyString(128);
boolean result = TRUE, currentResult = TRUE;
skipPath = ndr2->paths;
incPath = ndr2->paths->next;

if(pathContainsIntron(ndr2, skipPath, "chr14", 43771385+15, 43772095-15, "-") != TRUE)
    currentResult = FALSE;
if(pathContainsIntron(ndr2, skipPath, "chr14", 43771385+15, 43771715-15, "-") != FALSE)
    currentResult = FALSE;
if(pathContainsIntron(ndr2, skipPath, "chr14", 43771727+15, 43772095-15, "-") != FALSE)
    currentResult = FALSE;
if(currentResult == FALSE)
    {
    dyStringPrintf(error, "Problem with placing introns on skip path, ");
    result = FALSE;
    }

currentResult = TRUE;
if(pathContainsIntron(ndr2, incPath, "chr14", 43771385+15, 43772095-15, "-") != FALSE)
    currentResult = FALSE;
if(pathContainsIntron(ndr2, incPath, "chr14", 43771385+15, 43771715-15, "-") != TRUE)
    currentResult = FALSE;
if(pathContainsIntron(ndr2, incPath, "chr14", 43771727+15, 43772095-15, "-") != TRUE)
    currentResult = FALSE;
if(currentResult == FALSE)
    {
    dyStringPrintf(error, "Problem with placing introns on skip path, ");
    result = FALSE;
    }

currentResult = TRUE;
if(pathContainsBlock(ndr2, incPath,  "chr14", 43771727, 43771727+15, "-", TRUE) != TRUE)
    currentResult = FALSE;
if(pathContainsBlock(ndr2, incPath,  "chr14", 43771703, 43771735, "-", TRUE) != TRUE)
    currentResult = FALSE;
if(pathContainsBlock(ndr2, skipPath,  "chr14", 43771703, 43771735, "-", TRUE) != FALSE)
    currentResult = FALSE;
if(currentResult == FALSE)
    {
    dyStringPrintf(error, "Problem placing blocks with all bases, ");
    result = FALSE;
    }

currentResult = TRUE;
if(pathContainsBlock(ndr2, incPath,  "chr14", 43771727, 43771727+16, "-", TRUE) != FALSE)
    currentResult = FALSE;
if(pathContainsBlock(ndr2, incPath,  "chr14", 43771727, 43771727+16, "-", FALSE) != TRUE)
    currentResult = FALSE;
if(currentResult == FALSE)
    {
    dyStringPrintf(error, "Problem placing blocks with not all bases, ");
    result = FALSE;
    }
    
test->errorMsg = cloneString(error->string);
dyStringFree(&error);
return result;
}

boolean testPathContainsBed(struct unitTest *test)
/* Test to see if a path contains the right stuff. */
{
struct splice *ndr2 = ndr2CassTest();
struct bed *bedList = ndr2BedTest(), *bed = NULL;
/* 1 include, 2 skip, 3 include, 4 gene. */
struct path *skipPath = NULL, *incPath = NULL;
boolean result = TRUE;
struct dyString *error = newDyString(128);
skipPath = ndr2->paths;
incPath = ndr2->paths->next;

/* First include. */
bed = bedList; 
if(pathContainsBed(ndr2, skipPath, bed, TRUE) != FALSE)
    {
    result = FALSE;
    dyStringPrintf(error, "Matching include bed to skip, ");
    }
if(pathContainsBed(ndr2, incPath, bed, TRUE) != TRUE)
    {
    result = FALSE;
    dyStringPrintf(error, "Not matching include bed to include, ");
    }

/* Skip probe set. */
bed = bed->next;
if(pathContainsBed(ndr2, skipPath, bed, TRUE) != TRUE)
    {
    result = FALSE;
    dyStringPrintf(error, "Not matching skip bed to skip, ");
    }
if(pathContainsBed(ndr2, incPath, bed, TRUE) != FALSE)
    {
    result = FALSE;
    dyStringPrintf(error, "Matching skip bed to include, ");
    }

/* Second include. */
bed = bed->next;
if(pathContainsBed(ndr2, skipPath, bed, TRUE) != FALSE)
    {
    result = FALSE;
    dyStringPrintf(error, "Matching 2nd include bed to skip, ");
    }
if(pathContainsBed(ndr2, incPath, bed, TRUE) != TRUE)
    {
    result = FALSE;
    dyStringPrintf(error, "Not matching 2nd include bed to include, ");
    }

/* Gene probe set. */
bed = bed->next;
if(pathContainsBed(ndr2, skipPath, bed, FALSE) != FALSE)
    {
    result = FALSE;
    dyStringPrintf(error, "Matching gene set to skip path,  ");
    }

if(pathContainsBed(ndr2, incPath, bed, FALSE) != FALSE)
    {
    result = FALSE;
    dyStringPrintf(error, "Matching gene set to include path,  ");
    }
test->errorMsg = cloneString(error->string);
dyStringFree(&error);
return result;
}

struct altEvent *setupNdr2AltEvent()
/* Setup an ndr2 fake event for testing. */
{
struct splice *splice = ndr2CassTest();
struct altEvent *event = NULL;
struct path *path = NULL;
struct altPath *altPath = NULL;
struct dMatrix *probM = ndr2BedTestMat(TRUE);
struct dMatrix *intenM = ndr2BedTestMat(FALSE);
double expression = 0;

AllocVar(event);
event->splice = splice;
AllocArray(event->geneExpVals, 1);
AllocArray(event->genePVals, 1);
event->geneProbeCount = 1;
event->altPathProbeCount = 2;
event->geneExpVals[0] = intenM->matrix[3];
event->genePVals[0] = probM->matrix[3];

AllocVar(altPath);
altPath->path = splice->paths;
altPath->probeCount = 1;
AllocArray(altPath->expVals, altPath->probeCount);
altPath->expVals[0] =  intenM->matrix[1];
AllocArray(altPath->pVals, altPath->probeCount);
altPath->pVals[0] = probM->matrix[1];
slAddHead(&event->altPathList, altPath);
    
AllocVar(altPath);
altPath->path = splice->paths->next;
altPath->probeCount = 2;
AllocArray(altPath->expVals, altPath->probeCount);
altPath->expVals[0] = intenM->matrix[0];
altPath->expVals[1] = intenM->matrix[2];
AllocArray(altPath->pVals, altPath->probeCount);
altPath->pVals[0] = probM->matrix[0];
altPath->pVals[1] = probM->matrix[2];
slAddHead(&event->altPathList, altPath);

slReverse(&event->altPathList);
return event;
}

boolean testAltPathProbesExpressed(struct unitTest *test)
/* Test to see if presense abs software is calling. */
{
struct altEvent *event = setupNdr2AltEvent();
struct dyString *error = newDyString(128);
double expression = 0;
boolean result = TRUE;
useMaxProbeSet = TRUE;
if(altPathProbesExpressed(event, event->altPathList->next, 1, 0, &expression) != FALSE)
    {
    result = FALSE;
    dyStringPrintf(error, "Max probe set calling false positive, ");
    }
if(altPathProbesExpressed(event, event->altPathList, 0, 3, &expression) != TRUE)
    {
    result = FALSE;
    dyStringPrintf(error, "Max probe set calling positive false, ");
    }

useMaxProbeSet = FALSE;
useComboProbes = TRUE;
if(altPathProbesExpressed(event, event->altPathList->next, 1, 0, &expression) != TRUE)
    {
    result = FALSE;
    dyStringPrintf(error, "Combo probes not combining correctly, ");
    }
test->errorMsg = cloneString(error->string);
dyStringFree(&error);
return result;
}

boolean sameDouble(double d1, double d2)
/* Check to see if these are the same doubles to 6 sig digits. */
{
char buff1[128], buff2[128];
safef(buff1, sizeof(buff1), "%.6f", d1);
safef(buff2, sizeof(buff2), "%.6f", d2);
return sameString(buff1, buff2);
}

boolean testExpressionRatio(struct unitTest *test)
/* Test to see if the ratios are calculated properly for expresionRatio() */
{
struct altEvent *event = setupNdr2AltEvent();
struct dyString *error = newDyString(128);
double expression = 0;
boolean result = TRUE;
boolean current = FALSE;
struct altPath *skipPath = event->altPathList;
struct altPath *incPath = event->altPathList->next;
double tmpThresh = presThresh;
presThresh = absThresh;

current = sameDouble(expressionRatio(event, skipPath, incPath, 0), 1);
result &= unitTestFunct(test, current, error, "Averaging");

current = sameDouble(expressionRatio(event, skipPath, incPath, 1), -1);
result &= unitTestFunct(test, current, error, "Not expressed");

current = sameDouble(expressionRatio(event, skipPath, incPath, 2), 2);
result &= unitTestFunct(test, current, error, "Low expressed");

current = sameDouble(expressionRatio(event, skipPath, incPath, 3), .5);
result &= unitTestFunct(test, current, error, "Averaging");

presThresh = tmpThresh;
return result;
}

boolean testBrainSpecific(struct unitTest *test)
/* Test to see if ndr2 is brain specific (which it should be). */
{
struct altEvent *event = setupNdr2AltEvent();
boolean result = FALSE;
int **expressed = NULL;
int **notExpressed = NULL;
double **expression = NULL;
struct altPath *altPath = NULL;
int pathCount = slCount(event->altPathList);
int pathIx = 0;
int pathExpCount = 0;
int i = 0;
struct dMatrix *probM = ndr2BedTestMat(TRUE);
struct dMatrix *intenM = ndr2BedTestMat(FALSE);
/* Allocate a 2D array for expression. */
AllocArray(expressed, pathCount);
AllocArray(notExpressed, pathCount);
AllocArray(expression, pathCount);
for(i = 0; i < pathCount; i++)
    {
    AllocArray(expressed[i], probM->colCount);
    AllocArray(notExpressed[i], probM->colCount);
    AllocArray(expression[i], intenM->colCount);
    }
for(altPath = event->altPathList; altPath != NULL; altPath = altPath->next)
    {
    if(altPathExpressed(event, altPath, intenM, probM, 
			expressed, notExpressed, expression, pathIx))
	pathExpCount++;
    pathIx++;
    }
result = brainSpecific(event, 1, expressed, notExpressed, 2, probM);
if(result != TRUE)
    test->errorMsg = cloneString("Not showing brain expressed.");
/* Cleanup some memory. */
for(i = 0; i < pathCount; i++)
    {
    freez(&expressed[i]);
    freez(&notExpressed[i]);
    freez(&expression[i]);
    }
freez(&notExpressed);
freez(&expressed);
freez(&expression);
return result;
}

void initTests()
{
struct unitTest *test = NULL;

/* Testing stat routines. */
AllocVar(test);
test->test = testCdfChiSqP;
test->description = "GSL ChiSq function";
slAddHead(&tests, test);

/* Testing motif matching. */
AllocVar(test);
test->test = testStringOccurance;
test->description = "Motif searching";
slAddHead(&tests, test);

/* Testing finding blocks on paths. */
AllocVar(test);
test->test = testPathContains;
test->description = "Match introns and exon blocks to paths";
slAddHead(&tests, test);

/* Testing finding blocks on paths. */
AllocVar(test);
test->test = testPathContainsBed;
test->description = "Match beds to paths";
slAddHead(&tests, test);

/* Testing finding blocks on paths. */
AllocVar(test);
test->test = testAltPathProbesExpressed;
test->description = "Call paths as expressed or not.";
slAddHead(&tests, test);

/* Testing expression ratio generation. */
AllocVar(test);
test->test = testExpressionRatio;
test->description = "Generate expression ratios.";
slAddHead(&tests, test);

/* Testing brain specificity. */
AllocVar(test);
test->test = testBrainSpecific;
test->description = "Brain Specific detection.";
slAddHead(&tests, test);

slReverse(&tests);
}

void runTests()
{
struct unitTest *test, *testNext, *passed = NULL, *failed = NULL;
boolean passedAll = TRUE;
int passCount = 0, *failCount = 0;
for(test = tests; test != NULL; test = testNext)
    {
    testNext = test->next;
    if(test->test(test) != FALSE)
	{
	passCount++;
	slAddHead(&passed, test);
	}
    else
	{
	slAddHead(&failed, test);
	passedAll = FALSE;
	failCount++;
	}
    }
if(passedAll)
    {
    fprintf(stdout, "Passed all %d tests.\n", passCount);
    exit(0);
    }
else
    {
    fprintf(stdout, "Failed tests:\n");
    for(test = failed; test != NULL; test = test->next)
	{
	fprintf(stdout, "%-40s\tFAILED\t%s\n", test->description,
		test->errorMsg == NULL ? "" : test->errorMsg);
	}
    exit(1);
    }
}

void initCassetteBedsOut()
{
struct dyString *buff = NULL;
char *prefix = optionVal("cassetteBeds", NULL);
assert(prefix);
buff = newDyString(strlen(prefix)+20);

dyStringClear(buff);
dyStringPrintf(buff, "%s.altExpressed.bed", prefix);
altCassetteBedOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.expressed.bed", prefix);
expressedCassetteBedOut = mustOpen(buff->string, "w");

dyStringClear(buff);
dyStringPrintf(buff, "%s.notExpressed.bed", prefix);
notExpressedCassetteBedOut = mustOpen(buff->string, "w");
}

void setOptions()
/* Set up some options. */
{
char *selectedPValues = optionVal("selectedPValues", NULL);

if(optionExists("doTests"))
    {
    initTests();
    runTests();
    }
tissueExpThresh = optionInt("tissueExpThresh", 1);
otherTissueExpThres = optionInt("otherTissueExpThres", 1);
useMaxProbeSet = optionExists("useMaxProbeSet");
db = optionVal("db", NULL);
browserName = optionVal("browser", "hgwdev-sugnet.cse.ucsc.edu");
useComboProbes = optionExists("combinePSets");
useExonBedsToo = optionExists("useExonBeds");
skipMotifControls = optionExists("skipMotifControls");
newDb = optionVal("newDb", db);
if(optionExists("newDb"))
 initLiftOver();


if(optionExists("psToRefGene"))
    initPsToRefGene();

if(optionExists("splicePVals"))
    initSplicePVals();

if(optionExists("cassetteBeds"))
    initCassetteBedsOut();

if(selectedPValues != NULL)
    {
    char *selectedPValFile = optionVal("selectedPValFile", NULL);
    char buff[2048];

    if(selectedPValFile == NULL)
	errAbort("Must specify a selectedPValFile when enabling selectedPValues");
    readSelectedList(selectedPValues);
    pathProbabilitiesOut = mustOpen(selectedPValFile, "w");
    safef(buff, sizeof(buff), "%s.bed", selectedPValFile);
    pathProbabilitiesBedOut = mustOpen(buff, "w");
    safef(buff, sizeof(buff), "%s.intensity", selectedPValFile);
    pathExpressionOut = mustOpen(buff, "w");
    }

if(useMaxProbeSet)
    warn("Using max value from all probe sets in a path.");
/* Set up the datbase. */
db = optionVal("db", NULL);
if(db != NULL)
    hSetDb(db);
else
    errAbort("Must specify database.");
initBindSiteList();
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
