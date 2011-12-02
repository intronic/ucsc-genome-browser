/* omimTitle.c was originally generated by the autoSql program, which also 
 * generated omimTitle.h and omimTitle.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "omimTitle.h"


void omimTitleStaticLoad(char **row, struct omimTitle *ret)
/* Load a row from omimTitle table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->omimId = sqlSigned(row[0]);
strcpy(ret->type, row[1]);
ret->geneSymbol = row[2];
ret->title = row[3];
}

struct omimTitle *omimTitleLoad(char **row)
/* Load a omimTitle from row fetched with select * from omimTitle
 * from database.  Dispose of this with omimTitleFree(). */
{
struct omimTitle *ret;

AllocVar(ret);
ret->omimId = sqlSigned(row[0]);
strcpy(ret->type, row[1]);
ret->geneSymbol = cloneString(row[2]);
ret->title = cloneString(row[3]);
return ret;
}

struct omimTitle *omimTitleLoadAll(char *fileName) 
/* Load all omimTitle from a whitespace-separated file.
 * Dispose of this with omimTitleFreeList(). */
{
struct omimTitle *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = omimTitleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct omimTitle *omimTitleLoadAllByChar(char *fileName, char chopper) 
/* Load all omimTitle from a chopper separated file.
 * Dispose of this with omimTitleFreeList(). */
{
struct omimTitle *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = omimTitleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct omimTitle *omimTitleCommaIn(char **pS, struct omimTitle *ret)
/* Create a omimTitle out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new omimTitle */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->omimId = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->type, sizeof(ret->type));
ret->geneSymbol = sqlStringComma(&s);
ret->title = sqlStringComma(&s);
*pS = s;
return ret;
}

void omimTitleFree(struct omimTitle **pEl)
/* Free a single dynamically allocated omimTitle such as created
 * with omimTitleLoad(). */
{
struct omimTitle *el;

if ((el = *pEl) == NULL) return;
freeMem(el->geneSymbol);
freeMem(el->title);
freez(pEl);
}

void omimTitleFreeList(struct omimTitle **pList)
/* Free a list of dynamically allocated omimTitle's */
{
struct omimTitle *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    omimTitleFree(&el);
    }
*pList = NULL;
}

void omimTitleOutput(struct omimTitle *el, FILE *f, char sep, char lastSep) 
/* Print out omimTitle.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->omimId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->geneSymbol);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->title);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

