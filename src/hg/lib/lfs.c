/* lfs.c was originally generated by the autoSql program, which also 
 * generated lfs.h and lfs.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "lfs.h"


struct lfs *lfsLoad(char **row)
/* Load a lfs from row fetched with select * from lfs
 * from database.  Dispose of this with lfsFree(). */
{
struct lfs *ret;
int sizeOne;

AllocVar(ret);
ret->lfCount = sqlUnsigned(row[7]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->pslTable = cloneString(row[6]);
sqlUnsignedDynamicArray(row[8], &ret->lfStarts, &sizeOne);
assert(sizeOne == ret->lfCount);
sqlUnsignedDynamicArray(row[9], &ret->lfSizes, &sizeOne);
assert(sizeOne == ret->lfCount);
sqlStringDynamicArray(row[10], &ret->lfNames, &sizeOne);
assert(sizeOne == ret->lfCount);
return ret;
}

struct lfs *lfsLoadAll(char *fileName) 
/* Load all lfs from a tab-separated file.
 * Dispose of this with lfsFreeList(). */
{
struct lfs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = lfsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct lfs *lfsCommaIn(char **pS, struct lfs *ret)
/* Create a lfs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new lfs */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->bin = sqlSignedComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->pslTable = sqlStringComma(&s);
ret->lfCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->lfStarts, ret->lfCount);
for (i=0; i<ret->lfCount; ++i)
    {
    ret->lfStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->lfSizes, ret->lfCount);
for (i=0; i<ret->lfCount; ++i)
    {
    ret->lfSizes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->lfNames, ret->lfCount);
for (i=0; i<ret->lfCount; ++i)
    {
    ret->lfNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void lfsFree(struct lfs **pEl)
/* Free a single dynamically allocated lfs such as created
 * with lfsLoad(). */
{
struct lfs *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->pslTable);
freeMem(el->lfStarts);
freeMem(el->lfSizes);
/* All strings in lfNames are allocated at once, so only need to free first. */
if (el->lfNames != NULL)
    freeMem(el->lfNames[0]);
freeMem(el->lfNames);
freez(pEl);
}

void lfsFreeList(struct lfs **pList)
/* Free a list of dynamically allocated lfs's */
{
struct lfs *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    lfsFree(&el);
    }
*pList = NULL;
}

void lfsOutput(struct lfs *el, FILE *f, char sep, char lastSep) 
/* Print out lfs.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%d", el->bin);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->pslTable);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->lfCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->lfCount; ++i)
    {
    fprintf(f, "%u", el->lfStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->lfCount; ++i)
    {
    fprintf(f, "%u", el->lfSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->lfCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->lfNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

