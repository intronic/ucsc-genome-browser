/* ancientRref.c was originally generated by the autoSql program, which also 
 * generated ancientRref.h and ancientRref.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "ancientRref.h"

void ancientRrefStaticLoad(char **row, struct ancientRref *ret)
/* Load a row from ancientRref table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->id = row[0];
ret->hseq = row[1];
ret->mseq = row[2];
}

struct ancientRref *ancientRrefLoad(char **row)
/* Load a ancientRref from row fetched with select * from ancientRref
 * from database.  Dispose of this with ancientRrefFree(). */
{
struct ancientRref *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->id = cloneString(row[0]);
ret->hseq = cloneString(row[1]);
ret->mseq = cloneString(row[2]);
return ret;
}

struct ancientRref *ancientRrefLoadAll(char *fileName) 
/* Load all ancientRref from a tab-separated file.
 * Dispose of this with ancientRrefFreeList(). */
{
struct ancientRref *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = ancientRrefLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ancientRref *ancientRrefCommaIn(char **pS, struct ancientRref *ret)
/* Create a ancientRref out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ancientRref */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlStringComma(&s);
ret->hseq = sqlStringComma(&s);
ret->mseq = sqlStringComma(&s);
*pS = s;
return ret;
}

void ancientRrefFree(struct ancientRref **pEl)
/* Free a single dynamically allocated ancientRref such as created
 * with ancientRrefLoad(). */
{
struct ancientRref *el;

if ((el = *pEl) == NULL) return;
freeMem(el->id);
freeMem(el->hseq);
freeMem(el->mseq);
freez(pEl);
}

void ancientRrefFreeList(struct ancientRref **pList)
/* Free a list of dynamically allocated ancientRref's */
{
struct ancientRref *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ancientRrefFree(&el);
    }
*pList = NULL;
}

void ancientRrefOutput(struct ancientRref *el, FILE *f, char sep, char lastSep) 
/* Print out ancientRref.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hseq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mseq);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

