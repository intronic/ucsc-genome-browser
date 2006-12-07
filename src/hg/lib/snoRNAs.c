/* snoRNAs.c was originally generated by the autoSql program, which also 
 * generated snoRNAs.h and snoRNAs.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "snoRNAs.h"

static char const rcsid[] = "$Id: snoRNAs.c,v 1.1 2006/12/07 23:11:43 lowe Exp $";

void snoRNAsStaticLoad(char **row, struct snoRNAs *ret)
/* Load a row from snoRNAs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->snoScore = atof(row[6]);
ret->targetList = row[7];
ret->orthologs = row[8];
ret->guideLen = row[9];
ret->guideStr = row[10];
ret->guideScore = row[11];
ret->cBox = row[12];
ret->dBox = row[13];
ret->cpBox = row[14];
ret->dpBox = row[15];
ret->snoscanOutput = row[16];
}

struct snoRNAs *snoRNAsLoad(char **row)
/* Load a snoRNAs from row fetched with select * from snoRNAs
 * from database.  Dispose of this with snoRNAsFree(). */
{
struct snoRNAs *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->snoScore = atof(row[6]);
ret->targetList = cloneString(row[7]);
ret->orthologs = cloneString(row[8]);
ret->guideLen = cloneString(row[9]);
ret->guideStr = cloneString(row[10]);
ret->guideScore = cloneString(row[11]);
ret->cBox = cloneString(row[12]);
ret->dBox = cloneString(row[13]);
ret->cpBox = cloneString(row[14]);
ret->dpBox = cloneString(row[15]);
ret->snoscanOutput = cloneString(row[16]);
return ret;
}

struct snoRNAs *snoRNAsLoadAll(char *fileName) 
/* Load all snoRNAs from a whitespace-separated file.
 * Dispose of this with snoRNAsFreeList(). */
{
struct snoRNAs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileRow(lf, row))
    {
    el = snoRNAsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snoRNAs *snoRNAsLoadAllByChar(char *fileName, char chopper) 
/* Load all snoRNAs from a chopper separated file.
 * Dispose of this with snoRNAsFreeList(). */
{
struct snoRNAs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = snoRNAsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snoRNAs *snoRNAsCommaIn(char **pS, struct snoRNAs *ret)
/* Create a snoRNAs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snoRNAs */
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
ret->snoScore = sqlFloatComma(&s);
ret->targetList = sqlStringComma(&s);
ret->orthologs = sqlStringComma(&s);
ret->guideLen = sqlStringComma(&s);
ret->guideStr = sqlStringComma(&s);
ret->guideScore = sqlStringComma(&s);
ret->cBox = sqlStringComma(&s);
ret->dBox = sqlStringComma(&s);
ret->cpBox = sqlStringComma(&s);
ret->dpBox = sqlStringComma(&s);
ret->snoscanOutput = sqlStringComma(&s);
*pS = s;
return ret;
}

void snoRNAsFree(struct snoRNAs **pEl)
/* Free a single dynamically allocated snoRNAs such as created
 * with snoRNAsLoad(). */
{
struct snoRNAs *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->targetList);
freeMem(el->orthologs);
freeMem(el->guideLen);
freeMem(el->guideStr);
freeMem(el->guideScore);
freeMem(el->cBox);
freeMem(el->dBox);
freeMem(el->cpBox);
freeMem(el->dpBox);
freeMem(el->snoscanOutput);
freez(pEl);
}

void snoRNAsFreeList(struct snoRNAs **pList)
/* Free a list of dynamically allocated snoRNAs's */
{
struct snoRNAs *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    snoRNAsFree(&el);
    }
*pList = NULL;
}

void snoRNAsOutput(struct snoRNAs *el, FILE *f, char sep, char lastSep) 
/* Print out snoRNAs.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%g", el->snoScore);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->targetList);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->orthologs);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->guideLen);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->guideStr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->guideScore);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->cBox);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dBox);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->cpBox);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dpBox);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->snoscanOutput);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

