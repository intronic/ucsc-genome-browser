/* scoredRef.c was originally generated by the autoSql program, which also 
 * generated scoredRef.h and scoredRef.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "scoredRef.h"

void scoredRefStaticLoad(char **row, struct scoredRef *ret)
/* Load a row from scoredRef table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->extFile = sqlUnsigned(row[3]);
ret->offset = sqlLongLong(row[4]);
ret->score = atof(row[5]);
}

struct scoredRef *scoredRefLoad(char **row)
/* Load a scoredRef from row fetched with select * from scoredRef
 * from database.  Dispose of this with scoredRefFree(). */
{
struct scoredRef *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->extFile = sqlUnsigned(row[3]);
ret->offset = sqlLongLong(row[4]);
ret->score = atof(row[5]);
return ret;
}

struct scoredRef *scoredRefLoadAll(char *fileName) 
/* Load all scoredRef from a whitespace-separated file.
 * Dispose of this with scoredRefFreeList(). */
{
struct scoredRef *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = scoredRefLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct scoredRef *scoredRefCommaIn(char **pS, struct scoredRef *ret)
/* Create a scoredRef out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new scoredRef */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->extFile = sqlUnsignedComma(&s);
ret->offset = sqlLongLongComma(&s);
ret->score = sqlFloatComma(&s);
*pS = s;
return ret;
}

void scoredRefFree(struct scoredRef **pEl)
/* Free a single dynamically allocated scoredRef such as created
 * with scoredRefLoad(). */
{
struct scoredRef *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freez(pEl);
}

void scoredRefFreeList(struct scoredRef **pList)
/* Free a list of dynamically allocated scoredRef's */
{
struct scoredRef *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    scoredRefFree(&el);
    }
*pList = NULL;
}

void scoredRefOutput(struct scoredRef *el, FILE *f, char sep, char lastSep) 
/* Print out scoredRef.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->extFile);
fputc(sep,f);
fprintf(f, "%lld", el->offset);
fputc(sep,f);
fprintf(f, "%f", el->score);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

