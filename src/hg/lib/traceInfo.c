/* traceInfo.c was originally generated by the autoSql program, which also 
 * generated traceInfo.h and traceInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "traceInfo.h"

static char const rcsid[] = "$Id: traceInfo.c,v 1.2 2003/05/06 07:22:23 kate Exp $";

void traceInfoStaticLoad(char **row, struct traceInfo *ret)
/* Load a row from traceInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->ti = row[0];
ret->templateId = row[1];
ret->size = sqlUnsigned(row[2]);
}

struct traceInfo *traceInfoLoad(char **row)
/* Load a traceInfo from row fetched with select * from traceInfo
 * from database.  Dispose of this with traceInfoFree(). */
{
struct traceInfo *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->ti = cloneString(row[0]);
ret->templateId = cloneString(row[1]);
ret->size = sqlUnsigned(row[2]);
return ret;
}

struct traceInfo *traceInfoLoadAll(char *fileName) 
/* Load all traceInfo from a tab-separated file.
 * Dispose of this with traceInfoFreeList(). */
{
struct traceInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = traceInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct traceInfo *traceInfoCommaIn(char **pS, struct traceInfo *ret)
/* Create a traceInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new traceInfo */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->ti = sqlStringComma(&s);
ret->templateId = sqlStringComma(&s);
ret->size = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void traceInfoFree(struct traceInfo **pEl)
/* Free a single dynamically allocated traceInfo such as created
 * with traceInfoLoad(). */
{
struct traceInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->ti);
freeMem(el->templateId);
freez(pEl);
}

void traceInfoFreeList(struct traceInfo **pList)
/* Free a list of dynamically allocated traceInfo's */
{
struct traceInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    traceInfoFree(&el);
    }
*pList = NULL;
}

void traceInfoOutput(struct traceInfo *el, FILE *f, char sep, char lastSep) 
/* Print out traceInfo.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ti);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->templateId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->size);
fputc(lastSep,f);
}

