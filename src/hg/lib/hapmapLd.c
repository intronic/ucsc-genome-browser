/* hapmapLd.c was originally generated by the autoSql program, which also 
 * generated hapmapLd.h and hapmapLd.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hapmapLd.h"

static char const rcsid[] = "$Id: hapmapLd.c,v 1.1 2005/12/22 00:22:37 daryl Exp $";

void hapmapLdStaticLoad(char **row, struct hapmapLd *ret)
/* Load a row from hapmapLd table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->dprime = row[5];
ret->rsquared = row[6];
ret->lod = row[7];
}

struct hapmapLd *hapmapLdLoad(char **row)
/* Load a hapmapLd from row fetched with select * from hapmapLd
 * from database.  Dispose of this with hapmapLdFree(). */
{
struct hapmapLd *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->dprime = cloneString(row[5]);
ret->rsquared = cloneString(row[6]);
ret->lod = cloneString(row[7]);
return ret;
}

struct hapmapLd *hapmapLdLoadAll(char *fileName) 
/* Load all hapmapLd from a whitespace-separated file.
 * Dispose of this with hapmapLdFreeList(). */
{
struct hapmapLd *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = hapmapLdLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapLd *hapmapLdLoadAllByChar(char *fileName, char chopper) 
/* Load all hapmapLd from a chopper separated file.
 * Dispose of this with hapmapLdFreeList(). */
{
struct hapmapLd *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hapmapLdLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapLd *hapmapLdLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all hapmapLd from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hapmapLdFreeList(). */
{
struct hapmapLd *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = hapmapLdLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void hapmapLdSaveToDb(struct sqlConnection *conn, struct hapmapLd *el, char *tableName, int updateSize)
/* Save hapmapLd as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hapmapLdSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s')", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->dprime,  el->rsquared,  el->lod);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void hapmapLdSaveToDbEscaped(struct sqlConnection *conn, struct hapmapLd *el, char *tableName, int updateSize)
/* Save hapmapLd as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hapmapLdSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *dprime, *rsquared, *lod;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
dprime = sqlEscapeString(el->dprime);
rsquared = sqlEscapeString(el->rsquared);
lod = sqlEscapeString(el->lod);

dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s')", 
	tableName,  chrom, el->chromStart , el->chromEnd ,  name, el->score ,  dprime,  rsquared,  lod);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&dprime);
freez(&rsquared);
freez(&lod);
}

struct hapmapLd *hapmapLdCommaIn(char **pS, struct hapmapLd *ret)
/* Create a hapmapLd out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hapmapLd */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
ret->dprime = sqlStringComma(&s);
ret->rsquared = sqlStringComma(&s);
ret->lod = sqlStringComma(&s);
*pS = s;
return ret;
}

void hapmapLdFree(struct hapmapLd **pEl)
/* Free a single dynamically allocated hapmapLd such as created
 * with hapmapLdLoad(). */
{
struct hapmapLd *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->dprime);
freeMem(el->rsquared);
freeMem(el->lod);
freez(pEl);
}

void hapmapLdFreeList(struct hapmapLd **pList)
/* Free a list of dynamically allocated hapmapLd's */
{
struct hapmapLd *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hapmapLdFree(&el);
    }
*pList = NULL;
}

void hapmapLdOutput(struct hapmapLd *el, FILE *f, char sep, char lastSep) 
/* Print out hapmapLd.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dprime);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rsquared);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->lod);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

