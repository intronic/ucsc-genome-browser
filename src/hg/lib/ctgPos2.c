/* ctgPos2.c was originally generated by the autoSql program, which also 
 * generated ctgPos2.h and ctgPos2.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "ctgPos2.h"


void ctgPos2StaticLoad(char **row, struct ctgPos2 *ret)
/* Load a row from ctgPos2 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->contig = row[0];
ret->size = sqlUnsigned(row[1]);
ret->chrom = row[2];
ret->chromStart = sqlUnsigned(row[3]);
ret->chromEnd = sqlUnsigned(row[4]);
strcpy(ret->type, row[5]);
}

struct ctgPos2 *ctgPos2Load(char **row)
/* Load a ctgPos2 from row fetched with select * from ctgPos2
 * from database.  Dispose of this with ctgPos2Free(). */
{
struct ctgPos2 *ret;

AllocVar(ret);
ret->contig = cloneString(row[0]);
ret->size = sqlUnsigned(row[1]);
ret->chrom = cloneString(row[2]);
ret->chromStart = sqlUnsigned(row[3]);
ret->chromEnd = sqlUnsigned(row[4]);
strcpy(ret->type, row[5]);
return ret;
}

struct ctgPos2 *ctgPos2LoadAll(char *fileName) 
/* Load all ctgPos2 from a whitespace-separated file.
 * Dispose of this with ctgPos2FreeList(). */
{
struct ctgPos2 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = ctgPos2Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ctgPos2 *ctgPos2LoadAllByChar(char *fileName, char chopper) 
/* Load all ctgPos2 from a chopper separated file.
 * Dispose of this with ctgPos2FreeList(). */
{
struct ctgPos2 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = ctgPos2Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ctgPos2 *ctgPos2CommaIn(char **pS, struct ctgPos2 *ret)
/* Create a ctgPos2 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ctgPos2 */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->contig = sqlStringComma(&s);
ret->size = sqlUnsignedComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->type, sizeof(ret->type));
*pS = s;
return ret;
}

void ctgPos2Free(struct ctgPos2 **pEl)
/* Free a single dynamically allocated ctgPos2 such as created
 * with ctgPos2Load(). */
{
struct ctgPos2 *el;

if ((el = *pEl) == NULL) return;
freeMem(el->contig);
freeMem(el->chrom);
freez(pEl);
}

void ctgPos2FreeList(struct ctgPos2 **pList)
/* Free a list of dynamically allocated ctgPos2's */
{
struct ctgPos2 *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ctgPos2Free(&el);
    }
*pList = NULL;
}

void ctgPos2Output(struct ctgPos2 *el, FILE *f, char sep, char lastSep) 
/* Print out ctgPos2.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->contig);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->size);
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
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

