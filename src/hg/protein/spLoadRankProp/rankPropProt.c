/* rankPropProt.c was originally generated by the autoSql program, which also 
 * generated rankPropProt.h and rankPropProt.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rankPropProt.h"


void rankPropProtStaticLoad(char **row, struct rankPropProt *ret)
/* Load a row from rankPropProt table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->qSpId = row[0];
ret->tSpId = row[1];
ret->score = atof(row[2]);
ret->qtEVal = atof(row[3]);
ret->tqEVal = atof(row[4]);
}

struct rankPropProt *rankPropProtLoad(char **row)
/* Load a rankPropProt from row fetched with select * from rankPropProt
 * from database.  Dispose of this with rankPropProtFree(). */
{
struct rankPropProt *ret;

AllocVar(ret);
ret->qSpId = cloneString(row[0]);
ret->tSpId = cloneString(row[1]);
ret->score = atof(row[2]);
ret->qtEVal = atof(row[3]);
ret->tqEVal = atof(row[4]);
return ret;
}

struct rankPropProt *rankPropProtLoadAll(char *fileName) 
/* Load all rankPropProt from a whitespace-separated file.
 * Dispose of this with rankPropProtFreeList(). */
{
struct rankPropProt *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileRow(lf, row))
    {
    el = rankPropProtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rankPropProt *rankPropProtLoadAllByChar(char *fileName, char chopper) 
/* Load all rankPropProt from a chopper separated file.
 * Dispose of this with rankPropProtFreeList(). */
{
struct rankPropProt *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rankPropProtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rankPropProt *rankPropProtCommaIn(char **pS, struct rankPropProt *ret)
/* Create a rankPropProt out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rankPropProt */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->qSpId = sqlStringComma(&s);
ret->tSpId = sqlStringComma(&s);
ret->score = sqlFloatComma(&s);
ret->qtEVal = sqlDoubleComma(&s);
ret->tqEVal = sqlDoubleComma(&s);
*pS = s;
return ret;
}

void rankPropProtFree(struct rankPropProt **pEl)
/* Free a single dynamically allocated rankPropProt such as created
 * with rankPropProtLoad(). */
{
struct rankPropProt *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qSpId);
freeMem(el->tSpId);
freez(pEl);
}

void rankPropProtFreeList(struct rankPropProt **pList)
/* Free a list of dynamically allocated rankPropProt's */
{
struct rankPropProt *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rankPropProtFree(&el);
    }
*pList = NULL;
}

void rankPropProtOutput(struct rankPropProt *el, FILE *f, char sep, char lastSep) 
/* Print out rankPropProt.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qSpId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tSpId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->score);
fputc(sep,f);
fprintf(f, "%g", el->qtEVal);
fputc(sep,f);
fprintf(f, "%g", el->tqEVal);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

