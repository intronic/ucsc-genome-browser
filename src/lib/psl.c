/* psl.c was originally generated by the autoSql program, which also 
 * generated as_psl.h and as_psl.sql.  This module links the database and the RAM 
 * representation of objects. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "sqlNum.h"
#include "sqlList.h"
#include "localmem.h"
#include "psl.h"
#include "linefile.h"
#include "dnaseq.h"
#include "dystring.h"
#include "fuzzyFind.h"
#include "aliType.h"

static char const rcsid[] = "$Id: psl.c,v 1.28 2003/09/11 14:14:02 braney Exp $";

static char *createString = 
"CREATE TABLE %s (\n"
    "%s"				/* Optional bin */
    "matches int unsigned not null,	# Number of bases that match that aren't repeats\n"
    "misMatches int unsigned not null,	# Number of bases that don't match\n"
    "repMatches int unsigned not null,	# Number of bases that match but are part of repeats\n"
    "nCount int unsigned not null,	# Number of 'N' bases\n"
    "qNumInsert int unsigned not null,	# Number of inserts in query\n"
    "qBaseInsert int unsigned not null,	# Number of bases inserted in query\n"
    "tNumInsert int unsigned not null,	# Number of inserts in target\n"
    "tBaseInsert int unsigned not null,	# Number of bases inserted in target\n"
    "strand char(2) not null,	# + or - for strand.  First character is query, second is target.\n"
    "qName varchar(255) not null,	# Query sequence name\n"
    "qSize int unsigned not null,	# Query sequence size\n"
    "qStart int unsigned not null,	# Alignment start position in query\n"
    "qEnd int unsigned not null,	# Alignment end position in query\n"
    "tName varchar(255) not null,	# Target sequence name\n"
    "tSize int unsigned not null,	# Target sequence size\n"
    "tStart int unsigned not null,	# Alignment start position in target\n"
    "tEnd int unsigned not null,	# Alignment end position in target\n"
    "blockCount int unsigned not null,	# Number of blocks in alignment\n"
    "blockSizes longblob not null,	# Size of each block\n"
    "qStarts longblob not null,	# Start of each block in query.\n"
    "tStarts longblob not null,	# Start of each block in target.\n";

static char *indexString = 
	  "#Indices\n"
    "%s"                            /* Optional bin. */
    "INDEX(%stStart),\n"
    "INDEX(qName(12)),\n"
    "INDEX(%stEnd)\n"
")\n";


struct psl *pslLoad(char **row)
/* Load a psl from row fetched with select * from psl
 * from database.  Dispose of this with pslFree(). */
{
struct psl *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->blockCount = sqlUnsigned(row[17]);
ret->match = sqlUnsigned(row[0]);
ret->misMatch = sqlUnsigned(row[1]);
ret->repMatch = sqlUnsigned(row[2]);
ret->nCount = sqlUnsigned(row[3]);
ret->qNumInsert = sqlUnsigned(row[4]);
ret->qBaseInsert = sqlSigned(row[5]);
ret->tNumInsert = sqlUnsigned(row[6]);
ret->tBaseInsert = sqlSigned(row[7]);
strcpy(ret->strand, row[8]);
ret->qName = cloneString(row[9]);
ret->qSize = sqlUnsigned(row[10]);
ret->qStart = sqlUnsigned(row[11]);
ret->qEnd = sqlUnsigned(row[12]);
ret->tName = cloneString(row[13]);
ret->tSize = sqlUnsigned(row[14]);
ret->tStart = sqlUnsigned(row[15]);
ret->tEnd = sqlUnsigned(row[16]);
sqlUnsignedDynamicArray(row[18], &ret->blockSizes, &sizeOne);
if (sizeOne != ret->blockCount)
    {
    printf("sizeOne bloxksizes %d bs %d block=%s\n",sizeOne, ret->blockCount,row[18]);
    }
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[19], &ret->qStarts, &sizeOne);
if (sizeOne != ret->blockCount)
    {
    printf("sizeOne qStarts %d bs %d\n",sizeOne, ret->blockCount);
    }
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[20], &ret->tStarts, &sizeOne);
if (sizeOne != ret->blockCount)
    {
    printf("sizeOne tStarts %d bs %d\n",sizeOne, ret->blockCount);
    }
assert(sizeOne == ret->blockCount);
return ret;
}


