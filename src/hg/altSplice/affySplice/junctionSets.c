/* junctionSets.c Program to group together affy probes (beds) that share
   a splice site. Any group of probes that share a splice site
   but link to another splice site indicate alternative splicing. */
#include "common.h"
#include "bed.h"
#include "hash.h"
#include "options.h"
#include "chromKeeper.h"
#include "binRange.h"
#include "obscure.h"
#include "hash.h"
#include "linefile.h"

struct junctionSet 
/* A set of beds with a common start or end. */
{
    struct junctionSet *next;   /* Next in list. */
    char *chrom;                /* Chromosome. */
    int chromStart;             /* Smallest start. */
    int chromEnd;               /* Largest end. */
    char *name;                 /* Name of junction. */
    int junctCount;             /* Number of junctions in set. */
    char strand[2];             /* + or - depending on which strand. */
    char *genePSet;             /* Gene probe set name. */
    int maxJunctCount;          /* Maximum size of bedArray. */
    struct bed **bedArray;      /* Beds in the cluster. */
    int junctDupCount;          /* Number of redundant junctions in set. */
    int maxJunctDupCount;       /* Maximum size of bedDupArray. */
    struct bed **bedDupArray;   /* Redundant beds in the cluster. */
    boolean merged;             /* Has this set been merged? */
};

static struct optionSpec optionSpecs[] = 
/* Our acceptable options to be called with. */
{
    {"help", OPTION_BOOLEAN},
    {"bedFile", OPTION_STRING},
    {"setFile", OPTION_STRING},
    {"exonFile", OPTION_STRING},
    {"genePSet", OPTION_STRING},
    {"db", OPTION_STRING},
    {"ctFile", OPTION_STRING},
    {"ctName", OPTION_STRING},
    {"ctDesc", OPTION_STRING},
    {"ctColor", OPTION_STRING},
    {NULL, 0}
};

static char *optionDescripts[] = 
/* Description of our options for usage summary. */
{
    "Display this message.",
    "File containing beds that represent affy junction clusters.",
    "File to output sets of probes that share a splice site to.",
    "File containing beds with exons that can't be used (i.e. transcription start or end).",
    "File containing list of gene probe sets.",
    "File database that coordinates correspond to.",
    "File that custom track will end up in.",
    "Name of custom track.",
    "Description of custom track."
    "Color of custom track in r,g,b format (i.e. 255,0,0)."
};

int setCount = 0;        /* Number of sets we have, used for naming sets. */
int setStartCount = 0;   /* Number of sets with more than one start. */
int setEndCount = 0;     /* Number of sets with more than one end. */
int setMergedCount = 0;  /* Number of sets that were merged and contain more than one probe. */
FILE *ctFile = NULL;     /* File to write beds in sets as custom track to. */
boolean chromKeeperOpen = FALSE; /* Is chromkeeper initialized? */

void usage()
/** Print usage and quit. */
{
int i=0;
warn("junctionSets - Program to group affymetrix probe sets in bed form\n"
     "by their splice sites. By definition if two or more probes sets share a\n"
     "splice juction but join to different splice junctions alternative\n"
     "splicing is possible.\n"
     "options are:");
for(i=0; i<ArraySize(optionSpecs) -1; i++)
    fprintf(stderr, "  -%s -- %s\n", optionSpecs[i].name, optionDescripts[i]);
errAbort("\nusage:\n   "
	 "junctionSets -bedFile=junctionProbeSets.bed -setFile=sets.tab");
}

void initCtFile(char *fileName)
/* Initialize the custom track file handle. */
{
char *name = optionVal("ctName", "junctionSets");
char *desc = optionVal("ctDesc", "Alt-Splicing Sets of Junction Probes");
char *color = optionVal("ctColor", "255,0,0");
ctFile = mustOpen(fileName, "w");
warn("Opening file %s to write", fileName);
fprintf(ctFile, "track name=\"%s\" desc=\"%s\" color=%s\n", name, desc, color);
}

