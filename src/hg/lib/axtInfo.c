/* axtInfo.c was originally generated by the autoSql program, which also 
 * generated axtInfo.h and axtInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "axtInfo.h"

void axtInfoStaticLoad(char **row, struct axtInfo *ret)
/* Load a row from axtInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->species = row[0];
ret->alignment = row[1];
ret->chrom = row[2];
ret->fileName = row[3];
}

struct axtInfo *axtInfoLoad(char **row)
/* Load a axtInfo from row fetched with select * from axtInfo
 * from database.  Dispose of this with axtInfoFree(). */
{
struct axtInfo *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->species = cloneString(row[0]);
ret->alignment = cloneString(row[1]);
ret->chrom = cloneString(row[2]);
ret->fileName = cloneString(row[3]);
return ret;
}

struct axtInfo *axtInfoLoadAll(char *fileName) 
/* Load all axtInfo from a tab-separated file.
 * Dispose of this with axtInfoFreeList(). */
{
struct axtInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = axtInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct axtInfo *axtInfoLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all axtInfo from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with axtInfoFreeList(). */
{
struct axtInfo *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = axtInfoLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct axtInfo *axtInfoCommaIn(char **pS, struct axtInfo *ret)
/* Create a axtInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new axtInfo */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->species = sqlStringComma(&s);
ret->alignment = sqlStringComma(&s);
ret->chrom = sqlStringComma(&s);
ret->fileName = sqlStringComma(&s);
*pS = s;
return ret;
}

void axtInfoFree(struct axtInfo **pEl)
/* Free a single dynamically allocated axtInfo such as created
 * with axtInfoLoad(). */
{
struct axtInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->species);
freeMem(el->alignment);
freeMem(el->chrom);
freeMem(el->fileName);
freez(pEl);
}

void axtInfoFreeList(struct axtInfo **pList)
/* Free a list of dynamically allocated axtInfo's */
{
struct axtInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    axtInfoFree(&el);
    }
*pList = NULL;
}

void axtInfoOutput(struct axtInfo *el, FILE *f, char sep, char lastSep) 
/* Print out axtInfo.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->species);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alignment);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fileName);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

