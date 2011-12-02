/* taxonName.c was originally generated by the autoSql program, which also 
 * generated taxonName.h and taxonName.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "taxonName.h"


void taxonNameStaticLoad(char **row, struct taxonName *ret)
/* Load a row from taxonName table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->taxon = sqlUnsigned(row[1]);
ret->class = row[2];
}

struct taxonName *taxonNameLoad(char **row)
/* Load a taxonName from row fetched with select * from taxonName
 * from database.  Dispose of this with taxonNameFree(). */
{
struct taxonName *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->taxon = sqlUnsigned(row[1]);
ret->class = cloneString(row[2]);
return ret;
}

struct taxonName *taxonNameLoadAll(char *fileName) 
/* Load all taxonName from a whitespace-separated file.
 * Dispose of this with taxonNameFreeList(). */
{
struct taxonName *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = taxonNameLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct taxonName *taxonNameLoadAllByChar(char *fileName, char chopper) 
/* Load all taxonName from a chopper separated file.
 * Dispose of this with taxonNameFreeList(). */
{
struct taxonName *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = taxonNameLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct taxonName *taxonNameLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all taxonName from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with taxonNameFreeList(). */
{
struct taxonName *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = taxonNameLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void taxonNameSaveToDb(struct sqlConnection *conn, struct taxonName *el, char *tableName, int updateSize)
/* Save taxonName as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use taxonNameSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,'%s')", 
	tableName,  el->name,  el->taxon,  el->class);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void taxonNameSaveToDbEscaped(struct sqlConnection *conn, struct taxonName *el, char *tableName, int updateSize)
/* Save taxonName as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than taxonNameSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *name, *class;
name = sqlEscapeString(el->name);
class = sqlEscapeString(el->class);

dyStringPrintf(update, "insert into %s values ( '%s',%u,'%s')", 
	tableName,  name, el->taxon ,  class);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&name);
freez(&class);
}

struct taxonName *taxonNameCommaIn(char **pS, struct taxonName *ret)
/* Create a taxonName out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new taxonName */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->taxon = sqlUnsignedComma(&s);
ret->class = sqlStringComma(&s);
*pS = s;
return ret;
}

void taxonNameFree(struct taxonName **pEl)
/* Free a single dynamically allocated taxonName such as created
 * with taxonNameLoad(). */
{
struct taxonName *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->class);
freez(pEl);
}

void taxonNameFreeList(struct taxonName **pList)
/* Free a list of dynamically allocated taxonName's */
{
struct taxonName *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    taxonNameFree(&el);
    }
*pList = NULL;
}

void taxonNameOutput(struct taxonName *el, FILE *f, char sep, char lastSep) 
/* Print out taxonName.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->taxon);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->class);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

