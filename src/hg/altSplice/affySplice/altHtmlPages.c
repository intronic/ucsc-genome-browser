/* altHtmlPages.c - Program to create a web based browser
   of altsplice predicitions. */

#include "common.h"
#include "bed.h"
#include "hash.h"
#include "options.h"
#include "chromKeeper.h"
#include "binRange.h"
#include "obscure.h"
#include "hash.h"
#include "linefile.h"
#include "dystring.h"
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics_double.h>

struct junctSet 
/* A set of beds with a common start or end. */
{
    struct junctSet *next;   /* Next in list. */
    char *chrom;                /* Chromosome. */
    int chromStart;             /* Smallest start. */
    int chromEnd;               /* Largest end. */
    char *name;                 /* Name of junction. */
    int junctCount;             /* Number of junctions in set. */
    char strand[2];             /* + or - depending on which strand. */
    int genePSetCount;          /* Number of gene probe sets. */
    char **genePSets;           /* Gene probe set name. */
    char *hName;                /* Human name for locus or affy id if not present. */
    int maxJunctCount;          /* Maximum size of bedArray. */
    char **junctPSets;          /* Beds in the cluster. */
    int junctDupCount;          /* Number of redundant junctions in set. */
    int maxJunctDupCount;       /* Maximum size of bedDupArray. */
    char **dupJunctPSets;       /* Redundant beds in the cluster. */
    int junctUsedCount;         /* Number of juction sets used. */
    char **junctUsed;           /* Names of actual junction probe sets used. */
    boolean cassette;           /* Is cassette exon set? */
    char *exonPsName;           /* Cassette exon probe set if available. */
    double **junctProbs;        /* Matrix of junction probabilities, rows are junctUsed. */ 
    double **geneProbs;         /* Matrix of gene set probabilities, rows are genePSets. */
    double *exonPsProbs;        /* Exon probe set probabilities if cassette. */
    double **junctIntens;       /* Matrix of junction intensities, rows are junctUsed. */ 
    double **geneIntens;        /* Matrix of gene set intensities, rows are genePSets. */
    double *exonPsIntens;       /* Exon probe set intensities if cassette. */
    boolean expressed;          /* TRUE if one form of this junction set is expressed. */
    boolean altExpressed;       /* TRUE if more than one form of this junction set is expressed. */
    double score;               /* Score of being expressed overall. */
    double exonCorr;            /* Correlation of exon probe set with include junction if appropriate. */
    double probCorr;            /* Correlation of exon probe set prob and include probs. */
    double exonSjPercent;       /* Percentage of includes confirmed by exon probe set. */
    double sjExonPercent;       /* Percentage of exons confirmed by sj probe set. */
    int exonAgree;              /* Number of times exon agrees with matched splice junction */
    int exonDisagree;           /* Number of times exon disagrees with matched splice junction. */
    int sjAgree;                /* Number of times sj agrees with matched exon */
    int sjDisagree;             /* Number of times sj disagrees with matched exon. */
    int sjExp;                  /* Number of times sj expressed. */
    int exonExp;                /* Number of times exon expressed. */
};

struct resultM 
/* Wrapper for a matrix of results. */
{
    struct hash *nameIndex;  /* Hash with integer index for each row name. */
    int colCount;            /* Number of columns in matrix. */
    char **colNames;         /* Column Names for matrix. */
    int rowCount;            /* Number of rows in matrix. */
    char **rowNames;         /* Row names for matrix. */
    double **matrix;         /* The actual data for the resultM. */
};


static struct optionSpec optionSpecs[] = 
/* Our acceptable options to be called with. */
{
    {"help", OPTION_BOOLEAN},
    {"junctFile", OPTION_STRING},
    {"probFile", OPTION_STRING},
    {"intensityFile", OPTION_STRING},
    {"bedFile", OPTION_STRING},
    {"sortByExonCorr", OPTION_BOOLEAN},
    {"sortByExonPercent", OPTION_BOOLEAN},
    {"htmlPrefix", OPTION_STRING},
    {"hybeFile", OPTION_STRING},
    {"presThresh", OPTION_FLOAT},
    {"absThresh", OPTION_FLOAT},
    {"strictDisagree", OPTION_BOOLEAN},
    {"exonStats", OPTION_STRING},
    {"spreadSheet", OPTION_STRING},
    {"doSjRatios", OPTION_BOOLEAN},
    {"ratioFile", OPTION_STRING},
    {NULL, 0}
};

static char *optionDescripts[] = 
/* Description of our options for usage summary. */
{
    "Display this message.",
    "File containing junctionSets from plotAndCountAltsMultGs.R log.",
    "File containing summarized probability of expression for each probe set.",
    "File containing expression values for each probe set.",
    "File containing beds for each probe set.",
    "Flag to sort by exon correlation when present.",
    "Flag to sort by exon percent agreement with splice junction when present.",
    "Prefix for html files, i.e. <prefix>lists.html and <prefix>frame.html",
    "File giving names and ids for hybridizations.",
    "Optional threshold at which to call expressed.",
    "Optional threshold at which to call not expressed.",
    "Call disagreements only if below absense threshold instead of below present thresh.",
    "File to output confirming exon stats to.",
    "File to output spreadsheet format picks to.",
    "[optional] Calculate ratios of sj to other sj in same junction set.",
    "[optional] File to store sjRatios in."
};

double presThresh = 0;     /* Probability above which we consider
			    * something expressed. */
double absThresh = 0;      /* Probability below which we consider
			    * something not expressed. */
double disagreeThresh = 0; /* Probability below which we consider
			    * something to disagree. */
FILE *exonPsStats = NULL;  /* File to output exon probe set 
			      confirmation stats into. */
void usage()
/** Print usage and quit. */
{
int i=0;
warn("altHtmlPages - Program to create a web based browser\n"
     "of altsplice predicitions. Now extended to do some intitial\n"
     "analysis as well.\n"
     "options are:");
for(i=0; i<ArraySize(optionSpecs) -1; i++)
    fprintf(stderr, "  -%s -- %s\n", optionSpecs[i].name, optionDescripts[i]);
errAbort("\nusage:\n   ");
}

