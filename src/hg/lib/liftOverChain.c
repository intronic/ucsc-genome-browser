/* liftOverChain.c was originally generated by the autoSql program, which also 
 * generated liftOverChain.h and liftOverChain.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "liftOverChain.h"

static char const rcsid[] = "$Id: liftOverChain.c,v 1.3 2005/11/21 09:21:21 aamp Exp $";

void liftOverChainStaticLoad(char **row, struct liftOverChain *ret)
/* Load a row from liftOverChain table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->fromDb = row[0];
ret->toDb = row[1];
ret->path = row[2];
ret->minMatch = atof(row[3]);
ret->minSizeT = sqlUnsigned(row[4]);
ret->minSizeQ = sqlUnsigned(row[5]);
ret->minChainT = sqlUnsigned(row[6]);
ret->minChainQ = sqlUnsigned(row[7]);
strcpy(ret->multiple, row[8]);
ret->minBlocks = atof(row[9]);
strcpy(ret->fudgeThick, row[10]);
}

struct liftOverChain *liftOverChainLoad(char **row)
/* Load a liftOverChain from row fetched with select * from liftOverChain
 * from database.  Dispose of this with liftOverChainFree(). */
{
struct liftOverChain *ret;

AllocVar(ret);
ret->fromDb = cloneString(row[0]);
ret->toDb = cloneString(row[1]);
ret->path = cloneString(row[2]);
ret->minMatch = atof(row[3]);
ret->minSizeT = sqlUnsigned(row[4]);
ret->minSizeQ = sqlUnsigned(row[5]);
ret->minChainT = sqlUnsigned(row[6]);
ret->minChainQ = sqlUnsigned(row[7]);
strcpy(ret->multiple, row[8]);
ret->minBlocks = atof(row[9]);
strcpy(ret->fudgeThick, row[10]);
return ret;
}

struct liftOverChain *liftOverChainLoadAll(char *fileName) 
/* Load all liftOverChain from a whitespace-separated file.
 * Dispose of this with liftOverChainFreeList(). */
{
struct liftOverChain *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = liftOverChainLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct liftOverChain *liftOverChainLoadAllByChar(char *fileName, char chopper) 
/* Load all liftOverChain from a chopper separated file.
 * Dispose of this with liftOverChainFreeList(). */
{
struct liftOverChain *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = liftOverChainLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct liftOverChain *liftOverChainLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all liftOverChain from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with liftOverChainFreeList(). */
{
struct liftOverChain *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = liftOverChainLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void liftOverChainSaveToDb(struct sqlConnection *conn, struct liftOverChain *el, char *tableName, int updateSize)
/* Save liftOverChain as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use liftOverChainSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s','%s',%s,%g,%u,%u,%u,%u,'%s',%g,'%s')", 
	tableName,  el->fromDb,  el->toDb,  el->path,  el->minMatch,  el->minSizeT,  el->minSizeQ,  el->minChainT,  el->minChainQ,  el->multiple,  el->minBlocks,  el->fudgeThick);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void liftOverChainSaveToDbEscaped(struct sqlConnection *conn, struct liftOverChain *el, char *tableName, int updateSize)
/* Save liftOverChain as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than liftOverChainSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *fromDb, *toDb, *path, *multiple, *fudgeThick;
fromDb = sqlEscapeString(el->fromDb);
toDb = sqlEscapeString(el->toDb);
path = sqlEscapeString(el->path);
multiple = sqlEscapeString(el->multiple);
fudgeThick = sqlEscapeString(el->fudgeThick);

dyStringPrintf(update, "insert into %s values ( '%s','%s','%s',%g,%u,%u,%u,%u,'%s',%g,'%s')", 
	tableName,  fromDb,  toDb,  path, el->minMatch , el->minSizeT , el->minSizeQ , el->minChainT , el->minChainQ ,  multiple, el->minBlocks ,  fudgeThick);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&fromDb);
freez(&toDb);
freez(&path);
freez(&multiple);
freez(&fudgeThick);
}

struct liftOverChain *liftOverChainCommaIn(char **pS, struct liftOverChain *ret)
/* Create a liftOverChain out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new liftOverChain */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->fromDb = sqlStringComma(&s);
ret->toDb = sqlStringComma(&s);
ret->path = sqlStringComma(&s);
ret->minMatch = sqlFloatComma(&s);
ret->minSizeT = sqlUnsignedComma(&s);
ret->minSizeQ = sqlUnsignedComma(&s);
ret->minChainT = sqlUnsignedComma(&s);
ret->minChainQ = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->multiple, sizeof(ret->multiple));
ret->minBlocks = sqlFloatComma(&s);
sqlFixedStringComma(&s, ret->fudgeThick, sizeof(ret->fudgeThick));
*pS = s;
return ret;
}

void liftOverChainFree(struct liftOverChain **pEl)
/* Free a single dynamically allocated liftOverChain such as created
 * with liftOverChainLoad(). */
{
struct liftOverChain *el;

if ((el = *pEl) == NULL) return;
freeMem(el->fromDb);
freeMem(el->toDb);
freeMem(el->path);
freez(pEl);
}

void liftOverChainFreeList(struct liftOverChain **pList)
/* Free a list of dynamically allocated liftOverChain's */
{
struct liftOverChain *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    liftOverChainFree(&el);
    }
*pList = NULL;
}

void liftOverChainOutput(struct liftOverChain *el, FILE *f, char sep, char lastSep) 
/* Print out liftOverChain.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fromDb);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->toDb);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->path);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->minMatch);
fputc(sep,f);
fprintf(f, "%u", el->minSizeT);
fputc(sep,f);
fprintf(f, "%u", el->minSizeQ);
fputc(sep,f);
fprintf(f, "%u", el->minChainT);
fputc(sep,f);
fprintf(f, "%u", el->minChainQ);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->multiple);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->minBlocks);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fudgeThick);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

