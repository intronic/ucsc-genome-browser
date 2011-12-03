/* exoFish.c was originally generated by the autoSql program, which also 
 * generated exoFish.h and exoFish.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "exoFish.h"


void exoFishStaticLoad(char **row, struct exoFish *ret)
/* Load a row from exoFish table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
}

struct exoFish *exoFishLoad(char **row)
/* Load a exoFish from row fetched with select * from exoFish
 * from database.  Dispose of this with exoFishFree(). */
{
struct exoFish *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
return ret;
}

struct exoFish *exoFishCommaIn(char **pS, struct exoFish *ret)
/* Create a exoFish out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new exoFish */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void exoFishFree(struct exoFish **pEl)
/* Free a single dynamically allocated exoFish such as created
 * with exoFishLoad(). */
{
struct exoFish *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void exoFishFreeList(struct exoFish **pList)
/* Free a list of dynamically allocated exoFish's */
{
struct exoFish *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    exoFishFree(&el);
    }
*pList = NULL;
}

void exoFishOutput(struct exoFish *el, FILE *f, char sep, char lastSep) 
/* Print out exoFish.  Separate fields with sep. Follow last field with lastSep. */
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
fputc(lastSep,f);
}

