/* affyAllExonProbe.c was originally generated by the autoSql program, which also 
 * generated affyAllExonProbe.h and affyAllExonProbe.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "affyAllExonProbe.h"


void affyAllExonProbeStaticLoad(char **row, struct affyAllExonProbe *ret)
/* Load a row from affyAllExonProbe table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->numIndependentProbes = sqlUnsigned(row[1]);
ret->exonClusterId = row[2];
ret->numNonOverlapProbes = sqlUnsigned(row[3]);
ret->probeCount = sqlUnsigned(row[4]);
ret->transcriptClustId = row[5];
ret->probesetType = sqlUnsigned(row[6]);
ret->numXHybeProbe = sqlUnsigned(row[7]);
ret->psrId = sqlUnsigned(row[8]);
ret->level = row[9];
ret->evidence = row[10];
ret->bounded = sqlUnsigned(row[11]);
ret->cds = sqlUnsigned(row[12]);
}

struct affyAllExonProbe *affyAllExonProbeLoad(char **row)
/* Load a affyAllExonProbe from row fetched with select * from affyAllExonProbe
 * from database.  Dispose of this with affyAllExonProbeFree(). */
{
struct affyAllExonProbe *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->numIndependentProbes = sqlUnsigned(row[1]);
ret->exonClusterId = cloneString(row[2]);
ret->numNonOverlapProbes = sqlUnsigned(row[3]);
ret->probeCount = sqlUnsigned(row[4]);
ret->transcriptClustId = cloneString(row[5]);
ret->probesetType = sqlUnsigned(row[6]);
ret->numXHybeProbe = sqlUnsigned(row[7]);
ret->psrId = sqlUnsigned(row[8]);
ret->level = cloneString(row[9]);
ret->evidence = cloneString(row[10]);
ret->bounded = sqlUnsigned(row[11]);
ret->cds = sqlUnsigned(row[12]);
return ret;
}

struct affyAllExonProbe *affyAllExonProbeLoadAll(char *fileName) 
/* Load all affyAllExonProbe from a whitespace-separated file.
 * Dispose of this with affyAllExonProbeFreeList(). */
{
struct affyAllExonProbe *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileRow(lf, row))
    {
    el = affyAllExonProbeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyAllExonProbe *affyAllExonProbeLoadAllByChar(char *fileName, char chopper) 
/* Load all affyAllExonProbe from a chopper separated file.
 * Dispose of this with affyAllExonProbeFreeList(). */
{
struct affyAllExonProbe *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = affyAllExonProbeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyAllExonProbe *affyAllExonProbeLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all affyAllExonProbe from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with affyAllExonProbeFreeList(). */
{
struct affyAllExonProbe *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = affyAllExonProbeLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void affyAllExonProbeSaveToDb(struct sqlConnection *conn, struct affyAllExonProbe *el, char *tableName, int updateSize)
/* Save affyAllExonProbe as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use affyAllExonProbeSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%u,'%s',%u,%u,'%s',%u,%u,%u,'%s','%s',%u,%u)", 
	tableName,  el->name,  el->numIndependentProbes,  el->exonClusterId,  el->numNonOverlapProbes,  el->probeCount,  el->transcriptClustId,  el->probesetType,  el->numXHybeProbe,  el->psrId,  el->level,  el->evidence,  el->bounded,  el->cds);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void affyAllExonProbeSaveToDbEscaped(struct sqlConnection *conn, struct affyAllExonProbe *el, char *tableName, int updateSize)
/* Save affyAllExonProbe as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than affyAllExonProbeSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *name, *exonClusterId, *transcriptClustId, *level, *evidence;
name = sqlEscapeString(el->name);
exonClusterId = sqlEscapeString(el->exonClusterId);
transcriptClustId = sqlEscapeString(el->transcriptClustId);
level = sqlEscapeString(el->level);
evidence = sqlEscapeString(el->evidence);

dyStringPrintf(update, "insert into %s values ( '%s',%u,'%s',%u,%u,'%s',%u,%u,%u,'%s','%s',%u,%u)", 
	tableName,  name, el->numIndependentProbes ,  exonClusterId, el->numNonOverlapProbes , el->probeCount ,  transcriptClustId, el->probesetType , el->numXHybeProbe , el->psrId ,  level,  evidence, el->bounded , el->cds );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&name);
freez(&exonClusterId);
freez(&transcriptClustId);
freez(&level);
freez(&evidence);
}

struct affyAllExonProbe *affyAllExonProbeCommaIn(char **pS, struct affyAllExonProbe *ret)
/* Create a affyAllExonProbe out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new affyAllExonProbe */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->numIndependentProbes = sqlUnsignedComma(&s);
ret->exonClusterId = sqlStringComma(&s);
ret->numNonOverlapProbes = sqlUnsignedComma(&s);
ret->probeCount = sqlUnsignedComma(&s);
ret->transcriptClustId = sqlStringComma(&s);
ret->probesetType = sqlUnsignedComma(&s);
ret->numXHybeProbe = sqlUnsignedComma(&s);
ret->psrId = sqlUnsignedComma(&s);
ret->level = sqlStringComma(&s);
ret->evidence = sqlStringComma(&s);
ret->bounded = sqlUnsignedComma(&s);
ret->cds = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void affyAllExonProbeFree(struct affyAllExonProbe **pEl)
/* Free a single dynamically allocated affyAllExonProbe such as created
 * with affyAllExonProbeLoad(). */
{
struct affyAllExonProbe *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->exonClusterId);
freeMem(el->transcriptClustId);
freeMem(el->level);
freeMem(el->evidence);
freez(pEl);
}

void affyAllExonProbeFreeList(struct affyAllExonProbe **pList)
/* Free a list of dynamically allocated affyAllExonProbe's */
{
struct affyAllExonProbe *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    affyAllExonProbeFree(&el);
    }
*pList = NULL;
}

void affyAllExonProbeOutput(struct affyAllExonProbe *el, FILE *f, char sep, char lastSep) 
/* Print out affyAllExonProbe.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->numIndependentProbes);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->exonClusterId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->numNonOverlapProbes);
fputc(sep,f);
fprintf(f, "%u", el->probeCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->transcriptClustId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->probesetType);
fputc(sep,f);
fprintf(f, "%u", el->numXHybeProbe);
fputc(sep,f);
fprintf(f, "%u", el->psrId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->level);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->evidence);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->bounded);
fputc(sep,f);
fprintf(f, "%u", el->cds);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

