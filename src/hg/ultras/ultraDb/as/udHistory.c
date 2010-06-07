/* udHistory.c was originally generated by the autoSql program, which also 
 * generated udHistory.h and udHistory.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "udHistory.h"

static char const rcsid[] = "$Id: udHistory.c,v 1.1 2004/10/06 17:29:46 kent Exp $";

void udHistoryStaticLoad(char **row, struct udHistory *ret)
/* Load a row from udHistory table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->who = row[0];
ret->program = row[1];
ret->time = row[2];
ret->changes = row[3];
}

struct udHistory *udHistoryLoad(char **row)
/* Load a udHistory from row fetched with select * from udHistory
 * from database.  Dispose of this with udHistoryFree(). */
{
struct udHistory *ret;

AllocVar(ret);
ret->who = cloneString(row[0]);
ret->program = cloneString(row[1]);
ret->time = cloneString(row[2]);
ret->changes = cloneString(row[3]);
return ret;
}

struct udHistory *udHistoryLoadAll(char *fileName) 
/* Load all udHistory from a whitespace-separated file.
 * Dispose of this with udHistoryFreeList(). */
{
struct udHistory *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = udHistoryLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct udHistory *udHistoryLoadAllByChar(char *fileName, char chopper) 
/* Load all udHistory from a chopper separated file.
 * Dispose of this with udHistoryFreeList(). */
{
struct udHistory *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = udHistoryLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct udHistory *udHistoryCommaIn(char **pS, struct udHistory *ret)
/* Create a udHistory out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new udHistory */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->who = sqlStringComma(&s);
ret->program = sqlStringComma(&s);
ret->time = sqlStringComma(&s);
ret->changes = sqlStringComma(&s);
*pS = s;
return ret;
}

void udHistoryFree(struct udHistory **pEl)
/* Free a single dynamically allocated udHistory such as created
 * with udHistoryLoad(). */
{
struct udHistory *el;

if ((el = *pEl) == NULL) return;
freeMem(el->who);
freeMem(el->program);
freeMem(el->time);
freeMem(el->changes);
freez(pEl);
}

void udHistoryFreeList(struct udHistory **pList)
/* Free a list of dynamically allocated udHistory's */
{
struct udHistory *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    udHistoryFree(&el);
    }
*pList = NULL;
}

void udHistoryOutput(struct udHistory *el, FILE *f, char sep, char lastSep) 
/* Print out udHistory.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->who);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->program);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->time);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->changes);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

