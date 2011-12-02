/* jaxQTL.c was originally generated by the autoSql program, which also 
 * generated jaxQTL.h and jaxQTL.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "jaxQTL.h"


void jaxQTLStaticLoad(char **row, struct jaxQTL *ret)
/* Load a row from jaxQTL table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->marker = row[6];
ret->mgiID = row[7];
ret->description = row[8];
ret->cMscore = atof(row[9]);
}

struct jaxQTL *jaxQTLLoad(char **row)
/* Load a jaxQTL from row fetched with select * from jaxQTL
 * from database.  Dispose of this with jaxQTLFree(). */
{
struct jaxQTL *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->marker = cloneString(row[6]);
ret->mgiID = cloneString(row[7]);
ret->description = cloneString(row[8]);
ret->cMscore = atof(row[9]);
return ret;
}

struct jaxQTL *jaxQTLLoadAll(char *fileName) 
/* Load all jaxQTL from a tab-separated file.
 * Dispose of this with jaxQTLFreeList(). */
{
struct jaxQTL *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileRow(lf, row))
    {
    el = jaxQTLLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct jaxQTL *jaxQTLLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all jaxQTL from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with jaxQTLFreeList(). */
{
struct jaxQTL *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = jaxQTLLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct jaxQTL *jaxQTLCommaIn(char **pS, struct jaxQTL *ret)
/* Create a jaxQTL out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new jaxQTL */
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
ret->marker = sqlStringComma(&s);
ret->mgiID = sqlStringComma(&s);
ret->description = sqlStringComma(&s);
ret->cMscore = sqlFloatComma(&s);
*pS = s;
return ret;
}

void jaxQTLFree(struct jaxQTL **pEl)
/* Free a single dynamically allocated jaxQTL such as created
 * with jaxQTLLoad(). */
{
struct jaxQTL *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->marker);
freeMem(el->mgiID);
freeMem(el->description);
freez(pEl);
}

void jaxQTLFreeList(struct jaxQTL **pList)
/* Free a list of dynamically allocated jaxQTL's */
{
struct jaxQTL *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    jaxQTLFree(&el);
    }
*pList = NULL;
}

void jaxQTLOutput(struct jaxQTL *el, FILE *f, char sep, char lastSep) 
/* Print out jaxQTL.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->marker);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mgiID);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->description);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->cMscore);
fputc(lastSep,f);
}

