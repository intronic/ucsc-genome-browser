/* affyTransLifted.c was originally generated by the autoSql program, which also 
 * generated affyTransLifted.h and affyTransLifted.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "affyTransLifted.h"

static char const rcsid[] = "$Id: affyTransLifted.c,v 1.2 2003/05/06 07:22:20 kate Exp $";

void affyTransLiftedStaticLoad(char **row, struct affyTransLifted *ret)
/* Load a row from affyTransLifted table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromPos = sqlUnsigned(row[1]);
ret->xCoord = sqlSigned(row[2]);
ret->yCoord = sqlSigned(row[3]);
ret->rawPM = sqlSigned(row[4]);
ret->rawMM = sqlSigned(row[5]);
ret->normPM = atof(row[6]);
ret->normMM = atof(row[7]);
}

struct affyTransLifted *affyTransLiftedLoad(char **row)
/* Load a affyTransLifted from row fetched with select * from affyTransLifted
 * from database.  Dispose of this with affyTransLiftedFree(). */
{
struct affyTransLifted *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromPos = sqlUnsigned(row[1]);
ret->xCoord = sqlSigned(row[2]);
ret->yCoord = sqlSigned(row[3]);
ret->rawPM = sqlSigned(row[4]);
ret->rawMM = sqlSigned(row[5]);
ret->normPM = atof(row[6]);
ret->normMM = atof(row[7]);
return ret;
}

struct affyTransLifted *affyTransLiftedLoadAll(char *fileName) 
/* Load all affyTransLifted from a tab-separated file.
 * Dispose of this with affyTransLiftedFreeList(). */
{
struct affyTransLifted *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = affyTransLiftedLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyTransLifted *affyTransLiftedCommaIn(char **pS, struct affyTransLifted *ret)
/* Create a affyTransLifted out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new affyTransLifted */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromPos = sqlUnsignedComma(&s);
ret->xCoord = sqlSignedComma(&s);
ret->yCoord = sqlSignedComma(&s);
ret->rawPM = sqlSignedComma(&s);
ret->rawMM = sqlSignedComma(&s);
ret->normPM = sqlFloatComma(&s);
ret->normMM = sqlFloatComma(&s);
*pS = s;
return ret;
}

void affyTransLiftedFree(struct affyTransLifted **pEl)
/* Free a single dynamically allocated affyTransLifted such as created
 * with affyTransLiftedLoad(). */
{
struct affyTransLifted *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freez(pEl);
}

void affyTransLiftedFreeList(struct affyTransLifted **pList)
/* Free a list of dynamically allocated affyTransLifted's */
{
struct affyTransLifted *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    affyTransLiftedFree(&el);
    }
*pList = NULL;
}

void affyTransLiftedOutput(struct affyTransLifted *el, FILE *f, char sep, char lastSep) 
/* Print out affyTransLifted.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromPos);
fputc(sep,f);
fprintf(f, "%d", el->xCoord);
fputc(sep,f);
fprintf(f, "%d", el->yCoord);
fputc(sep,f);
fprintf(f, "%d", el->rawPM);
fputc(sep,f);
fprintf(f, "%d", el->rawMM);
fputc(sep,f);
fprintf(f, "%f", el->normPM);
fputc(sep,f);
fprintf(f, "%f", el->normMM);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

