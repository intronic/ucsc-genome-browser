/* growthCondition.c was originally generated by the autoSql program, which also 
 * generated growthCondition.h and growthCondition.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "growthCondition.h"

static char const rcsid[] = "$Id: growthCondition.c,v 1.1 2004/10/06 20:45:40 kent Exp $";

void growthConditionsStaticLoad(char **row, struct growthConditions *ret)
/* Load a row from growthConditions table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->shortLabel = row[1];
ret->longLabel = row[2];
}

struct growthConditions *growthConditionsLoad(char **row)
/* Load a growthConditions from row fetched with select * from growthConditions
 * from database.  Dispose of this with growthConditionsFree(). */
{
struct growthConditions *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->shortLabel = cloneString(row[1]);
ret->longLabel = cloneString(row[2]);
return ret;
}

struct growthConditions *growthConditionsLoadAll(char *fileName) 
/* Load all growthConditions from a whitespace-separated file.
 * Dispose of this with growthConditionsFreeList(). */
{
struct growthConditions *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = growthConditionsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct growthConditions *growthConditionsLoadAllByChar(char *fileName, char chopper) 
/* Load all growthConditions from a chopper separated file.
 * Dispose of this with growthConditionsFreeList(). */
{
struct growthConditions *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = growthConditionsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct growthConditions *growthConditionsCommaIn(char **pS, struct growthConditions *ret)
/* Create a growthConditions out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new growthConditions */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->shortLabel = sqlStringComma(&s);
ret->longLabel = sqlStringComma(&s);
*pS = s;
return ret;
}

void growthConditionsFree(struct growthConditions **pEl)
/* Free a single dynamically allocated growthConditions such as created
 * with growthConditionsLoad(). */
{
struct growthConditions *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->shortLabel);
freeMem(el->longLabel);
freez(pEl);
}

void growthConditionsFreeList(struct growthConditions **pList)
/* Free a list of dynamically allocated growthConditions's */
{
struct growthConditions *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    growthConditionsFree(&el);
    }
*pList = NULL;
}

void growthConditionsOutput(struct growthConditions *el, FILE *f, char sep, char lastSep) 
/* Print out growthConditions.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shortLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->longLabel);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