struct resultM *readResultMatrix(char *fileName)
/* Read in an R style result table. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct resultM *rM = NULL;
char **words = NULL;
int colCount=0, i=0;
char *line = NULL;
double **M = NULL;
int rowCount =0;
int rowMax = 1000;
char **rowNames = NULL;
char **colNames = NULL;
struct hash *iHash = newHash(12);
char buff[256];
char *tmp;
/* Get the headers. */
lineFileNextReal(lf, &line);
while(line[0] == '\t')
    line++;
colCount = chopString(line, "\t", NULL, 0);
AllocArray(colNames, colCount);
AllocArray(words, colCount+1);
AllocArray(rowNames, rowMax);
AllocArray(M, rowMax);
tmp = cloneString(line);
chopByChar(tmp, '\t', colNames, colCount);
while(lineFileNextRow(lf, words, colCount+1))
    {
    if(rowCount+1 >=rowMax)
	{
	ExpandArray(rowNames, rowMax, 2*rowMax);
	ExpandArray(M, rowMax, 2*rowMax);
	rowMax = rowMax*2;
	}
    safef(buff, sizeof(buff), "%s", words[0]);
    tmp=strchr(buff,':');
    if(tmp != NULL)
	{
	assert(tmp);
	tmp=strchr(tmp+1,':');
	assert(tmp);
	tmp[0]='\0';
	}
    hashAddInt(iHash, buff, rowCount);
    rowNames[rowCount] = cloneString(words[0]);
    AllocArray(M[rowCount], colCount);
    for(i=1; i<=colCount; i++) /* Starting at 1 here as name is 0. */
	{
	M[rowCount][i-1] = atof(words[i]);
	}
    rowCount++;
    }
AllocVar(rM);
rM->nameIndex = iHash;
rM->colCount = colCount;
rM->colNames = colNames;
rM->rowCount = rowCount;
rM->rowNames = rowNames;
rM->matrix = M;
return rM;
}

struct junctSet *junctSetLoad(char *row[], int colCount)
/* Load up a junct set. */
{
struct junctSet *js = NULL;
int index = 1;
int count = 0;
AllocVar(js);
if(colCount > 14)
    index = 1;
else
    index = 0;
js->chrom = cloneString(row[index++]);
js->chromStart = sqlUnsigned(row[index++]);
js->chromEnd = sqlUnsigned(row[index++]);
js->name = cloneString(row[index++]);
js->junctCount = js->maxJunctDupCount = sqlUnsigned(row[index++]);
js->strand[0] = row[index++][0];
js->genePSetCount = sqlUnsigned(row[index++]);
sqlStringDynamicArray(row[index++], &js->genePSets, &count);
js->hName = cloneString(row[index++]);
sqlStringDynamicArray(row[index++], &js->junctPSets, &js->maxJunctCount);
js->junctDupCount = sqlUnsigned(row[index++]);
sqlStringDynamicArray(row[index++], &js->dupJunctPSets, &count);
js->cassette = atoi(row[index++]);
js->exonPsName = cloneString(row[index++]);
if(colCount != 14) /* If the junctions used fields not present fill them in later. */
    {
    js->junctUsedCount = sqlUnsigned(row[index++]);
    sqlStringDynamicArray(row[index++], &js->junctUsed, &count);
    }
return js;
}

struct junctSet *junctSetLoadAll(char *fileName)
/* Load all of the junct sets out of a fileName. */
{
struct junctSet *list = NULL, *el = NULL;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
int numFields = 0;
char *line = NULL;
char **row = NULL;
lineFileNextReal(lf, &line);
numFields = chopByWhite(line, NULL, 0);
assert(numFields);
lineFileReuse(lf);
AllocArray(row, numFields);
while(lineFileNextRowTab(lf, row, numFields))
    {
    el = junctSetLoad(row, numFields);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&lf);
return list;
}

void fillInProbs(struct junctSet *jsList, struct resultM *probM)
/* Fill in the probability of expression for each junctSet's junction
   and gene probe sets. */
{
struct junctSet *js = NULL;
int i = 0, j = 0;
for(js = jsList; js != NULL; js = js->next)
    {
    /* Allocate junction probability matrix arrays. */
    AllocArray(js->junctProbs, js->junctUsedCount);
    for(i = 0; i < js->junctUsedCount; i++)
	AllocArray(js->junctProbs[i], probM->colCount);
    
    /* Fill in the values. */
    for(i = 0; i < js->junctUsedCount; i++)
	{
	int row = hashIntValDefault(probM->nameIndex, js->junctUsed[i], -1);
	if(row != -1)
	    {
	    for(j = 0; j < probM->colCount; j++) 
		js->junctProbs[i][j] = probM->matrix[row][j];
	    }
	else
	    {
	    for(j = 0; j < probM->colCount; j++) 
		js->junctProbs[i][j] = -1;
	    }
	}

    /* Allocate memory for gene sets. */
    AllocArray(js->geneProbs, js->genePSetCount);
    for(i = 0; i < js->genePSetCount; i++)
	AllocArray(js->geneProbs[i], probM->colCount);

    /* Fill in the values. */
    for(i = 0; i < js->genePSetCount; i++)
	{
	int row = hashIntValDefault(probM->nameIndex, js->genePSets[i], -1);
	if(row != -1)
	    {
	    for(j = 0; j < probM->colCount; j++) 
		js->geneProbs[i][j] = probM->matrix[row][j];
	    }
	else
	    {
	    for(j = 0; j < probM->colCount; j++) 
		js->geneProbs[i][j] = -1;
	    }
	}

    /* Allocate the cassette exon probabilities if present. */
    if(js->cassette && differentWord(js->exonPsName, "NA"))
	{
	int row = hashIntValDefault(probM->nameIndex, js->exonPsName, -1);
	if(row != -1)
	    {
	    AllocArray(js->exonPsProbs, probM->colCount);
	    for(j = 0; j < probM->colCount; j++)
		js->exonPsProbs[j] = probM->matrix[row][j];
	    }
	}
    }
}

