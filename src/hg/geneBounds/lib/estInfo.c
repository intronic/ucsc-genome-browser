/* estInfo.c was originally generated by the autoSql program, which also 
 * generated estInfo.h and estInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "estInfo.h"

void estInfoStaticLoad(char **row, struct estInfo *ret)
/* Load a row from estInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->intronOrientation = sqlSigned(row[4]);
ret->sizePolyA = sqlSigned(row[5]);
ret->revSizePolyA = sqlSigned(row[6]);
ret->signalPos = sqlSigned(row[7]);
ret->revSignalPos = sqlSigned(row[8]);
}

struct estInfo *estInfoLoad(char **row)
/* Load a estInfo from row fetched with select * from estInfo
 * from database.  Dispose of this with estInfoFree(). */
{
struct estInfo *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->intronOrientation = sqlSigned(row[4]);
ret->sizePolyA = sqlSigned(row[5]);
ret->revSizePolyA = sqlSigned(row[6]);
ret->signalPos = sqlSigned(row[7]);
ret->revSignalPos = sqlSigned(row[8]);
return ret;
}

struct estInfo *estInfoLoadAll(char *fileName) 
/* Load all estInfo from a tab-separated file.
 * Dispose of this with estInfoFreeList(). */
{
struct estInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = estInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct estInfo *estInfoLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all estInfo from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with estInfoFreeList(). */
{
struct estInfo *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = estInfoLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct estInfo *estInfoCommaIn(char **pS, struct estInfo *ret)
/* Create a estInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new estInfo */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->intronOrientation = sqlSignedComma(&s);
ret->sizePolyA = sqlSignedComma(&s);
ret->revSizePolyA = sqlSignedComma(&s);
ret->signalPos = sqlSignedComma(&s);
ret->revSignalPos = sqlSignedComma(&s);
*pS = s;
return ret;
}

void estInfoFree(struct estInfo **pEl)
/* Free a single dynamically allocated estInfo such as created
 * with estInfoLoad(). */
{
struct estInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void estInfoFreeList(struct estInfo **pList)
/* Free a list of dynamically allocated estInfo's */
{
struct estInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    estInfoFree(&el);
    }
*pList = NULL;
}

void estInfoOutput(struct estInfo *el, FILE *f, char sep, char lastSep) 
/* Print out estInfo.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%d", el->intronOrientation);
fputc(sep,f);
fprintf(f, "%d", el->sizePolyA);
fputc(sep,f);
fprintf(f, "%d", el->revSizePolyA);
fputc(sep,f);
fprintf(f, "%d", el->signalPos);
fputc(sep,f);
fprintf(f, "%d", el->revSignalPos);
fputc(lastSep,f);
}