void loadSoftExons(char *exonFile)
/* Load all of the exons into our chromKeeper structure. */
{
struct bed *bed = NULL, *bedList = NULL;
char *db = optionVal("db", NULL);
if(db == NULL)
    errAbort("Must specify a database for binKeeper sizes.");
bedList = bedLoadAll(exonFile);
warn("Placing beds in binKeeper");
chromKeeperInit(db);
for(bed = bedList; bed != NULL; bed = bed->next)
    chromKeeperAdd(bed->chrom, bed->chromStart, bed->chromEnd, bed);
chromKeeperOpen = TRUE;
}

boolean overlapSoftExon(char *chrom, int start, int end)
/* Return TRUE if region overlaps a bed in the softExonFile. */
{
struct binElement *binList = NULL, *bin = NULL;
boolean match = FALSE;
if(chromKeeperOpen == FALSE)
    return FALSE;
binList = chromKeeperFind(chrom, start, end);
if(binList != NULL)
    match = TRUE;
slFreeList(&binList);
return match;
}

int bedSpliceStart(const struct bed *bed)
/* Return the start of the bed splice site. */
{
assert(bed->blockSizes);
return bed->chromStart + bed->blockSizes[0];
}

int bedSpliceEnd(const struct bed *bed)
/* Return the end of the bed splice site. */
{
assert(bed->blockSizes);
return bed->chromStart + bed->chromStarts[1];
}

int bedCmpEnd(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
int dif;
assert(a->blockCount > 1 && b->blockCount > 1);
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = bedSpliceEnd(a) - bedSpliceEnd(b);
return dif;
}

int bedCmpStart(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
int dif;
assert(a->blockCount > 1 && b->blockCount > 1);
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = bedSpliceStart(a) - bedSpliceStart(b);
return dif;
}

void junctionSetTabOut(struct junctionSet *js, FILE *out)
/* Write out junctionSet to a file. */
{
int i;
fprintf(out, "%s\t%d\t%d\t%s\t%d\t%s\t%s\t", js->chrom, js->chromStart, js->chromEnd, 
	js->name, js->junctCount, js->strand, js->genePSet);
for(i = 0; i < js->junctCount - 1; i++) 
    {
    fprintf(out, "%s,", js->bedArray[i]->name);
    }
fprintf(out, "%s\t", js->bedArray[i]->name);

/* Print out the duplicates. */
fprintf(out, "%d\t",  js->junctDupCount);
for(i = 0; i < js->junctDupCount - 1; i++) 
    {
    fprintf(out, "%s,", js->bedDupArray[i]->name);
    }
if(js->junctDupCount == 0)
    fprintf(out, "NA\n");
else
    fprintf(out, "%s\n", js->bedDupArray[i]->name);
}

struct junctionSet *findEndSets(struct bed *bedList)
/* Create a list of juctionSets for all the probes that have the
   same end.*/
{
int currentEnd = 0;
char *currentChrom = NULL;
struct bed *bed = NULL;
struct junctionSet *setList = NULL, *set = NULL;
char buff[256];
char currentStrand[2];
slSort(&bedList, bedCmpEnd);

for(bed = bedList; bed != NULL; bed = bed->next)
    {
    if(currentChrom == NULL ||
       differentString(bed->chrom, currentChrom) || 
       bedSpliceEnd(bed) != currentEnd ||
       bed->strand[0] != currentStrand[0])
	{
	if(set != NULL)
	    {
	    slSafeAddHead(&setList, set);
	    if(set->junctCount > 1)
		setEndCount++;
	    }
	safef(buff, sizeof(buff), "%s.%d", bed->chrom, setCount++);
	AllocVar(set);
	set->maxJunctCount = 5;
	set->chromStart = bedSpliceStart(bed);
	set->chromEnd = bedSpliceEnd(bed);
	AllocArray(set->bedArray, set->maxJunctCount);
	set->chrom = cloneString(bed->chrom);
	set->name = cloneString(buff);
	safef(set->strand, sizeof(set->strand), "%s", bed->strand);
	set->bedArray[set->junctCount++] = bed;

	/* Set the current chrom and end. */
	currentChrom = set->chrom;
	currentEnd = set->chromEnd;
	currentStrand[0] = set->strand[0];
	}
    else if(sameString(bed->chrom, currentChrom) && 
	    bedSpliceEnd(bed) == currentEnd)
	{
	/* Only want junctions that connect to non-soft exons, i.e. not
	   transcription start and ends. */
	if(!overlapSoftExon(bed->chrom, bed->chromStart, bedSpliceStart(bed)))
	    {
	    if(set->junctCount + 1 >= set->maxJunctCount)
		{
		ExpandArray(set->bedArray, set->maxJunctCount, 2*set->maxJunctCount);
		set->maxJunctCount = 2 * set->maxJunctCount;
		}
	    set->chromEnd = max(set->chromEnd, bedSpliceEnd(bed));
	    set->chromStart = min(set->chromStart, bedSpliceStart(bed));
	    set->bedArray[set->junctCount++] = bed;
	    }
	}
    else
	errAbort("bed %s doesn't seem to fit the mold with chrom %s and chromEnd %d",
		 bed->name, currentChrom, currentEnd);
    }
slReverse(&setList);
return setList;
}

