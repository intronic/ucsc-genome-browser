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
#include "nib.h"
#include "fa.h"
#include "psl.h"
#include "dystring.h"
#include "dlist.h"

static char const rcsid[] = "$Id: chainToPsl.c,v 1.6 2003/05/23 09:12:20 baertsch Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "chainToPsl - Convert chain file to psl format\n"
  "usage:\n"
  "   chainToPsl in.chain tSizes qSizes target.lst query.lst out.psl\n"
  "Where tSizes and qSizes are tab-delimited files with\n"
  "       <seqName><size>\n"
  "columns.\n"
  "The target and query lists can either be fasta files, nib files, \n"
  "or a list of fasta and/or nib files one per line\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

struct seqFilePos
/* Where a sequence is in a file. */
    {
    struct filePos *next;	/* Next in list. */
    char *name;	/* Sequence name. Allocated in hash. */
    char *file;	/* Sequence file name, allocated in hash. */
    long pos; /* Position in fa file/size of nib. */
    bool isNib;	/* True if a nib file. */
    };

boolean isFa(char *file)
/* Return TRUE if looks like a .fa file. */
{
FILE *f = mustOpen(file, "r");
int c = fgetc(f);
fclose(f);
return c == '>';
}

void addNib(char *file, struct hash *fileHash, struct hash *seqHash)
/* Add a nib file to hashes. */
{
struct seqFilePos *sfp;
char root[128];
int size;
FILE *f = NULL;
splitPath(file, NULL, root, NULL);
AllocVar(sfp);
hashAddSaveName(seqHash, root, sfp, &sfp->name);
sfp->file = hashStoreName(fileHash, file);
sfp->isNib = TRUE;
nibOpenVerify(file, &f, &size);
carefulClose(&f);
sfp->pos = size;
}

void addFa(char *file, struct hash *fileHash, struct hash *seqHash)
/* Add a fa file to hashes. */
{
struct lineFile *lf = lineFileOpen(file, TRUE);
char *line, *name;
char root[128];
char *rFile = hashStoreName(fileHash, file);

while (lineFileNext(lf, &line, NULL))
    {
    if (line[0] == '>')
        {
	struct seqFilePos *sfp;
	line += 1;
	name = nextWord(&line);
	if (name == NULL)
	   errAbort("bad line %d of %s", lf->lineIx, lf->fileName);
	AllocVar(sfp);
	hashAddSaveName(seqHash, name, sfp, &sfp->name);
	sfp->file = rFile;
	sfp->pos = lineFileTell(lf);
	}
    }
lineFileClose(&lf);
}


struct cachedFile
/* File in cache. */
    {
    struct cachedFile *next;	/* next in list. */
    char *name;		/* File name (allocated here) */
    FILE *f;		/* Open file. */
    };

FILE *openFromCache(struct dlList *cache, char *fileName)
/* Return open file handle via cache.  The simple logic here
 * depends on not more than N files being returned at once. */
{
static int maxCacheSize=16;
int cacheSize = 0;
struct dlNode *node;
struct cachedFile *cf;

/* First loop through trying to find it in cache, counting
 * cache size as we go. */
for (node = cache->head; !dlEnd(node); node = node->next)
    {
    ++cacheSize;
    cf = node->val;
    if (sameString(fileName, cf->name))
        {
	dlRemove(node);
	dlAddHead(cache, node);
	return cf->f;
	}
    }

/* If cache has reached max size free least recently used. */
if (cacheSize >= maxCacheSize)
    {
    node = dlPopTail(cache);
    cf = node->val;
    carefulClose(&cf->f);
    freeMem(cf->name);
    freeMem(cf);
    freeMem(node);
    }

/* Cache new file. */
AllocVar(cf);
cf->name = cloneString(fileName);
cf->f = mustOpen(fileName, "rb");
dlAddValHead(cache, cf);
return cf->f;
}

