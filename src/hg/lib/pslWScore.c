/* pslWScore.c was originally generated by the autoSql program, which also 
 * generated pslWScore.h and pslWScore.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "pslWScore.h"

static char const rcsid[] = "$Id: pslWScore.c,v 1.3 2005/04/13 06:25:55 markd Exp $";

struct pslWScore *pslWScoreLoad(char **row)
/* Load a pslWScore from row fetched with select * from pslWScore
 * from database.  Dispose of this with pslWScoreFree(). */
{
struct pslWScore *ret;
int sizeOne;

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
ret->score = atof(row[21]);
return ret;
}

struct pslWScore *pslWScoreLoadAll(char *fileName) 
/* Load all pslWScore from a tab-separated file.
 * Dispose of this with pslWScoreFreeList(). */
{
struct pslWScore *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[22];

while (lineFileRow(lf, row))
    {
    el = pslWScoreLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pslWScore *pslWScoreCommaIn(char **pS, struct pslWScore *ret)
/* Create a pslWScore out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pslWScore */
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
ret->score = sqlSignedComma(&s);
*pS = s;
return ret;
}

void pslWScoreFree(struct pslWScore **pEl)
/* Free a single dynamically allocated pslWScore such as created
 * with pslWScoreLoad(). */
{
struct pslWScore *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qName);
freeMem(el->tName);
freeMem(el->blockSizes);
freeMem(el->qStarts);
freeMem(el->tStarts);
freez(pEl);
}

void pslWScoreFreeList(struct pslWScore **pList)
/* Free a list of dynamically allocated pslWScore's */
{
struct pslWScore *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pslWScoreFree(&el);
    }
*pList = NULL;
}

void pslWScoreOutput(struct pslWScore *el, FILE *f, char sep, char lastSep) 
/* Print out pslWScore.  Separate fields with sep. Follow last field with lastSep. */
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
fputc(sep,f);
fprintf(f, "%f", el->score);
fputc(lastSep,f);
}

