/* transRegCodeProbe.c was originally generated by the autoSql program, which also 
 * generated transRegCodeProbe.h and transRegCodeProbe.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "transRegCodeProbe.h"

static char const rcsid[] = "$Id: transRegCodeProbe.c,v 1.2 2004/09/26 06:29:27 kent Exp $";

struct transRegCodeProbe *transRegCodeProbeLoad(char **row)
/* Load a transRegCodeProbe from row fetched with select * from transRegCodeProbe
 * from database.  Dispose of this with transRegCodeProbeFree(). */
{
struct transRegCodeProbe *ret;

AllocVar(ret);
ret->tfCount = sqlUnsigned(row[4]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
{
int sizeOne;
sqlStringDynamicArray(row[5], &ret->tfList, &sizeOne);
assert(sizeOne == ret->tfCount);
}
{
int sizeOne;
sqlFloatDynamicArray(row[6], &ret->bindVals, &sizeOne);
assert(sizeOne == ret->tfCount);
}
return ret;
}

struct transRegCodeProbe *transRegCodeProbeLoadAll(char *fileName) 
/* Load all transRegCodeProbe from a whitespace-separated file.
 * Dispose of this with transRegCodeProbeFreeList(). */
{
struct transRegCodeProbe *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = transRegCodeProbeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct transRegCodeProbe *transRegCodeProbeLoadAllByChar(char *fileName, char chopper) 
/* Load all transRegCodeProbe from a chopper separated file.
 * Dispose of this with transRegCodeProbeFreeList(). */
{
struct transRegCodeProbe *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = transRegCodeProbeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct transRegCodeProbe *transRegCodeProbeCommaIn(char **pS, struct transRegCodeProbe *ret)
/* Create a transRegCodeProbe out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new transRegCodeProbe */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->tfCount = sqlUnsignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->tfList, ret->tfCount);
for (i=0; i<ret->tfCount; ++i)
    {
    ret->tfList[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->bindVals, ret->tfCount);
for (i=0; i<ret->tfCount; ++i)
    {
    ret->bindVals[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void transRegCodeProbeFree(struct transRegCodeProbe **pEl)
/* Free a single dynamically allocated transRegCodeProbe such as created
 * with transRegCodeProbeLoad(). */
{
struct transRegCodeProbe *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
/* All strings in tfList are allocated at once, so only need to free first. */
if (el->tfList != NULL)
    freeMem(el->tfList[0]);
freeMem(el->tfList);
freeMem(el->bindVals);
freez(pEl);
}

void transRegCodeProbeFreeList(struct transRegCodeProbe **pList)
/* Free a list of dynamically allocated transRegCodeProbe's */
{
struct transRegCodeProbe *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    transRegCodeProbeFree(&el);
    }
*pList = NULL;
}

void transRegCodeProbeOutput(struct transRegCodeProbe *el, FILE *f, char sep, char lastSep) 
/* Print out transRegCodeProbe.  Separate fields with sep. Follow last field with lastSep. */
{
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
fprintf(f, "%u", el->tfCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->tfCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->tfList[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->tfCount; ++i)
    {
    fprintf(f, "%g", el->bindVals[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