void fillInIntens(struct junctSet *jsList, struct resultM *intenM)
/* Fill in the intenability of expression for each junctSet's junction
   and gene probe sets. */
{
struct junctSet *js = NULL;
int i = 0, j = 0;
for(js = jsList; js != NULL; js = js->next)
    {
    /* Allocate junction intenability matrix arrays. */
    AllocArray(js->junctIntens, js->junctUsedCount);
    for(i = 0; i < js->junctUsedCount; i++)
	AllocArray(js->junctIntens[i], intenM->colCount);
    
    /* Fill in the values. */
    for(i = 0; i < js->junctUsedCount; i++)
	{
	int row = hashIntValDefault(intenM->nameIndex, js->junctUsed[i], -1);
	if(row != -1)
	    {
	    for(j = 0; j < intenM->colCount; j++) 
		js->junctIntens[i][j] = intenM->matrix[row][j];
	    }
	else
	    {
	    for(j = 0; j < intenM->colCount; j++) 
		js->junctIntens[i][j] = -1;
	    }
	}

    /* Allocate memory for gene sets. */
    AllocArray(js->geneIntens, js->genePSetCount);
    for(i = 0; i < js->genePSetCount; i++)
	AllocArray(js->geneIntens[i], intenM->colCount);

    /* Fill in the values. */
    for(i = 0; i < js->genePSetCount; i++)
	{
	int row = hashIntValDefault(intenM->nameIndex, js->genePSets[i], -1);
	if(row != -1)
	    {
	    for(j = 0; j < intenM->colCount; j++) 
		js->geneIntens[i][j] = intenM->matrix[row][j];
	    }
	else
	    {
	    for(j = 0; j < intenM->colCount; j++) 
		js->geneIntens[i][j] = -1;
	    }
	}

    /* Allocate the cassette exon intenabilities if present. */
    if(js->cassette && differentWord(js->exonPsName, "NA"))
	{
	int row = hashIntValDefault(intenM->nameIndex, js->exonPsName, -1);
	if(row != -1)
	    {
	    AllocArray(js->exonPsIntens, intenM->colCount);
	    for(j = 0; j < intenM->colCount; j++)
		js->exonPsIntens[j] = intenM->matrix[row][j];
	    }
	}
    }
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

double calcMinCorrelation(struct junctSet *js, int *expressed, int tissueCount, int colCount)
/* Loop through all combinations of junction sets that are expressed
   and calculate the minimum correlation between the two in tissues
   if they are expressed in at least 1. */
{
double minCorr = 2; /* Correlation always <= 1, 2 is a good max. */
double corr = 0;
int i = 0, j = 0;
for(i = 0; i < js->junctUsedCount; i++)
    {
    /* If there isn't any expression for this junction don't bother. */
    if(expressed[i] == 0)
	continue;
    for(j = 0; j < js->junctUsedCount; j++)
	{
	/* Corralation when i == j is 1, don't bother. */
	if(i == j)
	    continue;

	/* If there isn't any expression for this junction don't bother. */
	if(expressed[j] == 0)
	    continue;

	/* Calculate correlation. */
	corr = correlation(js->junctIntens[i], js->junctIntens[j], colCount);
	if(minCorr > corr)
	    minCorr = corr;
	}
    }
return minCorr;
}

int includeJsIx(struct junctSet *js, struct hash *bedHash)
/* Return the index of the include junction for a cassette exon. */
{
int includeIx = 0;
struct bed *bed1 = NULL, *bed2 = NULL;

assert(js->cassette);
assert(js->junctUsedCount == 2);

bed1 = hashMustFindVal(bedHash, js->junctUsed[0]);
bed2 = hashMustFindVal(bedHash, js->junctUsed[1]);
if(bed1->chromEnd - bed1->chromStart > bed2->chromEnd - bed2->chromStart)
    includeIx = 1;
else
    includeIx = 0;
return includeIx;
}

boolean junctionExpressed(struct junctSet *js, int colIx, int junctIx)
/* Is the junction at junctIx expressed above the 
   threshold in colIx? */
{
boolean geneExp = FALSE;
boolean junctExp = FALSE;
int i = 0;
for(i = 0; i < js->genePSetCount; i++)
    {
    if(js->geneProbs[i][colIx] >= presThresh)
	geneExp = TRUE;
    }
if(geneExp && js->junctProbs[junctIx][colIx] >= presThresh)
    junctExp = TRUE;
return junctExp;
}

boolean exonExpressed(struct junctSet *js, int colIx)
/* Is the junction at junctIx expressed above the 
   threshold in colIx? */
{
boolean geneExp = FALSE;
boolean exonExp = FALSE;
int i = 0;
for(i = 0; i < js->genePSetCount; i++)
    {
    if(js->geneProbs[i][colIx] >= presThresh)
	geneExp = TRUE;
    }
if(geneExp && js->exonPsProbs[colIx] >= presThresh)
    exonExp = TRUE;
return exonExp;
}


double calcExonPercent(struct junctSet *js, struct hash *bedHash, struct resultM *probM)
/* Calculate how often the exon probe set is called expressed
   when probe set is. */
{
int includeIx = includeJsIx(js, bedHash);
int both = 0;
int jsOnly = 0;
double percent = 0;
int i = 0;
for(i = 0; i < probM->colCount; i++)
    {
    if(junctionExpressed(js, i, includeIx) &&
       js->exonPsProbs[i] >= presThresh)
	{
	both++;
	jsOnly++;
	js->exonAgree++;
	}
    else if(junctionExpressed(js, i, includeIx) &&
	    js->exonPsProbs[i] < disagreeThresh)
	{
	js->exonDisagree++;
	jsOnly++;
	}
    }
if(jsOnly > 0)
    percent = (double)both/jsOnly;
else 
    percent = -1;
return percent;
}

double calcSjPercent(struct junctSet *js, struct hash *bedHash, struct resultM *probM)
/* Calculate how often the sj probe set is called expressed
   when exon probe set is. */
{
int includeIx = includeJsIx(js, bedHash);
int both = 0;
int exonOnly = 0;
double percent = 0;
int i = 0;
for(i = 0; i < probM->colCount; i++)
    {
    if(junctionExpressed(js, i, includeIx) &&
       js->exonPsProbs[i] >= presThresh)
	{
	both++;
	exonOnly++;
	js->sjAgree++;
	}
    else if(exonExpressed(js, i) &&
	    js->junctProbs[includeIx][i] < disagreeThresh)
	{
	exonOnly++;
	js->sjDisagree++;
	}
    }
if(exonOnly > 0)
    percent = (double)both/exonOnly;
else 
    percent = -1;
return percent;
}

double calcExonSjCorrelation(struct junctSet *js, struct hash *bedHash, struct resultM *intenM)
/* Calcualate the correltation between the exon probe
   set and the appropriate splice junction set. */
{
double corr = -2;
int includeIx = includeJsIx(js, bedHash);
corr = correlation(js->junctIntens[includeIx], js->exonPsIntens, intenM->colCount);
return corr;
}

double calcExonSjProbCorrelation(struct junctSet *js, struct hash *bedHash, struct resultM *probM)
/* Calcualate the correltation between the exon probe
   set and the appropriate splice junction set. */
{
double corr = -2;
int includeIx = includeJsIx(js, bedHash);
corr = correlation(js->junctProbs[includeIx], js->exonPsProbs, probM->colCount);
return corr;
}

void calcExonCorrelation(struct junctSet *jsList, struct hash *bedHash, 
			 struct resultM *intenM, struct resultM *probM)
/* Calculate exon correlation if available. */
{
struct junctSet *js = NULL;
for(js = jsList; js != NULL; js = js->next)
    {
    if(js->junctUsedCount == 2 &&
       js->cassette == 1 && differentWord(js->exonPsName, "NA") && 
       js->exonPsIntens != NULL && js->exonPsProbs != NULL) 
	{
	js->exonCorr = calcExonSjCorrelation(js, bedHash, intenM);
	js->probCorr = calcExonSjProbCorrelation(js, bedHash, probM);
	js->exonSjPercent = calcExonPercent(js, bedHash, probM);
	js->sjExonPercent = calcSjPercent(js, bedHash, probM);
	if(js->altExpressed && exonPsStats)
	    fprintf(exonPsStats, "%s\t%.2f\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%.2f\n", 
		    js->exonPsName, js->exonCorr, js->exonSjPercent, js->sjExonPercent, 
		    js->exonAgree, js->exonDisagree, js->sjAgree, js->sjDisagree, js->probCorr);
	}
    }
}


int calcExpressed(struct junctSet *jsList, struct resultM *probMat)
/* Loop through and calculate expression and score where score is correlation. */
{
struct junctSet *js = NULL;
int i = 0, j = 0, k = 0;
double minCor = 2; /* Correlation is always between -1 and 1, 2 is
		    * good max. */
double *X = NULL;
double *Y = NULL;
int elementCount;
int colCount = probMat->colCount;
int rowCount = probMat->rowCount;     
int colIx = 0, junctIx = 0;           /* Column and Junction indexes. */
int junctOneExp = 0, junctTwoExp = 0; /* What are the counts of
				       * expressed and alts. */

for(js = jsList; js != NULL; js = js->next)
    {
    int *expressed = NULL;
    int totalTissues = 0;
    int junctExpressed = 0;
    AllocArray(expressed, js->junctUsedCount);

    /* Loop through and see how many junctions
       are actually expressed and in how many tissues. */
    for(colIx = 0; colIx < colCount; colIx++)
	{
	boolean anyExpressed = FALSE;
	for(junctIx = 0; junctIx < js->junctUsedCount; junctIx++)
	    {
	    if(junctionExpressed(js, colIx, junctIx) == TRUE)
		{
		expressed[junctIx]++;
		anyExpressed = TRUE;
		}
	    }
	if(anyExpressed)
	    totalTissues++;
	}
    
    /* Set number expressed. */
    for(i = 0; i < js->junctUsedCount; i++)
	{
	if(expressed[i] > 0)
	    junctExpressed++;
	}

    /* If one tissue expressed set is expressed. */
    if(junctExpressed >= 1)
	{
	js->expressed = TRUE;
	junctOneExp++;
	}
    
    /* If two tissues are expressed set is alternative expressed. */
    if(junctExpressed >= 2)
	{
	js->altExpressed = TRUE;
	junctTwoExp++;
	}

    if(js->altExpressed == TRUE)
	js->score = calcMinCorrelation(js, expressed, totalTissues, colCount);
    else
	js->score = BIGNUM;
    freez(&expressed);
    }

warn("%d junctions expressed above p-val %.2f, %d alternative (%.2f%%)",
     junctOneExp, presThresh, junctTwoExp, 100*((double)junctTwoExp/junctOneExp));
}

int junctSetScoreCmp(const void *va, const void *vb)
/* Compare to sort based on score, smallest to largest. */
{
const struct junctSet *a = *((struct junctSet **)va);
const struct junctSet *b = *((struct junctSet **)vb);
if( a->score > b->score)
    return 1;
else if(a->score < b->score)
    return -1;
return 0;	    
}

int junctSetExonCorrCmp(const void *va, const void *vb)
/* Compare to sort based on score, smallest to largest. */
{
const struct junctSet *a = *((struct junctSet **)va);
const struct junctSet *b = *((struct junctSet **)vb);
if( a->exonCorr > b->exonCorr)
    return -1;
else if(a->exonCorr < b->exonCorr)
    return 1;
return 0;	    
}

int junctSetExonPercentCmp(const void *va, const void *vb)
/* Compare to sort based on score, smallest to largest. */
{
const struct junctSet *a = *((struct junctSet **)va);
const struct junctSet *b = *((struct junctSet **)vb);
double dif = a->exonSjPercent > b->exonSjPercent;
if(dif == 0)
    {
    if( a->exonCorr > b->exonCorr)
	return -1;
    else if(a->exonCorr < b->exonCorr)
	return 1;
    }
if( a->exonSjPercent > b->exonSjPercent)
    return -1;
else if(a->exonSjPercent < b->exonSjPercent)
    return 1;
return 0;	    
}

void makePlotLinks(struct junctSet *js, struct dyString *buff)
{
dyStringClear(buff);
dyStringPrintf(buff, "<a target=\"plots\" href=\"./noAltTranStartJunctSet/altPlot/%s.%d-%d.%s.png\">[all]</a> ",
	       js->chrom, js->chromStart, js->chromEnd, js->hName);
dyStringPrintf(buff, "<a target=\"plots\" href=\"./noAltTranStartJunctSet/altPlot.median/%s.%d-%d.%s.png\">[median]</a> ",
	       js->chrom, js->chromStart, js->chromEnd, js->hName);
}

boolean geneExpressed(struct junctSet *js, int colIx)
/* Are any of the gene sets expressed in this sample? */
{
int i = 0;
boolean expressed = FALSE;
for(i = 0; i < js->genePSetCount; i++)
    {
    if(js->geneProbs[i][colIx] >= presThresh)
	expressed = TRUE;
    }
return expressed;
}

void makeJunctMdbGenericLink(struct junctSet *js, struct dyString *buff, char *name, struct resultM *probM, char *suffix)
{
int offSet = 100;
int i = 0;
dyStringClear(buff);
dyStringPrintf(buff, "<a target=\"plots\" href=\"http://mdb1-sugnet.cse.ucsc.edu/cgi-bin/mdbSpliceGraph?mdbsg.calledSelf=on&coordString=%s:%c:%s:%d-%d&mdbsg.cs=%d&mdbsg.ce=%d&mdbsg.expName=%s&mdbsg.probeSummary=on&mdbsg.toScale=on%s\">", 
	       "mm2", js->strand[0], js->chrom, js->chromStart, js->chromEnd, 
	       js->chromStart - offSet, js->chromEnd + offSet, "AffyMouseSplice1-02-2004", suffix);
dyStringPrintf(buff, "%s", name);
dyStringPrintf(buff, "</a> ");
}


void makeJunctExpressedTissues(struct junctSet *js, struct dyString *buff, int junctIx, struct resultM *probM)
{
int offSet = 100;
int i = 0;
struct dyString *dy = newDyString(1048);
char anchor[128];
dyStringClear(buff);
for(i = 0; i < probM->colCount; i++)
    {
    if(js->junctProbs[junctIx][i] >= presThresh && 
       geneExpressed(js, i))
	{
	safef(anchor, sizeof(anchor), "#%s", probM->colNames[i]);
	makeJunctMdbGenericLink(js, dy, probM->colNames[i], probM, anchor);
	dyStringPrintf(buff, "%s, ", dy->string);
	}
    }
dyStringFree(&dy);
}

/* void makeJunctExpressedTissues(struct junctSet *js, struct dyString *buff, int junctIx, struct resultM *probM) */
/* { */
/* int offSet = 100; */
/* int i = 0; */
/* dyStringClear(buff); */
/* for(i = 0; i < probM->colCount; i++) */
/*     { */
/*     if(js->junctProbs[junctIx][i] >= presThresh &&  */
/*        geneExpressed(js, i)) */
/* 	dyStringPrintf(buff, "%s, ", probM->colNames[i]); */
/*     } */
/* } */

void makeJunctExpressedLink(struct junctSet *js, struct dyString *buff, int junctIx, struct resultM *probM)
{
int offSet = 100;
int i = 0;
dyStringClear(buff);
dyStringPrintf(buff, "<a target=\"plots\" href=\"http://mdb1-sugnet.cse.ucsc.edu/cgi-bin/mdbSpliceGraph?mdbsg.calledSelf=on&coordString=%s:%c:%s:%d-%d&mdbsg.cs=%d&mdbsg.ce=%d&mdbsg.expName=%s&mdbsg.probeSummary=on&mdbsg.toScale=on\">", 
	       "mm2", js->strand[0], js->chrom, js->chromStart, js->chromEnd, 
	       js->chromStart - offSet, js->chromEnd + offSet, "AffyMouseSplice1-02-2004");
for(i = 0; i < probM->colCount; i++)
    {
    if(js->junctProbs[junctIx][i] >= presThresh && 
       geneExpressed(js, i))
	dyStringPrintf(buff, "%s,", probM->colNames[i]);
    }
dyStringPrintf(buff, "</a> ");
}


void makeJunctNotExpressedTissues(struct junctSet *js, struct dyString *buff, int junctIx, struct resultM *probM)
{
int offSet = 100;
int i = 0;
struct dyString *dy = newDyString(1048);
char anchor[128];
dyStringClear(buff);

for(i = 0; i < probM->colCount; i++)
    {
    if(js->junctProbs[junctIx][i] <= absThresh)
	{
	safef(anchor, sizeof(anchor), "#%s", probM->colNames[i]);
	makeJunctMdbGenericLink(js, dy, probM->colNames[i], probM, anchor);
	dyStringPrintf(buff, "%s, ", dy->string);
	}
    }
dyStringFree(&dy);
}

void makeJunctMdbLink(struct junctSet *js, struct hash *bedHash,
		      struct dyString *buff, int junctIx, struct resultM *probM)
{
int offSet = 100;
int i = 0;
dyStringClear(buff);
dyStringPrintf(buff, "<a target=\"plots\" href=\"http://mdb1-sugnet.cse.ucsc.edu/cgi-bin/mdbSpliceGraph?mdbsg.calledSelf=on&coordString=%s:%c:%s:%d-%d&mdbsg.cs=%d&mdbsg.ce=%d&mdbsg.expName=%s&mdbsg.probeSummary=on&mdbsg.toScale=on\">", 
	       "mm2", js->strand[0], js->chrom, js->chromStart, js->chromEnd, 
	       js->chromStart - offSet, js->chromEnd + offSet, "AffyMouseSplice1-02-2004");
dyStringPrintf(buff, "%s", js->junctUsed[junctIx]);
dyStringPrintf(buff, "</a> ");
if(js->cassette == 1 && js->exonPsProbs != NULL) 
    {
    if(junctIx == includeJsIx(js, bedHash))
	{
	dyStringPrintf(buff, "<b> Inc.</b>");
	}
    }
}

void makeGbCoordLink(struct junctSet *js, int chromStart, int chromEnd, struct dyString *buff)
/* Make a link to the genome browser with links highlighted. */
{
int junctIx = 0;
int offSet = 100;
dyStringClear(buff);
dyStringPrintf(buff, "<a target=\"browser\" href=\"http://hgwdev-sugnet.cse.ucsc.edu/cgi-bin/hgTracks?position=%s:%d-%d&splicesPFt=red&splicesPlt=or&splicesP_name=", js->chrom, chromStart - offSet, chromEnd + offSet);
for(junctIx = 0; junctIx < js->junctUsedCount; junctIx++)
    dyStringPrintf(buff, "%s ", js->junctUsed[junctIx]);
if(js->exonPsName != NULL && differentWord(js->exonPsName, "NA"))
    dyStringPrintf(buff, "%s ", js->exonPsName);
dyStringPrintf(buff, "\">%s</a>", js->name);
}

void makeGbClusterLink(struct junctSet *js, struct dyString *buff)
{
makeGbCoordLink(js, js->chromStart, js->chromEnd, buff);
}

void makeGbLink(struct junctSet *js, struct dyString *buff)
{
int junctIx = 0;
int offSet = 100;
dyStringClear(buff);
if(js->hName[0] != 'G' || strlen(js->hName) != 8)
    {
    dyStringPrintf(buff, "<a target=\"browser\" href=\"http://hgwdev-sugnet.cse.ucsc.edu/cgi-bin/hgTracks?position=%s&splicesPFt=red&splicesPlt=or&splicesP_name=", js->hName);
    for(junctIx = 0; junctIx < js->junctUsedCount; junctIx++)
	dyStringPrintf(buff, "%s ", js->junctUsed[junctIx]);
    dyStringPrintf(buff, "\">%s</a>", js->hName);
    }
else
    dyStringPrintf(buff, "%s", js->hName);
}

void outputLinks(struct junctSet **jsList, struct hash *bedHash, struct resultM *probM, FILE *out)
/* Output a report for each junction set. */
{
struct junctSet *js = NULL;
int colIx = 0, junctIx = 0;
struct dyString *dy = newDyString(2048);
if(optionExists("sortByExonCorr")) 
    slSort(jsList, junctSetExonCorrCmp);
else if(optionExists("sortByExonPercent"))
    slSort(jsList, junctSetExonPercentCmp);
else
    slSort(jsList, junctSetScoreCmp);
fprintf(out, "<ol>\n");
for(js = *jsList; js != NULL; js = js->next)
    {
    if(js->score <= 1)
	{
	char affyName[256];
	boolean gray = FALSE;
	char *tmp = NULL;
	char *col = NULL;
	fprintf(out, "<li>");
	fprintf(out, "<table bgcolor=\"#000000\" border=0 cellspacing=0 cellpadding=1><tr><td>");
	fprintf(out, "<table bgcolor=\"#FFFFFF\" width=100%%>\n");
	fprintf(out, "<tr><td bgcolor=\"#7896be\">");
	makeGbClusterLink(js, dy);
	fprintf(out, "<b>Id:</b> %s ", dy->string);
	makeGbLink(js, dy);
	fprintf(out, "<b>GeneName:</b> %s ", dy->string);
	makePlotLinks(js, dy);
	fprintf(out, "<b>Plots:</b> %s ", dy->string);
	fprintf(out, "<b>Score:</b> %.2f", js->score);
	safef(affyName, sizeof(affyName), "%s", js->junctUsed[0]);
	tmp = strchr(affyName, '@');
	assert(tmp);
	*tmp = '\0';
	fprintf(out, "<a target=_new href=\"https://bioinfo.affymetrix.com/Collab/"
		"Manuel_Ares/WebTools/asViewer.asp?gene=%s\"> <b> Affy </b></a>\n", affyName);
	fprintf(out, "</td></tr>\n<tr><td>");
	if(js->cassette == 1 && js->exonPsProbs != NULL)
	    {
	    fprintf(out, 
		    "<b>Exon Corr:</b> %.2f <b>Exon %%:</b> %.2f <b>Junct %%:</b> %.2f "
		    "<b>Expressed:</b> %d</td></tr><tr><td>\n",
		    js->exonCorr, js->exonSjPercent, js->sjExonPercent, js->exonAgree + js->exonDisagree);
	    fprintf(out, "<b>Exon ps:</b> %s</td></tr><tr><td>\n", js->exonPsName);
	    }
	fprintf(out, "<table width=100%%>\n");

	for(junctIx = 0; junctIx < js->junctUsedCount; junctIx++)
	    {
	    col = "#bebebe";
	    makeJunctMdbLink(js, bedHash, dy, junctIx, probM);
	    fprintf(out, "<tr><td bgcolor=\"%s\"><b>Junct:</b> %s</td></tr>", col, dy->string);
	    makeJunctExpressedTissues(js, dy, junctIx, probM);
	    col = "#ffffff";
	    fprintf(out, "<tr><td bgcolor=\"%s\"><b>Exp:</b> %s </td></tr>",col, dy->string);
	    makeJunctNotExpressedTissues(js, dy, junctIx, probM);
	    fprintf(out, "<tr><td bgcolor=\"%s\"><b>Not Exp:</b> %s </td></tr>", col, dy->string);
	    }
	fprintf(out, "</table></td></tr>\n");
	fprintf(out, "</table></td></tr></table><br></li>\n");
	}
    }
}

char *findBestDuplicate(struct junctSet *js, struct resultM *intenM)
/* Loop through the junction duplicate set and see which one
   has the best correlation to a gene set. */
{
double bestCorr = -2;
double corr = 0;
int gsRow = 0;
int sjRow = 0;
char *bestJunct = NULL;
int junctIx = 0, geneIx = 0;
for(junctIx = 0; junctIx < js->junctDupCount; junctIx++)
    {
    for(geneIx = 0; geneIx < js->genePSetCount; geneIx++)
	{
	gsRow = hashIntValDefault(intenM->nameIndex, js->genePSets[geneIx], -1);
	sjRow = hashIntValDefault(intenM->nameIndex, js->dupJunctPSets[junctIx], -1);
	if(gsRow == -1 || sjRow == -1)
	    continue;
	corr = correlation(intenM->matrix[gsRow], intenM->matrix[sjRow], intenM->colCount);
	if(corr >= bestCorr)
	    {
	    bestCorr = corr;
	    bestJunct = js->dupJunctPSets[junctIx];
	    }
	}
    }
return bestJunct;
}

void calcJunctSetRatios(struct junctSet *jsList, struct resultM *intenM)
/* Caluclate the ratio of junction to every other junction in 
   the set. */
{
char *outFile = optionVal("ratioFile", NULL);
struct junctSet *js = NULL;
int i = 0, j = 0, expIx = 0;
int numIx = 0, denomIx = 0;
FILE *out = NULL;
double **mat = intenM->matrix;
int colCount = intenM->colCount;

struct hash *nIndex = intenM->nameIndex;

if(outFile == NULL)
    errAbort("Must specify a ratioFile to print ratios to.");

out = mustOpen(outFile, "w");
/* Print the header. */
for(i = 0; i < colCount-1; i++)
    fprintf(out, "%s\t", intenM->colNames[i]);
fprintf(out, "%s\n", intenM->colNames[i]);
for(js = jsList; js != NULL; js = js->next) 
    {
    char **jPSets = js->junctPSets;
    char **djPSets = js->dupJunctPSets;
    int jCount = js->maxJunctCount;
    int dCount = js->junctDupCount;

    for(i = 0; i < jCount; i++)
	{
	numIx = hashIntValDefault(nIndex, jPSets[i], -1);
	if(numIx == -1)
	    errAbort("Can't find %s in hash.", jPSets[i]);

	/* Calc ratios for the rest of the probe sets. */
	for(j = i+1; j < jCount; j++)
	    {
	    denomIx = hashIntValDefault(nIndex, jPSets[j], -1);
	    if(numIx == -1)
		errAbort("Can't find %s in hash.", jPSets[j]);
	    fprintf(out, "%s|%s\t", jPSets[i], jPSets[j]);
	    for(expIx = 0; expIx < colCount - 1; expIx++)
		{
		assert(pow(2,mat[denomIx][expIx]) != 0);
		fprintf(out, "%f\t", pow(2,mat[numIx][expIx]) / pow(2,mat[denomIx][expIx]));
		}
	    fprintf(out, "%f\n", pow(2,mat[numIx][expIx]) / pow(2,mat[denomIx][expIx]));
	    }
	/* Calc ratios for the dup probe sets. */
	for(j = 0; j < dCount; j++)
	    {
	    denomIx = hashIntValDefault(nIndex, djPSets[j], -1);
	    if(numIx == -1)
		errAbort("Can't find %s in hash.", djPSets[j]);
	    fprintf(out, "%s|%s\t", jPSets[i], djPSets[j]);
	    for(expIx = 0; expIx < colCount - 1; expIx++)
		{
		assert(pow(2,mat[denomIx][expIx]) != 0);
		fprintf(out, "%f\t", pow(2,mat[numIx][expIx]) / pow(2,mat[denomIx][expIx]));
		}
		fprintf(out, "%f\n", pow(2,mat[numIx][expIx]) / pow(2,mat[denomIx][expIx]));
	    }
	}
    
    /* Get the ratios of the duplicate probes to eachother. */
    for(i = 0; i < dCount; i++)
	{
	numIx = hashIntValDefault(nIndex, djPSets[i], -1);
	if(numIx == -1)
	    errAbort("Can't find %s in hash.", djPSets[i]);

	/* Calc ratios for the rest of the dup probe sets. */
	for(j = i+1; j < dCount; j++)
	    {
	    denomIx = hashIntValDefault(nIndex, djPSets[j], -1);
	    if(numIx == -1)
		errAbort("Can't find %s in hash.", djPSets[j]);
	    fprintf(out, "%s|%s\t", djPSets[i], djPSets[j]);
	    for(expIx = 0; expIx < colCount - 1; expIx++)
		{
		assert(pow(2,mat[denomIx][expIx]) != 0);
		fprintf(out, "%f\t", pow(2,mat[numIx][expIx]) / pow(2,mat[denomIx][expIx]));
		}
	    fprintf(out, "%f\n", pow(2,mat[numIx][expIx]) / pow(2,mat[denomIx][expIx]));
	    }
	}
    }
}


void fillInJunctUsed(struct junctSet *jsList, struct resultM *intenM)
/* If it isn't already determined, figures out which junctions we're
   using. */
{
struct junctSet *js = NULL;
char *bestDup = NULL;
int i = 0;
for(js = jsList; js != NULL; js = js->next)
    {
    if(js->junctUsedCount != 0) /* If it has already been filled in forget about it. */
	continue;
    if(js->junctDupCount == 0)  /* Trivial if there are no redundant sets to choose from. */
	{
	js->junctUsedCount = js->maxJunctCount;
	AllocArray(js->junctUsed, js->junctUsedCount);
	for(i = 0; i < js->junctUsedCount; i++)
	    js->junctUsed[i] = cloneString(js->junctPSets[i]);
	}
    else /* Go find the redundant junction that correlates best with a gene probe set. */
	{
	js->junctUsedCount = js->maxJunctCount + 1;
	AllocArray(js->junctUsed, js->junctUsedCount);
	for(i = 0; i < js->junctUsedCount - 1; i++)
	    js->junctUsed[i] = cloneString(js->junctPSets[i]);
	bestDup = findBestDuplicate(js, intenM);
	if(bestDup != NULL)
	    js->junctUsed[i] = cloneString(bestDup);
	else
	    js->junctUsedCount--;
	}
    }
}
	
struct hash *hashBeds(char *fileName)
/* Hash all of the beds in a file. */
{
struct bed *bedList = NULL, *bed = NULL;
struct hash *bedHash = newHash(12);

bedList = bedLoadAll(fileName);
for(bed = bedList; bed != NULL; bed = bed->next)
    hashAddUnique(bedHash, bed->name, bed);
return bedHash;
}

void altHtmlPages(char *junctFile, char *probFile, char *intensityFile, char *bedFile)
/* Do a top level summary. */
{
struct junctSet *jsList = NULL;
struct junctSet *js = NULL;
struct resultM *probM = NULL;
struct resultM *intenM = NULL;
int setCount = 0;
int junctIx = 0, i = 0;
FILE *htmlOut = NULL;
FILE *htmlFrame = NULL;
FILE *spreadSheet = NULL;
char *spreadSheetName = optionVal("spreadSheet", NULL);
char *browserName = "hgwdev-sugnet.cse";
char nameBuff[2048];
char *htmlPrefix = optionVal("htmlPrefix", "");
struct hash *bedHash = NULL;
char *db = "mm2";
assert(junctFile);
assert(intensityFile);
jsList = junctSetLoadAll(junctFile);
warn("Loaded %d records from %s", slCount(jsList), junctFile);
intenM = readResultMatrix(intensityFile);
warn("Loaded %d rows and %d columns from %s", intenM->rowCount, intenM->colCount, intensityFile);
if(optionExists("doSjRatios"))
    {
    calcJunctSetRatios(jsList, intenM);
    exit(0);
    }

assert(probFile);
assert(bedFile);
probM = readResultMatrix(probFile);
warn("Loaded %d rows and %d columns from %s", probM->rowCount, probM->colCount, probFile);


fillInJunctUsed(jsList, intenM);
fillInProbs(jsList, probM);
fillInIntens(jsList, intenM);

warn("Calculating expression");
calcExpressed(jsList, probM);

bedHash = hashBeds(bedFile);
calcExonCorrelation(jsList, bedHash, intenM, probM);

/* Write out the lists. */
warn("Writing out links.");
safef(nameBuff, sizeof(nameBuff), "%s%s", htmlPrefix, "lists.html");
htmlOut = mustOpen(nameBuff, "w");
fprintf(htmlOut, "<html><body>\n");
outputLinks(&jsList, bedHash, probM, htmlOut);
fprintf(htmlOut, "</body></html>\n");
carefulClose(&htmlOut);

/* Loop through and output a spreadsheet compatible file
   with the names of junction sets. */
if(spreadSheetName != NULL)
    {
    spreadSheet = mustOpen(spreadSheetName, "w");
    for(js = jsList; js != NULL; js = js->next) 
	{
	if(js->altExpressed) 
	    {
	    fprintf(spreadSheet, "%s\t%s\t", js->hName,  js->name);
	    for(junctIx = 0; junctIx < js->junctUsedCount; junctIx++)
		{
		fprintf(spreadSheet, "\t%s\t", js->junctUsed[junctIx]);
		for(i = 0; i < probM->colCount; i++)
		    {
		    if(js->junctProbs[junctIx][i] >= presThresh && 
		       geneExpressed(js, i))
			fprintf(spreadSheet, "%s,", probM->colNames[i]);
		    }
		}
	    fprintf(spreadSheet, "\n");
	    }
	}
    carefulClose(&spreadSheet);
    }


/* Write out the frames. */
safef(nameBuff, sizeof(nameBuff), "%s%s", htmlPrefix, "frame.html");
htmlFrame = mustOpen(nameBuff, "w");
fprintf(htmlFrame, "<html><head><title>Junction Sets</title></head>\n"
	"<frameset cols=\"30%,70%\">\n"
	"   <frame name=\"_list\" src=\"./%s%s\">\n"
	"   <frameset rows=\"50%,50%\">\n"
	"      <frame name=\"browser\" src=\"http://%s.ucsc.edu/cgi-bin/hgTracks?db=%s&position=%s:%d-%d\">\n"
	"      <frame name=\"plots\" src=\"http://%s.ucsc.edu/cgi-bin/hgTracks?db=%s&position=%s:%d-%d\">\n"
	"   </frameset>\n"
	"</frameset>\n"
	"</html>\n", htmlPrefix, "lists.html", 
	browserName, db, jsList->chrom, jsList->chromStart, jsList->chromEnd,
	browserName, db, jsList->chrom, jsList->chromStart, jsList->chromEnd);
}

int main(int argc, char *argv[])
{
char *exonPsFile = NULL;
if(argc == 1)
    usage();
optionInit(&argc, argv, optionSpecs);
if(optionExists("help"))
    usage();
presThresh = optionFloat("presThresh", .9);
absThresh = optionFloat("absThresh", .1);
if((exonPsFile = optionVal("exonStats", NULL)) != NULL)
    {
    exonPsStats = mustOpen(exonPsFile, "w");
    fprintf(exonPsStats, 
	    "#name\tcorrelation\texonSjPercent\tsjExonPercent\t"
	    "exonAgree\texonDisagree\tsjAgree\tsjDisagree\n");
    }
if(optionExists("strictDisagree"))
    disagreeThresh = absThresh;
else
    disagreeThresh = presThresh;
altHtmlPages(optionVal("junctFile", NULL), optionVal("probFile", NULL), 
	     optionVal("intensityFile", NULL), optionVal("bedFile", NULL));
carefulClose(&exonPsStats);
return 0;
}