struct dnaSeq *readSeqFromFaPos(struct seqFilePos *sfp,  FILE *f)
/* Read part of FA file. */
{
struct dnaSeq *seq;
fseek(f, sfp->pos, SEEK_SET);
if (!faReadNext(f, "", TRUE, NULL, &seq))
    errAbort("Couldn't faReadNext on %s in %s\n", sfp->name, sfp->file);
return seq;
}
struct dnaSeq *readCachedSeq(char *seqName, struct hash *hash, 
	struct dlList *fileCache)
/* Read sequence hopefully using file cashe. */
{
struct seqFilePos *sfp = hashMustFindVal(hash, seqName);
FILE *f = openFromCache(fileCache, sfp->file);
if (sfp->isNib)
    {
    return nibLdPart(sfp->file, f, sfp->pos, 0, sfp->pos);
    }
else
    {
    return readSeqFromFaPos(sfp, f);
    }
}

void readCachedSeqPart(char *seqName, int start, int size, 
     struct hash *hash, struct dlList *fileCache, 
     struct dnaSeq **retSeq, int *retOffset, boolean *retIsNib)
/* Read sequence hopefully using file cashe. If sequence is in a nib
 * file just read part of it. */
{
struct seqFilePos *sfp = hashMustFindVal(hash, seqName);
FILE *f = openFromCache(fileCache, sfp->file);
if (sfp->isNib)
    {
    *retSeq = nibLdPart(sfp->file, f, sfp->pos, start, size);
    *retOffset = start;
    *retIsNib = TRUE;
    }
else
    {
    *retSeq = readSeqFromFaPos(sfp, f);
    *retOffset = 0;
    *retIsNib = FALSE;
    }
}

