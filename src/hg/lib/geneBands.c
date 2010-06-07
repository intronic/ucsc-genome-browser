/* geneBands.c was originally generated by the autoSql program, which also 
 * generated geneBands.h and geneBands.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "geneBands.h"

static char const rcsid[] = "$Id: geneBands.c,v 1.3 2005/04/13 06:25:53 markd Exp $";

struct geneBands *geneBandsLoad(char **row)
/* Load a geneBands from row fetched with select * from geneBands
 * from database.  Dispose of this with geneBandsFree(). */
{
struct geneBands *ret;
int sizeOne;

AllocVar(ret);
ret->count = sqlSigned(row[2]);
ret->name = cloneString(row[0]);
ret->mrnaAcc = cloneString(row[1]);
sqlStringDynamicArray(row[3], &ret->bands, &sizeOne);
assert(sizeOne == ret->count);
return ret;
}

struct geneBands *geneBandsLoadAll(char *fileName) 
/* Load all geneBands from a tab-separated file.
 * Dispose of this with geneBandsFreeList(). */
{
struct geneBands *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = geneBandsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct geneBands *geneBandsLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all geneBands from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with geneBandsFreeList(). */
{
struct geneBands *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = geneBandsLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct geneBands *geneBandsCommaIn(char **pS, struct geneBands *ret)
/* Create a geneBands out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new geneBands */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->mrnaAcc = sqlStringComma(&s);
ret->count = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->bands, ret->count);
for (i=0; i<ret->count; ++i)
    {
    ret->bands[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void geneBandsFree(struct geneBands **pEl)
/* Free a single dynamically allocated geneBands such as created
 * with geneBandsLoad(). */
{
struct geneBands *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->mrnaAcc);
/* All strings in bands are allocated at once, so only need to free first. */
if (el->bands != NULL)
    freeMem(el->bands[0]);
freeMem(el->bands);
freez(pEl);
}

void geneBandsFreeList(struct geneBands **pList)
/* Free a list of dynamically allocated geneBands's */
{
struct geneBands *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    geneBandsFree(&el);
    }
*pList = NULL;
}

void geneBandsOutput(struct geneBands *el, FILE *f, char sep, char lastSep) 
/* Print out geneBands.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mrnaAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->count);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->count; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->bands[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

