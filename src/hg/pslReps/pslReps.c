/* pslReps - analyse repeats and generate list of best alignments
 * from a genome wide, sorted by mRNA .psl alignment file.
 */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "memalloc.h"
#include "psl.h"
#include "cheapcgi.h"

static char const rcsid[] = "$Id: pslReps.c,v 1.14 2003/08/11 21:28:33 sugnet Exp $";

double minAli = 0.93;
double nearTop = 0.01;
double minCover = 0.0;
boolean ignoreNs = FALSE; /* Ignore the n's when calculating coverage score, other
			     wise n's count as mismatches. */
boolean ignoreSize = FALSE;
boolean noIntrons = FALSE;
boolean singleHit = FALSE;
boolean noHead = FALSE;
boolean quiet = FALSE;
int minNearTopSize = 30;

void usage()
/* Print usage instructions and exit. */
{
errAbort(
    "pslReps - analyse repeats and generate genome wide best\n"
    "alignments from a sorted set of local alignments\n"
    "usage:\n"
    "    pslReps in.psl out.psl out.psr\n"
    "where in.psl is an alignment file generated by psLayout and\n"
    "sorted by pslSort, out.psl is the best alignment output\n"
    "and out.psr contains repeat info\n"
    "options:\n"
    "    -nohead don't add PSL header\n"
    "    -ignoreSize Will not weigh in favor of larger alignments so much\n"
    "    -noIntrons Will not penalize for not having introns when calculating\n"
    "              size factor\n"
    "    -singleHit  Takes single best hit, not splitting into parts\n"
    "    -minCover=0.N minimum coverage to output.  Default is 0.\n"
    "    -ignoreNs Ignore 'N's when calculating minCover.\n"
    "    -minAli=0.N minimum alignment ratio\n"
    "               default is 0.93\n"
    "    -nearTop=0.N how much can deviate from top and be taken\n"
    "               default is 0.01\n"
    "    -minNearTopSize=N  Minimum size of alignment that is near top\n"
    "               for aligmnent to be kept.  Default 30.\n");
}

int calcMilliScore(struct psl *psl)
/* Figure out percentage score. */
{
return 1000-pslCalcMilliBad(psl, TRUE);
}

int intronFactor(struct psl *psl)
/* Figure bonus for having introns.  An intron is worth 3 bases... 
 * An intron in this case is just a gap of 0 bases in query and
 * 30 or more in target. */
{
int i, blockCount = psl->blockCount;
int ts, qs, te, qe, sz;
int bonus = 0;
if (blockCount <= 1)
    return 0;
sz = psl->blockSizes[0];
qe = psl->qStarts[0] + sz;
te = psl->tStarts[0] + sz;
for (i=1; i<blockCount; ++i)
    {
    qs = psl->qStarts[i];
    ts = psl->tStarts[i];
    if (qs == qe && ts - te >= 30)
        bonus += 3;
    sz = psl->blockSizes[i];
    qe = qs + sz;
    te = ts + sz;
    }
if (bonus > 10) bonus = 10;
return bonus;
}

int sizeFactor(struct psl *psl)
/* Return a factor that will favor longer alignments. */
{
int score;
if (ignoreSize) return 0;
score = 4*round(sqrt(psl->match + psl->repMatch/4));
if (!noIntrons)
    score += intronFactor(psl);
return score;
}

int calcSizedScore(struct psl *psl)
/* Return score that includes base matches and size. */
{
int score = calcMilliScore(psl) + sizeFactor(psl);
return score;
}

boolean uglyTarget(struct psl *psl)
/* Return TRUE if it's the one we're snooping on. */
{
return sameString(psl->qName, "AF103731");
}