void hashFileList(char *fileList, struct hash *fileHash, struct hash *seqHash)
/* Read file list into hash */
{
if (endsWith(fileList, ".nib"))
    addNib(fileList, fileHash, seqHash);
else if (isFa(fileList))
    addFa(fileList, fileHash, seqHash);
else
    {
    struct lineFile *lf = lineFileOpen(fileList, TRUE);
    char *row[1];
    while (lineFileRow(lf, row))
        {
	char *file = row[0];
	if (endsWith(file, ".nib"))
	    addNib(file, fileHash, seqHash);
	else
	    addFa(file, fileHash, seqHash);
	}
    lineFileClose(&lf);
    }
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


void aliStringToPsl(struct lineFile *lf, char *qNameParm, char *tNameParm, 
	int qSize, int tSize, int aliSize, 
	int qStart, int qEnd, int tStart, int tEnd, char strand, FILE *f, struct chain *chain, struct hash *tHash, struct hash *qHash, struct dlList *fileCache )
/* Output alignment in a pair of strings with insert chars
 * to a psl line in a file. */
{
static char *tName = NULL, *qName = NULL;
static struct dnaSeq *tSeq = NULL, *qSeq = NULL;
struct dyString *q = newDyString(16*1024);
struct dyString *t = newDyString(16*1024);
unsigned match = 0;	/* Number of bases that match */
unsigned misMatch = 0;	/* Number of bases that don't match */
unsigned qNumInsert = 0;	/* Number of inserts in query */
int qBaseInsert = 0;	/* Number of bases inserted in query */
unsigned tNumInsert = 0;	/* Number of inserts in target */
int tBaseInsert = 0;	/* Number of bases inserted in target */
boolean qInInsert = FALSE; /* True if in insert state on query. */
boolean tInInsert = FALSE; /* True if in insert state on target. */
boolean eitherInsert = FALSE;	/* True if either in insert state. */
int qOffset = 0;
int tOffset = 0;
boolean qIsNib = FALSE;
static boolean tIsNib ;
int blockCount = 1, blockIx=0;
boolean qIsRc = FALSE;
int i,j;
int qs,qe,ts,te;
int *blocks = NULL, *qStarts = NULL, *tStarts = NULL;
struct boxIn *b, *nextB;
int qbSize = 0, tbSize = 0; /* sum of block sizes */
int qtSize = 0, ttSize = 0; /* sum of block + gap sizes */

/* Don't ouput if either query or target is zero length */
 if ((qStart == qEnd) || (tStart == tEnd))
     return;

if (qName == NULL || !sameString(qName, qNameParm))
    {
    freeDnaSeq(&qSeq);
    freez(&qName);
    qName = cloneString(qNameParm);
    readCachedSeqPart(qName, qStart, qEnd-qStart, 
    	qHash, fileCache, &qSeq, &qOffset, &qIsNib);
    if (qIsNib && strand == '-')
	    qOffset = qSize - qEnd;
    }
if (tIsNib || tName == NULL || !sameString(tName, tNameParm) )
    {
    freeDnaSeq(&tSeq);
    freez(&tName);
    tName = cloneString(tNameParm);
    readCachedSeqPart(tName, tStart, tEnd-tStart, 
	tHash, fileCache, &tSeq, &tOffset, &tIsNib);
    }
if (strand == '-')
    reverseComplement(qSeq->dna, qSeq->size);
for (b = chain->blockList; b != NULL; b = nextB)
    {
    blockCount++;
    qbSize += b->qEnd - b->qStart + 1;
    tbSize += b->tEnd - b->tStart + 1;
    nextB = b->next;
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
            j = b->tStart-tStart;
            for (i = b->qStart ; i < b->qStart+(b->tEnd - b->tStart); i++)
                {
                char qq = qSeq->dna[i];
                char tt = tSeq->dna[j++];
                //printf("qs ts %d %d %c %c %d %d %s\n",i,j,qq,tt, match, misMatch, qName);
                if (toupper(qq) == toupper(tt))
                    ++match;
                else 
                    ++misMatch;
                }
	    ++blockIx;
	    eitherInsert = TRUE;
        nextB = b->next;
    }

assert(blockIx == blockCount-1);

/*
qs = qStart;
qe = qStart + match + misMatch + tBaseInsert;
assert(qe == qEnd); 
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
fprintf(f, "%s\t", qNameParm);
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
fprintf(f, "%s\t", tNameParm);
fprintf(f, "%d\t", tSize);
fprintf(f, "%d\t", tStart);
fprintf(f, "%d\t", tEnd);
fprintf(f, "%d\t", blockCount-1);
if (ferror(f))
    {
    perror("Error writing psl file\n");
    errAbort("\n");
    }

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


void chainToPsl(char *inName, char *tSizeFile, char *qSizeFile,  char *targetList, char *queryList, char *outName)
/* chainToPsl - Convert chain file to psl format. */
{
struct hash *tSizeHash = readSizes(tSizeFile);
struct hash *qSizeHash = readSizes(qSizeFile);
struct lineFile *lf = lineFileOpen(inName, TRUE);
FILE *f = mustOpen(outName, "w");
struct hash *fileHash = newHash(0);  /* No value. */
struct hash *tHash = newHash(20);  /* seqFilePos value. */
struct hash *qHash = newHash(20);  /* seqFilePos value. */
struct dlList *fileCache = newDlList();
//FILE *f = mustOpen(prettyName, "w");
FILE *checkFile = NULL;
struct psl *psl;
struct chain *chain;
int q,t;

printf("Scanning %s\n", targetList);
hashFileList(targetList, fileHash, tHash);
printf("Scanning %s\n", queryList);
hashFileList(queryList, fileHash, qHash);
printf("Converting %s\n", inName);

while ((chain = chainRead(lf)) != NULL)
    {
    //uglyf("chain %s %s \n",chain->tName,chain->qName); 
    q = findSize(qSizeHash, chain->qName);
    t = findSize(tSizeHash, chain->tName);
    aliStringToPsl(lf, chain->qName, chain->tName, chain->qSize, chain->tSize,
	min(chain->tEnd-chain->tStart, chain->qEnd-chain->qStart), chain->qStart, chain->qEnd, chain->tStart, chain->tEnd,
        chain->qStrand, f, chain, tHash, qHash, fileCache);
    chainFree(&chain);
    }
lineFileClose(&lf);
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 7)
    {
    usage();
    }
chainToPsl(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
return 0;
}
