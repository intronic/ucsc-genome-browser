/* rnaGroup.c was originally generated by the autoSql program, which also 
 * generated rnaGroup.h and rnaGroup.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rnaGroup.h"

struct rnaGroup *rnaGroupLoad(char **row)
/* Load a rnaGroup from row fetched with select * from rnaGroup
 * from database.  Dispose of this with rnaGroupFree(). */
{
struct rnaGroup *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->refSeqCount = sqlUnsigned(row[6]);
ret->genBankCount = sqlUnsigned(row[8]);
ret->rikenCount = sqlUnsigned(row[10]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
sqlStringDynamicArray(row[7], &ret->refSeqs, &sizeOne);
assert(sizeOne == ret->refSeqCount);
sqlStringDynamicArray(row[9], &ret->genBanks, &sizeOne);
assert(sizeOne == ret->genBankCount);
sqlStringDynamicArray(row[11], &ret->rikens, &sizeOne);
assert(sizeOne == ret->rikenCount);
return ret;
}

struct rnaGroup *rnaGroupLoadAll(char *fileName) 
/* Load all rnaGroup from a tab-separated file.
 * Dispose of this with rnaGroupFreeList(). */
{
struct rnaGroup *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileRow(lf, row))
    {
    el = rnaGroupLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rnaGroup *rnaGroupLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all rnaGroup from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with rnaGroupFreeList(). */
{
struct rnaGroup *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    dyStringPrintf(query, " where %s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = rnaGroupLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct rnaGroup *rnaGroupCommaIn(char **pS, struct rnaGroup *ret)
/* Create a rnaGroup out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rnaGroup */
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
ret->refSeqCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->refSeqs, ret->refSeqCount);
for (i=0; i<ret->refSeqCount; ++i)
    {
    ret->refSeqs[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->genBankCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->genBanks, ret->genBankCount);
for (i=0; i<ret->genBankCount; ++i)
    {
    ret->genBanks[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->rikenCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->rikens, ret->rikenCount);
for (i=0; i<ret->rikenCount; ++i)
    {
    ret->rikens[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void rnaGroupFree(struct rnaGroup **pEl)
/* Free a single dynamically allocated rnaGroup such as created
 * with rnaGroupLoad(). */
{
struct rnaGroup *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
/* All strings in refSeqs are allocated at once, so only need to free first. */
if (el->refSeqs != NULL)
    freeMem(el->refSeqs[0]);
freeMem(el->refSeqs);
/* All strings in genBanks are allocated at once, so only need to free first. */
if (el->genBanks != NULL)
    freeMem(el->genBanks[0]);
freeMem(el->genBanks);
/* All strings in rikens are allocated at once, so only need to free first. */
if (el->rikens != NULL)
    freeMem(el->rikens[0]);
freeMem(el->rikens);
freez(pEl);
}

void rnaGroupFreeList(struct rnaGroup **pList)
/* Free a list of dynamically allocated rnaGroup's */
{
struct rnaGroup *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rnaGroupFree(&el);
    }
*pList = NULL;
}

void rnaGroupOutput(struct rnaGroup *el, FILE *f, char sep, char lastSep) 
/* Print out rnaGroup.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->refSeqCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->refSeqCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->refSeqs[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->genBankCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->genBankCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->genBanks[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->rikenCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->rikenCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->rikens[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

