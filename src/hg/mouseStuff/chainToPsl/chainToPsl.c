/* chainToPsl - Convert chain to psl format. 
 * This file is copyright 2002 Robert Baertsch , but license is hereby
 * granted for all use - public, private or commercial. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "obscure.h"
#include "chainBlock.h"
#include "dnautil.h"

static char const rcsid[] = "$Id: chainToPsl.c,v 1.3 2003/05/16 22:02:22 baertsch Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "chainToPsl - Convert chain file to psl format\n"
  "usage:\n"
  "   chainToPsl in.chain tSizes qSizes out.psl\n"
  "Where tSizes and qSizes are tab-delimited files with\n"
  "       <seqName><size>\n"
  "columns.\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

struct hash *readSizes(char *fileName)
/* Read tab-separated file into hash with
 * name key size value. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct hash *hash = newHash(0);
char *row[2];
while (lineFileRow(lf, row))
    {
    char *name = row[0];
    int size = lineFileNeedNum(lf, row, 1);
    if (hashLookup(hash, name) != NULL)
        warn("Duplicate %s, ignoring all but first\n", name);
    else
	hashAdd(hash, name, intToPt(size));
    }
lineFileClose(&lf);
return hash;
}

int findSize(struct hash *hash, char *name)
/* Find size of name in hash or die trying. */
{
void *val = hashMustFindVal(hash, name);
return ptToInt(val);
}

void countInserts(char *s, int size, int *retNumInsert, int *retBaseInsert)
/* Count up number and total size of inserts in s. */
{
char c, lastC = s[0];
int i;
int baseInsert = 0, numInsert = 0;
if (lastC == '-')
    errAbort("%s starts with -", s);
for (i=0; i<size; ++i)
    {
    c = s[i];
    if (c == '-')
        {
	if (lastC != '-')
	     numInsert += 1;
	baseInsert += 1;
	}
    lastC = c;
    }
*retNumInsert = numInsert;
*retBaseInsert = baseInsert;
}

int countInitialChars(char *s, char c)
/* Count number of initial chars in s that match c. */
{
int count = 0;
char d;
while ((d = *s++) != 0)
    {
    if (c == d)
        ++count;
    else
        break;
    }
return count;
}

int countNonInsert(char *s, int size)
/* Count number of characters in initial part of s that
 * are not '-'. */
{
int count = 0;
int i;
for (i=0; i<size; ++i)
    if (*s++ != '-')
        ++count;
return count;
}

int countTerminalChars(char *s, char c)
/* Count number of initial chars in s that match c. */
{
int len = strlen(s), i;
int count = 0;
for (i=len-1; i>=0; --i)
    {
    if (c == s[i])
        ++count;
    else
        break;
    }
return count;
}


void aliStringToPsl(struct lineFile *lf, char *qName, char *tName, 
	int qSize, int tSize, int aliSize, 
	int qStart, int qEnd, int tStart, int tEnd, char strand, FILE *f, struct chain *chain)
