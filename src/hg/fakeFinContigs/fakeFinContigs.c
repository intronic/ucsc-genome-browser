/* fakeFinContigs - Fake up contigs for a finished chromosome. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "agpFrag.h"
#include "fa.h"
#include "portable.h"
#include "hCommon.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "fakeFinContigs - Fake up contigs for a finished chromosome\n"
  "usage:\n"
  "   fakeFinContigs fin.agp fin.fa finDir rootName finFaDir\n"
  "This will scan fin.agp for gaps, and create contigs in finDir\n"
  "for each section between gaps\n"
  "Example:\n"
  "   fakeFinContigs chr20.agp chr20.fa . ctg20fin ~/gs/fin/fa\n"
  );
}

struct contig
/* A bunch of sequence without a gap. */
    {
    struct contig *next;	/* Next in list. */
    char name[16];		/* Contig name. */
    int startOffset;		/* Start offset in chromosome. */
    int endOffset;		/* End offset in chromosome. */
    struct agpFrag *agpList;	/* List of fragments that make it up. */
    };

void fakeFinContigs(char *agpName, char *faName, char *finDir, char *rootName, char *finFaDir)
/* fakeFinContigs - Fake up contigs for a finished chromosome. */
{
struct contig *contigList = NULL, *contig = NULL;
struct agpFrag *agp;
struct lineFile *lf = lineFileOpen(agpName, TRUE);
char *line, *words[16];
int lineSize, wordCount;
int contigIx = 0;
char liftDir[512], contigDir[512], path[512];
char chrom[128];
FILE *f;
struct dnaSeq *seq;
int fragIx;

/* Build up contig list by scanning agp file. */
printf("Reading %s\n", lf->fileName);
while (lineFileNext(lf, &line, &lineSize))
    {
    if (line[0] == '#' || line[0] == 0)
        continue;
    wordCount = chopLine(line, words);
    if (wordCount < 5)
        errAbort("Expecting at least 5 words line %d of %s", lf->lineIx, lf->fileName);
    if (words[4][0] == 'N')
	{
        contig = NULL;
        continue;
	}
    lineFileExpectWords(lf, 9, wordCount);
    agp = agpFragLoad(words);
    if (contig == NULL)
	{
        AllocVar(contig);
	sprintf(contig->name, "%s%d", rootName, ++contigIx);
	contig->startOffset = agp->chromStart;
	slAddHead(&contigList, contig);
	}
    else 
        {
	if (contig->agpList != NULL && contig->agpList->chromEnd != agp->chromStart)
	    errAbort("Start doesn't match previous end line %d of %s", 
	    	lf->lineIx, lf->fileName);
	}
    if (agp->chromEnd - agp->chromStart != agp->fragEnd - agp->fragStart)
        errAbort("Chrom and frag size mismatch line %d of %s", lf->lineIx, lf->fileName);
    slAddHead(&contig->agpList, agp);
    contig->endOffset = agp->chromEnd;
    }
slReverse(&contigList);
for (contig = contigList; contig != NULL; contig = contig->next)
    slReverse(&contig->agpList);
lineFileClose(&lf);

/* Load up chromosome sequence and make sure it is in one piece. */
printf("Reading %s\n", faName);
seq = faReadAllDna(faName);
if (slCount(seq) != 1)
    errAbort("Got %d sequences in %s, can only handle one.", slCount(seq), faName);

/* Fix up agp coordinates. Make a directory for each contig.  Fill it with 
 * .fa .agp barge.99 files for that contig. */
printf("Writing contig dirs\n");
for (contig = contigList; contig != NULL; contig = contig->next)
    {
    /* Make Contig dir. */
    sprintf(contigDir, "%s/%s", finDir, contig->name);
    makeDir(contigDir);

    /* Make contig.agp file. */
    sprintf(path, "%s/%s.agp", contigDir, contig->name);
    f = mustOpen(path, "w");
    fragIx = 0;
    for (agp = contig->agpList; agp != NULL; agp = agp->next)
	{
	agp->chromStart -= contig->startOffset;
	agp->chromEnd -= contig->startOffset;
	agp->ix = ++fragIx;
	agpFragTabOut(agp, f);
	}
    carefulClose(&f);

    /* Make contig.fa file. */
    sprintf(path, "%s/%s.fa", contigDir, contig->name);
    faWrite(path, contig->name, seq->dna + contig->startOffset, 
    	contig->endOffset - contig->startOffset);

    /* Make contig/barge file. */
    sprintf(path, "%s/barge.99", contigDir);
    f = mustOpen(path, "w");
    fprintf(f, "Barge (Connected Clone) File ooGreedy Version 99\n");
    fprintf(f, "\n");
    fprintf(f, "start  accession  size overlap maxClone maxOverlap\n");
    fprintf(f, "------------------------------------------------------------\n");
    for (agp = contig->agpList; agp != NULL; agp = agp->next)
        {
	char clone[128];
	strcpy(clone, agp->frag);
	chopSuffix(clone);
	
	fprintf(f, "%d\t%s\t%d\t100\tn/a\t0\n", agp->chromStart, clone,
		agp->chromEnd - agp->chromStart);
	}
    carefulClose(&f);

    /* Make contig/gold file. */
    sprintf(path, "%s/gold.99", contigDir);
    f = mustOpen(path, "w");
    fragIx = 0;
    for (agp = contig->agpList; agp != NULL; agp = agp->next)
        {
	char fragName[128];
	struct agpFrag frag = *agp;
	sprintf(fragName, "%s_1", agp->frag);
	frag.frag = fragName;
	frag.type[0] = '0';
	agpFragTabOut(&frag, f);
	}
    carefulClose(&f);
    }

/* Create lift subdirectory. */
printf("Creating lift files\n");
sprintf(liftDir, "%s/lift", finDir);
makeDir(liftDir);

/* Create lift/oOut.lst file (just a list of contigs). */
sprintf(path, "%s/oOut.lst", liftDir);
f = mustOpen(path, "w");
for (contig = contigList; contig != NULL; contig = contig->next)
    fprintf(f, "%s/%s.fa.out\n", contig->name, contig->name);
carefulClose(&f);

/* Create lift/ordered.lft file. */
sprintf(path, "%s/ordered.lft", liftDir);
f = mustOpen(path, "w");
splitPath(faName, NULL, chrom, NULL);
for (contig = contigList; contig != NULL; contig = contig->next)
    fprintf(f, "%d\t%s/%s\t%d\t%s\t%d\n", 
	contig->startOffset, skipChr(chrom), contig->name,  
	contig->endOffset - contig->startOffset,
	chrom, seq->size);
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
if (argc != 6)
    usage();
fakeFinContigs(argv[1], argv[2], argv[3], argv[4], argv[5]);
return 0;
}
