#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "axt.h"
#include "obscure.h"
#include "genoFind.h"

extern int version;	/* Blat version number. */
extern char *databaseName;		/* File name of database. */
extern int databaseSeqCount;	/* Number of sequences in database. */
extern unsigned long databaseLetterCount;	/* Number of bases in database. */

struct targetHits
/* Collection of hits to a single target. */
    {
    struct targetHits *next;
    char *name;	    	    /* Target name */
    int size;		    /* Target size */
    struct axt *axtList;    /* List of axts, sorted by score. */
    int score;		    /* Score of best element */
    };

void targetHitsFree(struct targetHits **pObj)
/* Free one target hits structure. */
{
struct targetHits *obj = *pObj;
if (obj != NULL)
    {
    freeMem(obj->name);
    axtFreeList(&obj->axtList);
    freez(pObj);
    }
}

void targetHitsFreeList(struct targetHits **pList)
/* Free a list of dynamically allocated targetHits's */
{
struct targetHits *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    targetHitsFree(&el);
    }
*pList = NULL;
}


int targetHitsCmpScore(const void *va, const void *vb)
/* Compare to sort based on score. */
{
const struct targetHits *a = *((struct targetHits **)va);
const struct targetHits *b = *((struct targetHits **)vb);

return b->score - a->score;
}

int blastzToWublastScore(int bzScore)
/* Convert from 100 points/match blastz score to 5 points/match
 * wu-blast score. */
{
return bzScore/19;
}

double blastzScoreToWuBits(int bzScore)
/* Convert from blastz score to bit score.  The magic number
 * 32.1948 was derived from the wu-blast bit to score ratio.
 * I'm not sure I agree with this, but am doing it to be compatible.   
 * I'd tend to give 2 bits for each matching base more or less. 
 * This is much less. */
{
return blastzToWublastScore(bzScore) * 0.1553;
}

double blastzScoreToWuExpectation(int bzScore, double databaseSize)
/* I'm puzzled by the wu-blast expectation score.  I would
 * think it would be  databaseSize / (2^bitScore)  but
 * it's not.   I think the best I can do is approximate
 * it with something scaled to be close to this expectation. */
{
double logProbOne = -log(2) * bzScore / 32.1948;
return databaseSize * exp(logProbOne);
}

double expectationToProbability(double e)
/* Convert expected number of hits to probability of at least
 * one hit.  This is a crude approximation, but actually pretty precise
 * for e < 0.1, which is all that really matters.... */
{
if (e < 0.999)
    return e;
else
    return 0.999;
}

int countMatches(char *a, char *b, int size)
/* Count number of characters that match between a and b. */
{
int count = 0;
int i;
for (i=0; i<size; ++i)
    if (toupper(a[i]) == toupper(b[i]))
        ++count;
return count;
}

int plusStrandPos(int pos, int size, char strand, boolean isEnd)
/* Return position on plus strand, one based. */
{
if (strand == '-')
    {
    pos = size - pos;
    if (isEnd)
       ++pos;
    }
else
    {
    if (!isEnd)
        ++pos;
    }
return pos;
}

void blastiodAxtOutput(FILE *f, struct axt *axt, int tSize, int qSize, 
	int lineSize)
/* Output base-by-base part of alignment in blast-like fashion. */
{
int tOff = axt->tStart;
int qOff = axt->qStart;
int lineStart, lineEnd, i;

for (lineStart = 0; lineStart < axt->symCount; lineStart = lineEnd)
    {
    lineEnd = lineStart + lineSize;
    if (lineEnd > axt->symCount) lineEnd = axt->symCount;
    fprintf(f, "Query: %6d ", plusStrandPos(qOff, qSize, axt->qStrand, FALSE));
    for (i=lineStart; i<lineEnd; ++i)
	{
	char c = axt->qSym[i];
	fputc(c, f);
	if (c != '-')
	    ++qOff;
	}
    fprintf(f, " %-d\n", plusStrandPos(qOff, qSize, axt->qStrand, TRUE));
    fprintf(f, "              ");
    for (i=lineStart; i<lineEnd; ++i)
        {
	char c = ((axt->qSym[i] == axt->tSym[i]) ? '|' : ' ');
	fputc(c, f);
	}
    fprintf(f, "\n");
    fprintf(f, "Sbjct: %6d ", plusStrandPos(tOff, tSize, axt->tStrand, FALSE));
    for (i=lineStart; i<lineEnd; ++i)
	{
	char c = axt->tSym[i];
	fputc(c, f);
	if (c != '-')
	    ++tOff;
	}
    fprintf(f, " %-d\n", plusStrandPos(tOff, tSize, axt->tStrand, TRUE));
    fprintf(f, "\n");
    }
}

