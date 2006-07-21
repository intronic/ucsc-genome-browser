/* expRecord.c was originally generated by the autoSql program, which also 
 * generated expRecord.h and expRecord.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "expRecord.h"

static char const rcsid[] = "$Id: expRecord.c,v 1.5 2006/07/21 22:14:22 aamp Exp $";

struct expRecord *expRecordLoad(char **row)
/* Load a expRecord from row fetched with select * from expRecord
 * from database.  Dispose of this with expRecordFree(). */
{
struct expRecord *ret;
int sizeOne;

AllocVar(ret);
ret->numExtras = sqlUnsigned(row[6]);
ret->id = sqlUnsigned(row[0]);
ret->name = cloneString(row[1]);
ret->description = cloneString(row[2]);
ret->url = cloneString(row[3]);
ret->ref = cloneString(row[4]);
ret->credit = cloneString(row[5]);
sqlStringDynamicArray(row[7], &ret->extras, &sizeOne);
assert(sizeOne == ret->numExtras);
return ret;
}

struct expRecord *expRecordLoadAll(char *fileName) 
/* Load all expRecord from a tab-separated file.
 * Dispose of this with expRecordFreeList(). */
{
struct expRecord *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = expRecordLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct expRecord *expRecordLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all expRecord from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with expRecordFreeList(). */
{
struct expRecord *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = expRecordLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void expRecordSaveToDb(struct sqlConnection *conn, struct expRecord *el, char *tableName, int updateSize)
/* Save expRecord as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use expRecordSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
char  *extrasArray;
extrasArray = sqlStringArrayToString(el->extras, el->numExtras);
dyStringPrintf(update, "insert into %s values ( %u,'%s','%s','%s','%s','%s',%u,'%s')", 
	tableName,  el->id,  el->name,  el->description,  el->url,  el->ref,  el->credit,  el->numExtras,  extrasArray );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&extrasArray);
}

void expRecordSaveToDbEscaped(struct sqlConnection *conn, struct expRecord *el, char *tableName, int updateSize)
/* Save expRecord as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than expRecordSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *name, *description, *url, *ref, *credit, *extrasArray;
name = sqlEscapeString(el->name);
description = sqlEscapeString(el->description);
url = sqlEscapeString(el->url);
ref = sqlEscapeString(el->ref);
credit = sqlEscapeString(el->credit);

extrasArray = sqlStringArrayToString(el->extras, el->numExtras);
dyStringPrintf(update, "insert into %s values ( %u,'%s','%s','%s','%s','%s',%u,'%s')", 
	tableName, el->id ,  name,  description,  url,  ref,  credit, el->numExtras ,  extrasArray );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&name);
freez(&description);
freez(&url);
freez(&ref);
freez(&credit);
freez(&extrasArray);
}

struct expRecord *expRecordCommaIn(char **pS, struct expRecord *ret)
/* Create a expRecord out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new expRecord */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->description = sqlStringComma(&s);
ret->url = sqlStringComma(&s);
ret->ref = sqlStringComma(&s);
ret->credit = sqlStringComma(&s);
ret->numExtras = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->extras, ret->numExtras);
for (i=0; i<ret->numExtras; ++i)
    {
    ret->extras[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void expRecordFree(struct expRecord **pEl)
/* Free a single dynamically allocated expRecord such as created
 * with expRecordLoad(). */
{
struct expRecord *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->description);
freeMem(el->url);
freeMem(el->ref);
freeMem(el->credit);
/* All strings in extras are allocated at once, so only need to free first. */
if (el->extras != NULL)
    freeMem(el->extras[0]);
freeMem(el->extras);
freez(pEl);
}

void expRecordFreeList(struct expRecord **pList)
/* Free a list of dynamically allocated expRecord's */
{
struct expRecord *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    expRecordFree(&el);
    }
*pList = NULL;
}

void expRecordOutput(struct expRecord *el, FILE *f, char sep, char lastSep) 
/* Print out expRecord.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->description);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->url);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ref);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->credit);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->numExtras);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->numExtras; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->extras[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void expRecordCreateTable(struct sqlConnection *conn, char *table)
/* Create expression record format table of given name. */
{
char query[1024];

safef(query, sizeof(query),
"CREATE TABLE %s (\n"
"    id int unsigned not null,	# internal id of experiment\n"
"    name varchar(255) not null,	# name of experiment\n"
"    description longblob not null,	# description of experiment\n"
"    url longblob not null,	# url relevant to experiment\n"
"    ref longblob not null,	# reference for experiment\n"
"    credit longblob not null,	# who to credit with experiment\n"
"    numExtras int unsigned not null,	# number of extra things\n"
"    extras longblob not null,	# extra things of interest, i.e. classifications\n"
"              #Indices\n"
"    PRIMARY KEY(id)\n"
")\n",   table);
sqlRemakeTable(conn, table, query);
}

struct expRecord *expRecordLoadTable(struct sqlConnection *conn, char *table)
/* Load expression record format table of given name. */
{
char query[256];
char **row;
struct expRecord *ers = NULL;
safef(query, sizeof(query), "select * from  %s", table);
struct sqlResult *sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct expRecord *addMe = expRecordLoad(row);
    slAddHead(&ers, addMe);
    }
slReverse(&ers);
sqlFreeResult(&sr);
return ers;
}

struct expRecord *expRecordConnectAndLoadTable(char *database, char *table)
/* Load expression record format table of given name. */
{
struct sqlConnection *conn = sqlConnect(database);
struct expRecord *ers = expRecordLoadTable(conn, table);
sqlDisconnect(&conn);
return ers;
}