/* Output alignment in a pair of strings with insert chars
 * to a psl line in a file. */
{
unsigned match = 0;	/* Number of bases that match */
unsigned misMatch = 0;	/* Number of bases that don't match */
unsigned qNumInsert = 0;	/* Number of inserts in query */
int qBaseInsert = 0;	/* Number of bases inserted in query */
unsigned tNumInsert = 0;	/* Number of inserts in target */
int tBaseInsert = 0;	/* Number of bases inserted in target */
boolean qInInsert = FALSE; /* True if in insert state on query. */
boolean tInInsert = FALSE; /* True if in insert state on target. */
boolean eitherInsert = FALSE;	/* True if either in insert state. */
int blockCount = 1, blockIx=0;
boolean qIsRc = FALSE;
int i;
char q,t;
int qs,qe,ts,te;
int *blocks = NULL, *qStarts = NULL, *tStarts = NULL;
struct boxIn *b, *nextB;
int qbSize = 0, tbSize = 0; /* sum of block sizes */
int qtSize = 0, ttSize = 0; /* sum of block + gap sizes */

/* Don't ouput if either query or target is zero length */
 if ((qStart == qEnd) || (tStart == tEnd))
     return;

for (b = chain->blockList; b != NULL; b = nextB)
    {
    blockCount++;
    qbSize += b->qEnd - b->qStart + 1;
    tbSize += b->tEnd - b->tStart + 1;
    nextB = b->next;
    }
//printf("blockCount %d qbSize %d tbSize %d %d %s\n",blockCount, qbSize, tbSize, 0, chain->qName);
/* Count up match/mismatch. */
/*
for (i=0; i<aliSize; ++i)
    {
    q = qString[i];
    t = tString[i];
    if (q != '-' && t != '-')
	{
	if (q == t)
	    ++match;
	else
	    ++misMatch;
	}
    }
*/
//match=chain->score * tbSize / 70;
/* Deal with minus strand. */
/*
qs = qStart;
qe = qStart + match + misMatch + tBaseInsert;
assert(qe == qEnd); 
if (strand == '-')
    {
    reverseIntRange(&qs, &qe, qSize);
    }
assert(qs < qe);
te = tStart + match + misMatch + qBaseInsert;
assert(te == tEnd);
assert(tStart < te);
*/

/* Output header */
fprintf(f, "%d\t", match);
fprintf(f, "%d\t", misMatch);
fprintf(f, "0\t");
fprintf(f, "0\t");
fprintf(f, "%d\t", qNumInsert);
fprintf(f, "%d\t", qBaseInsert);
fprintf(f, "%d\t", tNumInsert);
fprintf(f, "%d\t", tBaseInsert);
fprintf(f, "%c\t", strand);
fprintf(f, "%s\t", qName);
fprintf(f, "%d\t", qSize);
if (strand == '+')
    {
    fprintf(f, "%d\t", qStart);
    fprintf(f, "%d\t", qEnd);
    }
    else
    {
    fprintf(f, "%d\t", qSize - qEnd);
    fprintf(f, "%d\t", qSize - qStart);
    }
fprintf(f, "%s\t", tName);
fprintf(f, "%d\t", tSize);
fprintf(f, "%d\t", tStart);
fprintf(f, "%d\t", tEnd);
fprintf(f, "%d\t", blockCount-1);
if (ferror(f))
    {
    perror("Error writing psl file\n");
    errAbort("\n");
    }

/* Allocate dynamic memory for block lists. */
AllocArray(blocks, blockCount);
AllocArray(qStarts, blockCount);
AllocArray(tStarts, blockCount);

/* Figure block sizes and starts. */
eitherInsert = FALSE;
qs = qe = qStart;
ts = te = tStart;
nextB = NULL;
for (b = chain->blockList; b != NULL; b = nextB)
    {
	    qStarts[blockIx] = b->qStart;
	    tStarts[blockIx] = b->tStart;
	    blocks[blockIx] = b->tEnd - b->tStart;
	    ++blockIx;
	    eitherInsert = TRUE;
        nextB = b->next;
    }
assert(blockIx == blockCount-1);
/*qStarts[blockIx] = b->qStart;
tStarts[blockIx] = b->tStart;
blocks[blockIx] = b->tEnd - b->tStart;
*/

/* Output block sizes */
for (i=0; i<blockCount-1; ++i)
    fprintf(f, "%d,", blocks[i]);
fprintf(f, "\t");

/* Output qStarts */
for (i=0; i<blockCount-1; ++i)
    fprintf(f, "%d,", qStarts[i]);
fprintf(f, "\t");

/* Output tStarts */
for (i=0; i<blockCount-1; ++i)
    fprintf(f, "%d,", tStarts[i]);
fprintf(f, "\n");

/* Clean Up. */
freez(&blocks);
freez(&qStarts);
freez(&tStarts);
}


void chainToPsl(char *inName, char *tSizeFile, char *qSizeFile, char *outName)
/* chainToPsl - Convert chain file to psl format. */
{
struct hash *tSizeHash = readSizes(tSizeFile);
struct hash *qSizeHash = readSizes(qSizeFile);
struct lineFile *lf = lineFileOpen(inName, TRUE);
FILE *f = mustOpen(outName, "w");
struct chain *chain;
int q,t;

while ((chain = chainRead(lf)) != NULL)
    {
    uglyf("chain %s %s \n",chain->tName,chain->qName); 
    q = findSize(qSizeHash, chain->qName);
    t = findSize(tSizeHash, chain->tName);
    aliStringToPsl(lf, chain->qName, chain->tName, chain->qSize, chain->tSize,
	0 /* count */, chain->qStart, chain->qEnd, chain->tStart, chain->tEnd,
        chain->qStrand, f, chain);
    chainFree(&chain);
    }
lineFileClose(&lf);
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 5)
    usage();
chainToPsl(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