struct psl *pslCommaIn(char **pS, struct psl *ret)
/* Create a psl out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new psl */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->match = sqlUnsignedComma(&s);
ret->misMatch = sqlUnsignedComma(&s);
ret->repMatch = sqlUnsignedComma(&s);
ret->nCount = sqlUnsignedComma(&s);
ret->qNumInsert = sqlUnsignedComma(&s);
ret->qBaseInsert = sqlSignedComma(&s);
ret->tNumInsert = sqlUnsignedComma(&s);
ret->tBaseInsert = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->qName = sqlStringComma(&s);
ret->qSize = sqlUnsignedComma(&s);
ret->qStart = sqlUnsignedComma(&s);
ret->qEnd = sqlUnsignedComma(&s);
ret->tName = sqlStringComma(&s);
ret->tSize = sqlUnsignedComma(&s);
ret->tStart = sqlUnsignedComma(&s);
ret->tEnd = sqlUnsignedComma(&s);
ret->blockCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->blockSizes, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->blockSizes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->qStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->qStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->tStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->tStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void pslFree(struct psl **pEl)
/* Free a single dynamically allocated psl such as created
 * with pslLoad(). */
{
struct psl *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qName);
freeMem(el->tName);
freeMem(el->blockSizes);
freeMem(el->qStarts);
freeMem(el->tStarts);
freez(pEl);
}

void pslFreeList(struct psl **pList)
/* Free a list of dynamically allocated psl's */
{
struct psl *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pslFree(&el);
    }
*pList = NULL;
}

void pslOutput(struct psl *el, FILE *f, char sep, char lastSep) 
/* Print out psl.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->match);
fputc(sep,f);
fprintf(f, "%u", el->misMatch);
fputc(sep,f);
fprintf(f, "%u", el->repMatch);
fputc(sep,f);
fprintf(f, "%u", el->nCount);
fputc(sep,f);
fprintf(f, "%u", el->qNumInsert);
fputc(sep,f);
fprintf(f, "%d", el->qBaseInsert);
fputc(sep,f);
fprintf(f, "%u", el->tNumInsert);
fputc(sep,f);
fprintf(f, "%d", el->tBaseInsert);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qSize);
fputc(sep,f);
fprintf(f, "%u", el->qStart);
fputc(sep,f);
fprintf(f, "%u", el->qEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tSize);
fputc(sep,f);
fprintf(f, "%u", el->tStart);
fputc(sep,f);
fprintf(f, "%u", el->tEnd);
fputc(sep,f);
fprintf(f, "%u", el->blockCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->blockSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->qStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->tStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
if (ferror(f))
    {
    perror("Error writing psl file\n");
    errAbort("\n");
    }
}

/* ----- end autoSql generated part --------------- */

void pslOutputShort(struct psl *el, FILE *f, char sep, char lastSep) 
/* Print out psl.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->match);
fputc(sep,f);
fprintf(f, "%u", el->misMatch);
fputc(sep,f);
fprintf(f, "%u", el->repMatch);
fputc(sep,f);
fprintf(f, "%u", el->qNumInsert);
fputc(sep,f);
fprintf(f, "%d", el->qBaseInsert);
fputc(sep,f);
fprintf(f, "%u", el->tNumInsert);
fputc(sep,f);
fprintf(f, "%d", el->tBaseInsert);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qStart);
fputc(sep,f);
fprintf(f, "%u", abs(el->qEnd - el->qStart));
fputc(sep,f);
//fprintf(f, "%u", el->qEnd);
//fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tStart);
fputc(sep,f);
fprintf(f, "%u", abs(el->tEnd - el->tStart));
fputc(sep,f);
fprintf(f, "%u", el->blockCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
//for (i=0; i<el->blockCount; ++i)
//    {
//    fprintf(f, "%u", el->blockSizes[i]);
    //fputc(',', f);
//    }
//if (sep == ',') fputc('}',f);
//fputc(sep,f);
//if (sep == ',') fputc('{',f);
//for (i=0; i<el->blockCount; ++i)
    //{
    //fprintf(f, "%u", el->qStarts[i]);
    //fputc(',', f);
    //}
//if (sep == ',') fputc('}',f);
//fputc(sep,f);
//if (sep == ',') fputc('{',f);
//for (i=0; i<el->blockCount; ++i)
    //{
    //fprintf(f, "%u", el->tStarts[i]);
    //fputc(',', f);
    //}
//if (sep == ',') fputc('}',f);
fputc(lastSep,f);
if (ferror(f))
    {
    perror("Error writing psl file\n");
    errAbort("\n");
    }
}
struct psl *pslLoadAll(char *fileName)
/* Load all psl's in file. */
{
struct lineFile *lf = pslFileOpen(fileName);
struct psl *pslList = NULL, *psl;
while ((psl = pslNext(lf)) != NULL)
    {
    slAddHead(&pslList, psl);
    }
slReverse(&pslList);
lineFileClose(&lf);
return pslList;
}


