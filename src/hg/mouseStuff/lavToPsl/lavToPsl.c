/* lavToPsl - Convert blastz lav to psl format. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"

static char const rcsid[] = "$Id: lavToPsl.c,v 1.6 2003/05/06 07:22:28 kate Exp $";

/* strand to us for target */
char* targetStrand = "+";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "lavToPsl - Convert blastz lav to psl format\n"
  "usage:\n"
  "   lavToPsl in.lav out.psl\n"
  "options:\n"
  "   -target-strand=c set the target strand to c (default is no strand)\n"
  );
}

struct block
/* A block of an alignment. */
    {
    struct block *next;
    int qStart, qEnd;	/* Query position. */
    int tStart, tEnd;	/* Target position. */
    int percentId;	/* Percentage identity. */
    };

void outputBlocks(struct block *blockList, FILE *f, boolean isRc, 
	char *qName, int qSize, char *tName, int tSize, boolean psl)
/* Output block list to file. */
{
int qNumInsert = 0, qBaseInsert = 0, tNumInsert = 0, tBaseInsert = 0;
int matchOne, match = 0, mismatch = 0, bases;
double scale;
struct block *lastBlock = NULL;
int score, blockCount = 0;
int qTotalStart = BIGNUM, qTotalEnd = 0, tTotalStart = BIGNUM, tTotalEnd = 0;
struct block *block;

if (blockList == NULL)
    return;

/* Count up overall statistics. */
for (block = blockList; block != NULL; block = block->next)
    {
    ++blockCount;
    scale = 0.01 * block->percentId;
    bases = block->qEnd - block->qStart;
    matchOne = round(scale*bases);
    match += matchOne;
    mismatch += bases - matchOne;
    if (lastBlock != NULL)
	{
	if (block->qStart != lastBlock->qEnd)
	    {
	    qNumInsert += 1;
	    qBaseInsert += block->qStart - lastBlock->qEnd;
	    }
	if (block->tStart != lastBlock->tEnd)
	    {
	    tNumInsert += 1;
	    tBaseInsert += block->tStart - lastBlock->tEnd;
	    }
	qTotalEnd = block->qEnd;
	tTotalEnd = block->tEnd;
	}
    else
	{
	qTotalStart = block->qStart;
	qTotalEnd = block->qEnd;
	tTotalStart = block->tStart;
	tTotalEnd = block->tEnd;
	}
    lastBlock = block;
    }

if (psl)
    {
    fprintf(f, "%d\t%d\t0\t0\t", match, mismatch);
    fprintf(f, "%d\t%d\t%d\t%d\t", qNumInsert, qBaseInsert, tNumInsert, tBaseInsert);
    fprintf(f, "%c%s\t", (isRc ? '-' : '+'), targetStrand);
    /* if query is - strand, convert start/end to genomic */
    if (isRc)
        fprintf(f, "%s\t%d\t%d\t%d\t", qName, qSize,
                (qSize - qTotalEnd), (qSize - qTotalStart));
    else
        fprintf(f, "%s\t%d\t%d\t%d\t", qName, qSize, qTotalStart, qTotalEnd);
    fprintf(f, "%s\t%d\t%d\t%d\t", tName, tSize, tTotalStart, tTotalEnd);
    fprintf(f, "%d\t", blockCount);
    for (block = blockList; block != NULL; block = block->next)
        fprintf(f, "%d,", block->tEnd - block->tStart);
    fprintf(f, "\t");
    for (block = blockList; block != NULL; block = block->next)
        fprintf(f, "%d,", block->qStart);
    fprintf(f, "\t");
    for (block = blockList; block != NULL; block = block->next)
	fprintf(f, "%d,", block->tStart);
    fprintf(f, "\n");
    }
else  /* Output bed line. */
    {
    score = (match - mismatch - qNumInsert*2) * 1000 / (qTotalEnd - qTotalStart);
    fprintf(f, "%s\t%d\t%d\t%s\t%d\t", tName, tTotalStart, tTotalEnd,
	qName, score);
    fprintf(f, "%c\t", (isRc ? '-' : '+'));
    fprintf(f, "%d\t%d\t0\t%d\t", tTotalStart, tTotalEnd, blockCount);
    for (block = blockList; block != NULL; block = block->next)
	fprintf(f, "%d,", block->tEnd - block->tStart);
    fprintf(f, "\t");
    for (block = blockList; block != NULL; block = block->next)
	fprintf(f, "%d,", block->tStart);
    fprintf(f, "\n");
    }
}

void unexpectedEof(struct lineFile *lf)
/* Squawk about unexpected end of file. */
{
errAbort("Unexpected end of file in %s", lf->fileName);
}

