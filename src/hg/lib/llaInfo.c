/* llaInfo.c was originally generated by the autoSql program, which also 
 * generated llaInfo.h and llaInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "llaInfo.h"

static char const rcsid[] = "$Id: llaInfo.c,v 1.2 2005/04/13 06:25:55 markd Exp $";

struct llaInfo *llaInfoLoad(char **row)
/* Load a llaInfo from row fetched with select * from llaInfo
 * from database.  Dispose of this with llaInfoFree(). */
{
struct llaInfo *ret;
int sizeOne;

AllocVar(ret);
ret->numCorrs = sqlUnsigned(row[17]);
ret->name = cloneString(row[0]);
ret->type = cloneString(row[1]);
ret->SnTm = atof(row[2]);
ret->SnGc = atof(row[3]);
ret->SnSc = atof(row[4]);
ret->Sn3pSc = atof(row[5]);
ret->AsnTm = atof(row[6]);
ret->AsnGc = atof(row[7]);
ret->AsnSc = atof(row[8]);
ret->Asn3pSc = atof(row[9]);
ret->prodLen = sqlUnsigned(row[10]);
ret->ORFLen = sqlUnsigned(row[11]);
ret->meltTm = atof(row[12]);
ret->frcc = atof(row[13]);
ret->fr3pcc = atof(row[14]);
ret->SnSeq = cloneString(row[15]);
ret->AsnSeq = cloneString(row[16]);
sqlStringDynamicArray(row[18], &ret->corrNames, &sizeOne);
assert(sizeOne == ret->numCorrs);
sqlFloatDynamicArray(row[19], &ret->corrs, &sizeOne);
assert(sizeOne == ret->numCorrs);
return ret;
}

