/* sample.c was originally generated by the autoSql program, which also 
 * generated sample.h and sample.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "sample.h"

struct sample *sampleLoad(char **row)
/* Load a sample from row fetched with select * from sample
 * from database.  Dispose of this with sampleFree(). */
{
struct sample *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->sampleCount = sqlUnsigned(row[6]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlSigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
sqlUnsignedDynamicArray(row[7], &ret->samplePosition, &sizeOne);
assert(sizeOne == ret->sampleCount);
sqlUnsignedDynamicArray(row[8], &ret->sampleHeight, &sizeOne);
assert(sizeOne == ret->sampleCount);
return ret;
}

struct sample *sampleLoadAll(char *fileName) 
/* Load all sample from a tab-separated file.
 * Dispose of this with sampleFreeList(). */
{
struct sample *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = sampleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct sample *sampleLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all sample from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with sampleFreeList(). */
{
struct sample *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = sampleLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct sample *sampleCommaIn(char **pS, struct sample *ret)
/* Create a sample out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new sample */
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
ret->sampleCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->samplePosition, ret->sampleCount);
for (i=0; i<ret->sampleCount; ++i)
    {
    ret->samplePosition[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->sampleHeight, ret->sampleCount);
for (i=0; i<ret->sampleCount; ++i)
    {
    ret->sampleHeight[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void sampleFree(struct sample **pEl)
/* Free a single dynamically allocated sample such as created
 * with sampleLoad(). */
{
struct sample *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->samplePosition);
freeMem(el->sampleHeight);
freez(pEl);
}

void sampleFreeList(struct sample **pList)
/* Free a list of dynamically allocated sample's */
{
struct sample *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    sampleFree(&el);
    }
*pList = NULL;
}

void sampleOutput(struct sample *el, FILE *f, char sep, char lastSep) 
/* Print out sample.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->sampleCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->sampleCount; ++i)
    {
    fprintf(f, "%u", el->samplePosition[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->sampleCount; ++i)
    {
    fprintf(f, "%u", el->sampleHeight[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

