/* hapmapAllelesCombined.c was originally generated by the autoSql program, which also 
 * generated hapmapAllelesCombined.h and hapmapAllelesCombined.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hapmapAllelesCombined.h"


void hapmapAllelesCombinedStaticLoad(char **row, struct hapmapAllelesCombined *ret)
/* Load a row from hapmapAllelesCombined table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->observed = row[6];
safecpy(ret->allele1, sizeof(ret->allele1), row[7]);
ret->allele1CountCEU = sqlUnsigned(row[8]);
ret->allele1CountCHB = sqlUnsigned(row[9]);
ret->allele1CountJPT = sqlUnsigned(row[10]);
ret->allele1CountYRI = sqlUnsigned(row[11]);
ret->allele2 = row[12];
ret->allele2CountCEU = sqlUnsigned(row[13]);
ret->allele2CountCHB = sqlUnsigned(row[14]);
ret->allele2CountJPT = sqlUnsigned(row[15]);
ret->allele2CountYRI = sqlUnsigned(row[16]);
ret->heteroCountCEU = sqlUnsigned(row[17]);
ret->heteroCountCHB = sqlUnsigned(row[18]);
ret->heteroCountJPT = sqlUnsigned(row[19]);
ret->heteroCountYRI = sqlUnsigned(row[20]);
}

struct hapmapAllelesCombined *hapmapAllelesCombinedLoad(char **row)
/* Load a hapmapAllelesCombined from row fetched with select * from hapmapAllelesCombined
 * from database.  Dispose of this with hapmapAllelesCombinedFree(). */
{
struct hapmapAllelesCombined *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->observed = cloneString(row[6]);
safecpy(ret->allele1, sizeof(ret->allele1), row[7]);
ret->allele1CountCEU = sqlUnsigned(row[8]);
ret->allele1CountCHB = sqlUnsigned(row[9]);
ret->allele1CountJPT = sqlUnsigned(row[10]);
ret->allele1CountYRI = sqlUnsigned(row[11]);
ret->allele2 = cloneString(row[12]);
ret->allele2CountCEU = sqlUnsigned(row[13]);
ret->allele2CountCHB = sqlUnsigned(row[14]);
ret->allele2CountJPT = sqlUnsigned(row[15]);
ret->allele2CountYRI = sqlUnsigned(row[16]);
ret->heteroCountCEU = sqlUnsigned(row[17]);
ret->heteroCountCHB = sqlUnsigned(row[18]);
ret->heteroCountJPT = sqlUnsigned(row[19]);
ret->heteroCountYRI = sqlUnsigned(row[20]);
return ret;
}

struct hapmapAllelesCombined *hapmapAllelesCombinedLoadAll(char *fileName) 
/* Load all hapmapAllelesCombined from a whitespace-separated file.
 * Dispose of this with hapmapAllelesCombinedFreeList(). */
{
struct hapmapAllelesCombined *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[21];

while (lineFileRow(lf, row))
    {
    el = hapmapAllelesCombinedLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapAllelesCombined *hapmapAllelesCombinedLoadAllByChar(char *fileName, char chopper) 
/* Load all hapmapAllelesCombined from a chopper separated file.
 * Dispose of this with hapmapAllelesCombinedFreeList(). */
{
struct hapmapAllelesCombined *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[21];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hapmapAllelesCombinedLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hapmapAllelesCombined *hapmapAllelesCombinedCommaIn(char **pS, struct hapmapAllelesCombined *ret)
/* Create a hapmapAllelesCombined out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hapmapAllelesCombined */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->observed = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->allele1, sizeof(ret->allele1));
ret->allele1CountCEU = sqlUnsignedComma(&s);
ret->allele1CountCHB = sqlUnsignedComma(&s);
ret->allele1CountJPT = sqlUnsignedComma(&s);
ret->allele1CountYRI = sqlUnsignedComma(&s);
ret->allele2 = sqlStringComma(&s);
ret->allele2CountCEU = sqlUnsignedComma(&s);
ret->allele2CountCHB = sqlUnsignedComma(&s);
ret->allele2CountJPT = sqlUnsignedComma(&s);
ret->allele2CountYRI = sqlUnsignedComma(&s);
ret->heteroCountCEU = sqlUnsignedComma(&s);
ret->heteroCountCHB = sqlUnsignedComma(&s);
ret->heteroCountJPT = sqlUnsignedComma(&s);
ret->heteroCountYRI = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void hapmapAllelesCombinedFree(struct hapmapAllelesCombined **pEl)
/* Free a single dynamically allocated hapmapAllelesCombined such as created
 * with hapmapAllelesCombinedLoad(). */
{
struct hapmapAllelesCombined *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->observed);
freeMem(el->allele2);
freez(pEl);
}

void hapmapAllelesCombinedFreeList(struct hapmapAllelesCombined **pList)
/* Free a list of dynamically allocated hapmapAllelesCombined's */
{
struct hapmapAllelesCombined *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hapmapAllelesCombinedFree(&el);
    }
*pList = NULL;
}

void hapmapAllelesCombinedOutput(struct hapmapAllelesCombined *el, FILE *f, char sep, char lastSep) 
/* Print out hapmapAllelesCombined.  Separate fields with sep. Follow last field with lastSep. */
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
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->observed);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->allele1);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->allele1CountCEU);
fputc(sep,f);
fprintf(f, "%u", el->allele1CountCHB);
fputc(sep,f);
fprintf(f, "%u", el->allele1CountJPT);
fputc(sep,f);
fprintf(f, "%u", el->allele1CountYRI);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->allele2);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->allele2CountCEU);
fputc(sep,f);
fprintf(f, "%u", el->allele2CountCHB);
fputc(sep,f);
fprintf(f, "%u", el->allele2CountJPT);
fputc(sep,f);
fprintf(f, "%u", el->allele2CountYRI);
fputc(sep,f);
fprintf(f, "%u", el->heteroCountCEU);
fputc(sep,f);
fprintf(f, "%u", el->heteroCountCHB);
fputc(sep,f);
fprintf(f, "%u", el->heteroCountJPT);
fputc(sep,f);
fprintf(f, "%u", el->heteroCountYRI);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

