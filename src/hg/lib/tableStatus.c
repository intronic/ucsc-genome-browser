/* tableStatus.c was originally generated by the autoSql program, which also 
 * generated tableStatus.h and tableStatus.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "tableStatus.h"


void tableStatusStaticLoad(char **row, struct tableStatus *ret)
/* Load a row from tableStatus table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->type = row[1];
ret->rowFormat = row[2];
ret->rows = sqlUnsigned(row[3]);
ret->aveRowLength = sqlUnsigned(row[4]);
ret->dataLength = sqlLongLong(row[5]);
ret->maxDataLength = sqlLongLong(row[6]);
ret->indexLength = sqlLongLong(row[7]);
ret->dataFree = sqlLongLong(row[8]);
ret->autoIncrement = row[9];
ret->createTime = row[10];
ret->updateTime = row[11];
ret->checkTime = row[12];
ret->createOptions = row[13];
ret->comment = row[14];
}

struct tableStatus *tableStatusLoad(char **row)
/* Load a tableStatus from row fetched with select * from tableStatus
 * from database.  Dispose of this with tableStatusFree(). */
{
struct tableStatus *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->type = cloneString(row[1]);
ret->rowFormat = cloneString(row[2]);
ret->rows = sqlUnsigned(row[3]);
ret->aveRowLength = sqlUnsigned(row[4]);
ret->dataLength = sqlLongLong(row[5]);
ret->maxDataLength = sqlLongLong(row[6]);
ret->indexLength = sqlLongLong(row[7]);
ret->dataFree = sqlLongLong(row[8]);
ret->autoIncrement = cloneString(row[9]);
ret->createTime = cloneString(row[10]);
ret->updateTime = cloneString(row[11]);
ret->checkTime = cloneString(row[12]);
ret->createOptions = cloneString(row[13]);
ret->comment = cloneString(row[14]);
return ret;
}

struct tableStatus *tableStatusLoadAll(char *fileName) 
/* Load all tableStatus from a whitespace-separated file.
 * Dispose of this with tableStatusFreeList(). */
{
struct tableStatus *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[15];

while (lineFileRow(lf, row))
    {
    el = tableStatusLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct tableStatus *tableStatusLoadAllByChar(char *fileName, char chopper) 
/* Load all tableStatus from a chopper separated file.
 * Dispose of this with tableStatusFreeList(). */
{
struct tableStatus *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[15];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = tableStatusLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct tableStatus *tableStatusCommaIn(char **pS, struct tableStatus *ret)
/* Create a tableStatus out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new tableStatus */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->type = sqlStringComma(&s);
ret->rowFormat = sqlStringComma(&s);
ret->rows = sqlUnsignedComma(&s);
ret->aveRowLength = sqlUnsignedComma(&s);
ret->dataLength = sqlLongLongComma(&s);
ret->maxDataLength = sqlLongLongComma(&s);
ret->indexLength = sqlLongLongComma(&s);
ret->dataFree = sqlLongLongComma(&s);
ret->autoIncrement = sqlStringComma(&s);
ret->createTime = sqlStringComma(&s);
ret->updateTime = sqlStringComma(&s);
ret->checkTime = sqlStringComma(&s);
ret->createOptions = sqlStringComma(&s);
ret->comment = sqlStringComma(&s);
*pS = s;
return ret;
}

void tableStatusFree(struct tableStatus **pEl)
/* Free a single dynamically allocated tableStatus such as created
 * with tableStatusLoad(). */
{
struct tableStatus *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->type);
freeMem(el->rowFormat);
freeMem(el->autoIncrement);
freeMem(el->createTime);
freeMem(el->updateTime);
freeMem(el->checkTime);
freeMem(el->createOptions);
freeMem(el->comment);
freez(pEl);
}

void tableStatusFreeList(struct tableStatus **pList)
/* Free a list of dynamically allocated tableStatus's */
{
struct tableStatus *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    tableStatusFree(&el);
    }
*pList = NULL;
}

void tableStatusOutput(struct tableStatus *el, FILE *f, char sep, char lastSep) 
/* Print out tableStatus.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rowFormat);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->rows);
fputc(sep,f);
fprintf(f, "%u", el->aveRowLength);
fputc(sep,f);
fprintf(f, "%lld", el->dataLength);
fputc(sep,f);
fprintf(f, "%lld", el->maxDataLength);
fputc(sep,f);
fprintf(f, "%lld", el->indexLength);
fputc(sep,f);
fprintf(f, "%lld", el->dataFree);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->autoIncrement);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->createTime);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->updateTime);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->checkTime);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->createOptions);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->comment);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