void wublastQueryOut(FILE *f, void *data)
/* Do wublast-like output at end of processing query. */
{
struct gfSaveAxtData *aod = data;
char asciiNum[32];
struct hash *targetHash = newHash(10);
struct targetHits *targetList = NULL, *target;
struct gfAxtBundle *gab;
struct axtScoreScheme *ss = axtScoreSchemeDefault();
char *queryName;
int isRc;
int querySize = 0;

++aod->queryIx;
if (aod->bundleList == NULL)
    return;

/* Print out stuff that doesn't depend on query or database. */
fprintf(f, "BLASTN 2.0MP-WashU [blat v%d]\n", version);
fprintf(f, "\n");
fprintf(f, "Copyright (C) 2000-2002 Jim Kent\n");
fprintf(f, "All Rights Reserved\n");
fprintf(f, "\n");
fprintf(f, "Reference:  Kent, WJ. (2002) BLAT - The BLAST-like alignment tool\n");
fprintf(f, "\n");
fprintf(f, "Notice:  this program and its default parameter settings are optimized to find\n");
fprintf(f, "nearly identical sequences very rapidly.  For slower but more sensitive\n");
fprintf(f, "alignments please use other methods.\n");
fprintf(f, "\n");

/* Print query and database info. */
queryName = aod->bundleList->axtList->qName;
fprintf(f, "Query=  %s\n", queryName);
fprintf(f, "        (%d letters; record %d)\n", aod->bundleList->qSize, aod->queryIx);
fprintf(f, "\n");
fprintf(f, "Database:  %s\n",  databaseName);
sprintLongWithCommas(asciiNum, databaseLetterCount);
fprintf(f, "           %d sequences; %s total letters\n",  databaseSeqCount, asciiNum);
fprintf(f, "Searching....10....20....30....40....50....60....70....80....90....100%% done\n");
fprintf(f, "\n");

/* Build up a list of targets in database hit by query sorted by
 * score of hits. */
for (gab = aod->bundleList; gab != NULL; gab = gab->next)
    {
    struct axt *axt, *next;
    querySize = gab->qSize;
    for (axt = gab->axtList; axt != NULL; axt = next)
	{
	next = axt->next;
	axt->score = axtScore(axt, ss);
	target = hashFindVal(targetHash, axt->tName);
	if (target == NULL)
	    {
	    AllocVar(target);
	    slAddHead(&targetList, target);
	    hashAdd(targetHash, axt->tName, target);
	    target->name = cloneString(axt->tName);
	    target->size = gab->tSize;
	    }
	if (axt->score > target->score)
	    target->score = axt->score;
	slAddHead(&target->axtList, axt);
	}
    gab->axtList = NULL;
    }
slSort(&targetList, targetHitsCmpScore);
for (target = targetList; target != NULL; target = target->next)
    slSort(&target->axtList, axtCmpScore);


/* Print out summary of hits. */
fprintf(f, "                                                                     Smallest\n");
fprintf(f, "                                                                       Sum\n");
fprintf(f, "                                                              High  Probability\n");
fprintf(f, "Sequences producing High-scoring Segment Pairs:              Score  P(N)      N\n");
fprintf(f, "\n");
for (target = targetList; target != NULL; target = target->next)
    {
    double expectation = blastzScoreToWuExpectation(target->score, databaseLetterCount);
    double p = expectationToProbability(expectation);
    fprintf(f, "%-61s %4d  %8.1e %2d\n", target->name, 
    	blastzToWublastScore(target->score), p, slCount(target->axtList));
    }

/* Print out details on each target. */
for (target = targetList; target != NULL; target = target->next)
    {
    fprintf(f, "\n\n>%s\n", target->name);
    fprintf(f, "        Length = %d\n", target->size);
    fprintf(f, "\n");
    for (isRc=0; isRc <= 1; ++isRc)
	{
	boolean saidStrand = FALSE;
	char strand = (isRc ? '-' : '+');
	char *strandName = (isRc ? "Minus" : "Plus");
	struct axt *axt;
	for (axt = target->axtList; axt != NULL; axt = axt->next)
	    {
	    if (axt->qStrand == strand)
		{
		int matches = countMatches(axt->qSym, axt->tSym, axt->symCount);
		if (!saidStrand)
		    {
		    saidStrand = TRUE;
		    fprintf(f, "  %s Strand HSPs:\n\n", strandName);
		    }
		fprintf(f, " Score = %d (%2.1f bits), Expect = %5.1e, Sum P(%d) = %5.1e\n",
		     blastzToWublastScore(axt->score), 
		     blastzScoreToWuBits(axt->score),
		     blastzScoreToWuExpectation(axt->score, databaseLetterCount),
		     4, 
		     blastzScoreToWuExpectation(axt->score, databaseLetterCount));
		fprintf(f, " Identities = %d/%d (%d%%), Positives = %d/%d (%d%%), Strand = %s / Plus\n",
		     matches, axt->symCount, round(100.0 * matches / axt->symCount),
		     matches, axt->symCount, round(100.0 * matches / axt->symCount),
		     strandName);
		fprintf(f, "\n");
		blastiodAxtOutput(f, axt, target->size, querySize, 60);
		}
	    }
	}
    }

/* Cleanup time. */
gfAxtBundleFreeList(&aod->bundleList);
hashFree(&targetHash);
targetHitsFreeList(&targetList);
}

