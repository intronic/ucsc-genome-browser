/* stanMad.c was originally generated by the autoSql program, which also 
 * generated stanMad.h and stanMad.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "stanMad.h"

void stanMadStaticLoad(char **row, struct stanMad *ret)
/* Load a row from stanMad table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->exp = row[0];
ret->name = row[1];
ret->type = row[2];
ret->ch1i = sqlSigned(row[3]);
ret->ch1b = sqlSigned(row[4]);
ret->ch1d = sqlSigned(row[5]);
ret->ch2i = sqlSigned(row[6]);
ret->ch2b = sqlSigned(row[7]);
ret->ch2d = sqlSigned(row[8]);
ret->ch2in = sqlSigned(row[9]);
ret->ch2bn = sqlSigned(row[10]);
ret->ch2dn = sqlSigned(row[11]);
ret->rat1 = atof(row[12]);
ret->rat2 = atof(row[13]);
ret->rat1n = atof(row[14]);
ret->rat2n = atof(row[15]);
ret->mrat = atof(row[16]);
ret->crt1 = atof(row[17]);
ret->crt2 = atof(row[18]);
ret->regr = atof(row[19]);
ret->corr = atof(row[20]);
ret->edge = atof(row[21]);
ret->fing = sqlSigned(row[22]);
ret->grid = sqlSigned(row[23]);
ret->arow = sqlSigned(row[24]);
ret->row = sqlSigned(row[25]);
ret->acol = sqlSigned(row[26]);
ret->col = sqlSigned(row[27]);
ret->plat = sqlSigned(row[28]);
ret->prow = row[29];
ret->pcol = sqlSigned(row[30]);
ret->flag = sqlSigned(row[31]);
ret->clid = sqlSigned(row[32]);
ret->spot = sqlSigned(row[33]);
ret->left = sqlSigned(row[34]);
ret->top = sqlSigned(row[35]);
ret->right = sqlSigned(row[36]);
ret->bot = sqlSigned(row[37]);
ret->acc5 = row[38];
ret->acc3 = row[39];
}

struct stanMad *stanMadLoad(char **row)
/* Load a stanMad from row fetched with select * from stanMad
 * from database.  Dispose of this with stanMadFree(). */
{
struct stanMad *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->exp = cloneString(row[0]);
ret->name = cloneString(row[1]);
ret->type = cloneString(row[2]);
ret->ch1i = sqlSigned(row[3]);
ret->ch1b = sqlSigned(row[4]);
ret->ch1d = sqlSigned(row[5]);
ret->ch2i = sqlSigned(row[6]);
ret->ch2b = sqlSigned(row[7]);
ret->ch2d = sqlSigned(row[8]);
ret->ch2in = sqlSigned(row[9]);
ret->ch2bn = sqlSigned(row[10]);
ret->ch2dn = sqlSigned(row[11]);
ret->rat1 = atof(row[12]);
ret->rat2 = atof(row[13]);
ret->rat1n = atof(row[14]);
ret->rat2n = atof(row[15]);
ret->mrat = atof(row[16]);
ret->crt1 = atof(row[17]);
ret->crt2 = atof(row[18]);
ret->regr = atof(row[19]);
ret->corr = atof(row[20]);
ret->edge = atof(row[21]);
ret->fing = sqlSigned(row[22]);
ret->grid = sqlSigned(row[23]);
ret->arow = sqlSigned(row[24]);
ret->row = sqlSigned(row[25]);
ret->acol = sqlSigned(row[26]);
ret->col = sqlSigned(row[27]);
ret->plat = sqlSigned(row[28]);
ret->prow = cloneString(row[29]);
ret->pcol = sqlSigned(row[30]);
ret->flag = sqlSigned(row[31]);
ret->clid = sqlSigned(row[32]);
ret->spot = sqlSigned(row[33]);
ret->left = sqlSigned(row[34]);
ret->top = sqlSigned(row[35]);
ret->right = sqlSigned(row[36]);
ret->bot = sqlSigned(row[37]);
ret->acc5 = cloneString(row[38]);
ret->acc3 = cloneString(row[39]);
return ret;
}

