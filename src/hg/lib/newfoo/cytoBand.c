/* cytoBand.c was originally generated by the autoSql program, which also 
 * generated cytoBand.h and cytoBand.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "cytoBand.h"

void cytoBandStaticLoad(char **row, struct cytoBand *ret)
/* Load a row from cytoBand table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->gieStain = row[4];
ret->firmStart = sqlUnsigned(row[5]);
ret->firmEnd = sqlUnsigned(row[6]);
}

struct cytoBand *cytoBandLoad(char **row)
/* Load a cytoBand from row fetched with select * from cytoBand
 * from database.  Dispose of this with cytoBandFree(). */
{
struct cytoBand *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->gieStain = cloneString(row[4]);
ret->firmStart = sqlUnsigned(row[5]);
ret->firmEnd = sqlUnsigned(row[6]);
return ret;
}

struct cytoBand *cytoBandCommaIn(char **pS, struct cytoBand *ret)
/* Create a cytoBand out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cytoBand */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->gieStain = sqlStringComma(&s);
ret->firmStart = sqlUnsignedComma(&s);
ret->firmEnd = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void cytoBandFree(struct cytoBand **pEl)
/* Free a single dynamically allocated cytoBand such as created
 * with cytoBandLoad(). */
{
struct cytoBand *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->gieStain);
freez(pEl);
}

void cytoBandFreeList(struct cytoBand **pList)
/* Free a list of dynamically allocated cytoBand's */
{
struct cytoBand *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cytoBandFree(&el);
    }
*pList = NULL;
}

void cytoBandOutput(struct cytoBand *el, FILE *f, char sep, char lastSep) 
/* Print out cytoBand.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gieStain, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->firmStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->firmEnd, lastSep);
fputc(lastSep,f);
}

