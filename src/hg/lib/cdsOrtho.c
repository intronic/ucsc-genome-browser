/* cdsOrtho.c was originally generated by the autoSql program, which also 
 * generated cdsOrtho.h and cdsOrtho.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "cdsOrtho.h"


void cdsOrthoStaticLoad(char **row, struct cdsOrtho *ret)
/* Load a row from cdsOrtho table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->start = sqlSigned(row[1]);
ret->end = sqlSigned(row[2]);
ret->species = row[3];
ret->missing = sqlSigned(row[4]);
ret->orthoSize = sqlSigned(row[5]);
ret->possibleSize = sqlSigned(row[6]);
ret->ratio = sqlDouble(row[7]);
}

struct cdsOrtho *cdsOrthoLoad(char **row)
/* Load a cdsOrtho from row fetched with select * from cdsOrtho
 * from database.  Dispose of this with cdsOrthoFree(). */
{
struct cdsOrtho *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->start = sqlSigned(row[1]);
ret->end = sqlSigned(row[2]);
ret->species = cloneString(row[3]);
ret->missing = sqlSigned(row[4]);
ret->orthoSize = sqlSigned(row[5]);
ret->possibleSize = sqlSigned(row[6]);
ret->ratio = sqlDouble(row[7]);
return ret;
}

struct cdsOrtho *cdsOrthoLoadAll(char *fileName) 
/* Load all cdsOrtho from a whitespace-separated file.
 * Dispose of this with cdsOrthoFreeList(). */
{
struct cdsOrtho *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = cdsOrthoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cdsOrtho *cdsOrthoLoadAllByChar(char *fileName, char chopper) 
/* Load all cdsOrtho from a chopper separated file.
 * Dispose of this with cdsOrthoFreeList(). */
{
struct cdsOrtho *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = cdsOrthoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cdsOrtho *cdsOrthoCommaIn(char **pS, struct cdsOrtho *ret)
/* Create a cdsOrtho out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cdsOrtho */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->start = sqlSignedComma(&s);
ret->end = sqlSignedComma(&s);
ret->species = sqlStringComma(&s);
ret->missing = sqlSignedComma(&s);
ret->orthoSize = sqlSignedComma(&s);
ret->possibleSize = sqlSignedComma(&s);
ret->ratio = sqlDoubleComma(&s);
*pS = s;
return ret;
}

void cdsOrthoFree(struct cdsOrtho **pEl)
/* Free a single dynamically allocated cdsOrtho such as created
 * with cdsOrthoLoad(). */
{
struct cdsOrtho *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->species);
freez(pEl);
}

void cdsOrthoFreeList(struct cdsOrtho **pList)
/* Free a list of dynamically allocated cdsOrtho's */
{
struct cdsOrtho *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cdsOrthoFree(&el);
    }
*pList = NULL;
}

void cdsOrthoOutput(struct cdsOrtho *el, FILE *f, char sep, char lastSep) 
/* Print out cdsOrtho.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->start);
fputc(sep,f);
fprintf(f, "%d", el->end);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->species);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->missing);
fputc(sep,f);
fprintf(f, "%d", el->orthoSize);
fputc(sep,f);
fprintf(f, "%d", el->possibleSize);
fputc(sep,f);
fprintf(f, "%g", el->ratio);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