int pslCmpQuery(const void *va, const void *vb)
/* Compare to sort based on query start. */
{
const struct psl *a = *((struct psl **)va);
const struct psl *b = *((struct psl **)vb);
int dif;
dif = strcmp(a->qName, b->qName);
if (dif == 0)
    dif = a->qStart - b->qStart;
return dif;
}

int pslCmpTarget(const void *va, const void *vb)
/* Compare to sort based on target start. */
{
const struct psl *a = *((struct psl **)va);
const struct psl *b = *((struct psl **)vb);
int dif;
dif = strcmp(a->tName, b->tName);
if (dif == 0)
    dif = a->tStart - b->tStart;
return dif;
}


static void pslLabelColumns(FILE *f)
/* Write column info. */
{
fputs("\n"
"match\tmis- \trep. \tN's\tQ gap\tQ gap\tT gap\tT gap\tstrand\tQ        \tQ   \tQ    \tQ  \tT        \tT   \tT    \tT  \tblock\tblockSizes \tqStarts\t tStarts\n"
"     \tmatch\tmatch\t   \tcount\tbases\tcount\tbases\t      \tname     \tsize\tstart\tend\tname     \tsize\tstart\tend\tcount\n" 
"---------------------------------------------------------------------------------------------------------------------------------------------------------------\n",
f);
}

void pslxWriteHead(FILE *f, enum gfType qType, enum gfType tType)
/* Write header for extended (possibly protein) psl file. */
{
fprintf(f, "psLayout version 4 %s %s\n", gfTypeName(qType), gfTypeName(tType));
pslLabelColumns(f);
}

void pslWriteHead(FILE *f)
/* Write head of psl. */
{
fputs("psLayout version 3\n", f);
pslLabelColumns(f);
}

void pslWriteAll(struct psl *pslList, char *fileName, boolean writeHeader)
/* Write a psl file from list. */
{
FILE *f;
struct psl *psl;

f = mustOpen(fileName, "w");
if (writeHeader)
    pslWriteHead(f);
for (psl = pslList; psl != NULL; psl = psl->next)
    pslTabOut(psl, f);
fclose(f);
}

void pslxFileOpen(char *fileName, enum gfType *retQueryType, enum gfType *retTargetType, struct lineFile **retLf)
/* Read header part of psl and make sure it's right.  Return
 * sequence types and file handle. */
{
char *line;
int lineSize;
char *words[30];
char *version;
int wordCount;
int i;
enum gfType qt = gftRna,  tt = gftDna;
struct lineFile *lf = lineFileOpen(fileName, TRUE);

if (!lineFileNext(lf, &line, &lineSize))
    warn("%s is empty", fileName);
else
    {
    if (startsWith("psLayout version", line))
	{
	wordCount = chopLine(line, words);
	if (wordCount < 3)
	    errAbort("%s is not a psLayout file", fileName);
	version = words[2];
	if (sameString(version, "3"))
	    {
	    }
	else if (sameString(version, "4"))
	    {
	    qt = gfTypeFromName(words[3]);
	    tt = gfTypeFromName(words[4]);
	    }
	else
	    {
	    errAbort("%s is version %s of psLayout, this program can only handle through version 4",
		fileName,  version);
	    }
	for (i=0; i<4; ++i)
	    {
	    if (!lineFileNext(lf, &line, &lineSize))
		errAbort("%s severely truncated", fileName);
	    }
	}
    else
        {
	char *s = cloneString(line);
	wordCount = chopLine(s, words);
	if (wordCount < 21 || wordCount > 23 || (words[8][0] != '+' && words[8][0] != '-'))
	    errAbort("%s is not a psLayout file", fileName);
	else
	    lineFileReuse(lf); 
	freeMem(s);
	}
    }
*retQueryType = qt;
*retTargetType = tt;
*retLf = lf;
}

struct lineFile *pslFileOpen(char *fileName)
/* Read header part of psl and make sure it's right. 
 * Return line file handle to it. */
{
enum gfType qt, tt;
struct lineFile *lf;
pslxFileOpen(fileName, &qt, &tt, &lf);
return lf;
}

struct psl *pslNext(struct lineFile *lf)
/* Read next line from file and convert it to psl.  Return
 * NULL at eof. */
{
char *line;
int lineSize;
char *words[32];
int wordCount;
struct psl *psl;
static int lineAlloc = 0;
static char *chopBuf = NULL;

if (!lineFileNext(lf, &line, &lineSize))
    {
    return NULL;
    }
if (lineSize >= lineAlloc)
    {
    lineAlloc = lineSize+256;
    chopBuf = needMoreMem(chopBuf, 0, lineAlloc);
    }
memcpy(chopBuf, line, lineSize+1);
wordCount = chopLine(chopBuf, words);
if (wordCount >= 21)
    {
    return pslLoad(words);
    }
else
    {
    errAbort("Bad line %d of %s", lf->lineIx, lf->fileName);
    return NULL;
    }
}

