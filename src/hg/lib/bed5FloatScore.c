/* bed5FloatScore.c was originally generated by the autoSql program, which also 
 * generated bed5FloatScore.h and bed5FloatScore.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "bed5FloatScore.h"

static char const rcsid[] = "$Id: bed5FloatScore.c,v 1.1 2005/06/22 21:49:48 angie Exp $";

void bed5FloatScoreStaticLoad(char **row, struct bed5FloatScore *ret)
/* Load a row from bed5FloatScore table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlSigned(row[4]);
ret->floatScore = atof(row[5]);
}

struct bed5FloatScore *bed5FloatScoreLoad(char **row)
/* Load a bed5FloatScore from row fetched with select * from bed5FloatScore
 * from database.  Dispose of this with bed5FloatScoreFree(). */
{
struct bed5FloatScore *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlSigned(row[4]);
ret->floatScore = atof(row[5]);
return ret;
}

struct bed5FloatScore *bed5FloatScoreLoadAll(char *fileName) 
/* Load all bed5FloatScore from a whitespace-separated file.
 * Dispose of this with bed5FloatScoreFreeList(). */
{
struct bed5FloatScore *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = bed5FloatScoreLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bed5FloatScore *bed5FloatScoreLoadAllByChar(char *fileName, char chopper) 
/* Load all bed5FloatScore from a chopper separated file.
 * Dispose of this with bed5FloatScoreFreeList(). */
{
struct bed5FloatScore *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = bed5FloatScoreLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bed5FloatScore *bed5FloatScoreCommaIn(char **pS, struct bed5FloatScore *ret)
/* Create a bed5FloatScore out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed5FloatScore */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlSignedComma(&s);
ret->floatScore = sqlFloatComma(&s);
*pS = s;
return ret;
}

void bed5FloatScoreFree(struct bed5FloatScore **pEl)
/* Free a single dynamically allocated bed5FloatScore such as created
 * with bed5FloatScoreLoad(). */
{
struct bed5FloatScore *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void bed5FloatScoreFreeList(struct bed5FloatScore **pList)
/* Free a list of dynamically allocated bed5FloatScore's */
{
struct bed5FloatScore *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bed5FloatScoreFree(&el);
    }
*pList = NULL;
}

void bed5FloatScoreOutput(struct bed5FloatScore *el, FILE *f, char sep, char lastSep) 
/* Print out bed5FloatScore.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%d", el->score);
fputc(sep,f);
fprintf(f, "%g", el->floatScore);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

