/* itemConf.c was originally generated by the autoSql program, which also 
 * generated itemConf.h and itemConf.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "itemConf.h"

static char const rcsid[] = "$Id: itemConf.c,v 1.1 2007/09/22 23:53:30 hartera Exp $";

void itemConfStaticLoad(char **row, struct itemConf *ret)
/* Load a row from itemConf table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->id = row[0];
ret->probability = sqlFloat(row[1]);
ret->fdr = sqlFloat(row[2]);
}

struct itemConf *itemConfLoad(char **row)
/* Load a itemConf from row fetched with select * from itemConf
 * from database.  Dispose of this with itemConfFree(). */
{
struct itemConf *ret;

AllocVar(ret);
ret->id = cloneString(row[0]);
ret->probability = sqlFloat(row[1]);
ret->fdr = sqlFloat(row[2]);
return ret;
}

struct itemConf *itemConfLoadAll(char *fileName) 
/* Load all itemConf from a whitespace-separated file.
 * Dispose of this with itemConfFreeList(). */
{
struct itemConf *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = itemConfLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct itemConf *itemConfLoadAllByChar(char *fileName, char chopper) 
/* Load all itemConf from a chopper separated file.
 * Dispose of this with itemConfFreeList(). */
{
struct itemConf *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = itemConfLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct itemConf *itemConfCommaIn(char **pS, struct itemConf *ret)
/* Create a itemConf out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new itemConf */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlStringComma(&s);
ret->probability = sqlFloatComma(&s);
ret->fdr = sqlFloatComma(&s);
*pS = s;
return ret;
}

void itemConfFree(struct itemConf **pEl)
/* Free a single dynamically allocated itemConf such as created
 * with itemConfLoad(). */
{
struct itemConf *el;

if ((el = *pEl) == NULL) return;
freeMem(el->id);
freez(pEl);
}

void itemConfFreeList(struct itemConf **pList)
/* Free a list of dynamically allocated itemConf's */
{
struct itemConf *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    itemConfFree(&el);
    }
*pList = NULL;
}

void itemConfOutput(struct itemConf *el, FILE *f, char sep, char lastSep) 
/* Print out itemConf.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->probability);
fputc(sep,f);
fprintf(f, "%g", el->fdr);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