struct psl *pslLoadLm(char **row, struct lm *lm)
/* Load row into local memory psl. */
{
struct psl *ret;
int sizeOne,i;
char *s;

ret = lmAlloc(lm, sizeof(*ret));
ret->blockCount = sqlUnsigned(row[17]);
ret->match = sqlUnsigned(row[0]);
ret->misMatch = sqlUnsigned(row[1]);
ret->repMatch = sqlUnsigned(row[2]);
ret->nCount = sqlUnsigned(row[3]);
ret->qNumInsert = sqlUnsigned(row[4]);
ret->qBaseInsert = sqlSigned(row[5]);
ret->tNumInsert = sqlUnsigned(row[6]);
ret->tBaseInsert = sqlSigned(row[7]);
strcpy(ret->strand, row[8]);
ret->qName = lmCloneString(lm,row[9]);
ret->qSize = sqlUnsigned(row[10]);
ret->qStart = sqlUnsigned(row[11]);
ret->qEnd = sqlUnsigned(row[12]);
ret->tName = lmCloneString(lm, row[13]);
ret->tSize = sqlUnsigned(row[14]);
ret->tStart = sqlUnsigned(row[15]);
ret->tEnd = sqlUnsigned(row[16]);
ret->blockSizes = lmAlloc(lm, sizeof(ret->blockSizes[0]) * ret->blockCount);
sqlUnsignedArray(row[18], ret->blockSizes, ret->blockCount);
ret->qStarts = lmAlloc(lm, sizeof(ret->qStarts[0]) * ret->blockCount);
sqlUnsignedArray(row[19], ret->qStarts, ret->blockCount);
ret->tStarts = lmAlloc(lm, sizeof(ret->tStarts[0]) * ret->blockCount);
sqlUnsignedArray(row[20], ret->tStarts, ret->blockCount);
return ret;
}

boolean pslIsProtein(const struct psl *psl)
/* is psl a protein psl (are it's blockSizes and scores in protein space) */
{
int lastBlock = psl->blockCount - 1;

return  (((psl->strand[1] == '+' ) &&
    (psl->tEnd == psl->tStarts[lastBlock] + 3*psl->blockSizes[lastBlock])) ||
   ((psl->strand[1] == '-') && 
    (psl->tStart == (psl->tSize-(psl->tStarts[lastBlock] + 3*psl->blockSizes[lastBlock])))));
}
int pslCalcMilliBad(struct psl *psl, boolean isMrna)
/* Calculate badness in parts per thousand. */
{
int sizeMul = pslIsProtein(psl) ? 3 : 1;
int qAliSize, tAliSize, aliSize;
int milliBad;
int sizeDif;
int insertFactor;

qAliSize = sizeMul * (psl->qEnd - psl->qStart);
tAliSize = psl->tEnd - psl->tStart;
aliSize = min(qAliSize, tAliSize);
if (aliSize <= 0)
    return 0;
sizeDif = qAliSize - tAliSize;
if (sizeDif < 0)
    {
    if (isMrna)
	sizeDif = 0;
    else
	sizeDif = -sizeDif;
    }
insertFactor = psl->qNumInsert;
if (!isMrna)
    insertFactor += psl->tNumInsert;

milliBad = (1000 * (psl->misMatch*sizeMul + insertFactor + round(3*log(1+sizeDif)))) / (sizeMul * (psl->match + psl->repMatch + psl->misMatch));
return milliBad;
}

int pslScore(const struct psl *psl)
/* Return score for psl. */
{
int sizeMul = pslIsProtein(psl) ? 3 : 1;

return sizeMul * (psl->match + ( psl->repMatch>>1)) - 
	sizeMul * psl->misMatch - psl->qNumInsert - psl->tNumInsert;
}


struct ffAli *pslToFakeFfAli(struct psl *psl, DNA *needle, DNA *haystack)
/* Convert from psl to ffAli format.  In some cases you can pass NULL
 * for needle and haystack - depending what the post-processing is going
 * to be. */
{
struct ffAli *ffList = NULL, *ff;
int blockCount = psl->blockCount;
unsigned *blockSizes = psl->blockSizes;
unsigned *qStarts = psl->qStarts;
unsigned *tStarts = psl->tStarts;
int size;
int i;

for (i=0; i<blockCount; ++i)
    {
    size = blockSizes[i];
    AllocVar(ff);
    ff->left = ffList;
    ffList = ff;
    ff->nStart = ff->nEnd = needle + qStarts[i];
    ff->nEnd += size;
    ff->hStart = ff->hEnd = haystack + tStarts[i];
    ff->hEnd += size;
    }
ffList = ffMakeRightLinks(ffList);
return ffList;
}

