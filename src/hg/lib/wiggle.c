/* wiggle.c was originally generated by the autoSql program, which also 
 * generated wiggle.h and wiggle.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "wiggle.h"

static char const rcsid[] = "$Id: wiggle.c,v 1.6 2004/01/13 21:38:19 hiram Exp $";

void wiggleStaticLoad(char **row, struct wiggle *ret)
/* Load a row from wiggle table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->span = sqlUnsigned(row[4]);
ret->count = sqlUnsigned(row[5]);
ret->offset = sqlUnsigned(row[6]);
ret->file = row[7];
ret->lowerLimit = atof(row[8]);
ret->dataRange = atof(row[9]);
ret->validCount = sqlUnsigned(row[10]);
ret->sumData = atof(row[11]);
ret->sumSquares = atof(row[12]);
}

struct wiggle *wiggleLoad(char **row)
/* Load a wiggle from row fetched with select * from wiggle
 * from database.  Dispose of this with wiggleFree(). */
{
struct wiggle *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->span = sqlUnsigned(row[4]);
ret->count = sqlUnsigned(row[5]);
ret->offset = sqlUnsigned(row[6]);
ret->file = cloneString(row[7]);
ret->lowerLimit = atof(row[8]);
ret->dataRange = atof(row[9]);
ret->validCount = sqlUnsigned(row[10]);
ret->sumData = atof(row[11]);
ret->sumSquares = atof(row[12]);
return ret;
}

struct wiggle *wiggleLoadAll(char *fileName) 
/* Load all wiggle from a whitespace-separated file.
 * Dispose of this with wiggleFreeList(). */
{
struct wiggle *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileRow(lf, row))
    {
    el = wiggleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct wiggle *wiggleLoadAllByChar(char *fileName, char chopper) 
/* Load all wiggle from a chopper separated file.
 * Dispose of this with wiggleFreeList(). */
{
struct wiggle *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = wiggleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct wiggle *wiggleCommaIn(char **pS, struct wiggle *ret)
/* Create a wiggle out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wiggle */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->span = sqlUnsignedComma(&s);
ret->count = sqlUnsignedComma(&s);
ret->offset = sqlUnsignedComma(&s);
ret->file = sqlStringComma(&s);
ret->lowerLimit = sqlDoubleComma(&s);
ret->dataRange = sqlDoubleComma(&s);
ret->validCount = sqlUnsignedComma(&s);
ret->sumData = sqlDoubleComma(&s);
ret->sumSquares = sqlDoubleComma(&s);
*pS = s;
return ret;
}

void wiggleFree(struct wiggle **pEl)
/* Free a single dynamically allocated wiggle such as created
 * with wiggleLoad(). */
{
struct wiggle *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->file);
freez(pEl);
}

void wiggleFreeList(struct wiggle **pList)
/* Free a list of dynamically allocated wiggle's */
{
struct wiggle *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    wiggleFree(&el);
    }
*pList = NULL;
}

void wiggleOutput(struct wiggle *el, FILE *f, char sep, char lastSep) 
/* Print out wiggle.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->span);
fputc(sep,f);
fprintf(f, "%u", el->count);
fputc(sep,f);
fprintf(f, "%u", el->offset);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->file);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->lowerLimit);
fputc(sep,f);
fprintf(f, "%f", el->dataRange);
fputc(sep,f);
fprintf(f, "%u", el->validCount);
fputc(sep,f);
fprintf(f, "%f", el->sumData);
fputc(sep,f);
fprintf(f, "%f", el->sumSquares);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

