/* dnaMotifSql.c was originally generated by the autoSql program, which also 
 * generated dnaMotifSql.h and dnaMotifSql.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "dnaMotif.h"
#include "dnaMotifSql.h"
#include "dystring.h"

struct dnaMotif *dnaMotifLoad(char **row)
/* Load a dnaMotif from row fetched with select * from dnaMotif
 * from database.  Dispose of this with dnaMotifFree(). */
{
struct dnaMotif *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->columnCount = sqlSigned(row[1]);
ret->name = cloneString(row[0]);
sqlFloatDynamicArray(row[2], &ret->aProb, &sizeOne);
assert(sizeOne == ret->columnCount);
sqlFloatDynamicArray(row[3], &ret->cProb, &sizeOne);
assert(sizeOne == ret->columnCount);
sqlFloatDynamicArray(row[4], &ret->gProb, &sizeOne);
assert(sizeOne == ret->columnCount);
sqlFloatDynamicArray(row[5], &ret->tProb, &sizeOne);
assert(sizeOne == ret->columnCount);
return ret;
}

struct dnaMotif *dnaMotifLoadAll(char *fileName) 
/* Load all dnaMotif from a tab-separated file.
 * Dispose of this with dnaMotifFreeList(). */
{
struct dnaMotif *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = dnaMotifLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct dnaMotif *dnaMotifLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all dnaMotif from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with dnaMotifFreeList(). */
{
struct dnaMotif *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = dnaMotifLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