boolean closeToTop(struct psl *psl, int *scoreTrack)
/* Returns TRUE if psl is near the top scorer for at least 20 bases. */
{
int milliScore = calcSizedScore(psl);
int threshold = round(milliScore * (1.0+nearTop));
int i, blockIx;
int start, size, end;
int topCount = 0;
char strand = psl->strand[0];

if (uglyTarget(psl)) uglyf("%s:%d milliScore %d, threshold %d\n", psl->tName, psl->tStart, milliScore, threshold);
for (blockIx = 0; blockIx < psl->blockCount; ++blockIx)
    {
    start = psl->qStarts[blockIx];
    size = psl->blockSizes[blockIx];
    end = start+size;
    if (strand == '-')
	reverseIntRange(&start, &end, psl->qSize);
    for (i=start; i<end; ++i)
	{
	if (scoreTrack[i] <= threshold)
	    {
	    if (++topCount >= minNearTopSize)
		return TRUE;
	    }
	}
    }
return FALSE;
}

boolean passMinCoverage(struct psl *psl)
/* Does this psl have enough bases aligned to pass the minCover filter. */
{
if(ignoreNs)
    return (psl->match + psl->repMatch >= minCover * (psl->qSize - psl->nCount));
else
    return (psl->match + psl->repMatch >= minCover * psl->qSize);
}

boolean passFilters(struct psl *psl, int *scoreTrack)
/* Return TRUE if this psl passes the millScore, minCoverage and
   closeToTop thresholds.  If scoreTrack is NULL then closeToTop
   threshold is skipped. */
{
int milliMin = 1000*minAli;
if (calcMilliScore(psl) >= milliMin && 
    (scoreTrack == NULL || closeToTop(psl, scoreTrack)) &&
    passMinCoverage(psl))
    return TRUE;
return FALSE;
}

void processBestMulti(char *acc, struct psl *pslList, FILE *bestFile, FILE *repFile)
/* Find psl's that are best anywhere along their length. */
{
struct psl *psl;
int qSize;
int *repTrack = NULL;
int *scoreTrack = NULL;
int milliScore;
int pslIx;
int goodAliCount = 0;
int bestAliCount = 0;
int milliMin = 1000*minAli;


if (pslList == NULL)
    return;
qSize = pslList->qSize;
AllocArray(repTrack, qSize+1);
AllocArray(scoreTrack, qSize+1);

for (psl = pslList; psl != NULL; psl = psl->next)
    {
    int blockIx;
    char strand = psl->strand[0];
    milliScore = calcMilliScore(psl);
    if (milliScore >= milliMin)
	{
	++goodAliCount;
	milliScore += sizeFactor(psl);
if (uglyTarget(psl)) uglyf("@%s:%d milliScore %d\n", psl->tName, psl->tStart, milliScore);
	for (blockIx = 0; blockIx < psl->blockCount; ++blockIx)
	    {
	    int start = psl->qStarts[blockIx];
	    int size = psl->blockSizes[blockIx];
	    int end = start+size;
	    int i;
	    if (strand == '-')
	        reverseIntRange(&start, &end, psl->qSize);
	    if (start < 0 || end > qSize)
		{
		warn("Odd: qName %s tName %s qSize %d psl->qSize %d start %d end %d",
		    psl->qName, psl->tName, qSize, psl->qSize, start, end);
		if (start < 0)
		    start = 0;
		if (end > qSize)
		    end = qSize;
		}
	    for (i=start; i<end; ++i)
		{
		repTrack[i] += 1;
		if (milliScore > scoreTrack[i])
		    scoreTrack[i] = milliScore;
		}
	    }
	}
    }
if (uglyTarget(pslList)) uglyf("---finding best---\n");
/* Print out any alignments that are within 2% of top score. */
for (psl = pslList; psl != NULL; psl = psl->next)
    {
    if(passFilters(psl, scoreTrack))
	{
	if (uglyTarget(psl)) uglyf("accepted\n");
	pslTabOut(psl, bestFile);
	++bestAliCount;
	}
    }


/* Print out run-length-encoded repeat info.  */
    {
    int runVal = repTrack[0];
    int curVal;
    int runSize = 1;
    int runStart = 0;
    int packetCount = 0;
    int *packetSize = NULL;
    int *packetVal = NULL;
    int i;

    AllocArray(packetSize, qSize);
    AllocArray(packetVal, qSize);

    repTrack[qSize] = -1;	/* Sentinal value to simplify RLC loop. */
    fprintf(repFile, "%s\t%d\t%d\t", acc, bestAliCount, goodAliCount);

    for (i=1; i<=qSize; ++i)
	{
	if ((curVal = repTrack[i]) != runVal)
	    {
	    packetSize[packetCount] = runSize;
	    packetVal[packetCount] = runVal;
	    ++packetCount;
	    runSize = 1;
	    runVal = curVal;
	    }
	else
	    {
	    ++runSize;
	    }
	}
    fprintf(repFile, "%d\t", packetCount);
    for (i=0; i<packetCount; ++i)
	fprintf(repFile, "%d,", packetSize[i]);
    fprintf(repFile, "\t");
    for (i=0; i<packetCount; ++i)
	fprintf(repFile, "%d,", packetVal[i]);
    fprintf(repFile, "\n");

    carefulCheckHeap();
    freeMem(packetSize);
    freeMem(packetVal);
    }

carefulCheckHeap();
freeMem(repTrack);
freeMem(scoreTrack);
}

