/* sangRead.c was originally generated by the autoSql program, which also 
 * generated sangRead.h and sangRead.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "sangRead.h"


void sangReadStaticLoad(char **row, struct sangRead *ret)
/* Load a row from sangRead table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

strcpy(ret->id, row[0]);
strcpy(ret->pq, row[1]);
strcpy(ret->name, row[2]);
strcpy(ret->dir, row[3]);
}

struct sangRead *sangReadLoad(char **row)
/* Load a sangRead from row fetched with select * from sangRead
 * from database.  Dispose of this with sangReadFree(). */
{
struct sangRead *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
strcpy(ret->id, row[0]);
strcpy(ret->pq, row[1]);
strcpy(ret->name, row[2]);
strcpy(ret->dir, row[3]);
return ret;
}

struct sangRead *sangReadCommaIn(char **pS, struct sangRead *ret)
/* Create a sangRead out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new sangRead */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
sqlFixedStringComma(&s, ret->id, sizeof(ret->id));
sqlFixedStringComma(&s, ret->pq, sizeof(ret->pq));
sqlFixedStringComma(&s, ret->name, sizeof(ret->name));
sqlFixedStringComma(&s, ret->dir, sizeof(ret->dir));
*pS = s;
return ret;
}

void sangReadFree(struct sangRead **pEl)
/* Free a single dynamically allocated sangRead such as created
 * with sangReadLoad(). */
{
struct sangRead *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void sangReadFreeList(struct sangRead **pList)
/* Free a list of dynamically allocated sangRead's */
{
struct sangRead *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    sangReadFree(&el);
    }
*pList = NULL;
}

void sangReadOutput(struct sangRead *el, FILE *f, char sep, char lastSep) 
/* Print out sangRead.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s%c", el->id, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s%c", el->pq, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s%c", el->name, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s%c", el->dir, lastSep);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

