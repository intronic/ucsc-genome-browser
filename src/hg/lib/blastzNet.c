/* blastzNet.c was originally generated by the autoSql program, which also 
 * generated blastzNet.h and blastzNet.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "blastzNet.h"

static char const rcsid[] = "$Id: blastzNet.c,v 1.2 2003/05/06 07:22:20 kate Exp $";

void blastzNetStaticLoad(char **row, struct blastzNet *ret)
/* Load a row from blastzNet table into ret.  The contents of ret will
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
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
}

struct blastzNet *blastzNetLoad(char **row)
/* Load a blastzNet from row fetched with select * from blastzNet
 * from database.  Dispose of this with blastzNetFree(). */
{
struct blastzNet *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
return ret;
}

struct blastzNet *blastzNetLoadAll(char *fileName) 
/* Load all blastzNet from a tab-separated file.
 * Dispose of this with blastzNetFreeList(). */
{
struct blastzNet *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = blastzNetLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct blastzNet *blastzNetLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all blastzNet from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with blastzNetFreeList(). */
{
struct blastzNet *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = blastzNetLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct blastzNet *blastzNetCommaIn(char **pS, struct blastzNet *ret)
/* Create a blastzNet out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new blastzNet */
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
ret->thickStart = sqlUnsignedComma(&s);
ret->thickEnd = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void blastzNetFree(struct blastzNet **pEl)
/* Free a single dynamically allocated blastzNet such as created
 * with blastzNetLoad(). */
{
struct blastzNet *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void blastzNetFreeList(struct blastzNet **pList)
/* Free a list of dynamically allocated blastzNet's */
{
struct blastzNet *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    blastzNetFree(&el);
    }
*pList = NULL;
}

void blastzNetOutput(struct blastzNet *el, FILE *f, char sep, char lastSep) 
/* Print out blastzNet.  Separate fields with sep. Follow last field with lastSep. */
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
fputc(sep,f);
fprintf(f, "%u", el->thickStart);
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
fputc(lastSep,f);
}

