/* synteny100000.c was originally generated by the autoSql program, which also 
 * generated synteny100000.h and synteny100000.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "synteny100000.h"

static char const rcsid[] = "$Id: synteny100000.c,v 1.3 2003/05/06 07:22:23 kate Exp $";

void synteny100000StaticLoad(char **row, struct synteny100000 *ret)
/* Load a row from synteny100000 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->mouseChrom = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
}

struct synteny100000 *synteny100000Load(char **row)
/* Load a synteny100000 from row fetched with select * from synteny100000
 * from database.  Dispose of this with synteny100000Free(). */
{
struct synteny100000 *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->mouseChrom = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
return ret;
}

struct synteny100000 *synteny100000LoadAll(char *fileName) 
/* Load all synteny100000 from a tab-separated file.
 * Dispose of this with synteny100000FreeList(). */
{
struct synteny100000 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = synteny100000Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct synteny100000 *synteny100000LoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all synteny100000 from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with synteny100000FreeList(). */
{
struct synteny100000 *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = synteny100000Load(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct synteny100000 *synteny100000CommaIn(char **pS, struct synteny100000 *ret)
/* Create a synteny100000 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new synteny100000 */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->mouseChrom = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
*pS = s;
return ret;
}

void synteny100000Free(struct synteny100000 **pEl)
/* Free a single dynamically allocated synteny100000 such as created
 * with synteny100000Load(). */
{
struct synteny100000 *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->mouseChrom);
freez(pEl);
}

void synteny100000FreeList(struct synteny100000 **pList)
/* Free a list of dynamically allocated synteny100000's */
{
struct synteny100000 *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    synteny100000Free(&el);
    }
*pList = NULL;
}

void synteny100000Output(struct synteny100000 *el, FILE *f, char sep, char lastSep) 
/* Print out synteny100000.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->mouseChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

