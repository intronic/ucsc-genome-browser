/* dbSnpRS.c was originally generated by the autoSql program, which also 
 * generated dbSnpRS.h and dbSnpRS.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "dbSnpRS.h"

void dbSnpRSStaticLoad(char **row, struct dbSnpRS *ret)
/* Load a row from dbSnpRS table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->rsId = sqlUnsigned(row[0]);
ret->avHet = atof(row[1]);
ret->avHetSE = atof(row[2]);
ret->valid = row[3];
strcpy(ret->base1, row[4]);
strcpy(ret->base2, row[5]);
ret->assembly = row[6];
ret->alternate = row[7];
}

struct dbSnpRS *dbSnpRSLoad(char **row)
/* Load a dbSnpRS from row fetched with select * from dbSnpRS
 * from database.  Dispose of this with dbSnpRSFree(). */
{
struct dbSnpRS *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->rsId = sqlUnsigned(row[0]);
ret->avHet = atof(row[1]);
ret->avHetSE = atof(row[2]);
ret->valid = cloneString(row[3]);
strcpy(ret->base1, row[4]);
strcpy(ret->base2, row[5]);
ret->assembly = cloneString(row[6]);
ret->alternate = cloneString(row[7]);
return ret;
}

struct dbSnpRS *dbSnpRSLoadAll(char *fileName) 
/* Load all dbSnpRS from a tab-separated file.
 * Dispose of this with dbSnpRSFreeList(). */
{
struct dbSnpRS *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = dbSnpRSLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct dbSnpRS *dbSnpRSLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all dbSnpRS from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with dbSnpRSFreeList(). */
{
struct dbSnpRS *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = dbSnpRSLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void dbSnpRSSaveToDb(struct sqlConnection *conn, struct dbSnpRS *el, char *tableName, int updateSize)
/* Save dbSnpRS as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use dbSnpRSSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( %u,%f,%f,'%s','%s','%s','%s','%s')", 
	tableName,  el->rsId,  el->avHet,  el->avHetSE,  el->valid,  el->base1,  el->base2,  el->assembly,  el->alternate);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void dbSnpRSSaveToDbEscaped(struct sqlConnection *conn, struct dbSnpRS *el, char *tableName, int updateSize)
/* Save dbSnpRS as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than dbSnpRSSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *valid, *base1, *base2, *assembly, *alternate;
valid = sqlEscapeString(el->valid);
base1 = sqlEscapeString(el->base1);
base2 = sqlEscapeString(el->base2);
assembly = sqlEscapeString(el->assembly);
alternate = sqlEscapeString(el->alternate);

dyStringPrintf(update, "insert into %s values ( %u,%f,%f,'%s','%s','%s','%s','%s')", 
	tableName, el->rsId , el->avHet , el->avHetSE ,  valid,  base1,  base2,  assembly,  alternate);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&valid);
freez(&base1);
freez(&base2);
freez(&assembly);
freez(&alternate);
}

struct dbSnpRS *dbSnpRSCommaIn(char **pS, struct dbSnpRS *ret)
/* Create a dbSnpRS out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dbSnpRS */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->rsId = sqlUnsignedComma(&s);
ret->avHet = sqlFloatComma(&s);
ret->avHetSE = sqlFloatComma(&s);
ret->valid = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->base1, sizeof(ret->base1));
sqlFixedStringComma(&s, ret->base2, sizeof(ret->base2));
ret->assembly = sqlStringComma(&s);
ret->alternate = sqlStringComma(&s);
*pS = s;
return ret;
}

void dbSnpRSFree(struct dbSnpRS **pEl)
/* Free a single dynamically allocated dbSnpRS such as created
 * with dbSnpRSLoad(). */
{
struct dbSnpRS *el;

if ((el = *pEl) == NULL) return;
freeMem(el->valid);
freeMem(el->assembly);
freeMem(el->alternate);
freez(pEl);
}

void dbSnpRSFreeList(struct dbSnpRS **pList)
/* Free a list of dynamically allocated dbSnpRS's */
{
struct dbSnpRS *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    dbSnpRSFree(&el);
    }
*pList = NULL;
}

void dbSnpRSOutput(struct dbSnpRS *el, FILE *f, char sep, char lastSep) 
/* Print out dbSnpRS.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->rsId);
fputc(sep,f);
fprintf(f, "%f", el->avHet);
fputc(sep,f);
fprintf(f, "%f", el->avHetSE);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->valid);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->base1);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->base2);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->assembly);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alternate);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