void processBestSingle(char *acc, struct psl *pslList, FILE *bestFile, FILE *repFile)
/* Find single best psl in list. */
{
struct psl *bestPsl = NULL, *psl;
int bestScore = 0, score, threshold;

for (psl = pslList; psl != NULL; psl = psl->next)
    {
    score = pslScore(psl);
    if (score > bestScore)
        {
	bestScore = score;
	bestPsl = psl;
	}
    }
threshold = round((1.0 - nearTop)*bestScore);
for (psl = pslList; psl != NULL; psl = psl->next)
    {
    if (pslScore(psl) >= threshold && passFilters(psl, NULL))
        pslTabOut(psl, bestFile);
    }
}

void doOneAcc(char *acc, struct psl *pslList, FILE *bestFile, FILE *repFile)
/* Process alignments of one piece of mRNA. */
{
if (singleHit)
    processBestSingle(acc, pslList, bestFile, repFile);
else
    processBestMulti(acc, pslList, bestFile, repFile);
}

void pslReps(char *inName, char *bestAliName, char *repName)
/* Analyse inName and put best alignments for eacmRNA in estAliName.
 * Put repeat info in repName. */
{
struct lineFile *in = pslFileOpen(inName);
FILE *bestFile = mustOpen(bestAliName, "w");
FILE *repFile = mustOpen(repName, "w");
int lineSize;
char *line;
char *words[32];
int wordCount;
struct psl *pslList = NULL, *psl = NULL;
char lastName[512];
int aliCount = 0;
quiet = sameString(bestAliName, "stdout") || sameString(repName, "stdout");

if (!quiet)
    printf("Processing %s to %s and %s\n", inName, bestAliName, repName);
 if (!noHead)
     pslWriteHead(bestFile);
strcpy(lastName, "");
while (lineFileNext(in, &line, &lineSize))
    {
    if (((++aliCount & 0x1ffff) == 0) && !quiet)
        {
	printf(".");
	fflush(stdout);
	}
    wordCount = chopTabs(line, words);
    if (wordCount != 21)
	errAbort("Bad line %d of %s\n", in->lineIx, in->fileName);
    psl = pslLoad(words);
    if (!sameString(lastName, psl->qName))
	{
	doOneAcc(lastName, pslList, bestFile, repFile);
	pslFreeList(&pslList);
	safef(lastName, sizeof(lastName), "%s", psl->qName);
	}
    slAddHead(&pslList, psl);
    }
doOneAcc(lastName, pslList, bestFile, repFile);
pslFreeList(&pslList);
lineFileClose(&in);
fclose(bestFile);
fclose(repFile);
if (!quiet)
    printf("Processed %d alignments\n", aliCount);
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
if (argc != 4)
    usage();
minAli = cgiOptionalDouble("minAli", minAli);
nearTop = cgiOptionalDouble("nearTop", nearTop);
minCover = cgiOptionalDouble("minCover", minCover);
minNearTopSize = cgiOptionalInt("minNearTopSize", minNearTopSize);
ignoreSize = cgiBoolean("ignoreSize");
noIntrons = cgiBoolean("noIntrons");
singleHit = cgiBoolean("singleHit");
noHead = cgiBoolean("nohead");
ignoreNs = cgiBoolean("ignoreNs");
pslReps(argv[1], argv[2], argv[3]);
return 0;
}
