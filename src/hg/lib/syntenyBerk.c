/* syntenyBerk.c was originally generated by the autoSql program, which also 
 * generated syntenyBerk.h and syntenyBerk.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "syntenyBerk.h"

static char const rcsid[] = "$Id: syntenyBerk.c,v 1.2 2003/05/06 07:22:23 kate Exp $";

void syntenyBerkStaticLoad(char **row, struct syntenyBerk *ret)
/* Load a row from syntenyBerk table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
}

struct syntenyBerk *syntenyBerkLoad(char **row)
/* Load a syntenyBerk from row fetched with select * from syntenyBerk
 * from database.  Dispose of this with syntenyBerkFree(). */
{
struct syntenyBerk *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
return ret;
}

struct syntenyBerk *syntenyBerkLoadAll(char *fileName) 
/* Load all syntenyBerk from a tab-separated file.
 * Dispose of this with syntenyBerkFreeList(). */
{
struct syntenyBerk *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = syntenyBerkLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct syntenyBerk *syntenyBerkLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all syntenyBerk from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with syntenyBerkFreeList(). */
{
struct syntenyBerk *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = syntenyBerkLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct syntenyBerk *syntenyBerkCommaIn(char **pS, struct syntenyBerk *ret)
/* Create a syntenyBerk out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new syntenyBerk */
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
*pS = s;
return ret;
}

void syntenyBerkFree(struct syntenyBerk **pEl)
/* Free a single dynamically allocated syntenyBerk such as created
 * with syntenyBerkLoad(). */
{
struct syntenyBerk *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void syntenyBerkFreeList(struct syntenyBerk **pList)
/* Free a list of dynamically allocated syntenyBerk's */
{
struct syntenyBerk *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    syntenyBerkFree(&el);
    }
*pList = NULL;
}

void syntenyBerkOutput(struct syntenyBerk *el, FILE *f, char sep, char lastSep) 
/* Print out syntenyBerk.  Separate fields with sep. Follow last field with lastSep. */
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
fputc(lastSep,f);
}

