/* affyPairs.c was originally generated by the autoSql program, which also 
 * generated affyPairs.h and affyPairs.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "affyPairs.h"

static char const rcsid[] = "$Id: affyPairs.c,v 1.2 2003/05/06 07:22:20 kate Exp $";

void affyPairsStaticLoad(char **row, struct affyPairs *ret)
/* Load a row from affyPairs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->x = sqlUnsigned(row[0]);
ret->y = sqlUnsigned(row[1]);
ret->probeSet = row[2];
ret->method = row[3];
ret->tBase = sqlUnsigned(row[4]);
ret->pos = sqlUnsigned(row[5]);
ret->pm = atof(row[6]);
ret->pms = atof(row[7]);
ret->pmp = atof(row[8]);
ret->mm = atof(row[9]);
ret->mms = atof(row[10]);
ret->mmp = atof(row[11]);
}

struct affyPairs *affyPairsLoad(char **row)
/* Load a affyPairs from row fetched with select * from affyPairs
 * from database.  Dispose of this with affyPairsFree(). */
{
struct affyPairs *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->x = sqlUnsigned(row[0]);
ret->y = sqlUnsigned(row[1]);
ret->probeSet = cloneString(row[2]);
ret->method = cloneString(row[3]);
ret->tBase = sqlUnsigned(row[4]);
ret->pos = sqlUnsigned(row[5]);
ret->pm = atof(row[6]);
ret->pms = atof(row[7]);
ret->pmp = atof(row[8]);
ret->mm = atof(row[9]);
ret->mms = atof(row[10]);
ret->mmp = atof(row[11]);
return ret;
}

struct affyPairs *affyPairsLoadAll(char *fileName) 
/* Load all affyPairs from a tab-separated file.
 * Dispose of this with affyPairsFreeList(). */
{
struct affyPairs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

lineFileRow(lf, row); /* get rid of one line header. */
while (lineFileRow(lf, row))
    {
    el = affyPairsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyPairs *affyPairsCommaIn(char **pS, struct affyPairs *ret)
/* Create a affyPairs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new affyPairs */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->x = sqlUnsignedComma(&s);
ret->y = sqlUnsignedComma(&s);
ret->probeSet = sqlStringComma(&s);
ret->method = sqlStringComma(&s);
ret->tBase = sqlUnsignedComma(&s);
ret->pos = sqlUnsignedComma(&s);
ret->pm = sqlFloatComma(&s);
ret->pms = sqlFloatComma(&s);
ret->pmp = sqlFloatComma(&s);
ret->mm = sqlFloatComma(&s);
ret->mms = sqlFloatComma(&s);
ret->mmp = sqlFloatComma(&s);
*pS = s;
return ret;
}

void affyPairsFree(struct affyPairs **pEl)
/* Free a single dynamically allocated affyPairs such as created
 * with affyPairsLoad(). */
{
struct affyPairs *el;

if ((el = *pEl) == NULL) return;
freeMem(el->probeSet);
freeMem(el->method);
freez(pEl);
}

void affyPairsFreeList(struct affyPairs **pList)
/* Free a list of dynamically allocated affyPairs's */
{
struct affyPairs *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    affyPairsFree(&el);
    }
*pList = NULL;
}

void affyPairsOutput(struct affyPairs *el, FILE *f, char sep, char lastSep) 
/* Print out affyPairs.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->x);
fputc(sep,f);
fprintf(f, "%u", el->y);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->probeSet);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->method);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tBase);
fputc(sep,f);
fprintf(f, "%u", el->pos);
fputc(sep,f);
fprintf(f, "%f", el->pm);
fputc(sep,f);
fprintf(f, "%f", el->pms);
fputc(sep,f);
fprintf(f, "%f", el->pmp);
fputc(sep,f);
fprintf(f, "%f", el->mm);
fputc(sep,f);
fprintf(f, "%f", el->mms);
fputc(sep,f);
fprintf(f, "%f", el->mmp);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

