/* acemblyClass.c was originally generated by the autoSql program, which also 
 * generated acemblyClass.h and acemblyClass.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "acemblyClass.h"


void acemblyClassStaticLoad(char **row, struct acemblyClass *ret)
/* Load a row from acemblyClass table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->class = row[1];
}

struct acemblyClass *acemblyClassLoad(char **row)
/* Load a acemblyClass from row fetched with select * from acemblyClass
 * from database.  Dispose of this with acemblyClassFree(). */
{
struct acemblyClass *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->class = cloneString(row[1]);
return ret;
}

struct acemblyClass *acemblyClassLoadAll(char *fileName) 
/* Load all acemblyClass from a whitespace-separated file.
 * Dispose of this with acemblyClassFreeList(). */
{
struct acemblyClass *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = acemblyClassLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct acemblyClass *acemblyClassLoadAllByChar(char *fileName, char chopper) 
/* Load all acemblyClass from a chopper separated file.
 * Dispose of this with acemblyClassFreeList(). */
{
struct acemblyClass *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = acemblyClassLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct acemblyClass *acemblyClassCommaIn(char **pS, struct acemblyClass *ret)
/* Create a acemblyClass out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new acemblyClass */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->class = sqlStringComma(&s);
*pS = s;
return ret;
}

void acemblyClassFree(struct acemblyClass **pEl)
/* Free a single dynamically allocated acemblyClass such as created
 * with acemblyClassLoad(). */
{
struct acemblyClass *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->class);
freez(pEl);
}

void acemblyClassFreeList(struct acemblyClass **pList)
/* Free a list of dynamically allocated acemblyClass's */
{
struct acemblyClass *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    acemblyClassFree(&el);
    }
*pList = NULL;
}

void acemblyClassOutput(struct acemblyClass *el, FILE *f, char sep, char lastSep) 
/* Print out acemblyClass.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->class);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