struct psl *pslFromFakeFfAli(struct ffAli *ff, 
	DNA *needle, DNA *haystack, char strand,
	char *qName, int qSize, char *tName, int tSize)
/* This will create a basic psl structure from a sorted series of ffAli
 * blocks.  The fields that would need actual sequence to be filled in
 * are left zero however - fields including match, repMatch, mismatch. */
{
struct psl *psl;
unsigned *blockSizes;
unsigned *qStarts;
unsigned *tStarts;
int blockCount;
int i;
int nStart, hStart;
int nEnd, hEnd;

AllocVar(psl);
psl->blockCount = blockCount = ffAliCount(ff);
psl->blockSizes = AllocArray(blockSizes, blockCount);
psl->qStarts = AllocArray(qStarts, blockCount);
psl->tStarts = AllocArray(tStarts, blockCount);
psl->qName = cloneString(qName);
psl->qSize = qSize;
psl->tName = cloneString(tName);
psl->tSize = tSize;
psl->strand[0] = strand;

for (i=0; i<blockCount; ++i)
    {
    nStart = ff->nStart - needle;
    nEnd = ff->nEnd - needle;
    hStart = ff->hStart - haystack;
    hEnd = ff->hEnd - haystack;
    blockSizes[i] = nEnd - nStart;
    qStarts[i] = nStart;
    tStarts[i] = hStart;
    if (i == 0)
       {
       psl->qStart = nStart;
       psl->tStart = hStart;
       }
    if (i == blockCount-1)
       {
       psl->qEnd = nEnd;
       psl->tEnd = hEnd;
       }
    ff = ff->right;
    }
if (strand == '-')
    {
    reverseIntRange(&psl->qStart, &psl->qEnd, psl->qSize);
    }
return psl;
}

struct ffAli *pslToFfAli(struct psl *psl, struct dnaSeq *query, struct dnaSeq *target,
	int targetOffset)
/* Convert from psl to ffAli format.  Clip to parts that we actually
 * have sequence for. */
{
struct ffAli *ffList = NULL, *ff;
DNA *needle = query->dna;
DNA *haystack = target->dna;
int blockCount = psl->blockCount;
unsigned *blockSizes = psl->blockSizes;
unsigned *qStarts = psl->qStarts;
unsigned *tStarts = psl->tStarts;
int size;
int i;
int tMin = targetOffset;
int tMax = targetOffset + target->size;
int tStart, tEnd;
int clipStart, clipEnd, clipOffset, clipSize;

for (i=0; i<blockCount; ++i)
    {
    clipStart = tStart = tStarts[i];
    size = blockSizes[i];
    clipEnd = tEnd = tStart + size;
    if (tStart < tMax && tEnd > tMin)
	{
	if (clipStart < tMin) clipStart = tMin;
	if (clipEnd > tMax) clipEnd = tMax;
	clipOffset = clipStart - tStart;
	clipSize = clipEnd - clipStart;
	AllocVar(ff);
	ff->left = ffList;
	ffList = ff;
	ff->nStart = ff->nEnd = needle + qStarts[i] + clipOffset;
	ff->nEnd += clipSize;
	ff->hStart = ff->hEnd = haystack + clipStart - targetOffset;
	ff->hEnd += clipSize;
	}
    }
ffList = ffMakeRightLinks(ffList);
ffCountGoodEnds(ffList);
return ffList;
}

int pslOrientation(struct psl *psl)
/* Translate psl strand + or - to orientation +1 or -1 */
{
/* code below doesn't support negative target strand (translated blat) */
if (psl->strand[1] == '-')
    errAbort("pslOrientation doesn't support a negative target strand");
if (psl->strand[0] == '-')
    return -1;
else
    return 1;
}

int pslWeightedIntronOrientation(struct psl *psl, struct dnaSeq *genoSeq, int offset)
/* Return >0 if introns make it look like alignment is on + strand,
 *        <0 if introns make it look like alignment is on - strand,
 *        0 if can't tell.  The absolute value of the return indicates
 * how many splice sites we've seen supporting the orientation.
 * Sequence should NOT be reverse complemented.  */
{
int intronDir = 0;
int oneDir;
int i;
DNA *dna = genoSeq->dna;

/* code below doesn't support negative target strand (translated blat) */
if (psl->strand[1] == '-')
    errAbort("pslWeightedIntronOrientation doesn't support a negative target strand");

for (i=1; i<psl->blockCount; ++i)
    {
    int iStart, iEnd, blockSize = psl->blockSizes[i-1];
    if (psl->qStarts[i-1] + blockSize == psl->qStarts[i])
	{
	iStart = psl->tStarts[i-1] + psl->blockSizes[i-1] - offset;
	iEnd = psl->tStarts[i] - offset;
	oneDir = intronOrientation(dna+iStart, dna+iEnd);
	intronDir += oneDir;
	}
    }
return intronDir;
}

