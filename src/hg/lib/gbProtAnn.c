/* gbProtAnn.c was originally generated by the autoSql program, which also 
 * generated gbProtAnn.h and gbProtAnn.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "gbProtAnn.h"

static char const rcsid[] = "$Id: gbProtAnn.c,v 1.2 2003/05/06 07:22:21 kate Exp $";

void gbProtAnnStaticLoad(char **row, struct gbProtAnn *ret)
/* Load a row from gbProtAnn table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->product = row[4];
ret->note = row[5];
ret->proteinId = row[6];
ret->giId = sqlUnsigned(row[7]);
}

struct gbProtAnn *gbProtAnnLoad(char **row)
/* Load a gbProtAnn from row fetched with select * from gbProtAnn
 * from database.  Dispose of this with gbProtAnnFree(). */
{
struct gbProtAnn *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->product = cloneString(row[4]);
ret->note = cloneString(row[5]);
ret->proteinId = cloneString(row[6]);
ret->giId = sqlUnsigned(row[7]);
return ret;
}

struct gbProtAnn *gbProtAnnLoadAll(char *fileName) 
/* Load all gbProtAnn from a tab-separated file.
 * Dispose of this with gbProtAnnFreeList(). */
{
struct gbProtAnn *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = gbProtAnnLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct gbProtAnn *gbProtAnnCommaIn(char **pS, struct gbProtAnn *ret)
/* Create a gbProtAnn out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gbProtAnn */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->product = sqlStringComma(&s);
ret->note = sqlStringComma(&s);
ret->proteinId = sqlStringComma(&s);
ret->giId = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void gbProtAnnFree(struct gbProtAnn **pEl)
/* Free a single dynamically allocated gbProtAnn such as created
 * with gbProtAnnLoad(). */
{
struct gbProtAnn *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->product);
freeMem(el->note);
freeMem(el->proteinId);
freez(pEl);
}

void gbProtAnnFreeList(struct gbProtAnn **pList)
/* Free a list of dynamically allocated gbProtAnn's */
{
struct gbProtAnn *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    gbProtAnnFree(&el);
    }
*pList = NULL;
}

void gbProtAnnOutput(struct gbProtAnn *el, FILE *f, char sep, char lastSep) 
/* Print out gbProtAnn.  Separate fields with sep. Follow last field with lastSep. */
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
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->product);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->note);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->proteinId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->giId);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

