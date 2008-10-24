/* splatAli.c was originally generated by the autoSql program, which also 
 * generated splatAli.h and splatAli.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "sqlNum.h"
#include "sqlList.h"
#include "splatAli.h"

static char const rcsid[] = "$Id: splatAli.c,v 1.1 2008/10/24 22:40:20 kent Exp $";

void splatAliStaticLoad(char **row, struct splatAli *ret)
/* Load a row from splatAli table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlSigned(row[2]);
ret->alignedBases = row[3];
ret->score = sqlSigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->readName = row[6];
}

struct splatAli *splatAliLoad(char **row)
/* Load a splatAli from row fetched with select * from splatAli
 * from database.  Dispose of this with splatAliFree(). */
{
struct splatAli *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlSigned(row[2]);
ret->alignedBases = cloneString(row[3]);
ret->score = sqlSigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->readName = cloneString(row[6]);
return ret;
}

struct splatAli *splatAliLoadAll(char *fileName) 
/* Load all splatAli from a whitespace-separated file.
 * Dispose of this with splatAliFreeList(). */
{
struct splatAli *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = splatAliLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct splatAli *splatAliLoadAllByChar(char *fileName, char chopper) 
/* Load all splatAli from a chopper separated file.
 * Dispose of this with splatAliFreeList(). */
{
struct splatAli *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = splatAliLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct splatAli *splatAliCommaIn(char **pS, struct splatAli *ret)
/* Create a splatAli out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new splatAli */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlSignedComma(&s);
ret->chromEnd = sqlSignedComma(&s);
ret->alignedBases = sqlStringComma(&s);
ret->score = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->readName = sqlStringComma(&s);
*pS = s;
return ret;
}

void splatAliFree(struct splatAli **pEl)
/* Free a single dynamically allocated splatAli such as created
 * with splatAliLoad(). */
{
struct splatAli *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->alignedBases);
freeMem(el->readName);
freez(pEl);
}

void splatAliFreeList(struct splatAli **pList)
/* Free a list of dynamically allocated splatAli's */
{
struct splatAli *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    splatAliFree(&el);
    }
*pList = NULL;
}

void splatAliOutput(struct splatAli *el, FILE *f, char sep, char lastSep) 
/* Print out splatAli.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->chromStart);
fputc(sep,f);
fprintf(f, "%d", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alignedBases);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->readName);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

