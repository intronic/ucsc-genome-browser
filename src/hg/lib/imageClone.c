/* imageClone.c was originally generated by the autoSql program, which also 
 * generated imageClone.h and imageClone.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "imageClone.h"

static char const rcsid[] = "$Id: imageClone.c,v 1.3 2005/04/13 06:25:54 markd Exp $";

struct imageClone *imageCloneLoad(char **row)
/* Load a imageClone from row fetched with select * from imageClone
 * from database.  Dispose of this with imageCloneFree(). */
{
struct imageClone *ret;
int sizeOne;

AllocVar(ret);
ret->numGenbank = sqlSigned(row[7]);
ret->id = sqlUnsigned(row[0]);
ret->library = cloneString(row[1]);
ret->plateNum = sqlUnsigned(row[2]);
ret->row = cloneString(row[3]);
ret->column = sqlUnsigned(row[4]);
ret->libId = sqlUnsigned(row[5]);
ret->organism = cloneString(row[6]);
if(ret->numGenbank > 0) {
sqlStringDynamicArray(row[8], &ret->genbankIds, &sizeOne);
}
assert(sizeOne == ret->numGenbank);
return ret;
}

struct imageClone *imageCloneLoadAll(char *fileName) 
/* Load all imageClone from a tab-separated file.
 * Dispose of this with imageCloneFreeList(). */
{
struct imageClone *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = imageCloneLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct imageClone *imageCloneCommaIn(char **pS, struct imageClone *ret)
/* Create a imageClone out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new imageClone */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
ret->library = sqlStringComma(&s);
ret->plateNum = sqlUnsignedComma(&s);
ret->row = sqlStringComma(&s);
ret->column = sqlUnsignedComma(&s);
ret->libId = sqlUnsignedComma(&s);
ret->organism = sqlStringComma(&s);
ret->numGenbank = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->genbankIds, ret->numGenbank);
for (i=0; i<ret->numGenbank; ++i)
    {
    ret->genbankIds[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void imageCloneFree(struct imageClone **pEl)
/* Free a single dynamically allocated imageClone such as created
 * with imageCloneLoad(). */
{
struct imageClone *el;

if ((el = *pEl) == NULL) return;
freeMem(el->library);
freeMem(el->row);
freeMem(el->organism);
/* All strings in genbankIds are allocated at once, so only need to free first. */
if (el->genbankIds != NULL)
    freeMem(el->genbankIds[0]);
freeMem(el->genbankIds);
freez(pEl);
}

void imageCloneFreeList(struct imageClone **pList)
/* Free a list of dynamically allocated imageClone's */
{
struct imageClone *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    imageCloneFree(&el);
    }
*pList = NULL;
}

void imageCloneOutput(struct imageClone *el, FILE *f, char sep, char lastSep) 
/* Print out imageClone.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->library);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->plateNum);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->row);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->column);
fputc(sep,f);
fprintf(f, "%u", el->libId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->organism);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->numGenbank);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->numGenbank; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->genbankIds[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

