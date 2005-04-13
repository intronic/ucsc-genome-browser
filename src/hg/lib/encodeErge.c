/* encodeErge.c was originally generated by the autoSql program, which also 
 * generated encodeErge.h and encodeErge.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encodeErge.h"

static char const rcsid[] = "$Id: encodeErge.c,v 1.2 2005/04/13 06:25:52 markd Exp $";

struct encodeErge *encodeErgeLoad(char **row)
/* Load a encodeErge from row fetched with select * from encodeErge
 * from database.  Dispose of this with encodeErgeFree(). */
{
struct encodeErge *ret;
int sizeOne;

AllocVar(ret);
ret->blockCount = sqlUnsigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
sqlUnsignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
sqlUnsignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
ret->Id = cloneString(row[12]);
ret->color = cloneString(row[13]);
return ret;
}

struct encodeErge *encodeErgeLoadAll(char *fileName) 
/* Load all encodeErge from a whitespace-separated file.
 * Dispose of this with encodeErgeFreeList(). */
{
struct encodeErge *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileRow(lf, row))
    {
    el = encodeErgeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeErge *encodeErgeLoadAllByChar(char *fileName, char chopper) 
/* Load all encodeErge from a chopper separated file.
 * Dispose of this with encodeErgeFreeList(). */
{
struct encodeErge *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodeErgeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeErge *encodeErgeLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all encodeErge from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with encodeErgeFreeList(). */
{
struct encodeErge *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = encodeErgeLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void encodeErgeSaveToDb(struct sqlConnection *conn, struct encodeErge *el, char *tableName, int updateSize)
/* Save encodeErge as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use encodeErgeSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
char  *blockSizesArray, *chromStartsArray;
blockSizesArray = sqlUnsignedArrayToString(el->blockSizes, el->blockCount);
chromStartsArray = sqlUnsignedArrayToString(el->chromStarts, el->blockCount);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s',%u,%u,%u,%u,'%s','%s','%s','%s')", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->thickStart,  el->thickEnd,  el->reserved,  el->blockCount,  blockSizesArray ,  chromStartsArray ,  el->Id,  el->color);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&blockSizesArray);
freez(&chromStartsArray);
}

void encodeErgeSaveToDbEscaped(struct sqlConnection *conn, struct encodeErge *el, char *tableName, int updateSize)
/* Save encodeErge as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than encodeErgeSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *strand, *blockSizesArray, *chromStartsArray, *Id, *color;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
strand = sqlEscapeString(el->strand);
Id = sqlEscapeString(el->Id);
color = sqlEscapeString(el->color);

blockSizesArray = sqlUnsignedArrayToString(el->blockSizes, el->blockCount);
chromStartsArray = sqlUnsignedArrayToString(el->chromStarts, el->blockCount);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s',%u,%u,%u,%u,'%s','%s','%s','%s')", 
	tableName,  chrom, el->chromStart , el->chromEnd ,  name, el->score ,  strand, el->thickStart , el->thickEnd , el->reserved , el->blockCount ,  blockSizesArray ,  chromStartsArray ,  Id,  color);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&strand);
freez(&blockSizesArray);
freez(&chromStartsArray);
freez(&Id);
freez(&color);
}

struct encodeErge *encodeErgeCommaIn(char **pS, struct encodeErge *ret)
/* Create a encodeErge out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeErge */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->thickStart = sqlUnsignedComma(&s);
ret->thickEnd = sqlUnsignedComma(&s);
ret->reserved = sqlUnsignedComma(&s);
ret->blockCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->blockSizes, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->blockSizes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->chromStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->chromStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->Id = sqlStringComma(&s);
ret->color = sqlStringComma(&s);
*pS = s;
return ret;
}

void encodeErgeFree(struct encodeErge **pEl)
/* Free a single dynamically allocated encodeErge such as created
 * with encodeErgeLoad(). */
{
struct encodeErge *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->Id);
freeMem(el->color);
freez(pEl);
}

void encodeErgeFreeList(struct encodeErge **pList)
/* Free a list of dynamically allocated encodeErge's */
{
struct encodeErge *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodeErgeFree(&el);
    }
*pList = NULL;
}

void encodeErgeOutput(struct encodeErge *el, FILE *f, char sep, char lastSep) 
/* Print out encodeErge.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
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
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->thickStart);
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
fputc(sep,f);
fprintf(f, "%u", el->reserved);
fputc(sep,f);
fprintf(f, "%u", el->blockCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->blockSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->Id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->color);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

