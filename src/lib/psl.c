/* psl.c was originally generated by the autoSql program, which also 
 * generated as_psl.h and as_psl.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "sqlNum.h"
#include "sqlList.h"
#include "localmem.h"
#include "psl.h"
#include "linefile.h"
#include "dnaseq.h"
#include "fuzzyFind.h"
#include "aliType.h"

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
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[19], &ret->qStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[20], &ret->tStarts, &sizeOne);
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
fprintf(f, "%u", el->match, sep);
fputc(sep,f);
fprintf(f, "%u", el->misMatch, sep);
fputc(sep,f);
fprintf(f, "%u", el->repMatch, sep);
fputc(sep,f);
fprintf(f, "%u", el->nCount, sep);
fputc(sep,f);
fprintf(f, "%u", el->qNumInsert, sep);
fputc(sep,f);
fprintf(f, "%d", el->qBaseInsert, sep);
fputc(sep,f);
fprintf(f, "%u", el->tNumInsert, sep);
fputc(sep,f);
fprintf(f, "%d", el->tBaseInsert, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qSize, sep);
fputc(sep,f);
fprintf(f, "%u", el->qStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->qEnd, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tName, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tSize, sep);
fputc(sep,f);
fprintf(f, "%u", el->tStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->tEnd, sep);
fputc(sep,f);
fprintf(f, "%u", el->blockCount, sep);
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
return pslList;
}


int pslCmpQuery(const void *va, const void *vb)
/* Compare to sort based on query. */
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
/* Compare to sort based on query. */
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
	char *s = strdup(line);
	wordCount = chopLine(s, words);
	if (wordCount < 21 || wordCount > 22 || (words[8][0] != '+' && words[8][0] != '-'))
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

int pslCalcMilliBad(struct psl *psl, boolean isMrna)
/* Calculate badness in parts per thousand. */
{
int qAliSize, tAliSize, aliSize;
int milliBad;
int sizeDif;
int insertFactor;

qAliSize = psl->qEnd - psl->qStart;
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

milliBad = (1000 * (psl->misMatch + insertFactor + round(3*log(1+sizeDif)))) / aliSize;
return milliBad;
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
    reverseUnsignedRange(&psl->qStart, &psl->qEnd, psl->qSize);
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
if (psl->strand[0] == '-')
    return -1;
else
    return 1;
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

