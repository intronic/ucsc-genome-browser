/* hapmapPrimateAlleles.c was originally generated by the autoSql program, which also 
 * generated hapmapPrimateAlleles.h and hapmapPrimateAlleles.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hapmapPrimateAlleles.h"


void hapmapPrimateAllelesStaticLoad(char **row, struct hapmapPrimateAlleles *ret)
/* Load a row from hapmapPrimateAlleles table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->strand = row[5][0];
ret->refAllele = row[6][0];
ret->otherAllele = row[7][0];
ret->chimpChrom = row[8];
ret->chimpPos = sqlUnsigned(row[9]);
ret->chimpStrand = row[10][0];
ret->chimpAllele = row[11];
ret->chimpQual = sqlUnsigned(row[12]);
ret->rhesusChrom = row[13];
ret->rhesusPos = sqlUnsigned(row[14]);
ret->rhesusStrand = row[15][0];
ret->rhesusAllele = row[16];
ret->rhesusQual = sqlUnsigned(row[17]);
}

struct hapmapPrimateAlleles *hapmapPrimateAllelesLoad(char **row)
/* Load a hapmapPrimateAlleles from row fetched with select * from hapmapPrimateAlleles
 * from database.  Dispose of this with hapmapPrimateAllelesFree(). */
{
struct hapmapPrimateAlleles *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->strand = row[5][0];
ret->refAllele = row[6][0];
ret->otherAllele = row[7][0];
ret->chimpChrom = cloneString(row[8]);
ret->chimpPos = sqlUnsigned(row[9]);
ret->chimpStrand = row[10][0];
ret->chimpAllele = cloneString(row[11]);
ret->chimpQual = sqlUnsigned(row[12]);
ret->rhesusChrom = cloneString(row[13]);
ret->rhesusPos = sqlUnsigned(row[14]);
ret->rhesusStrand = row[15][0];
ret->rhesusAllele = cloneString(row[16]);
ret->rhesusQual = sqlUnsigned(row[17]);
return ret;
}

struct hapmapPrimateAlleles *hapmapPrimateAllelesLoadAll(char *fileName) 
/* Load all hapmapPrimateAlleles from a whitespace-separated file.
 * Dispose of this with hapmapPrimateAllelesFreeList(). */
{
struct hapmapPrimateAlleles *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[18];

while (lineFileRow(lf, row))
    {
    el = hapmapPrimateAllelesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapPrimateAlleles *hapmapPrimateAllelesLoadAllByChar(char *fileName, char chopper) 
/* Load all hapmapPrimateAlleles from a chopper separated file.
 * Dispose of this with hapmapPrimateAllelesFreeList(). */
{
struct hapmapPrimateAlleles *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[18];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hapmapPrimateAllelesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapPrimateAlleles *hapmapPrimateAllelesLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all hapmapPrimateAlleles from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hapmapPrimateAllelesFreeList(). */
{
struct hapmapPrimateAlleles *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = hapmapPrimateAllelesLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void hapmapPrimateAllelesSaveToDb(struct sqlConnection *conn, struct hapmapPrimateAlleles *el, char *tableName, int updateSize)
/* Save hapmapPrimateAlleles as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hapmapPrimateAllelesSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%c','%c','%c','%s',%u,'%c','%s',%u,'%s',%u,'%c','%s',%u)", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->refAllele,  el->otherAllele,  el->chimpChrom,  el->chimpPos,  el->chimpStrand,  el->chimpAllele,  el->chimpQual,  el->rhesusChrom,  el->rhesusPos,  el->rhesusStrand,  el->rhesusAllele,  el->rhesusQual);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void hapmapPrimateAllelesSaveToDbEscaped(struct sqlConnection *conn, struct hapmapPrimateAlleles *el, char *tableName, int updateSize)
/* Save hapmapPrimateAlleles as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hapmapPrimateAllelesSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *strand, *refAllele, *otherAllele, *chimpChrom, *chimpStrand, *chimpAllele, *rhesusChrom, *rhesusStrand, *rhesusAllele;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
strand = sqlEscapeString(el->strand);
refAllele = sqlEscapeString(el->refAllele);
otherAllele = sqlEscapeString(el->otherAllele);
chimpChrom = sqlEscapeString(el->chimpChrom);
chimpStrand = sqlEscapeString(el->chimpStrand);
chimpAllele = sqlEscapeString(el->chimpAllele);
rhesusChrom = sqlEscapeString(el->rhesusChrom);
rhesusStrand = sqlEscapeString(el->rhesusStrand);
rhesusAllele = sqlEscapeString(el->rhesusAllele);

dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s','%s','%s','%s',%u,'%s','%s',%u,'%s',%u,'%s','%s',%u)", 
	tableName,  chrom, el->chromStart , el->chromEnd ,  name, el->score ,  strand,  refAllele,  otherAllele,  chimpChrom, el->chimpPos ,  chimpStrand,  chimpAllele, el->chimpQual ,  rhesusChrom, el->rhesusPos ,  rhesusStrand,  rhesusAllele, el->rhesusQual );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&strand);
freez(&refAllele);
freez(&otherAllele);
freez(&chimpChrom);
freez(&chimpStrand);
freez(&chimpAllele);
freez(&rhesusChrom);
freez(&rhesusStrand);
freez(&rhesusAllele);
}

struct hapmapPrimateAlleles *hapmapPrimateAllelesCommaIn(char **pS, struct hapmapPrimateAlleles *ret)
/* Create a hapmapPrimateAlleles out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hapmapPrimateAlleles */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->strand), sizeof(ret->strand));
sqlFixedStringComma(&s, &(ret->refAllele), sizeof(ret->refAllele));
sqlFixedStringComma(&s, &(ret->otherAllele), sizeof(ret->otherAllele));
ret->chimpChrom = sqlStringComma(&s);
ret->chimpPos = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->chimpStrand), sizeof(ret->chimpStrand));
ret->chimpAllele = sqlStringComma(&s);
ret->chimpQual = sqlUnsignedComma(&s);
ret->rhesusChrom = sqlStringComma(&s);
ret->rhesusPos = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->rhesusStrand), sizeof(ret->rhesusStrand));
ret->rhesusAllele = sqlStringComma(&s);
ret->rhesusQual = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void hapmapPrimateAllelesFree(struct hapmapPrimateAlleles **pEl)
/* Free a single dynamically allocated hapmapPrimateAlleles such as created
 * with hapmapPrimateAllelesLoad(). */
{
struct hapmapPrimateAlleles *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->chimpChrom);
freeMem(el->chimpAllele);
freeMem(el->rhesusChrom);
freeMem(el->rhesusAllele);
freez(pEl);
}

void hapmapPrimateAllelesFreeList(struct hapmapPrimateAlleles **pList)
/* Free a list of dynamically allocated hapmapPrimateAlleles's */
{
struct hapmapPrimateAlleles *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hapmapPrimateAllelesFree(&el);
    }
*pList = NULL;
}

void hapmapPrimateAllelesOutput(struct hapmapPrimateAlleles *el, FILE *f, char sep, char lastSep) 
/* Print out hapmapPrimateAlleles.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%c", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->refAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->otherAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chimpChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chimpPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->chimpStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chimpAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chimpQual);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rhesusChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->rhesusPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->rhesusStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rhesusAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->rhesusQual);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