int pslIntronOrientation(struct psl *psl, struct dnaSeq *genoSeq, int offset)
/* Return 1 if introns make it look like alignment is on + strand,
 *       -1 if introns make it look like alignment is on - strand,
 *        0 if can't tell.
 * Sequence should NOT be reverse complemented.  */
{
int intronDir = pslWeightedIntronOrientation(psl, genoSeq, offset);
if (intronDir < 0)
    intronDir = -1;
else if (intronDir > 0)
    intronDir = 1;
return intronDir;
}

boolean pslHasIntron(struct psl *psl, struct dnaSeq *seq, int seqOffset)
/* Return TRUE if there's a probable intron. Sequence should NOT be
 * reverse complemented.*/
{
int blockCount = psl->blockCount, i;
unsigned *tStarts = psl->tStarts;
unsigned *blockSizes = psl->blockSizes;
unsigned *qStarts = psl->qStarts;
int blockSize, start, end;
DNA *dna = seq->dna;

for (i=1; i<blockCount; ++i)
    {
    blockSize = blockSizes[i-1];
    start = qStarts[i-1]+blockSize;
    end = qStarts[i];
    if (start == end)
        {
        start = tStarts[i-1] + blockSize;
        end = tStarts[i];
        if (psl->strand[1] == '-')
            reverseIntRange(&start, &end, psl->tSize);
        start -= seqOffset;
        end -= seqOffset;
	if (intronOrientation(dna+start, dna+end) != 0)
	    return TRUE;
	}
    }
return FALSE;
}

void pslTailSizes(struct psl *psl, int *retStartTail, int *retEndTail)
/* Find the length of "tails" (rather than extensions) implied by psl. */
{
int orientation = pslOrientation(psl);
int qFloppyStart, qFloppyEnd;
int tFloppyStart, tFloppyEnd;

if (orientation > 0)
    {
    qFloppyStart = psl->qStart;
    qFloppyEnd = psl->qSize - psl->qEnd;
    }
else
    {
    qFloppyStart = psl->qSize - psl->qEnd;
    qFloppyEnd = psl->qStart;
    }
tFloppyStart = psl->tStart;
tFloppyEnd = psl->tSize - psl->tEnd;
*retStartTail = min(qFloppyStart, tFloppyStart);
*retEndTail = min(qFloppyEnd, tFloppyEnd);
}

void pslRcBoth(struct psl *psl)
/* Swap around things in psl so it works as if the alignment
 * was done on the reverse strand of the target. */
{
int tSize = psl->tSize, qSize = psl->qSize;
int blockCount = psl->blockCount, i;
unsigned *tStarts = psl->tStarts, *qStarts = psl->qStarts, *blockSizes = psl->blockSizes;

reverseIntRange(&psl->tStart, &psl->tEnd, psl->tSize);
for (i=0; i<blockCount; ++i)
    {
    tStarts[i] = (int)tSize - ((int)tStarts[i] + (int)blockSizes[i]);
    qStarts[i] = (int)qSize - ((int)qStarts[i] + (int)blockSizes[i]);
    }
reverseUnsigned(tStarts, blockCount);
reverseUnsigned(qStarts, blockCount);
reverseUnsigned(blockSizes, blockCount);
}

void pslTargetOffset(struct psl *psl, int offset)
/* Add offset to target positions in psl. */
{
int i, blockCount = psl->blockCount;
unsigned *tStarts = psl->tStarts;
psl->tStart += offset;
psl->tEnd += offset;
for (i=0; i<blockCount; ++i)
   tStarts[i] += offset;
}

void pslDump(struct psl *psl, FILE *f)
/* Dump most of PSL to file - for debugging. */
{
int i;
fprintf(f, "<PRE>\n");
fprintf(f, "psl %s:%d-%d %s %s:%d-%d %d\n", 
	psl->qName, psl->qStart, psl->qEnd, psl->strand,
	psl->tName, psl->tStart, psl->tEnd, psl->blockCount);
for (i=0; i<psl->blockCount; ++i)
    fprintf(f, "  size %d, qStart %d, tStart %d\n", 
    	psl->blockSizes[i], psl->qStarts[i], psl->tStarts[i]);
fprintf(f, "</PRE>");
}

