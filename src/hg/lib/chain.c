/* chain.c was originally generated by the autoSql program, which also 
 * generated chain.h and chain.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "chainBlock.h"
#include "chain.h"

void chainStaticLoad(char **row, struct chain *ret)
/* Load a row from chain table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->qName = row[1];
ret->qSize = sqlUnsigned(row[2]);
ret->qStrand = row[3][0];
ret->qStart = sqlUnsigned(row[4]);
ret->qEnd = sqlUnsigned(row[5]);
ret->tName = row[6];
ret->tSize = sqlUnsigned(row[7]);
ret->tStart = sqlUnsigned(row[8]);
ret->tEnd = sqlUnsigned(row[9]);
ret->id = sqlUnsigned(row[10]);
}

struct chain *chainLoad(char **row)
/* Load a chain from row fetched with select * from chain
 * from database.  Dispose of this with chainFree(). */
{
struct chain *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->qName = cloneString(row[1]);
ret->qSize = sqlUnsigned(row[2]);
ret->qStrand = row[3][0];
ret->qStart = sqlUnsigned(row[4]);
ret->qEnd = sqlUnsigned(row[5]);
ret->tName = cloneString(row[6]);
ret->tSize = sqlUnsigned(row[7]);
ret->tStart = sqlUnsigned(row[8]);
ret->tEnd = sqlUnsigned(row[9]);
ret->id = sqlUnsigned(row[10]);
return ret;
}

struct chain *chainLoadAll(char *fileName) 
/* Load all chain from a tab-separated file.
 * Dispose of this with chainFreeList(). */
{
struct chain *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = chainLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct chain *chainCommaIn(char **pS, struct chain *ret)
/* Create a chain out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chain */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->score = sqlDoubleComma(&s);
ret->qName = sqlStringComma(&s);
ret->qSize = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->qStrand), sizeof(ret->qStrand));
ret->qStart = sqlUnsignedComma(&s);
ret->qEnd = sqlUnsignedComma(&s);
ret->tName = sqlStringComma(&s);
ret->tSize = sqlUnsignedComma(&s);
ret->tStart = sqlUnsignedComma(&s);
ret->tEnd = sqlUnsignedComma(&s);
ret->id = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void chainFree(struct chain **pEl)
/* Free a single dynamically allocated chain such as created
 * with chainLoad(). */
{
struct chain *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qName);
freeMem(el->tName);
freez(pEl);
}

void chainFreeList(struct chain **pList)
/* Free a list of dynamically allocated chain's */
{
struct chain *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    chainFree(&el);
    }
*pList = NULL;
}

void chainOutput(struct chain *el, FILE *f, char sep, char lastSep) 
/* Print out chain.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%f", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qSize);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->qStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qStart);
fputc(sep,f);
fprintf(f, "%u", el->qEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tSize);
fputc(sep,f);
fprintf(f, "%u", el->tStart);
fputc(sep,f);
fprintf(f, "%u", el->tEnd);
fputc(sep,f);
fprintf(f, "%u", el->id);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

