/* bed5Pval.c was originally generated by the autoSql program, which also 
 * generated bed5Pval.h and bed5Pval.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "bed5Pval.h"


void bed5PvalStaticLoad(char **row, struct bed5Pval *ret)
/* Load a row from bed5Pval table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlSigned(row[4]);
ret->pValue = sqlFloat(row[5]);
}

struct bed5Pval *bed5PvalLoad(char **row)
/* Load a bed5Pval from row fetched with select * from bed5Pval
 * from database.  Dispose of this with bed5PvalFree(). */
{
struct bed5Pval *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlSigned(row[4]);
ret->pValue = sqlFloat(row[5]);
return ret;
}

struct bed5Pval *bed5PvalLoadAll(char *fileName) 
/* Load all bed5Pval from a whitespace-separated file.
 * Dispose of this with bed5PvalFreeList(). */
{
struct bed5Pval *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = bed5PvalLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bed5Pval *bed5PvalLoadAllByChar(char *fileName, char chopper) 
/* Load all bed5Pval from a chopper separated file.
 * Dispose of this with bed5PvalFreeList(). */
{
struct bed5Pval *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = bed5PvalLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bed5Pval *bed5PvalCommaIn(char **pS, struct bed5Pval *ret)
/* Create a bed5Pval out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed5Pval */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlSignedComma(&s);
ret->pValue = sqlFloatComma(&s);
*pS = s;
return ret;
}

void bed5PvalFree(struct bed5Pval **pEl)
/* Free a single dynamically allocated bed5Pval such as created
 * with bed5PvalLoad(). */
{
struct bed5Pval *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void bed5PvalFreeList(struct bed5Pval **pList)
/* Free a list of dynamically allocated bed5Pval's */
{
struct bed5Pval *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bed5PvalFree(&el);
    }
*pList = NULL;
}

void bed5PvalOutput(struct bed5Pval *el, FILE *f, char sep, char lastSep) 
/* Print out bed5Pval.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%d", el->score);
fputc(sep,f);
fprintf(f, "%g", el->pValue);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

