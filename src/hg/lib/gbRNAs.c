/* gbRNAs.c was originally generated by the autoSql program, which also 
 * generated gbRNAs.h and gbRNAs.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "gbRNAs.h"


void gbRNAsStaticLoad(char **row, struct gbRNAs *ret)
/* Load a row from gbRNAs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->product = row[6];
ret->intron = row[7];
}

struct gbRNAs *gbRNAsLoad(char **row)
/* Load a gbRNAs from row fetched with select * from gbRNAs
 * from database.  Dispose of this with gbRNAsFree(). */
{
struct gbRNAs *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->product = cloneString(row[6]);
ret->intron = cloneString(row[7]);
return ret;
}

struct gbRNAs *gbRNAsLoadAll(char *fileName) 
/* Load all gbRNAs from a whitespace-separated file.
 * Dispose of this with gbRNAsFreeList(). */
{
struct gbRNAs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = gbRNAsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct gbRNAs *gbRNAsLoadAllByChar(char *fileName, char chopper) 
/* Load all gbRNAs from a chopper separated file.
 * Dispose of this with gbRNAsFreeList(). */
{
struct gbRNAs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = gbRNAsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct gbRNAs *gbRNAsCommaIn(char **pS, struct gbRNAs *ret)
/* Create a gbRNAs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gbRNAs */
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
ret->product = sqlStringComma(&s);
ret->intron = sqlStringComma(&s);
*pS = s;
return ret;
}

void gbRNAsFree(struct gbRNAs **pEl)
/* Free a single dynamically allocated gbRNAs such as created
 * with gbRNAsLoad(). */
{
struct gbRNAs *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->product);
freeMem(el->intron);
freez(pEl);
}

void gbRNAsFreeList(struct gbRNAs **pList)
/* Free a list of dynamically allocated gbRNAs's */
{
struct gbRNAs *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    gbRNAsFree(&el);
    }
*pList = NULL;
}

void gbRNAsOutput(struct gbRNAs *el, FILE *f, char sep, char lastSep) 
/* Print out gbRNAs.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->product);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->intron);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

