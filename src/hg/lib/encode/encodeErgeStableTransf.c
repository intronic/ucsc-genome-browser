/* encodeErgeStableTransf.c was originally generated by the autoSql program, which also 
 * generated encodeErgeStableTransf.h and encodeErgeStableTransf.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encode/encodeErgeStableTransf.h"

static char const rcsid[] = "$Id: encodeErgeStableTransf.c,v 1.1 2008/08/01 17:49:05 aamp Exp $";

struct encodeErgeStableTransf *encodeErgeStableTransfLoad(char **row)
/* Load a encodeErgeStableTransf from row fetched with select * from encodeErgeStableTransf
 * from database.  Dispose of this with encodeErgeStableTransfFree(). */
{
struct encodeErgeStableTransf *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->blockCount = sqlUnsigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
sqlUnsignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
ret->Id = cloneString(row[12]);
ret->color = cloneString(row[13]);
return ret;
}

struct encodeErgeStableTransf *encodeErgeStableTransfLoadAll(char *fileName) 
/* Load all encodeErgeStableTransf from a whitespace-separated file.
 * Dispose of this with encodeErgeStableTransfFreeList(). */
{
struct encodeErgeStableTransf *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileRow(lf, row))
    {
    el = encodeErgeStableTransfLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeErgeStableTransf *encodeErgeStableTransfLoadAllByChar(char *fileName, char chopper) 
/* Load all encodeErgeStableTransf from a chopper separated file.
 * Dispose of this with encodeErgeStableTransfFreeList(). */
{
struct encodeErgeStableTransf *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodeErgeStableTransfLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeErgeStableTransf *encodeErgeStableTransfCommaIn(char **pS, struct encodeErgeStableTransf *ret)
/* Create a encodeErgeStableTransf out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeErgeStableTransf */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->thickStart = sqlUnsignedComma(&s);
ret->thickEnd = sqlUnsignedComma(&s);
ret->reserved = sqlUnsignedComma(&s);
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
AllocArray(ret->chromStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->chromStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->Id = sqlStringComma(&s);
ret->color = sqlStringComma(&s);
*pS = s;
return ret;
}

void encodeErgeStableTransfFree(struct encodeErgeStableTransf **pEl)
/* Free a single dynamically allocated encodeErgeStableTransf such as created
 * with encodeErgeStableTransfLoad(). */
{
struct encodeErgeStableTransf *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->Id);
freeMem(el->color);
freez(pEl);
}

void encodeErgeStableTransfFreeList(struct encodeErgeStableTransf **pList)
/* Free a list of dynamically allocated encodeErgeStableTransf's */
{
struct encodeErgeStableTransf *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodeErgeStableTransfFree(&el);
    }
*pList = NULL;
}

void encodeErgeStableTransfOutput(struct encodeErgeStableTransf *el, FILE *f, char sep, char lastSep) 
/* Print out encodeErgeStableTransf.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
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
fprintf(f, "%u", el->thickStart);
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
fputc(sep,f);
fprintf(f, "%u", el->reserved);
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
    fprintf(f, "%u", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->Id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->color);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

