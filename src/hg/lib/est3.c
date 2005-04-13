/* est3.c was originally generated by the autoSql program, which also 
 * generated est3.h and est3.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "est3.h"

static char const rcsid[] = "$Id: est3.c,v 1.4 2005/04/13 06:25:52 markd Exp $";

void est3StaticLoad(char **row, struct est3 *ret)
/* Load a row from est3 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
strcpy(ret->strand, row[3]);
ret->estCount = sqlUnsigned(row[4]);
}

struct est3 *est3Load(char **row)
/* Load a est3 from row fetched with select * from est3
 * from database.  Dispose of this with est3Free(). */
{
struct est3 *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
strcpy(ret->strand, row[3]);
ret->estCount = sqlUnsigned(row[4]);
return ret;
}

struct est3 *est3CommaIn(char **pS, struct est3 *ret)
/* Create a est3 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new est3 */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->estCount = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void est3Free(struct est3 **pEl)
/* Free a single dynamically allocated est3 such as created
 * with est3Load(). */
{
struct est3 *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freez(pEl);
}

void est3FreeList(struct est3 **pList)
/* Free a list of dynamically allocated est3's */
{
struct est3 *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    est3Free(&el);
    }
*pList = NULL;
}

void est3Output(struct est3 *el, FILE *f, char sep, char lastSep) 
/* Print out est3.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->estCount);
fputc(lastSep,f);
}

