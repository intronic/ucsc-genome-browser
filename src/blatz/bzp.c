/* bzp - blatz parameters.  Input settings structure for aligner.  Routine to
 * set options from command line.  A debugging/profiling utility function. */
/* Copyright 2005 Jim Kent.  All rights reserved. */

#include "common.h"
#include "hash.h"
#include "options.h"
#include "axt.h"
#include "gapCalc.h"
#include "portable.h"
#include "bzp.h"
#include "blatz.h"

static int minWeight = 6, maxWeight=15;

struct bzp *bzpDefault()
/* Return default parameters */
{
struct bzp *bzp;
AllocVar(bzp);
bzp->weight = 11;
bzp->rna = 0;
bzp->minScore = 2000;
bzp->multiHits = 1;
bzp->minGapless = 1600;
bzp->minChain = 2000;
bzp->maxDrop = 1500;
bzp->maxExtend = 1500;
bzp->maxBandGap = 100;
bzp->expandWindow = 10000;
bzp->minExpand = 3000;
bzp->ss = axtScoreSchemeDefault();
bzp->cheapGap = gapCalcCheap();
bzp->gapCalc = gapCalcDefault();
bzp->out = "chain";
bzp->mafQ = "";
bzp->mafT = "";
return bzp;
}

void bzpServerOptionsHelp(struct bzp *bzp)
/* Explain options having to do with server side of alignments. */
{
printf("  -weight=%d - Set number of significant bases in seeds.  Allowed range\n"
       "               is %d to %d.  Smaller seeds are slower but more sensitive\n"
       "               This controls sensitivity at lowest level\n"
       , bzp->weight, minWeight, maxWeight);
}

void bzpClientOptionsHelp(struct bzp *bzp)
/* Explain options having to do with client side of alignments. */
{
printf("  -rna - If set will treat query as mRNA and look for introns in gaps\n");  
printf("  -multiHits=%d - If nonzero takes multiple hits on diagonal to trigger\n"
       "                  gapless extension (MSP). Greatly speeds up searches\n"
       "                  of larger databases at a modest cost in sensitivity\n", 
       	bzp->multiHits);
printf("  -minScore=%d - Minimum score of to output after final chaining.  Each \n"
       "                 matching base contributes roughly 100 to the score.  This has\n"
       "                 little effect on the speed, but higher minScores will weed out\n"
       "                 weaker alignments.  Controls sensitivity at highest level.\n"
       , bzp->minScore);
printf("  -minGapless=%d - Minimum score of maximal gapless alignment (MSP) to \n"
       "                  trigger first level of chaining.\n"
       , bzp->minGapless);
printf("  -minChain=%d - Minimum score of  first level chain to trigger \n"
       "                 banded Smith-Waterman extension.\n", bzp->minChain);
printf("  -maxDrop=%d - Maximum amount score is allowed to drop before terminating\n"
       "                banded extension\n",  bzp->maxDrop);
printf("  -maxExtend=%d - Maximum number of bases to add in banded extension.\n"
       , bzp->maxExtend);
printf("  -maxBandGap=%d - Maximum gap size allowed in banded extension phase\n"
       , bzp->maxBandGap);
printf("  -minExpand=%d - Minimum score for chain to try expansion by doing\n"
       "                  local alignment with a smaller seed.\n"
       , bzp->minExpand);
printf("  -expandWindow=%d - Maximum size of window between blocks of chains\n"
       "                    and before and after first block in which to look\n"
       "                    for alignments using seeds of smaller weight.\n"
       , bzp->expandWindow);
printf("  -matrix=fileName - Read scoring matrix from file.\n");
printf("  -gapCost=fileName - Read gap scoring scheme from file.\n");
printf("  -verbose=%d - Print progress info. 0=silent, 1=default, 2=wordy\n", 
	verboseLevel());
printf("  -out=%s - Output in given format.  Options are chain, axt, maf, psl.\n",
	bzp->out);
printf("                For maf there are -mafT=%s and -mafQ=%s options to control\n"
       "                the sequence prefixes in maf files\n", bzp->mafT, bzp->mafQ);
}

void bzpSetOptions(struct bzp *bzp)
/* Modify options from command line. */
{
bzp->weight = optionInt("weight", bzp->weight);
bzp->rna = optionExists("rna");
bzp->minScore = optionInt("minScore", bzp->minScore);
bzp->multiHits = optionInt("multiHits", bzp->multiHits);
bzp->minGapless = optionInt("minGapless", bzp->minGapless);
bzp->minChain = optionInt("minChain", bzp->minChain);
bzp->maxDrop = optionInt("maxDrop", bzp->maxDrop);
bzp->maxExtend = optionInt("maxExtend", bzp->maxExtend);
bzp->maxBandGap = optionInt("maxBandGap", bzp->maxBandGap);
bzp->expandWindow = optionInt("expandWindow", bzp->expandWindow);
bzp->minExpand = optionInt("minExpand", bzp->minExpand);
if (optionExists("matrix"))
    bzp->ss = axtScoreSchemeRead(optionVal("matrix", NULL));
if (optionExists("gapCost"))
    bzp->gapCalc = gapCalcFromFile(optionVal("gapCost", NULL));
else if (bzp->rna)
    bzp->gapCalc = gapCalcRnaDna();
bzp->out = optionVal("out", bzp->out);
bzp->mafT = optionVal("mafT", bzp->mafT);
bzp->mafQ = optionVal("mafQ", bzp->mafQ);

/* Do some checking */
if (bzp->weight < minWeight || bzp->weight > maxWeight)
    errAbort("weight must be between %d and %d", minWeight, maxWeight);
}

int bzpVersion()
/* Return version number. */
{
return 1;
}


boolean bzpTimeOn = TRUE;

void bzpTime(char *label, ...)
/* Print label and how long it's been since last call.  Call with 
 * a NULL label to initialize. */
{
if (bzpTimeOn && verboseLevel() > 1)
    {
    static long lastTime = 0;
    long time = clock1000();
    va_list args;
    va_start(args, label);
    if (label != NULL)
	{
	/* fprintf(stdout, "%ld (pid %d): ", time - lastTime, getpid()); */
	fprintf(stdout, "%ld: ", time - lastTime);
	vfprintf(stdout, label, args);
	fprintf(stdout, "\n");
	}
    lastTime = time;
    va_end(args);
    }
}