struct junctionSet *findStartSets(struct bed *bedList)
/* Create a list of juctionSets for all the probes that have the
   same start.*/
{
int currentStart = 0;
char *currentChrom = NULL;
struct bed *bed = NULL;
struct junctionSet *setList = NULL, *set = NULL;

char buff[256];
char currentStrand[2];
slSort(&bedList, bedCmpStart);

for(bed = bedList; bed != NULL; bed = bed->next)
    {
    if(currentChrom == NULL ||
       differentString(bed->chrom, currentChrom) || 
       bedSpliceStart(bed) != currentStart ||
       bed->strand[0] != currentStrand[0])
	{
	if(set != NULL)
	    {
	    slSafeAddHead(&setList, set);
	    if(set->junctCount > 1)
		setStartCount++;
	    }
	safef(buff, sizeof(buff), "%s.%d", bed->chrom, setCount++);
	AllocVar(set);
	set->maxJunctCount = 5;
	set->chromStart = bedSpliceStart(bed);
	set->chromEnd = bedSpliceEnd(bed);
	AllocArray(set->bedArray, set->maxJunctCount);
	set->chrom = cloneString(bed->chrom);
	set->name = cloneString(buff);
	safef(set->strand, sizeof(set->strand), "%s", bed->strand);
	set->bedArray[set->junctCount++] = bed;

	/* Set the current chrom and start. */
	currentChrom = set->chrom;
	currentStart = set->chromStart;
	currentStrand[0] = set->strand[0];
	}
    else if(sameString(bed->chrom, currentChrom) && 
	    bedSpliceStart(bed) == currentStart)
	{
	/* Only want junctions that connect to non-soft exons, i.e. not
	   transcription start and ends. */
	if(!overlapSoftExon(bed->chrom, bedSpliceEnd(bed), bed->chromEnd))
	    {
	    if(set->junctCount + 1 >= set->maxJunctCount)
		{
		ExpandArray(set->bedArray, set->maxJunctCount, 2*set->maxJunctCount);
		set->maxJunctCount = 2 * set->maxJunctCount;
		}
	    set->chromEnd = max(set->chromEnd, bedSpliceEnd(bed));
	    set->chromStart = min(set->chromStart, bedSpliceStart(bed));
	    set->bedArray[set->junctCount++] = bed;
	    }
	}
    else
	errAbort("bed %s doesn't seem to fit the mold with chrom %s and chromStart %d",
		 bed->name, currentChrom, currentStart);
    }
slReverse(&setList);
return setList;
}    



boolean mergeableSets(struct junctionSet *start, struct junctionSet *end)
/* Return TRUE if there is a probe set in common between start and end. */
{
int i,j;	
if(start->merged == TRUE || end->merged == TRUE)
    return FALSE;
if(start->junctCount <=1 || end->junctCount <=1)
    return FALSE;
for(i = 0; i < start->junctCount; i++)
    {
    for(j = 0; j < end->junctCount; j++)
	{
	if(start->bedArray[i] == end->bedArray[j])
	    return TRUE;
	}
    }
return FALSE;
}

