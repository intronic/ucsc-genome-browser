/* lowelabPfamHit.c was originally generated by the autoSql program, which also 
 * generated lowelabPfamHit.h and lowelabPfamHit.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "lowelabPfamHit.h"


void lowelabPfamHitsStaticLoad(char **row, struct lowelabPfamHits *ret)
/* Load a row from lowelabPfamHits table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->pfamAC = row[6];
ret->pfamID = row[7];
ret->swissAC = row[8];
ret->protCoord = row[9];
ret->ident = sqlUnsigned(row[10]);
ret->percLen = sqlUnsigned(row[11]);
}

struct lowelabPfamHits *lowelabPfamHitsLoad(char **row)
/* Load a lowelabPfamHits from row fetched with select * from lowelabPfamHits
 * from database.  Dispose of this with lowelabPfamHitsFree(). */
{
struct lowelabPfamHits *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->pfamAC = cloneString(row[6]);
ret->pfamID = cloneString(row[7]);
ret->swissAC = cloneString(row[8]);
ret->protCoord = cloneString(row[9]);
ret->ident = sqlUnsigned(row[10]);
ret->percLen = sqlUnsigned(row[11]);
return ret;
}

struct lowelabPfamHits *lowelabPfamHitsLoadAll(char *fileName) 
/* Load all lowelabPfamHits from a whitespace-separated file.
 * Dispose of this with lowelabPfamHitsFreeList(). */
{
struct lowelabPfamHits *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileRow(lf, row))
    {
    el = lowelabPfamHitsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct lowelabPfamHits *lowelabPfamHitsLoadAllByChar(char *fileName, char chopper) 
/* Load all lowelabPfamHits from a chopper separated file.
 * Dispose of this with lowelabPfamHitsFreeList(). */
{
struct lowelabPfamHits *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = lowelabPfamHitsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct lowelabPfamHits *lowelabPfamHitsCommaIn(char **pS, struct lowelabPfamHits *ret)
/* Create a lowelabPfamHits out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new lowelabPfamHits */
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
ret->pfamAC = sqlStringComma(&s);
ret->pfamID = sqlStringComma(&s);
ret->swissAC = sqlStringComma(&s);
ret->protCoord = sqlStringComma(&s);
ret->ident = sqlUnsignedComma(&s);
ret->percLen = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void lowelabPfamHitsFree(struct lowelabPfamHits **pEl)
/* Free a single dynamically allocated lowelabPfamHits such as created
 * with lowelabPfamHitsLoad(). */
{
struct lowelabPfamHits *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->pfamAC);
freeMem(el->pfamID);
freeMem(el->swissAC);
freeMem(el->protCoord);
freez(pEl);
}

void lowelabPfamHitsFreeList(struct lowelabPfamHits **pList)
/* Free a list of dynamically allocated lowelabPfamHits's */
{
struct lowelabPfamHits *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    lowelabPfamHitsFree(&el);
    }
*pList = NULL;
}

void lowelabPfamHitsOutput(struct lowelabPfamHits *el, FILE *f, char sep, char lastSep) 
/* Print out lowelabPfamHits.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->pfamAC);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->pfamID);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->swissAC);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->protCoord);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->ident);
fputc(sep,f);
fprintf(f, "%u", el->percLen);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