static void pslRecalcBounds(struct psl *psl)
/* Calculate qStart/qEnd tStart/tEnd at top level to be consistent
 * with blocks. */
{
int qStart, qEnd, tStart, tEnd, size;
int last = psl->blockCount - 1;
qStart = psl->qStarts[0];
tStart = psl->tStarts[0];
size = psl->blockSizes[last];
qEnd = psl->qStarts[last] + size;
tEnd = psl->tStarts[last] + size;
if (psl->strand[0] == '-')
    reverseIntRange(&qStart, &qEnd, psl->qSize);
if (psl->strand[1] == '-')
    reverseIntRange(&tStart, &tEnd, psl->tSize);
psl->qStart = qStart;
psl->qEnd = qEnd;
psl->tStart = tStart;
psl->tEnd = tEnd;
}

struct psl *pslTrimToTargetRange(struct psl *oldPsl, int tMin, int tMax)
/* Return psl trimmed to fit inside tMin/tMax.  Note this does not
 * update the match/misMatch and related fields. */
{
int newSize;
int oldBlockCount = oldPsl->blockCount;
boolean tIsRc = (oldPsl->strand[1] == '-');
boolean qIsRc = (oldPsl->strand[0] == '-');
int newBlockCount = 0, completeBlockCount = 0;
int i, newI = 0;
struct psl *newPsl = NULL;
int tMn = tMin, tMx = tMax;   /* tMin/tMax adjusted for strand. */

/* Deal with case where we're completely trimmed out quickly. */
newSize = rangeIntersection(oldPsl->tStart, oldPsl->tEnd, tMin, tMax);
if (newSize <= 0)
    return NULL;

if (tIsRc)
    reverseIntRange(&tMn, &tMx, oldPsl->tSize);

/* Count how many blocks will survive trimming. */
oldBlockCount = oldPsl->blockCount;
for (i=0; i<oldBlockCount; ++i)
    {
    int s = oldPsl->tStarts[i];
    int e = s + oldPsl->blockSizes[i];
    int sz = e - s;
    int overlap;
    if ((overlap = rangeIntersection(s, e, tMn, tMx)) > 0)
        ++newBlockCount;
    if (overlap == sz)
        ++completeBlockCount;
    }

if (newBlockCount == 0)
    return NULL;

/* Allocate new psl and fill in what we already know. */
AllocVar(newPsl);
strcpy(newPsl->strand, oldPsl->strand);
newPsl->qName = cloneString(oldPsl->qName);
newPsl->qSize = oldPsl->qSize;
newPsl->tName = cloneString(oldPsl->tName);
newPsl->tSize = oldPsl->tSize;
newPsl->blockCount = newBlockCount;
AllocArray(newPsl->blockSizes, newBlockCount);
AllocArray(newPsl->qStarts, newBlockCount);
AllocArray(newPsl->tStarts, newBlockCount);

/* Fill in blockSizes, qStarts, tStarts with real data. */
newBlockCount = completeBlockCount = 0;
for (i=0; i<oldBlockCount; ++i)
    {
    int oldSz = oldPsl->blockSizes[i];
    int sz = oldSz;
    int tS = oldPsl->tStarts[i];
    int tE = tS + sz;
    int qS = oldPsl->qStarts[i];
    int qE = qS + sz;
    if (rangeIntersection(tS, tE, tMn, tMx) > 0)
        {
	int diff;
	if ((diff = (tMn - tS)) > 0)
	    {
	    tS += diff;
	    qS += diff;
	    sz -= diff;
	    }
	if ((diff = (tE - tMx)) > 0)
	    {
	    tE -= diff;
	    qE -= diff;
	    sz -= diff;
	    }
	newPsl->qStarts[newBlockCount] = qS;
	newPsl->tStarts[newBlockCount] = tS;
	newPsl->blockSizes[newBlockCount] = sz;
	++newBlockCount;
	if (sz == oldSz)
	    ++completeBlockCount;
	}
    }
pslRecalcBounds(newPsl);
return newPsl;
}

char* pslGetCreateSql(char* table, unsigned options)
/* Get SQL required to create PSL table.  Options is a bit set consisting
 * of PSL_TNAMEIX, PSL_WITH_BIN, and PSL_XA_FORMAT */
{
struct dyString *sqlCmd = newDyString(2048);
char *sqlCmdStr;
char *extraIx = ((options & PSL_TNAMEIX) ? "tName(8)," : "" );
char *binIxString = "";
if (options & PSL_WITH_BIN)
    {
    if (options & PSL_TNAMEIX)
	binIxString = "INDEX(tName(8),bin),\n";
    else
        binIxString = "INDEX(bin),\n";
    }
dyStringPrintf(sqlCmd, createString, table, 
    ((options & PSL_WITH_BIN) ? "bin smallint unsigned not null,\n" : ""));
if (options & PSL_XA_FORMAT)
    {
    dyStringPrintf(sqlCmd, "qSeq longblob not null,\n");
    dyStringPrintf(sqlCmd, "tSeq longblob not null,\n");
    }
dyStringPrintf(sqlCmd, indexString, binIxString, extraIx, extraIx);
sqlCmdStr = cloneString(sqlCmd->string);
dyStringFree(&sqlCmd);
return sqlCmdStr;
}

