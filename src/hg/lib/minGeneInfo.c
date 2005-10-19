/* minGeneInfo.c was originally generated by the autoSql program, which also 
 * generated minGeneInfo.h and minGeneInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "minGeneInfo.h"

static char const rcsid[] = "$Id: minGeneInfo.c,v 1.6 2005/10/19 01:10:13 baertsch Exp $";

void minGeneInfoStaticLoad(char **row, struct minGeneInfo *ret)
/* Load a row from minGeneInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->gene = row[1];
ret->product = row[2];
ret->note = row[3];
ret->protein = row[4];
ret->gi = row[5];
ret->ec = row[6];
ret->entrezGene = row[7];
}

struct minGeneInfo *minGeneInfoLoad(char **row)
/* Load a minGeneInfo from row fetched with select * from minGeneInfo
 * from database.  Dispose of this with minGeneInfoFree(). */
{
struct minGeneInfo *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->gene = cloneString(row[1]);
ret->product = cloneString(row[2]);
ret->note = cloneString(row[3]);
ret->protein = cloneString(row[4]);
ret->gi = cloneString(row[5]);
ret->ec = cloneString(row[6]);
ret->entrezGene = cloneString(row[7]);
return ret;
}

struct minGeneInfo *minGeneInfoLoadAll(char *fileName) 
/* Load all minGeneInfo from a whitespace-separated file.
 * Dispose of this with minGeneInfoFreeList(). */
{
struct minGeneInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = minGeneInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct minGeneInfo *minGeneInfoLoadAllByChar(char *fileName, char chopper) 
/* Load all minGeneInfo from a chopper separated file.
 * Dispose of this with minGeneInfoFreeList(). */
{
struct minGeneInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = minGeneInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct minGeneInfo *minGeneInfoCommaIn(char **pS, struct minGeneInfo *ret)
/* Create a minGeneInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new minGeneInfo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->gene = sqlStringComma(&s);
ret->product = sqlStringComma(&s);
ret->note = sqlStringComma(&s);
ret->protein = sqlStringComma(&s);
ret->gi = sqlStringComma(&s);
ret->ec = sqlStringComma(&s);
ret->entrezGene = sqlStringComma(&s);
*pS = s;
return ret;
}

void minGeneInfoFree(struct minGeneInfo **pEl)
/* Free a single dynamically allocated minGeneInfo such as created
 * with minGeneInfoLoad(). */
{
struct minGeneInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->gene);
freeMem(el->product);
freeMem(el->note);
freeMem(el->protein);
freeMem(el->gi);
freeMem(el->ec);
freeMem(el->entrezGene);
freez(pEl);
}

void minGeneInfoFreeList(struct minGeneInfo **pList)
/* Free a list of dynamically allocated minGeneInfo's */
{
struct minGeneInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    minGeneInfoFree(&el);
    }
*pList = NULL;
}

void minGeneInfoOutput(struct minGeneInfo *el, FILE *f, char sep, char lastSep) 
/* Print out minGeneInfo.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gene);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->product);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->note);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->protein);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gi);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ec);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->entrezGene);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

