/* cartDb.c was originally generated by the autoSql program, which also 
 * generated cartDb.h and cartDb.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "cartDb.h"

void cartDbStaticLoad(char **row, struct cartDb *ret)
/* Load a row from cartDb table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->id = row[0];
ret->contents = row[1];
}

struct cartDb *cartDbLoad(char **row)
/* Load a cartDb from row fetched with select * from cartDb
 * from database.  Dispose of this with cartDbFree(). */
{
struct cartDb *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->id = cloneString(row[0]);
ret->contents = cloneString(row[1]);
return ret;
}

struct cartDb *cartDbLoadAll(char *fileName) 
/* Load all cartDb from a tab-separated file.
 * Dispose of this with cartDbFreeList(). */
{
struct cartDb *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = cartDbLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cartDb *cartDbLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all cartDb from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with cartDbFreeList(). */
{
struct cartDb *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = cartDbLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct cartDb *cartDbCommaIn(char **pS, struct cartDb *ret)
/* Create a cartDb out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cartDb */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlStringComma(&s);
ret->contents = sqlStringComma(&s);
*pS = s;
return ret;
}

void cartDbFree(struct cartDb **pEl)
/* Free a single dynamically allocated cartDb such as created
 * with cartDbLoad(). */
{
struct cartDb *el;

if ((el = *pEl) == NULL) return;
freeMem(el->id);
freeMem(el->contents);
freez(pEl);
}

void cartDbFreeList(struct cartDb **pList)
/* Free a list of dynamically allocated cartDb's */
{
struct cartDb *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cartDbFree(&el);
    }
*pList = NULL;
}

void cartDbOutput(struct cartDb *el, FILE *f, char sep, char lastSep) 
/* Print out cartDb.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->contents);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

