/* altSpliceSite.c was originally generated by the autoSql program, which also 
 * generated altSpliceSite.h and altSpliceSite.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "altSpliceSite.h"

static char const rcsid[] = "$Id: altSpliceSite.c,v 1.2 2003/07/21 04:35:28 sugnet Exp $";

struct altSpliceSite *altSpliceSiteLoad(char **row)
/* Load a altSpliceSite from row fetched with select * from altSpliceSite
 * from database.  Dispose of this with altSpliceSiteFree(). */
{
struct altSpliceSite *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->altCount = sqlUnsigned(row[7]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->agName = cloneString(row[3]);
strcpy(ret->strand, row[4]);
ret->index = sqlUnsigned(row[5]);
ret->type = sqlUnsigned(row[6]);
sqlUnsignedDynamicArray(row[8], &ret->vIndexes, &sizeOne);
assert(sizeOne == ret->altCount);
sqlUnsignedDynamicArray(row[9], &ret->altStarts, &sizeOne);
assert(sizeOne == ret->altCount);
sqlUnsignedDynamicArray(row[10], &ret->altTypes, &sizeOne);
assert(sizeOne == ret->altCount);
sqlUnsignedDynamicArray(row[11], &ret->spliceTypes, &sizeOne);
assert(sizeOne == ret->altCount);
sqlUnsignedDynamicArray(row[12], &ret->support, &sizeOne);
assert(sizeOne == ret->altCount);
sqlUnsignedDynamicArray(row[13], &ret->altBpStarts, &sizeOne);
assert(sizeOne == ret->altCount);
sqlUnsignedDynamicArray(row[14], &ret->altBpEnds, &sizeOne);
assert(sizeOne == ret->altCount);
sqlFloatDynamicArray(row[15], &ret->altCons, &sizeOne);
assert(sizeOne == ret->altCount);
sqlFloatDynamicArray(row[16], &ret->upStreamCons, &sizeOne);
assert(sizeOne == ret->altCount);
sqlFloatDynamicArray(row[17], &ret->downStreamCons, &sizeOne);
assert(sizeOne == ret->altCount);
return ret;
}

struct altSpliceSite *altSpliceSiteLoadAll(char *fileName) 
/* Load all altSpliceSite from a whitespace-separated file.
 * Dispose of this with altSpliceSiteFreeList(). */
{
struct altSpliceSite *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[18];

while (lineFileRow(lf, row))
    {
    el = altSpliceSiteLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct altSpliceSite *altSpliceSiteLoadAllByChar(char *fileName, char chopper) 
/* Load all altSpliceSite from a chopper separated file.
 * Dispose of this with altSpliceSiteFreeList(). */
{
struct altSpliceSite *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[18];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = altSpliceSiteLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct altSpliceSite *altSpliceSiteCommaIn(char **pS, struct altSpliceSite *ret)
/* Create a altSpliceSite out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new altSpliceSite */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->agName = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->index = sqlUnsignedComma(&s);
ret->type = sqlUnsignedComma(&s);
ret->altCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->vIndexes, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->vIndexes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->altStarts, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->altStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->altTypes, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->altTypes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->spliceTypes, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->spliceTypes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->support, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->support[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->altBpStarts, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->altBpStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->altBpEnds, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->altBpEnds[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->altCons, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->altCons[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->upStreamCons, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->upStreamCons[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->downStreamCons, ret->altCount);
for (i=0; i<ret->altCount; ++i)
    {
    ret->downStreamCons[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void altSpliceSiteFree(struct altSpliceSite **pEl)
/* Free a single dynamically allocated altSpliceSite such as created
 * with altSpliceSiteLoad(). */
{
struct altSpliceSite *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->agName);
freeMem(el->vIndexes);
freeMem(el->altStarts);
freeMem(el->altTypes);
freeMem(el->spliceTypes);
freeMem(el->support);
freeMem(el->altBpStarts);
freeMem(el->altBpEnds);
freeMem(el->altCons);
freeMem(el->upStreamCons);
freeMem(el->downStreamCons);
freez(pEl);
}

void altSpliceSiteFreeList(struct altSpliceSite **pList)
/* Free a list of dynamically allocated altSpliceSite's */
{
struct altSpliceSite *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    altSpliceSiteFree(&el);
    }
*pList = NULL;
}

void altSpliceSiteOutput(struct altSpliceSite *el, FILE *f, char sep, char lastSep) 
/* Print out altSpliceSite.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->agName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->index);
fputc(sep,f);
fprintf(f, "%u", el->type);
fputc(sep,f);
fprintf(f, "%u", el->altCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->vIndexes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->altStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->altTypes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->spliceTypes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->support[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->altBpStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%u", el->altBpEnds[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%f", el->altCons[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%f", el->upStreamCons[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->altCount; ++i)
    {
    fprintf(f, "%f", el->downStreamCons[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

