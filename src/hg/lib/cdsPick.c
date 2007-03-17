/* cdsPick.c was originally generated by the autoSql program, which also 
 * generated cdsPick.h and cdsPick.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "cdsPick.h"

static char const rcsid[] = "$Id: cdsPick.c,v 1.2 2007/03/17 22:35:28 kent Exp $";

void cdsPickStaticLoad(char **row, struct cdsPick *ret)
/* Load a row from cdsPick table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->start = sqlSigned(row[1]);
ret->end = sqlSigned(row[2]);
ret->source = row[3];
ret->score = sqlDouble(row[4]);
ret->startComplete = sqlUnsigned(row[5]);
ret->endComplete = sqlUnsigned(row[6]);
ret->swissProt = row[7];
ret->uniProt = row[8];
ret->refProt = row[9];
ret->refSeq = row[10];
}

struct cdsPick *cdsPickLoad(char **row)
/* Load a cdsPick from row fetched with select * from cdsPick
 * from database.  Dispose of this with cdsPickFree(). */
{
struct cdsPick *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->start = sqlSigned(row[1]);
ret->end = sqlSigned(row[2]);
ret->source = cloneString(row[3]);
ret->score = sqlDouble(row[4]);
ret->startComplete = sqlUnsigned(row[5]);
ret->endComplete = sqlUnsigned(row[6]);
ret->swissProt = cloneString(row[7]);
ret->uniProt = cloneString(row[8]);
ret->refProt = cloneString(row[9]);
ret->refSeq = cloneString(row[10]);
return ret;
}

struct cdsPick *cdsPickLoadAll(char *fileName) 
/* Load all cdsPick from a whitespace-separated file.
 * Dispose of this with cdsPickFreeList(). */
{
struct cdsPick *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = cdsPickLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cdsPick *cdsPickLoadAllByChar(char *fileName, char chopper) 
/* Load all cdsPick from a chopper separated file.
 * Dispose of this with cdsPickFreeList(). */
{
struct cdsPick *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = cdsPickLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cdsPick *cdsPickCommaIn(char **pS, struct cdsPick *ret)
/* Create a cdsPick out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cdsPick */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->start = sqlSignedComma(&s);
ret->end = sqlSignedComma(&s);
ret->source = sqlStringComma(&s);
ret->score = sqlDoubleComma(&s);
ret->startComplete = sqlUnsignedComma(&s);
ret->endComplete = sqlUnsignedComma(&s);
ret->swissProt = sqlStringComma(&s);
ret->uniProt = sqlStringComma(&s);
ret->refProt = sqlStringComma(&s);
ret->refSeq = sqlStringComma(&s);
*pS = s;
return ret;
}

void cdsPickFree(struct cdsPick **pEl)
/* Free a single dynamically allocated cdsPick such as created
 * with cdsPickLoad(). */
{
struct cdsPick *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->source);
freeMem(el->swissProt);
freeMem(el->uniProt);
freeMem(el->refProt);
freeMem(el->refSeq);
freez(pEl);
}

void cdsPickFreeList(struct cdsPick **pList)
/* Free a list of dynamically allocated cdsPick's */
{
struct cdsPick *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cdsPickFree(&el);
    }
*pList = NULL;
}

void cdsPickOutput(struct cdsPick *el, FILE *f, char sep, char lastSep) 
/* Print out cdsPick.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->source);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->score);
fputc(sep,f);
fprintf(f, "%u", el->startComplete);
fputc(sep,f);
fprintf(f, "%u", el->endComplete);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->swissProt);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->uniProt);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refProt);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refSeq);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

