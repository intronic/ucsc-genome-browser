/* recombRate.c was originally generated by the autoSql program, which also 
 * generated recombRate.h and recombRate.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "recombRate.h"

void recombRateStaticLoad(char **row, struct recombRate *ret)
/* Load a row from recombRate table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->recombRate = atof(row[4]);
ret->femaleRate = atof(row[5]);
ret->maleRate = atof(row[6]);
}

struct recombRate *recombRateLoad(char **row)
/* Load a recombRate from row fetched with select * from recombRate
 * from database.  Dispose of this with recombRateFree(). */
{
struct recombRate *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->recombRate = atof(row[4]);
ret->femaleRate = atof(row[5]);
ret->maleRate = atof(row[6]);
return ret;
}

struct recombRate *recombRateLoadAll(char *fileName) 
/* Load all recombRate from a tab-separated file.
 * Dispose of this with recombRateFreeList(). */
{
struct recombRate *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = recombRateLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct recombRate *recombRateLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all recombRate from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with recombRateFreeList(). */
{
struct recombRate *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = recombRateLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct recombRate *recombRateCommaIn(char **pS, struct recombRate *ret)
/* Create a recombRate out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new recombRate */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->recombRate = sqlFloatComma(&s);
ret->femaleRate = sqlFloatComma(&s);
ret->maleRate = sqlFloatComma(&s);
*pS = s;
return ret;
}

void recombRateFree(struct recombRate **pEl)
/* Free a single dynamically allocated recombRate such as created
 * with recombRateLoad(). */
{
struct recombRate *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void recombRateFreeList(struct recombRate **pList)
/* Free a list of dynamically allocated recombRate's */
{
struct recombRate *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    recombRateFree(&el);
    }
*pList = NULL;
}

void recombRateOutput(struct recombRate *el, FILE *f, char sep, char lastSep) 
/* Print out recombRate.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%f", el->recombRate);
fputc(sep,f);
fprintf(f, "%f", el->femaleRate);
fputc(sep,f);
fprintf(f, "%f", el->maleRate);
fputc(lastSep,f);
}

