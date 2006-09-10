/* rnaLpFold.c was originally generated by the autoSql program, which also 
 * generated rnaLpFold.h and rnaLpFold.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rnaLpFold.h"

static char const rcsid[] = "$Id: rnaLpFold.c,v 1.1 2006/09/10 05:11:16 daryl Exp $";

void rnaLpFoldStaticLoad(char **row, struct rnaLpFold *ret)
/* Load a row from rnaLpFold table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->score = sqlUnsigned(row[3]);
ret->colorIndex = row[4];
}

struct rnaLpFold *rnaLpFoldLoad(char **row)
/* Load a rnaLpFold from row fetched with select * from rnaLpFold
 * from database.  Dispose of this with rnaLpFoldFree(). */
{
struct rnaLpFold *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->score = sqlUnsigned(row[3]);
ret->colorIndex = cloneString(row[4]);
return ret;
}

struct rnaLpFold *rnaLpFoldLoadAll(char *fileName) 
/* Load all rnaLpFold from a whitespace-separated file.
 * Dispose of this with rnaLpFoldFreeList(). */
{
struct rnaLpFold *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileRow(lf, row))
    {
    el = rnaLpFoldLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaLpFold *rnaLpFoldLoadAllByChar(char *fileName, char chopper) 
/* Load all rnaLpFold from a chopper separated file.
 * Dispose of this with rnaLpFoldFreeList(). */
{
struct rnaLpFold *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rnaLpFoldLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaLpFold *rnaLpFoldLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all rnaLpFold from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with rnaLpFoldFreeList(). */
{
struct rnaLpFold *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = rnaLpFoldLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void rnaLpFoldSaveToDb(struct sqlConnection *conn, struct rnaLpFold *el, char *tableName, int updateSize)
/* Save rnaLpFold as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use rnaLpFoldSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,%u,'%s')", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->score,  el->colorIndex);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void rnaLpFoldSaveToDbEscaped(struct sqlConnection *conn, struct rnaLpFold *el, char *tableName, int updateSize)
/* Save rnaLpFold as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than rnaLpFoldSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *colorIndex;
chrom = sqlEscapeString(el->chrom);
colorIndex = sqlEscapeString(el->colorIndex);

dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,%u,'%s')", 
	tableName,  chrom, el->chromStart , el->chromEnd , el->score ,  colorIndex);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&colorIndex);
}

struct rnaLpFold *rnaLpFoldCommaIn(char **pS, struct rnaLpFold *ret)
/* Create a rnaLpFold out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rnaLpFold */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->score = sqlUnsignedComma(&s);
ret->colorIndex = sqlStringComma(&s);
*pS = s;
return ret;
}

void rnaLpFoldFree(struct rnaLpFold **pEl)
/* Free a single dynamically allocated rnaLpFold such as created
 * with rnaLpFoldLoad(). */
{
struct rnaLpFold *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->colorIndex);
freez(pEl);
}

void rnaLpFoldFreeList(struct rnaLpFold **pList)
/* Free a list of dynamically allocated rnaLpFold's */
{
struct rnaLpFold *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rnaLpFoldFree(&el);
    }
*pList = NULL;
}

void rnaLpFoldOutput(struct rnaLpFold *el, FILE *f, char sep, char lastSep) 
/* Print out rnaLpFold.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->colorIndex);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

