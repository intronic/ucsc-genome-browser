/* polyGenotype.c was originally generated by the autoSql program, which also 
 * generated polyGenotype.h and polyGenotype.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "polyGenotype.h"


void polyGenotypeStaticLoad(char **row, struct polyGenotype *ret)
/* Load a row from polyGenotype table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->ethnicGroup = row[1];
ret->plusPlus = sqlSigned(row[2]);
ret->plusMinus = sqlSigned(row[3]);
ret->minusMinus = sqlSigned(row[4]);
ret->sampleSize = sqlSigned(row[5]);
ret->alleleFrequency = sqlFloat(row[6]);
ret->unbiasedHeterozygosity = sqlFloat(row[7]);
}

struct polyGenotype *polyGenotypeLoad(char **row)
/* Load a polyGenotype from row fetched with select * from polyGenotype
 * from database.  Dispose of this with polyGenotypeFree(). */
{
struct polyGenotype *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->ethnicGroup = cloneString(row[1]);
ret->plusPlus = sqlSigned(row[2]);
ret->plusMinus = sqlSigned(row[3]);
ret->minusMinus = sqlSigned(row[4]);
ret->sampleSize = sqlSigned(row[5]);
ret->alleleFrequency = sqlFloat(row[6]);
ret->unbiasedHeterozygosity = sqlFloat(row[7]);
return ret;
}

struct polyGenotype *polyGenotypeLoadAll(char *fileName) 
/* Load all polyGenotype from a whitespace-separated file.
 * Dispose of this with polyGenotypeFreeList(). */
{
struct polyGenotype *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = polyGenotypeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct polyGenotype *polyGenotypeLoadAllByChar(char *fileName, char chopper) 
/* Load all polyGenotype from a chopper separated file.
 * Dispose of this with polyGenotypeFreeList(). */
{
struct polyGenotype *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = polyGenotypeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct polyGenotype *polyGenotypeLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all polyGenotype from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with polyGenotypeFreeList(). */
{
struct polyGenotype *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = polyGenotypeLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void polyGenotypeSaveToDb(struct sqlConnection *conn, struct polyGenotype *el, char *tableName, int updateSize)
/* Save polyGenotype as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use polyGenotypeSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s','%s',%d,%d,%d,%d,%g,%g)", 
	tableName,  el->name,  el->ethnicGroup,  el->plusPlus,  el->plusMinus,  el->minusMinus,  el->sampleSize,  el->alleleFrequency,  el->unbiasedHeterozygosity);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void polyGenotypeSaveToDbEscaped(struct sqlConnection *conn, struct polyGenotype *el, char *tableName, int updateSize)
/* Save polyGenotype as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than polyGenotypeSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *name, *ethnicGroup;
name = sqlEscapeString(el->name);
ethnicGroup = sqlEscapeString(el->ethnicGroup);

dyStringPrintf(update, "insert into %s values ( '%s','%s',%d,%d,%d,%d,%g,%g)", 
	tableName,  name,  ethnicGroup, el->plusPlus , el->plusMinus , el->minusMinus , el->sampleSize , el->alleleFrequency , el->unbiasedHeterozygosity );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&name);
freez(&ethnicGroup);
}

struct polyGenotype *polyGenotypeCommaIn(char **pS, struct polyGenotype *ret)
/* Create a polyGenotype out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polyGenotype */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->ethnicGroup = sqlStringComma(&s);
ret->plusPlus = sqlSignedComma(&s);
ret->plusMinus = sqlSignedComma(&s);
ret->minusMinus = sqlSignedComma(&s);
ret->sampleSize = sqlSignedComma(&s);
ret->alleleFrequency = sqlFloatComma(&s);
ret->unbiasedHeterozygosity = sqlFloatComma(&s);
*pS = s;
return ret;
}

void polyGenotypeFree(struct polyGenotype **pEl)
/* Free a single dynamically allocated polyGenotype such as created
 * with polyGenotypeLoad(). */
{
struct polyGenotype *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->ethnicGroup);
freez(pEl);
}

void polyGenotypeFreeList(struct polyGenotype **pList)
/* Free a list of dynamically allocated polyGenotype's */
{
struct polyGenotype *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    polyGenotypeFree(&el);
    }
*pList = NULL;
}

void polyGenotypeOutput(struct polyGenotype *el, FILE *f, char sep, char lastSep) 
/* Print out polyGenotype.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ethnicGroup);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->plusPlus);
fputc(sep,f);
fprintf(f, "%d", el->plusMinus);
fputc(sep,f);
fprintf(f, "%d", el->minusMinus);
fputc(sep,f);
fprintf(f, "%d", el->sampleSize);
fputc(sep,f);
fprintf(f, "%g", el->alleleFrequency);
fputc(sep,f);
fprintf(f, "%g", el->unbiasedHeterozygosity);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

