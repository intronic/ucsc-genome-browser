/* ctgPos.c was originally generated by the autoSql program, which also 
 * generated ctgPos.h and ctgPos.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "ctgPos.h"

void ctgPosStaticLoad(char **row, struct ctgPos *ret)
/* Load a row from ctgPos table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->contig = row[0];
ret->size = sqlUnsigned(row[1]);
ret->chrom = row[2];
ret->chromStart = sqlUnsigned(row[3]);
ret->chromEnd = sqlUnsigned(row[4]);
}

struct ctgPos *ctgPosLoad(char **row)
/* Load a ctgPos from row fetched with select * from ctgPos
 * from database.  Dispose of this with ctgPosFree(). */
{
struct ctgPos *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->contig = cloneString(row[0]);
ret->size = sqlUnsigned(row[1]);
ret->chrom = cloneString(row[2]);
ret->chromStart = sqlUnsigned(row[3]);
ret->chromEnd = sqlUnsigned(row[4]);
return ret;
}

struct ctgPos *ctgPosCommaIn(char **pS, struct ctgPos *ret)
/* Create a ctgPos out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ctgPos */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->contig = sqlStringComma(&s);
ret->size = sqlUnsignedComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void ctgPosFree(struct ctgPos **pEl)
/* Free a single dynamically allocated ctgPos such as created
 * with ctgPosLoad(). */
{
struct ctgPos *el;

if ((el = *pEl) == NULL) return;
freeMem(el->contig);
freeMem(el->chrom);
freez(pEl);
}

void ctgPosFreeList(struct ctgPos **pList)
/* Free a list of dynamically allocated ctgPos's */
{
struct ctgPos *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ctgPosFree(&el);
    }
*pList = NULL;
}

void ctgPosOutput(struct ctgPos *el, FILE *f, char sep, char lastSep) 
/* Print out ctgPos.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->contig, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->size, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd, lastSep);
fputc(lastSep,f);
}