struct llaInfo *llaInfoLoadAll(char *fileName) 
/* Load all llaInfo from a whitespace-separated file.
 * Dispose of this with llaInfoFreeList(). */
{
struct llaInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[20];

while (lineFileRow(lf, row))
    {
    el = llaInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct llaInfo *llaInfoLoadAllByChar(char *fileName, char chopper) 
/* Load all llaInfo from a chopper separated file.
 * Dispose of this with llaInfoFreeList(). */
{
struct llaInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[20];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = llaInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct llaInfo *llaInfoLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all llaInfo from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with llaInfoFreeList(). */
{
struct llaInfo *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = llaInfoLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void llaInfoSaveToDb(struct sqlConnection *conn, struct llaInfo *el, char *tableName, int updateSize)
/* Save llaInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use llaInfoSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
char  *corrNamesArray, *corrsArray;
corrNamesArray = sqlStringArrayToString(el->corrNames, el->numCorrs);
corrsArray = sqlFloatArrayToString(el->corrs, el->numCorrs);
dyStringPrintf(update, "insert into %s values ( '%s','%s',%f,%f,%f,%f,%f,%f,%f,%f,%u,%u,%f,%f,%f,%s,%s,%u,'%s','%s')", 
	tableName,  el->name,  el->type,  el->SnTm,  el->SnGc,  el->SnSc,  el->Sn3pSc,  el->AsnTm,  el->AsnGc,  el->AsnSc,  el->Asn3pSc,  el->prodLen,  el->ORFLen,  el->meltTm,  el->frcc,  el->fr3pcc,  el->SnSeq,  el->AsnSeq,  el->numCorrs,  corrNamesArray ,  corrsArray );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&corrNamesArray);
freez(&corrsArray);
}

void llaInfoSaveToDbEscaped(struct sqlConnection *conn, struct llaInfo *el, char *tableName, int updateSize)
/* Save llaInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than llaInfoSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *name, *type, *SnSeq, *AsnSeq, *corrNamesArray, *corrsArray;
name = sqlEscapeString(el->name);
type = sqlEscapeString(el->type);
SnSeq = sqlEscapeString(el->SnSeq);
AsnSeq = sqlEscapeString(el->AsnSeq);

corrNamesArray = sqlStringArrayToString(el->corrNames, el->numCorrs);
corrsArray = sqlFloatArrayToString(el->corrs, el->numCorrs);
dyStringPrintf(update, "insert into %s values ( '%s','%s',%f,%f,%f,%f,%f,%f,%f,%f,%u,%u,%f,%f,%f,'%s','%s',%u,'%s','%s')", 
	tableName,  name,  type, el->SnTm , el->SnGc , el->SnSc , el->Sn3pSc , el->AsnTm , el->AsnGc , el->AsnSc , el->Asn3pSc , el->prodLen , el->ORFLen , el->meltTm , el->frcc , el->fr3pcc ,  SnSeq,  AsnSeq, el->numCorrs ,  corrNamesArray ,  corrsArray );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&name);
freez(&type);
freez(&SnSeq);
freez(&AsnSeq);
freez(&corrNamesArray);
freez(&corrsArray);
}

struct llaInfo *llaInfoCommaIn(char **pS, struct llaInfo *ret)
/* Create a llaInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new llaInfo */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->type = sqlStringComma(&s);
ret->SnTm = sqlFloatComma(&s);
ret->SnGc = sqlFloatComma(&s);
ret->SnSc = sqlFloatComma(&s);
ret->Sn3pSc = sqlFloatComma(&s);
ret->AsnTm = sqlFloatComma(&s);
ret->AsnGc = sqlFloatComma(&s);
ret->AsnSc = sqlFloatComma(&s);
ret->Asn3pSc = sqlFloatComma(&s);
ret->prodLen = sqlUnsignedComma(&s);
ret->ORFLen = sqlUnsignedComma(&s);
ret->meltTm = sqlFloatComma(&s);
ret->frcc = sqlFloatComma(&s);
ret->fr3pcc = sqlFloatComma(&s);
ret->SnSeq = sqlStringComma(&s);
ret->AsnSeq = sqlStringComma(&s);
ret->numCorrs = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->corrNames, ret->numCorrs);
for (i=0; i<ret->numCorrs; ++i)
    {
    ret->corrNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->corrs, ret->numCorrs);
for (i=0; i<ret->numCorrs; ++i)
    {
    ret->corrs[i] = sqlFloatComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void llaInfoFree(struct llaInfo **pEl)
/* Free a single dynamically allocated llaInfo such as created
 * with llaInfoLoad(). */
{
struct llaInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->type);
freeMem(el->SnSeq);
freeMem(el->AsnSeq);
/* All strings in corrNames are allocated at once, so only need to free first. */
if (el->corrNames != NULL)
    freeMem(el->corrNames[0]);
freeMem(el->corrNames);
freeMem(el->corrs);
freez(pEl);
}

void llaInfoFreeList(struct llaInfo **pList)
/* Free a list of dynamically allocated llaInfo's */
{
struct llaInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    llaInfoFree(&el);
    }
*pList = NULL;
}

void llaInfoOutput(struct llaInfo *el, FILE *f, char sep, char lastSep) 
/* Print out llaInfo.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->SnTm);
fputc(sep,f);
fprintf(f, "%f", el->SnGc);
fputc(sep,f);
fprintf(f, "%f", el->SnSc);
fputc(sep,f);
fprintf(f, "%f", el->Sn3pSc);
fputc(sep,f);
fprintf(f, "%f", el->AsnTm);
fputc(sep,f);
fprintf(f, "%f", el->AsnGc);
fputc(sep,f);
fprintf(f, "%f", el->AsnSc);
fputc(sep,f);
fprintf(f, "%f", el->Asn3pSc);
fputc(sep,f);
fprintf(f, "%u", el->prodLen);
fputc(sep,f);
fprintf(f, "%u", el->ORFLen);
fputc(sep,f);
fprintf(f, "%f", el->meltTm);
fputc(sep,f);
fprintf(f, "%f", el->frcc);
fputc(sep,f);
fprintf(f, "%f", el->fr3pcc);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->SnSeq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->AsnSeq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->numCorrs);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->numCorrs; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->corrNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->numCorrs; ++i)
    {
    fprintf(f, "%f", el->corrs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