void seekEndOfStanza(struct lineFile *lf)
/* find end of stanza */
{
char *line;
for (;;)
    {
    if (!lineFileNext(lf, &line, NULL))
        unexpectedEof(lf);
    if (line[0] == '}')
        break;
    }
}

void parseS(struct lineFile *lf, int *tSize, int *qSize)
/* Parse z stanza and return tSize and qSize */
{
char *line, *words[3];
if (!lineFileRow(lf, words))
    unexpectedEof(lf);
*tSize = lineFileNeedNum(lf, words, 2);
if (!lineFileRow(lf, words))
    unexpectedEof(lf);
*qSize = lineFileNeedNum(lf, words, 2);
seekEndOfStanza(lf);
}

char *needNextWord(struct lineFile *lf, char **pLine)
/* Get next word from line or die trying. */
{
char *word = nextWord(pLine);
if (word == NULL)
   errAbort("Short line %d of %s\n", lf->lineIx, lf->fileName);
return word;
}

void parseH(struct lineFile *lf,  char **tName, char **qName, boolean *isRc)
/* Parse out H stanza */
{
char *line, *word, *e, *dupe;
int i;


/* Set return variables to default values. */
freez(qName);
freez(tName);
*isRc = FALSE;

for (i=0; ; ++i)
    {
    if (!lineFileNext(lf, &line, NULL))
       unexpectedEof(lf);
    if (line[0] == '#')
       continue;
    if (line[0] == '}')
       {
       if (i < 2)
	   errAbort("Short H stanza line %d of %s", lf->lineIx, lf->fileName);
       break;
       }
    word = needNextWord(lf, &line);
    word += 2;  /* Skip over "> */
    e = strchr(word, '"');
    if (e != NULL) 
        {
	*e = 0;
	if (line != NULL)
	    ++line;
	}
    if (i == 0)
        *tName = cloneString(word);
    else if (i == 1)
        *qName = cloneString(word);
    if ((line != NULL) && (stringIn("(reverse", line) != NULL))
        *isRc = TRUE;
    }
}


struct block *parseA(struct lineFile *lf)
/* Parse an alignment stanza into a block list. */
{
struct block *block, *blockList = NULL;
char *line, *words[6];
int wordCount;

while (lineFileNext(lf, &line, NULL))
    {
    if (line[0] == '#')
	continue;
    if (line[0] == '}')
	break;
    wordCount = chopLine(line, words);
    if (wordCount == 0)
	continue;
    if (words[0][0] == 'l')
	{
	lineFileExpectWords(lf, 6, wordCount);
	AllocVar(block);
	block->tStart = lineFileNeedNum(lf, words, 1) - 1;
	block->tEnd = lineFileNeedNum(lf, words, 3);
	block->qStart = lineFileNeedNum(lf, words, 2) - 1;
	block->qEnd = lineFileNeedNum(lf, words, 4);
	if (block->qEnd - block->qStart != block->tEnd - block->tStart)
	    errAbort("Block size mismatch line %d of %s", lf->lineIx, lf->fileName);
	block->percentId = lineFileNeedNum(lf, words, 5);
	slAddHead(&blockList, block);
	}
    }
slReverse(&blockList);
return blockList;
}

void parseIntoPsl(char *lavFile, FILE *f)
/* Parse a blastz lav file and put it into something .psl like. */
{
struct lineFile *lf = lineFileOpen(lavFile, TRUE);
char *line, *words[10];
boolean gotAli = FALSE;
int wordCount;
struct block *blockList = NULL, *block;
int matchOne, match = 0, mismatch = 0, bases;
double scale;
int id;
boolean isRc = FALSE;
char *tName = NULL, *qName = NULL;
int qSize = 0, tSize = 0;

/* Check header. */
if (!lineFileNext(lf, &line, NULL))
   errAbort("%s is empty", lf->fileName);
if (!startsWith("#:lav", line))
   errAbort("%s is not a lav file\n", lf->fileName);

while (lineFileNext(lf, &line, NULL))
    {
    if (startsWith("s {", line))
        {
	parseS(lf, &tSize, &qSize);
	}
    else if (startsWith("h {", line))
        {
	parseH(lf, &tName, &qName, &isRc);
	}
    else if (startsWith("a {", line))
        {
	blockList = parseA(lf);
	outputBlocks(blockList, f, isRc, qName, qSize, tName, tSize, TRUE);
	slFreeList(&blockList);
	}
    }
lineFileClose(&lf);
}


void lavToPsl(char *lavIn, char *pslOut)
/* lavToPsl - Convert blastz lav to psl format. */
{
FILE *f = mustOpen(pslOut, "w");
parseIntoPsl(lavIn, f);
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 3)
    usage();
targetStrand = optionVal("target-strand", "");
lavToPsl(argv[1], argv[2]);
return 0;
}
