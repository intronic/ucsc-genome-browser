/* dbRIP.c was originally generated by the autoSql program, which also 
 * generated dbRIP.h and dbRIP.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "dbRIP.h"


void dbRIPStaticLoad(char **row, struct dbRIP *ret)
/* Load a row from dbRIP table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->originalId = row[6];
ret->forwardPrimer = row[7];
ret->reversePrimer = row[8];
ret->polyClass = row[9];
ret->polyFamily = row[10];
ret->polySubfamily = row[11];
ret->polySeq = row[12];
ret->polySource = row[13];
ret->reference = row[14];
ret->ascertainingMethod = row[15];
ret->remarks = row[16];
ret->tm = atof(row[17]);
ret->filledSize = sqlSigned(row[18]);
ret->emptySize = sqlSigned(row[19]);
ret->disease = row[20];
ret->genoRegion = row[21];
}

struct dbRIP *dbRIPLoad(char **row)
/* Load a dbRIP from row fetched with select * from dbRIP
 * from database.  Dispose of this with dbRIPFree(). */
{
struct dbRIP *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->originalId = cloneString(row[6]);
ret->forwardPrimer = cloneString(row[7]);
ret->reversePrimer = cloneString(row[8]);
ret->polyClass = cloneString(row[9]);
ret->polyFamily = cloneString(row[10]);
ret->polySubfamily = cloneString(row[11]);
ret->polySeq = cloneString(row[12]);
ret->polySource = cloneString(row[13]);
ret->reference = cloneString(row[14]);
ret->ascertainingMethod = cloneString(row[15]);
ret->remarks = cloneString(row[16]);
ret->tm = atof(row[17]);
ret->filledSize = sqlSigned(row[18]);
ret->emptySize = sqlSigned(row[19]);
ret->disease = cloneString(row[20]);
ret->genoRegion = cloneString(row[21]);
return ret;
}

struct dbRIP *dbRIPLoadAll(char *fileName) 
/* Load all dbRIP from a whitespace-separated file.
 * Dispose of this with dbRIPFreeList(). */
{
struct dbRIP *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[22];

while (lineFileRow(lf, row))
    {
    el = dbRIPLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct dbRIP *dbRIPLoadAllByChar(char *fileName, char chopper) 
/* Load all dbRIP from a chopper separated file.
 * Dispose of this with dbRIPFreeList(). */
{
struct dbRIP *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[22];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = dbRIPLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct dbRIP *dbRIPLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all dbRIP from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with dbRIPFreeList(). */
{
struct dbRIP *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = dbRIPLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void dbRIPSaveToDb(struct sqlConnection *conn, struct dbRIP *el, char *tableName, int updateSize)
/* Save dbRIP as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use dbRIPSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s','%s','%s','%s','%s',%s,'%s',%s,'%s','%s',%g,%d,%d,'%s','%s')", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->originalId,  el->forwardPrimer,  el->reversePrimer,  el->polyClass,  el->polyFamily,  el->polySubfamily,  el->polySeq,  el->polySource,  el->reference,  el->ascertainingMethod,  el->remarks,  el->tm,  el->filledSize,  el->emptySize,  el->disease,  el->genoRegion);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void dbRIPSaveToDbEscaped(struct sqlConnection *conn, struct dbRIP *el, char *tableName, int updateSize)
/* Save dbRIP as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than dbRIPSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *strand, *originalId, *forwardPrimer, *reversePrimer, *polyClass, *polyFamily, *polySubfamily, *polySeq, *polySource, *reference, *ascertainingMethod, *remarks, *disease, *genoRegion;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
strand = sqlEscapeString(el->strand);
originalId = sqlEscapeString(el->originalId);
forwardPrimer = sqlEscapeString(el->forwardPrimer);
reversePrimer = sqlEscapeString(el->reversePrimer);
polyClass = sqlEscapeString(el->polyClass);
polyFamily = sqlEscapeString(el->polyFamily);
polySubfamily = sqlEscapeString(el->polySubfamily);
polySeq = sqlEscapeString(el->polySeq);
polySource = sqlEscapeString(el->polySource);
reference = sqlEscapeString(el->reference);
ascertainingMethod = sqlEscapeString(el->ascertainingMethod);
remarks = sqlEscapeString(el->remarks);
disease = sqlEscapeString(el->disease);
genoRegion = sqlEscapeString(el->genoRegion);

dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%g,%d,%d,'%s','%s')", 
	tableName,  chrom, el->chromStart , el->chromEnd ,  name, el->score ,  strand,  originalId,  forwardPrimer,  reversePrimer,  polyClass,  polyFamily,  polySubfamily,  polySeq,  polySource,  reference,  ascertainingMethod,  remarks, el->tm , el->filledSize , el->emptySize ,  disease,  genoRegion);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&strand);
freez(&originalId);
freez(&forwardPrimer);
freez(&reversePrimer);
freez(&polyClass);
freez(&polyFamily);
freez(&polySubfamily);
freez(&polySeq);
freez(&polySource);
freez(&reference);
freez(&ascertainingMethod);
freez(&remarks);
freez(&disease);
freez(&genoRegion);
}

struct dbRIP *dbRIPCommaIn(char **pS, struct dbRIP *ret)
/* Create a dbRIP out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dbRIP */
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
ret->originalId = sqlStringComma(&s);
ret->forwardPrimer = sqlStringComma(&s);
ret->reversePrimer = sqlStringComma(&s);
ret->polyClass = sqlStringComma(&s);
ret->polyFamily = sqlStringComma(&s);
ret->polySubfamily = sqlStringComma(&s);
ret->polySeq = sqlStringComma(&s);
ret->polySource = sqlStringComma(&s);
ret->reference = sqlStringComma(&s);
ret->ascertainingMethod = sqlStringComma(&s);
ret->remarks = sqlStringComma(&s);
ret->tm = sqlFloatComma(&s);
ret->filledSize = sqlSignedComma(&s);
ret->emptySize = sqlSignedComma(&s);
ret->disease = sqlStringComma(&s);
ret->genoRegion = sqlStringComma(&s);
*pS = s;
return ret;
}

void dbRIPFree(struct dbRIP **pEl)
/* Free a single dynamically allocated dbRIP such as created
 * with dbRIPLoad(). */
{
struct dbRIP *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->originalId);
freeMem(el->forwardPrimer);
freeMem(el->reversePrimer);
freeMem(el->polyClass);
freeMem(el->polyFamily);
freeMem(el->polySubfamily);
freeMem(el->polySeq);
freeMem(el->polySource);
freeMem(el->reference);
freeMem(el->ascertainingMethod);
freeMem(el->remarks);
freeMem(el->disease);
freeMem(el->genoRegion);
freez(pEl);
}

void dbRIPFreeList(struct dbRIP **pList)
/* Free a list of dynamically allocated dbRIP's */
{
struct dbRIP *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    dbRIPFree(&el);
    }
*pList = NULL;
}

void dbRIPOutput(struct dbRIP *el, FILE *f, char sep, char lastSep) 
/* Print out dbRIP.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->originalId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->forwardPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->reversePrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->polyClass);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->polyFamily);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->polySubfamily);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->polySeq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->polySource);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->reference);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ascertainingMethod);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->remarks);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->tm);
fputc(sep,f);
fprintf(f, "%d", el->filledSize);
fputc(sep,f);
fprintf(f, "%d", el->emptySize);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->disease);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genoRegion);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