struct junctionSet* mergeSets(struct junctionSet *start, struct junctionSet *end)
/* Merge two sets. All juntions that are common to the both sets
   stay in the bedArray, everything else goes into the bedDupeArray. */
{
struct junctionSet *set = NULL;
int i = 0,j = 0;
if(start->junctCount > 1 && end->junctCount > 1)
    {
    fprintf(stdout, "Merging clusters %s and %s new coords %s:%d-%d\n", start->name, end->name,
	 start->chrom, min(start->chromStart,end->chromStart)-4, max(start->chromEnd, end->chromEnd)+5);
    setMergedCount++;
    }
/* Fill in basics and allocate arrays. */
AllocVar(set);
set->chrom = cloneString(start->chrom);
set->maxJunctCount = start->junctCount + end->junctCount;
set->chromStart = min(start->chromStart,end->chromStart);
set->chromEnd = max(start->chromEnd, end->chromEnd);
set->name = cloneString(start->name);
safef(set->strand, sizeof(set->strand), "%s", start->strand);
AllocArray(set->bedArray, set->maxJunctCount);
AllocArray(set->bedDupArray, set->maxJunctCount);

/* Loop through start beds looking for beds that are found 
   in just the starts or in both. */
for(i = 0; i < start->junctCount; i++) 
    {
    boolean found = FALSE;
    for(j = 0; j < end->junctCount; j++) 
	{
	if(start->bedArray[i] == end->bedArray[j])
	    {
	    set->bedArray[set->junctCount++] = start->bedArray[i];
	    found = TRUE;
	    }
	}
    if(found == FALSE) 
	set->bedDupArray[set->junctDupCount++] = start->bedArray[i];
    }

/* Look for ends for beds that are found just in the ends. */
for(i = 0; i < end->junctCount; i++) 
    {
    boolean found = FALSE;
    for(j = 0; j < start->junctCount; j++) 
	{
	if(end->bedArray[i] == start->bedArray[j])
	    {
	    found = TRUE;
	    }
	}
    if(found == FALSE) 
	set->bedDupArray[set->junctDupCount++] = end->bedArray[i];
    }

return set;
}

struct junctionSet* mergeSetLists(struct junctionSet *starts, struct junctionSet *ends)
/* Try to merge sets that have the same probes in both junctions.
   Don't want to count cassette exons twice for example (as they will
   have a set in both the starts and the ends). Also want to keep track
   of probes that were in different clusters that have now been moved.

   Algorithm: Look for clusters that have the same junction in both
   start and end. Merge these two cluster putting junctions that
   don't bridge in bedDupeArray. Once a cluter has been merged it cannot
   be merged again. That avoids transitivity in the merging process.
*/
{
struct junctionSet *mergedList = NULL, *jsStart = NULL, *jsEnd = NULL, *merged = NULL, *jsNext = NULL;
dotForUserInit(1000);
for(jsStart = starts; jsStart != NULL; jsStart = jsNext)
    {
    dotForUser();
    jsNext = jsStart->next;
    for(jsEnd = ends; jsEnd != NULL; jsEnd = jsEnd->next)
	{
	/* If we can merge them do so and add them to final list. */
	if(mergeableSets(jsStart, jsEnd))
	    {
	    merged = mergeSets(jsStart, jsEnd);
	    slAddHead(&mergedList, merged);
	    jsStart->merged = TRUE;
	    jsEnd->merged = TRUE;
	    break;
	    }
	}
    /* If this set can't be merged add it to the final list. */
    if(!jsStart->merged && jsStart->junctCount > 1)
	{
	slAddHead(&mergedList, jsStart);
	}
    }

/* Go through and add all the unmerged ends to the list. */
for(jsEnd = ends; jsEnd != NULL; jsEnd = jsNext)
    {
    jsNext = jsEnd->next;
    if(!jsEnd->merged && jsEnd->junctCount > 1)
	{
	slAddHead(&mergedList, jsEnd);
	}
    }
warn("\nDone Merging");
return mergedList;
}

