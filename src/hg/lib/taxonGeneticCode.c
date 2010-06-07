/* taxonGeneticCode.c was originally generated by the autoSql program, which also 
 * generated taxonGeneticCode.h and taxonGeneticCode.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "taxonGeneticCode.h"

static char const rcsid[] = "$Id: taxonGeneticCode.c,v 1.1 2006/04/11 21:43:09 baertsch Exp $";

void taxonGeneticCodeStaticLoad(char **row, struct taxonGeneticCode *ret)
/* Load a row from taxonGeneticCode table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->id = sqlUnsigned(row[0]);
ret->abbr = row[1];
ret->name = row[2];
ret->tranlsation = row[3];
ret->starts = row[4];
ret->comments = row[5];
}

struct taxonGeneticCode *taxonGeneticCodeLoad(char **row)
/* Load a taxonGeneticCode from row fetched with select * from taxonGeneticCode
 * from database.  Dispose of this with taxonGeneticCodeFree(). */
{
struct taxonGeneticCode *ret;

AllocVar(ret);
ret->id = sqlUnsigned(row[0]);
ret->abbr = cloneString(row[1]);
ret->name = cloneString(row[2]);
ret->tranlsation = cloneString(row[3]);
ret->starts = cloneString(row[4]);
ret->comments = cloneString(row[5]);
return ret;
}

struct taxonGeneticCode *taxonGeneticCodeLoadAll(char *fileName) 
/* Load all taxonGeneticCode from a whitespace-separated file.
 * Dispose of this with taxonGeneticCodeFreeList(). */
{
struct taxonGeneticCode *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = taxonGeneticCodeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct taxonGeneticCode *taxonGeneticCodeLoadAllByChar(char *fileName, char chopper) 
/* Load all taxonGeneticCode from a chopper separated file.
 * Dispose of this with taxonGeneticCodeFreeList(). */
{
struct taxonGeneticCode *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = taxonGeneticCodeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct taxonGeneticCode *taxonGeneticCodeLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all taxonGeneticCode from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with taxonGeneticCodeFreeList(). */
{
struct taxonGeneticCode *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = taxonGeneticCodeLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void taxonGeneticCodeSaveToDb(struct sqlConnection *conn, struct taxonGeneticCode *el, char *tableName, int updateSize)
/* Save taxonGeneticCode as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use taxonGeneticCodeSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( %u,'%s','%s','%s','%s','%s')", 
	tableName,  el->id,  el->abbr,  el->name,  el->tranlsation,  el->starts,  el->comments);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void taxonGeneticCodeSaveToDbEscaped(struct sqlConnection *conn, struct taxonGeneticCode *el, char *tableName, int updateSize)
/* Save taxonGeneticCode as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than taxonGeneticCodeSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *abbr, *name, *tranlsation, *starts, *comments;
abbr = sqlEscapeString(el->abbr);
name = sqlEscapeString(el->name);
tranlsation = sqlEscapeString(el->tranlsation);
starts = sqlEscapeString(el->starts);
comments = sqlEscapeString(el->comments);

dyStringPrintf(update, "insert into %s values ( %u,'%s','%s','%s','%s','%s')", 
	tableName, el->id ,  abbr,  name,  tranlsation,  starts,  comments);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&abbr);
freez(&name);
freez(&tranlsation);
freez(&starts);
freez(&comments);
}

struct taxonGeneticCode *taxonGeneticCodeCommaIn(char **pS, struct taxonGeneticCode *ret)
/* Create a taxonGeneticCode out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new taxonGeneticCode */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
ret->abbr = sqlStringComma(&s);
ret->name = sqlStringComma(&s);
ret->tranlsation = sqlStringComma(&s);
ret->starts = sqlStringComma(&s);
ret->comments = sqlStringComma(&s);
*pS = s;
return ret;
}

void taxonGeneticCodeFree(struct taxonGeneticCode **pEl)
/* Free a single dynamically allocated taxonGeneticCode such as created
 * with taxonGeneticCodeLoad(). */
{
struct taxonGeneticCode *el;

if ((el = *pEl) == NULL) return;
freeMem(el->abbr);
freeMem(el->name);
freeMem(el->tranlsation);
freeMem(el->starts);
freeMem(el->comments);
freez(pEl);
}

void taxonGeneticCodeFreeList(struct taxonGeneticCode **pList)
/* Free a list of dynamically allocated taxonGeneticCode's */
{
struct taxonGeneticCode *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    taxonGeneticCodeFree(&el);
    }
*pList = NULL;
}

void taxonGeneticCodeOutput(struct taxonGeneticCode *el, FILE *f, char sep, char lastSep) 
/* Print out taxonGeneticCode.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->abbr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tranlsation);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->starts);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->comments);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

