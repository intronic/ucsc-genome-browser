/* codeBlast.c was originally generated by the autoSql program, which also 
 * generated codeBlast.h and codeBlast.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "codeBlast.h"

static char const rcsid[] = "$Id: codeBlast.c,v 1.2 2004/09/29 21:41:18 kschneid Exp $";

void codeBlastStaticLoad(char **row, struct codeBlast *ret)
/* Load a row from codeBlast table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->bin = sqlSigned(row[0]);
ret->chrom = row[1];
ret->chromStart = sqlUnsigned(row[2]);
ret->chromEnd = sqlUnsigned(row[3]);
ret->name = row[4];
ret->score = sqlUnsigned(row[5]);
strcpy(ret->strand, row[6]);
ret->code = row[7];
}

struct codeBlast *codeBlastLoad(char **row)
/* Load a codeBlast from row fetched with select * from codeBlast
 * from database.  Dispose of this with codeBlastFree(). */
{
struct codeBlast *ret;

AllocVar(ret);
ret->bin = sqlSigned(row[0]);
ret->chrom = cloneString(row[1]);
ret->chromStart = sqlUnsigned(row[2]);
ret->chromEnd = sqlUnsigned(row[3]);
ret->name = cloneString(row[4]);
ret->score = sqlUnsigned(row[5]);
strcpy(ret->strand, row[6]);
ret->code = cloneString(row[7]);
return ret;
}

struct codeBlast *codeBlastLoadAll(char *fileName) 
/* Load all codeBlast from a whitespace-separated file.
 * Dispose of this with codeBlastFreeList(). */
{
struct codeBlast *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = codeBlastLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct codeBlast *codeBlastLoadAllByChar(char *fileName, char chopper) 
/* Load all codeBlast from a chopper separated file.
 * Dispose of this with codeBlastFreeList(). */
{
struct codeBlast *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = codeBlastLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct codeBlast *codeBlastCommaIn(char **pS, struct codeBlast *ret)
/* Create a codeBlast out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new codeBlast */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->bin = sqlSignedComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->code = sqlStringComma(&s);
*pS = s;
return ret;
}

void codeBlastFree(struct codeBlast **pEl)
/* Free a single dynamically allocated codeBlast such as created
 * with codeBlastLoad(). */
{
struct codeBlast *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->code);
freez(pEl);
}

void codeBlastFreeList(struct codeBlast **pList)
/* Free a list of dynamically allocated codeBlast's */
{
struct codeBlast *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    codeBlastFree(&el);
    }
*pList = NULL;
}

void codeBlastOutput(struct codeBlast *el, FILE *f, char sep, char lastSep) 
/* Print out codeBlast.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->bin);
fputc(sep,f);
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
fprintf(f, "%s", el->code);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

