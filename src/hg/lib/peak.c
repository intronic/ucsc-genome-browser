/* peak.c was originally generated by the autoSql program, which also 
 * generated peak.h and peak.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "peak.h"

static char const rcsid[] = "$Id: peak.c,v 1.1 2008/07/31 01:19:03 aamp Exp $";

void peakStaticLoad(char **row, struct peak *ret)
/* Load a row from peak table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->signalValue = sqlFloat(row[3]);
ret->pValue = sqlFloat(row[4]);
ret->peak = sqlSigned(row[5]);
}

struct peak *peakLoad(char **row)
/* Load a peak from row fetched with select * from peak
 * from database.  Dispose of this with peakFree(). */
{
struct peak *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->signalValue = sqlFloat(row[3]);
ret->pValue = sqlFloat(row[4]);
ret->peak = sqlSigned(row[5]);
return ret;
}

struct peak *peakLoadAll(char *fileName) 
/* Load all peak from a whitespace-separated file.
 * Dispose of this with peakFreeList(). */
{
struct peak *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = peakLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct peak *peakLoadAllByChar(char *fileName, char chopper) 
/* Load all peak from a chopper separated file.
 * Dispose of this with peakFreeList(). */
{
struct peak *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = peakLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct peak *peakLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all peak from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with peakFreeList(). */
{
struct peak *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = peakLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void peakSaveToDb(struct sqlConnection *conn, struct peak *el, char *tableName, int updateSize)
/* Save peak as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use peakSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,%g,%g,%d)", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->signalValue,  el->pValue,  el->peak);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void peakSaveToDbEscaped(struct sqlConnection *conn, struct peak *el, char *tableName, int updateSize)
/* Save peak as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than peakSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom;
chrom = sqlEscapeString(el->chrom);

dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,%g,%g,%d)", 
	tableName,  chrom,  el->chromStart,  el->chromEnd,  el->signalValue,  el->pValue,  el->peak);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
}

struct peak *peakCommaIn(char **pS, struct peak *ret)
/* Create a peak out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new peak */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->signalValue = sqlFloatComma(&s);
ret->pValue = sqlFloatComma(&s);
ret->peak = sqlSignedComma(&s);
*pS = s;
return ret;
}

void peakFree(struct peak **pEl)
/* Free a single dynamically allocated peak such as created
 * with peakLoad(). */
{
struct peak *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freez(pEl);
}

void peakFreeList(struct peak **pList)
/* Free a list of dynamically allocated peak's */
{
struct peak *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    peakFree(&el);
    }
*pList = NULL;
}

void peakOutput(struct peak *el, FILE *f, char sep, char lastSep) 
/* Print out peak.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
fprintf(f, "%g", el->signalValue);
fputc(sep,f);
fprintf(f, "%g", el->pValue);
fputc(sep,f);
fprintf(f, "%d", el->peak);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

