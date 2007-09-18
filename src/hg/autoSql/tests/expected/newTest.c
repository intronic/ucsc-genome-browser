/* newTest.c was originally generated by the autoSql program, which also 
 * generated newTest.h and newTest.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "output/newTest.h"

static char const rcsid[] = "$Id:$";

struct point *pointCommaIn(char **pS, struct point *ret)
/* Create a point out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new point */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->x = sqlSignedComma(&s);
ret->y = sqlSignedComma(&s);
*pS = s;
return ret;
}

void pointFree(struct point **pEl)
/* Free a single dynamically allocated point such as created
 * with pointLoad(). */
{
struct point *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void pointFreeList(struct point **pList)
/* Free a list of dynamically allocated point's */
{
struct point *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pointFree(&el);
    }
*pList = NULL;
}

void pointOutput(struct point *el, FILE *f, char sep, char lastSep) 
/* Print out point.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->x);
fputc(sep,f);
fprintf(f, "%d", el->y);
fputc(lastSep,f);
}

struct autoTest *autoTestLoad(char **row)
/* Load a autoTest from row fetched with select * from autoTest
 * from database.  Dispose of this with autoTestFree(). */
{
struct autoTest *ret;

AllocVar(ret);
ret->ptCount = sqlSigned(row[4]);
ret->difCount = sqlSigned(row[6]);
ret->valCount = sqlSigned(row[9]);
ret->id = sqlUnsigned(row[0]);
safecpy(ret->shortName, sizeof(ret->shortName), row[1]);
ret->longName = cloneString(row[2]);
{
char *s = cloneString(row[3]);
sqlStringArray(s, ret->aliases, 3);
}
{
int sizeOne;
sqlShortDynamicArray(row[5], &ret->pts, &sizeOne);
assert(sizeOne == ret->ptCount);
}
{
int sizeOne;
sqlUbyteDynamicArray(row[7], &ret->difs, &sizeOne);
assert(sizeOne == ret->difCount);
}
{
char *s = row[8];
if(s != NULL && differentString(s, ""))
   ret->xy = pointCommaIn(&s, NULL);
}
{
int sizeOne;
sqlStringDynamicArray(row[10], &ret->vals, &sizeOne);
assert(sizeOne == ret->valCount);
}
return ret;
}

struct autoTest *autoTestLoadAll(char *fileName) 
/* Load all autoTest from a whitespace-separated file.
 * Dispose of this with autoTestFreeList(). */
{
struct autoTest *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = autoTestLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct autoTest *autoTestLoadAllByChar(char *fileName, char chopper) 
/* Load all autoTest from a chopper separated file.
 * Dispose of this with autoTestFreeList(). */
{
struct autoTest *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = autoTestLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct autoTest *autoTestLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all autoTest from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with autoTestFreeList(). */
{
struct autoTest *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = autoTestLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void autoTestSaveToDb(struct sqlConnection *conn, struct autoTest *el, char *tableName, int updateSize)
/* Save autoTest as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use autoTestSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
char  *aliasesArray, *ptsArray, *difsArray, *valsArray;
aliasesArray = sqlStringArrayToString(el->aliases, 3);
ptsArray = sqlShortArrayToString(el->pts, el->ptCount);
difsArray = sqlUbyteArrayToString(el->difs, el->difCount);
valsArray = sqlStringArrayToString(el->vals, el->valCount);
dyStringPrintf(update, "insert into %s values ( %u,'%s','%s','%s',%d,'%s',%d,'%s', NULL ,%d,'%s')", 
	tableName,  el->id,  el->shortName,  el->longName,  aliasesArray ,  el->ptCount,  ptsArray ,  el->difCount,  difsArray ,  el->valCount,  valsArray );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&aliasesArray);
freez(&ptsArray);
freez(&difsArray);
freez(&valsArray);
}

void autoTestSaveToDbEscaped(struct sqlConnection *conn, struct autoTest *el, char *tableName, int updateSize)
/* Save autoTest as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than autoTestSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *shortName, *longName, *aliasesArray, *ptsArray, *difsArray, *valsArray;
shortName = sqlEscapeString(el->shortName);
longName = sqlEscapeString(el->longName);

aliasesArray = sqlStringArrayToString(el->aliases, 3);
ptsArray = sqlShortArrayToString(el->pts, el->ptCount);
difsArray = sqlUbyteArrayToString(el->difs, el->difCount);
valsArray = sqlStringArrayToString(el->vals, el->valCount);
dyStringPrintf(update, "insert into %s values ( %u,'%s','%s','%s',%d,'%s',%d,'%s', NULL ,%d,'%s')", 
	tableName,  el->id,  shortName,  longName,  aliasesArray ,  el->ptCount,  ptsArray ,  el->difCount,  difsArray ,  el->valCount,  valsArray );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&shortName);
freez(&longName);
freez(&aliasesArray);
freez(&ptsArray);
freez(&difsArray);
freez(&valsArray);
}

struct autoTest *autoTestCommaIn(char **pS, struct autoTest *ret)
/* Create a autoTest out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new autoTest */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->shortName, sizeof(ret->shortName));
ret->longName = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
for (i=0; i<3; ++i)
    {
    ret->aliases[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->ptCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->pts, ret->ptCount);
for (i=0; i<ret->ptCount; ++i)
    {
    ret->pts[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->difCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->difs, ret->difCount);
for (i=0; i<ret->difCount; ++i)
    {
    ret->difs[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
s = sqlEatChar(s, '{');
if(s[0] != '}')    slSafeAddHead(&ret->xy, pointCommaIn(&s,NULL));
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->valCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->vals, ret->valCount);
for (i=0; i<ret->valCount; ++i)
    {
    ret->vals[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void autoTestFree(struct autoTest **pEl)
/* Free a single dynamically allocated autoTest such as created
 * with autoTestLoad(). */
{
struct autoTest *el;

if ((el = *pEl) == NULL) return;
freeMem(el->longName);
/* All strings in aliases are allocated at once, so only need to free first. */
if (el->aliases != NULL)
    freeMem(el->aliases[0]);
freeMem(el->pts);
freeMem(el->difs);
pointFreeList(&el->xy);
/* All strings in vals are allocated at once, so only need to free first. */
if (el->vals != NULL)
    freeMem(el->vals[0]);
freeMem(el->vals);
freez(pEl);
}

void autoTestFreeList(struct autoTest **pList)
/* Free a list of dynamically allocated autoTest's */
{
struct autoTest *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    autoTestFree(&el);
    }
*pList = NULL;
}

void autoTestOutput(struct autoTest *el, FILE *f, char sep, char lastSep) 
/* Print out autoTest.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shortName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->longName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<3; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->aliases[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->ptCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->ptCount; ++i)
    {
    fprintf(f, "%d", el->pts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->difCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->difCount; ++i)
    {
    fprintf(f, "%u", el->difs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
if (sep == ',') fputc('{',f);
if(el->xy != NULL)    pointCommaOut(el->xy,f);
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%d", el->valCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->valCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->vals[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