struct stanMad *stanMadLoadAll(char *fileName) 
/* Load all stanMad from a tab-separated file.
 * Dispose of this with stanMadFreeList(). */
{
struct stanMad *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[40];

while (lineFileRow(lf, row))
    {
    el = stanMadLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct stanMad *stanMadCommaIn(char **pS, struct stanMad *ret)
/* Create a stanMad out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stanMad */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->exp = sqlStringComma(&s);
ret->name = sqlStringComma(&s);
ret->type = sqlStringComma(&s);
ret->ch1i = sqlSignedComma(&s);
ret->ch1b = sqlSignedComma(&s);
ret->ch1d = sqlSignedComma(&s);
ret->ch2i = sqlSignedComma(&s);
ret->ch2b = sqlSignedComma(&s);
ret->ch2d = sqlSignedComma(&s);
ret->ch2in = sqlSignedComma(&s);
ret->ch2bn = sqlSignedComma(&s);
ret->ch2dn = sqlSignedComma(&s);
ret->rat1 = sqlFloatComma(&s);
ret->rat2 = sqlFloatComma(&s);
ret->rat1n = sqlFloatComma(&s);
ret->rat2n = sqlFloatComma(&s);
ret->mrat = sqlFloatComma(&s);
ret->crt1 = sqlFloatComma(&s);
ret->crt2 = sqlFloatComma(&s);
ret->regr = sqlFloatComma(&s);
ret->corr = sqlFloatComma(&s);
ret->edge = sqlFloatComma(&s);
ret->fing = sqlSignedComma(&s);
ret->grid = sqlSignedComma(&s);
ret->arow = sqlSignedComma(&s);
ret->row = sqlSignedComma(&s);
ret->acol = sqlSignedComma(&s);
ret->col = sqlSignedComma(&s);
ret->plat = sqlSignedComma(&s);
ret->prow = sqlStringComma(&s);
ret->pcol = sqlSignedComma(&s);
ret->flag = sqlSignedComma(&s);
ret->clid = sqlSignedComma(&s);
ret->spot = sqlSignedComma(&s);
ret->left = sqlSignedComma(&s);
ret->top = sqlSignedComma(&s);
ret->right = sqlSignedComma(&s);
ret->bot = sqlSignedComma(&s);
ret->acc5 = sqlStringComma(&s);
ret->acc3 = sqlStringComma(&s);
*pS = s;
return ret;
}

void stanMadFree(struct stanMad **pEl)
/* Free a single dynamically allocated stanMad such as created
 * with stanMadLoad(). */
{
struct stanMad *el;

if ((el = *pEl) == NULL) return;
freeMem(el->exp);
freeMem(el->name);
freeMem(el->type);
freeMem(el->prow);
freeMem(el->acc5);
freeMem(el->acc3);
freez(pEl);
}

void stanMadFreeList(struct stanMad **pList)
/* Free a list of dynamically allocated stanMad's */
{
struct stanMad *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    stanMadFree(&el);
    }
*pList = NULL;
}

void stanMadOutput(struct stanMad *el, FILE *f, char sep, char lastSep) 
/* Print out stanMad.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->exp);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->ch1i);
fputc(sep,f);
fprintf(f, "%d", el->ch1b);
fputc(sep,f);
fprintf(f, "%d", el->ch1d);
fputc(sep,f);
fprintf(f, "%d", el->ch2i);
fputc(sep,f);
fprintf(f, "%d", el->ch2b);
fputc(sep,f);
fprintf(f, "%d", el->ch2d);
fputc(sep,f);
fprintf(f, "%d", el->ch2in);
fputc(sep,f);
fprintf(f, "%d", el->ch2bn);
fputc(sep,f);
fprintf(f, "%d", el->ch2dn);
fputc(sep,f);
fprintf(f, "%f", el->rat1);
fputc(sep,f);
fprintf(f, "%f", el->rat2);
fputc(sep,f);
fprintf(f, "%f", el->rat1n);
fputc(sep,f);
fprintf(f, "%f", el->rat2n);
fputc(sep,f);
fprintf(f, "%f", el->mrat);
fputc(sep,f);
fprintf(f, "%f", el->crt1);
fputc(sep,f);
fprintf(f, "%f", el->crt2);
fputc(sep,f);
fprintf(f, "%f", el->regr);
fputc(sep,f);
fprintf(f, "%f", el->corr);
fputc(sep,f);
fprintf(f, "%f", el->edge);
fputc(sep,f);
fprintf(f, "%d", el->fing);
fputc(sep,f);
fprintf(f, "%d", el->grid);
fputc(sep,f);
fprintf(f, "%d", el->arow);
fputc(sep,f);
fprintf(f, "%d", el->row);
fputc(sep,f);
fprintf(f, "%d", el->acol);
fputc(sep,f);
fprintf(f, "%d", el->col);
fputc(sep,f);
fprintf(f, "%d", el->plat);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->prow);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->pcol);
fputc(sep,f);
fprintf(f, "%d", el->flag);
fputc(sep,f);
fprintf(f, "%d", el->clid);
fputc(sep,f);
fprintf(f, "%d", el->spot);
fputc(sep,f);
fprintf(f, "%d", el->left);
fputc(sep,f);
fprintf(f, "%d", el->top);
fputc(sep,f);
fprintf(f, "%d", el->right);
fputc(sep,f);
fprintf(f, "%d", el->bot);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->acc5);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->acc3);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

