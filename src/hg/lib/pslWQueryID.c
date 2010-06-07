/* pslWQueryID.c was originally generated by the autoSql program, which also 
 * generated pslWQueryID.h and pslWQueryID.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "pslWQueryID.h"

static char const rcsid[] = "$Id: pslWQueryID.c,v 1.1 2004/07/14 22:08:06 hartera Exp $";

struct pslWQueryID *pslWQueryIDLoad(char **row)
/* Load a pslWQueryID from row fetched with select * from pslWQueryID
 * from database.  Dispose of this with pslWQueryIDFree(). */
{
struct pslWQueryID *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->blockCount = sqlUnsigned(row[17]);
ret->matches = sqlUnsigned(row[0]);
ret->misMatches = sqlUnsigned(row[1]);
ret->repMatches = sqlUnsigned(row[2]);
ret->nCount = sqlUnsigned(row[3]);
ret->qNumInsert = sqlUnsigned(row[4]);
ret->qBaseInsert = sqlUnsigned(row[5]);
ret->tNumInsert = sqlUnsigned(row[6]);
ret->tBaseInsert = sqlUnsigned(row[7]);
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
ret->queryID = cloneString(row[21]);
return ret;
}

struct pslWQueryID *pslWQueryIDLoadAll(char *fileName) 
/* Load all pslWQueryID from a whitespace-separated file.
 * Dispose of this with pslWQueryIDFreeList(). */
{
struct pslWQueryID *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[22];

while (lineFileRow(lf, row))
    {
    el = pslWQueryIDLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pslWQueryID *pslWQueryIDLoadAllByChar(char *fileName, char chopper) 
/* Load all pslWQueryID from a chopper separated file.
 * Dispose of this with pslWQueryIDFreeList(). */
{
struct pslWQueryID *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[22];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pslWQueryIDLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pslWQueryID *pslWQueryIDCommaIn(char **pS, struct pslWQueryID *ret)
/* Create a pslWQueryID out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pslWQueryID */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->matches = sqlUnsignedComma(&s);
ret->misMatches = sqlUnsignedComma(&s);
ret->repMatches = sqlUnsignedComma(&s);
ret->nCount = sqlUnsignedComma(&s);
ret->qNumInsert = sqlUnsignedComma(&s);
ret->qBaseInsert = sqlUnsignedComma(&s);
ret->tNumInsert = sqlUnsignedComma(&s);
ret->tBaseInsert = sqlUnsignedComma(&s);
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
ret->queryID = sqlStringComma(&s);
*pS = s;
return ret;
}

void pslWQueryIDFree(struct pslWQueryID **pEl)
/* Free a single dynamically allocated pslWQueryID such as created
 * with pslWQueryIDLoad(). */
{
struct pslWQueryID *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qName);
freeMem(el->tName);
freeMem(el->blockSizes);
freeMem(el->qStarts);
freeMem(el->tStarts);
freeMem(el->queryID);
freez(pEl);
}

void pslWQueryIDFreeList(struct pslWQueryID **pList)
/* Free a list of dynamically allocated pslWQueryID's */
{
struct pslWQueryID *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pslWQueryIDFree(&el);
    }
*pList = NULL;
}

void pslWQueryIDOutput(struct pslWQueryID *el, FILE *f, char sep, char lastSep) 
/* Print out pslWQueryID.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->matches);
fputc(sep,f);
fprintf(f, "%u", el->misMatches);
fputc(sep,f);
fprintf(f, "%u", el->repMatches);
fputc(sep,f);
fprintf(f, "%u", el->nCount);
fputc(sep,f);
fprintf(f, "%u", el->qNumInsert);
fputc(sep,f);
fprintf(f, "%u", el->qBaseInsert);
fputc(sep,f);
fprintf(f, "%u", el->tNumInsert);
fputc(sep,f);
fprintf(f, "%u", el->tBaseInsert);
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
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->queryID);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

