/* hapmapSnps.c was originally generated by the autoSql program, which also 
 * generated hapmapSnps.h and hapmapSnps.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hapmapSnps.h"

static char const rcsid[] = "$Id: hapmapSnps.c,v 1.1 2006/02/10 04:12:21 daryl Exp $";

void hapmapSnpStaticLoad(char **row, struct hapmapSnp *ret)
/* Load a row from hapmapSnp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
strcpy(ret->hReference, row[6]);
strcpy(ret->hOther, row[7]);
strcpy(ret->cBase, row[8]);
strcpy(ret->rBase, row[9]);
ret->cQual = sqlUnsigned(row[10]);
ret->rQual = sqlUnsigned(row[11]);
ret->ceu = atof(row[12]);
ret->chb = atof(row[13]);
ret->jpt = atof(row[14]);
ret->jptchb = atof(row[15]);
ret->yri = atof(row[16]);
}

struct hapmapSnp *hapmapSnpLoad(char **row)
/* Load a hapmapSnp from row fetched with select * from hapmapSnp
 * from database.  Dispose of this with hapmapSnpFree(). */
{
struct hapmapSnp *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
strcpy(ret->hReference, row[6]);
strcpy(ret->hOther, row[7]);
strcpy(ret->cBase, row[8]);
strcpy(ret->rBase, row[9]);
ret->cQual = sqlUnsigned(row[10]);
ret->rQual = sqlUnsigned(row[11]);
ret->ceu = atof(row[12]);
ret->chb = atof(row[13]);
ret->jpt = atof(row[14]);
ret->jptchb = atof(row[15]);
ret->yri = atof(row[16]);
return ret;
}

struct hapmapSnp *hapmapSnpLoadAll(char *fileName) 
/* Load all hapmapSnp from a whitespace-separated file.
 * Dispose of this with hapmapSnpFreeList(). */
{
struct hapmapSnp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileRow(lf, row))
    {
    el = hapmapSnpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapSnp *hapmapSnpLoadAllByChar(char *fileName, char chopper) 
/* Load all hapmapSnp from a chopper separated file.
 * Dispose of this with hapmapSnpFreeList(). */
{
struct hapmapSnp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hapmapSnpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapSnp *hapmapSnpLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all hapmapSnp from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hapmapSnpFreeList(). */
{
struct hapmapSnp *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = hapmapSnpLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void hapmapSnpSaveToDb(struct sqlConnection *conn, struct hapmapSnp *el, char *tableName, int updateSize)
/* Save hapmapSnp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hapmapSnpSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s','%s','%s',%u,%u,%g,%g,%g,%g,%g)", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->hReference,  el->hOther,  el->cBase,  el->rBase,  el->cQual,  el->rQual,  el->ceu,  el->chb,  el->jpt,  el->jptchb,  el->yri);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void hapmapSnpSaveToDbEscaped(struct sqlConnection *conn, struct hapmapSnp *el, char *tableName, int updateSize)
/* Save hapmapSnp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hapmapSnpSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *strand, *hReference, *hOther, *cBase, *rBase;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
strand = sqlEscapeString(el->strand);
hReference = sqlEscapeString(el->hReference);
hOther = sqlEscapeString(el->hOther);
cBase = sqlEscapeString(el->cBase);
rBase = sqlEscapeString(el->rBase);

dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s','%s','%s',%u,%u,%g,%g,%g,%g,%g)", 
	tableName,  chrom, el->chromStart , el->chromEnd ,  name, el->score ,  strand,  hReference,  hOther,  cBase,  rBase, el->cQual , el->rQual , el->ceu , el->chb , el->jpt , el->jptchb , el->yri );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&strand);
freez(&hReference);
freez(&hOther);
freez(&cBase);
freez(&rBase);
}

struct hapmapSnp *hapmapSnpCommaIn(char **pS, struct hapmapSnp *ret)
/* Create a hapmapSnp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hapmapSnp */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
sqlFixedStringComma(&s, ret->hReference, sizeof(ret->hReference));
sqlFixedStringComma(&s, ret->hOther, sizeof(ret->hOther));
sqlFixedStringComma(&s, ret->cBase, sizeof(ret->cBase));
sqlFixedStringComma(&s, ret->rBase, sizeof(ret->rBase));
ret->cQual = sqlUnsignedComma(&s);
ret->rQual = sqlUnsignedComma(&s);
ret->ceu = sqlFloatComma(&s);
ret->chb = sqlFloatComma(&s);
ret->jpt = sqlFloatComma(&s);
ret->jptchb = sqlFloatComma(&s);
ret->yri = sqlFloatComma(&s);
*pS = s;
return ret;
}

void hapmapSnpFree(struct hapmapSnp **pEl)
/* Free a single dynamically allocated hapmapSnp such as created
 * with hapmapSnpLoad(). */
{
struct hapmapSnp *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void hapmapSnpFreeList(struct hapmapSnp **pList)
/* Free a list of dynamically allocated hapmapSnp's */
{
struct hapmapSnp *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hapmapSnpFree(&el);
    }
*pList = NULL;
}

void hapmapSnpOutput(struct hapmapSnp *el, FILE *f, char sep, char lastSep) 
/* Print out hapmapSnp.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hReference);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hOther);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->cBase);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rBase);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->cQual);
fputc(sep,f);
fprintf(f, "%u", el->rQual);
fputc(sep,f);
fprintf(f, "%g", el->ceu);
fputc(sep,f);
fprintf(f, "%g", el->chb);
fputc(sep,f);
fprintf(f, "%g", el->jpt);
fputc(sep,f);
fprintf(f, "%g", el->jptchb);
fputc(sep,f);
fprintf(f, "%g", el->yri);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

