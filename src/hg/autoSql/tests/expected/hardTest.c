/* hardTest.c was originally generated by the autoSql program, which also 
 * generated hardTest.h and hardTest.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "output/hardTest.h"

static char const rcsid[] = "$Id:$";

struct point *pointCommaIn(char **pS, struct point *ret)
/* Create a point out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new point */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->x = sqlSignedComma(&s);
ret->y = sqlSignedComma(&s);
ret->z = sqlSignedComma(&s);
*pS = s;
return ret;
}

void pointFree(struct point **pEl)
/* Free a single dynamically allocated point such as created
 * with pointLoad(). */
{
struct point *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void pointFreeList(struct point **pList)
/* Free a list of dynamically allocated point's */
{
struct point *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pointFree(&el);
    }
*pList = NULL;
}

void pointOutput(struct point *el, FILE *f, char sep, char lastSep) 
/* Print out point.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->x);
fputc(sep,f);
fprintf(f, "%d", el->y);
fputc(sep,f);
fprintf(f, "%d", el->z);
fputc(lastSep,f);
}

struct autoTest *autoTestLoad(char **row)
/* Load a autoTest from row fetched with select * from autoTest
 * from database.  Dispose of this with autoTestFree(). */
{
struct autoTest *ret;

AllocVar(ret);
ret->ptCount = sqlSigned(row[5]);
ret->difCount = sqlSigned(row[7]);
ret->valCount = sqlSigned(row[10]);
ret->id = sqlUnsigned(row[0]);
strcpy(ret->shortName, row[1]);
ret->longName = cloneString(row[2]);
{
char *s = cloneString(row[3]);
sqlStringArray(s, ret->aliases, 3);
}
{
char *s = row[4];
if(s != NULL && differentString(s, ""))
   ret->threeD = pointCommaIn(&s, NULL);
}
{
int sizeOne;
sqlShortDynamicArray(row[6], &ret->pts, &sizeOne);
assert(sizeOne == ret->ptCount);
}
{
int sizeOne;
sqlUbyteDynamicArray(row[8], &ret->difs, &sizeOne);
assert(sizeOne == ret->difCount);
}
sqlSignedArray(row[9], ret->xy, 2);
{
int sizeOne;
sqlStringDynamicArray(row[11], &ret->vals, &sizeOne);
assert(sizeOne == ret->valCount);
}
ret->dblVal = atof(row[12]);
ret->fltVal = atof(row[13]);
{
int sizeOne;
sqlDoubleDynamicArray(row[14], &ret->dblArray, &sizeOne);
assert(sizeOne == ret->valCount);
}
{
int sizeOne;
sqlFloatDynamicArray(row[15], &ret->fltArray, &sizeOne);
assert(sizeOne == ret->valCount);
}
return ret;
}

struct autoTest *autoTestLoadAll(char *fileName) 
/* Load all autoTest from a whitespace-separated file.
 * Dispose of this with autoTestFreeList(). */
{
struct autoTest *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[16];

while (lineFileRow(lf, row))
    {
    el = autoTestLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct autoTest *autoTestLoadAllByChar(char *fileName, char chopper) 
/* Load all autoTest from a chopper separated file.
 * Dispose of this with autoTestFreeList(). */
{
struct autoTest *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[16];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = autoTestLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct autoTest *autoTestCommaIn(char **pS, struct autoTest *ret)
/* Create a autoTest out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new autoTest */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->shortName, sizeof(ret->shortName));
ret->longName = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
for (i=0; i<3; ++i)
    {
    ret->aliases[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
s = sqlEatChar(s, '{');
if(s[0] != '}')    slSafeAddHead(&ret->threeD, pointCommaIn(&s,NULL));
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->ptCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->pts, ret->ptCount);
for (i=0; i<ret->ptCount; ++i)
    {
    ret->pts[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->difCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->difs, ret->difCount);
for (i=0; i<ret->difCount; ++i)
    {
    ret->difs[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
for (i=0; i<2; ++i)
    {
    ret->xy[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->valCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->vals, ret->valCount);
for (i=0; i<ret->valCount; ++i)
    {
    ret->vals[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->dblVal = sqlDoubleComma(&s);
ret->fltVal = sqlFloatComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->dblArray, ret->valCount);
for (i=0; i<ret->valCount; ++i)
    {
    ret->dblArray[i] = sqlDoubleComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->fltArray, ret->valCount);
for (i=0; i<ret->valCount; ++i)
    {
    ret->fltArray[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void autoTestFree(struct autoTest **pEl)
/* Free a single dynamically allocated autoTest such as created
 * with autoTestLoad(). */
{
struct autoTest *el;

if ((el = *pEl) == NULL) return;
freeMem(el->longName);
/* All strings in aliases are allocated at once, so only need to free first. */
if (el->aliases != NULL)
    freeMem(el->aliases[0]);
pointFreeList(&el->threeD);
freeMem(el->pts);
freeMem(el->difs);
/* All strings in vals are allocated at once, so only need to free first. */
if (el->vals != NULL)
    freeMem(el->vals[0]);
freeMem(el->vals);
freeMem(el->dblArray);
freeMem(el->fltArray);
freez(pEl);
}

void autoTestFreeList(struct autoTest **pList)
/* Free a list of dynamically allocated autoTest's */
{
struct autoTest *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    autoTestFree(&el);
    }
*pList = NULL;
}

void autoTestOutput(struct autoTest *el, FILE *f, char sep, char lastSep) 
/* Print out autoTest.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shortName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->longName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<3; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->aliases[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
if (sep == ',') fputc('{',f);
if(el->threeD != NULL)    pointCommaOut(el->threeD,f);
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%d", el->ptCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->ptCount; ++i)
    {
    fprintf(f, "%d", el->pts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->difCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->difCount; ++i)
    {
    fprintf(f, "%u", el->difs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<2; ++i)
    {
    fprintf(f, "%d", el->xy[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->valCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->valCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->vals[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%g", el->dblVal);
fputc(sep,f);
fprintf(f, "%g", el->fltVal);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->valCount; ++i)
    {
    fprintf(f, "%g", el->dblArray[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->valCount; ++i)
    {
    fprintf(f, "%g", el->fltArray[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