static void printPslDesc(char* pslDesc, FILE* out, struct psl* psl)
/* print description of a PSL on first error */
{
fprintf(out, "Error: invalid PSL: %s:%u-%u %s:%u-%u %s\n",
        psl->qName, psl->qStart, psl->qEnd,
        psl->tName, psl->tStart, psl->tEnd,
        pslDesc);
}

static void chkRanges(char* pslDesc, FILE* out, struct psl* psl,
                      char* pName, char* pLabel, char pCLabel, char pStrand,
                      unsigned pSize, unsigned pStart, unsigned pEnd,
                      unsigned blockCount, unsigned* blockSizes,
                      unsigned* pBlockStarts, int* errCountPtr)
/* check the target or query ranges in a PSL, increment errorCnt */
{
int errCount = *errCountPtr;
unsigned iBlk, prevBlkEnd = 0;

if (pStart >= pEnd)
    {
    if (errCount == 0)
        printPslDesc(pslDesc, out, psl);
    fprintf(out, "\t%s %cStart %u >= %cEnd %u\n",
            pName, pCLabel, pStart, pCLabel, pEnd);
    errCount++;
    }
for (iBlk = 0; iBlk < blockCount; iBlk++)
    {
    unsigned blkStart = pBlockStarts[iBlk];
    unsigned blkEnd = blkStart+blockSizes[iBlk];
    /* translate stand to genomic coords */
    unsigned gBlkStart = (pStrand == '+') ? blkStart : (pSize - blkEnd);
    unsigned gBlkEnd = (pStrand == '+') ? blkEnd : (pSize - blkStart);

    if (blockSizes[iBlk] == 0)
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u size is 0\n", pName, pLabel, iBlk);
        errCount++;
        }
    if ((pSize > 0) && (blkEnd > pSize))
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u end %u > %cSize %u\n",
                pName, pLabel, iBlk, blkEnd, pCLabel, pSize);
        errCount++;
        }
    if (gBlkStart < pStart)
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u translated start %u < %cStart %u\n",
                pName, pLabel, iBlk, gBlkStart, pCLabel, pStart);
        errCount++;
        }
    if (gBlkStart >= pEnd)
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u translated start %u >= %cEnd %u\n",
                pName, pLabel, iBlk, gBlkStart, pCLabel, pEnd);
        errCount++;
        }
    if (gBlkEnd < pStart)
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u translated end %u < %cStart %u\n",
                pName, pLabel, iBlk, gBlkEnd, pCLabel, pStart);
        errCount++;
        }
    if (gBlkEnd > pEnd)
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u translated end %u > %cEnd %u\n",
                pName, pLabel, iBlk, gBlkEnd, pCLabel, pEnd);
        errCount++;
        }
    if ((iBlk > 0) && (blkStart < prevBlkEnd))
        {
        if (errCount == 0)
            printPslDesc(pslDesc, out, psl);
        fprintf(out, "\t%s %s block %u start %u < previous block end %u\n",
                pName, pLabel, iBlk, blkStart, prevBlkEnd);
        errCount++;
        }
    prevBlkEnd = blkEnd;
    }
*errCountPtr = errCount;
}

int pslCheck(char *pslDesc, FILE* out, struct psl* psl)
/* Validate a PSL for consistency.  pslDesc is printed the error messages
 * to file out (open /dev/null to discard). Return count of errors. */
{
static char* VALID_STRANDS[] = {
    "+", "-", "++", "+-", "-+", "--", NULL
};
int i, errCount = 0;
char strand;

/* check strand value */
for (i = 0; VALID_STRANDS[i] != NULL; i++)
    {
    if (strcmp(psl->strand, VALID_STRANDS[i]) == 0)
        break;
    }
if (VALID_STRANDS[i] == NULL)
    {
    if (errCount == 0)
        printPslDesc(pslDesc, out, psl);
    fprintf(out, "\tinvalid PSL strand: \"%s\"\n", psl->strand);
    errCount++;
    }

/* check target */
strand = ((psl->strand[1] == '\0') ? '+' : psl->strand[1]);
chkRanges(pslDesc, out, psl, psl->tName, "target", 't',
          strand, psl->tSize, psl->tStart, psl->tEnd,
          psl->blockCount, psl->blockSizes, psl->tStarts,
          &errCount);

/* check query */
strand = psl->strand[0];
chkRanges(pslDesc, out, psl, psl->qName, "query", 'q',
          strand, psl->qSize, psl->qStart, psl->qEnd,
          psl->blockCount, psl->blockSizes, psl->qStarts,
          &errCount);
return errCount;
}

