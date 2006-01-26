/* snpFasta.c was originally generated by the autoSql program, which also 
 * generated snpFasta.h and snpFasta.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "snpFasta.h"

static char const rcsid[] = "$Id: snpFasta.c,v 1.3 2006/01/26 19:06:03 heather Exp $";

void snpFastaStaticLoad(char **row, struct snpFasta *ret)
/* Load a row from snpFasta table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->rsId = row[0];
ret->chrom = row[1];
ret->molType = row[2];
ret->class = row[3];
ret->observed = row[4];
ret->leftFlank = row[5];
ret->rightFlank = row[6];
}

struct snpFasta *snpFastaLoad(char **row)
/* Load a snpFasta from row fetched with select * from snpFasta
 * from database.  Dispose of this with snpFastaFree(). */
{
struct snpFasta *ret;

AllocVar(ret);
ret->rsId = cloneString(row[0]);
ret->chrom = cloneString(row[1]);
ret->molType = cloneString(row[2]);
ret->class = cloneString(row[3]);
ret->observed = cloneString(row[4]);
ret->leftFlank = cloneString(row[5]);
ret->rightFlank = cloneString(row[6]);
return ret;
}

struct snpFasta *snpFastaLoadAll(char *fileName) 
/* Load all snpFasta from a whitespace-separated file.
 * Dispose of this with snpFastaFreeList(). */
{
struct snpFasta *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = snpFastaLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snpFasta *snpFastaLoadAllByChar(char *fileName, char chopper) 
/* Load all snpFasta from a chopper separated file.
 * Dispose of this with snpFastaFreeList(). */
{
struct snpFasta *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = snpFastaLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snpFasta *snpFastaCommaIn(char **pS, struct snpFasta *ret)
/* Create a snpFasta out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snpFasta */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->rsId = sqlStringComma(&s);
ret->chrom = sqlStringComma(&s);
ret->molType = sqlStringComma(&s);
ret->class = sqlStringComma(&s);
ret->observed = sqlStringComma(&s);
ret->leftFlank = sqlStringComma(&s);
ret->rightFlank = sqlStringComma(&s);
*pS = s;
return ret;
}

void snpFastaFree(struct snpFasta **pEl)
/* Free a single dynamically allocated snpFasta such as created
 * with snpFastaLoad(). */
{
struct snpFasta *el;

if ((el = *pEl) == NULL) return;
freeMem(el->rsId);
freeMem(el->chrom);
freeMem(el->molType);
freeMem(el->class);
freeMem(el->observed);
freeMem(el->leftFlank);
freeMem(el->rightFlank);
freez(pEl);
}

void snpFastaFreeList(struct snpFasta **pList)
/* Free a list of dynamically allocated snpFasta's */
{
struct snpFasta *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    snpFastaFree(&el);
    }
*pList = NULL;
}

void snpFastaOutput(struct snpFasta *el, FILE *f, char sep, char lastSep) 
/* Print out snpFasta.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rsId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->molType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->class);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->observed);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->leftFlank);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rightFlank);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void snpFastaTableCreate(struct sqlConnection *conn, int indexSize)
/* create a snpFlank table */
{
char *createString =
"CREATE TABLE snpFasta (\n"
"    rsId varchar(255) not null,       \n"
"    chrom varchar(255) not null,       \n"
"    molType varchar(255) not null, \n"
"    class varchar(255) not null, \n"
"    observed longblob not null, \n"
"    leftFlank longblob not null, \n"
"    rightFlank longblob not null, \n"
"    INDEX rsId(rsId(12))\n"
")\n";

struct dyString *dy = newDyString(1024);
dyStringPrintf(dy, createString, indexSize, indexSize);
sqlRemakeTable(conn, "snpFasta", dy->string);
dyStringFree(&dy);
}