void populateGeneSetHash(struct hash *hash, char *geneSetFile)
/* Create a hash that contains gene probeSet names indexed by their
   GXXXXXX identifier. */
{
struct lineFile *lf = NULL;
char *string = NULL;
char buff[256];
char *mark = NULL;
lf = lineFileOpen(geneSetFile, TRUE);
while(lineFileNextReal(lf, &string))
    {
    safef(buff, sizeof(buff), "%s", string);
    mark = strchr(buff, '_');
    if(mark == NULL)
	errAbort("Can't parse gene probe set name %s\n", buff);
    *mark = '\0';
    hashAddUnique(hash, buff, cloneString(string));
    }
lineFileClose(&lf);
}

char *findGeneSet(struct junctionSet *js, struct hash *hash)
/* Find the corresponding gene probe set for this junction set. */
{
char buff[256];
char *mark = NULL;
char *name = NULL;
safef(buff, sizeof(buff), "%s", js->bedArray[0]->name);
mark = strchr(buff, '@');
if(mark == NULL)
    errAbort("Can't parse gene name from %s", buff);
*mark = '\0';
name = hashFindVal(hash, buff);
return name;
}

void junctionSets(char *bedFile, char *setFile, char *geneSetFile) 
/* Bin up the beds into sets that have the same splice sites. */
{
struct bed *bed = NULL, *bedList = NULL;
struct junctionSet *starts = NULL, *ends = NULL, *merged = NULL, *js = NULL;
FILE *out = mustOpen(setFile, "w");
char *exonFile = optionVal("exonFile", NULL);
int totalCount = 0;
struct hash *geneSetHash = newHash(12);
warn("Reading beds.");
bedList = bedLoadAll(bedFile);
populateGeneSetHash(geneSetHash, geneSetFile);

if(exonFile != NULL)
    loadSoftExons(exonFile);
warn("Clustering starts.");
starts = findStartSets(bedList);
warn("Clustering ends.");
ends = findEndSets(bedList); 
warn("Merging clusters.");
merged = mergeSetLists(starts, ends); 
for(js = merged; js != NULL; js = js->next)
    {
    int i;
    if((js->junctCount + js->junctDupCount) > 1)
	{
	js->genePSet = findGeneSet(js, geneSetHash);
	/* The only ones we should be missing are the pesky CXXXX ones.
	   about 30 genes, don't know where they come from. */
	if(js->genePSet != NULL)
	    {
	    if(ctFile != NULL)
		{
		fprintf(ctFile, "%s\n", js->genePSet);
		for(i = 0; i < js->junctCount; i++)
		    bedTabOutN(js->bedArray[i], 12, ctFile);
		for(i = 0; i < js->junctDupCount; i++)
		    bedTabOutN(js->bedDupArray[i], 12, ctFile);
		}
	    junctionSetTabOut(js,out);
	    totalCount++;
	    }
	}
    }
warn("%d total alt sets. %d alt at start, %d alt at end, %d merged.",
     totalCount, setStartCount, setEndCount, setMergedCount);
bedFreeList(&bedList);
warn("Done.");
}

int main(int argc, char *argv[])
{
char *bedFile = NULL;
char *setFile = NULL;
char *ctFileName = NULL;
char *genePName = NULL;
if(argc == 1)
    usage();
optionInit(&argc, argv, optionSpecs);
if(optionExists("help"))
    usage();
bedFile = optionVal("bedFile", NULL);
setFile = optionVal("setFile", NULL);
ctFileName = optionVal("ctFile", NULL);
genePName = optionVal("genePSet", NULL);
if(bedFile == NULL || setFile == NULL || genePName == NULL)
    errAbort("Must specify bedFile, setFile and genePSet files, use -help for usage.");
if(ctFileName != NULL)
    initCtFile(ctFileName);
if(bedFile == NULL || setFile == NULL)
    {
    warn("Error: Must specify both a bedFile and a setFile.");
    usage();
    }
junctionSets(bedFile, setFile, genePName);
carefulClose(&ctFile);
return 0;
}
