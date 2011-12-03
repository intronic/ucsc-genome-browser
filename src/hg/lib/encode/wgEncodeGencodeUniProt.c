/* wgEncodeGencodeUniProt.c was originally generated by the autoSql program, which also 
 * generated wgEncodeGencodeUniProt.h and wgEncodeGencodeUniProt.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encode/wgEncodeGencodeUniProt.h"


/* definitions for dataset column */
static char *values_dataset[] = {"SwissProt", "TrEMBL", NULL};
static struct hash *valhash_dataset = NULL;

void wgEncodeGencodeUniProtStaticLoad(char **row, struct wgEncodeGencodeUniProt *ret)
/* Load a row from wgEncodeGencodeUniProt table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->transcriptId = row[0];
ret->acc = row[1];
ret->name = row[2];
ret->dataset = sqlEnumParse(row[3], values_dataset, &valhash_dataset);
}

struct wgEncodeGencodeUniProt *wgEncodeGencodeUniProtLoad(char **row)
/* Load a wgEncodeGencodeUniProt from row fetched with select * from wgEncodeGencodeUniProt
 * from database.  Dispose of this with wgEncodeGencodeUniProtFree(). */
{
struct wgEncodeGencodeUniProt *ret;

AllocVar(ret);
ret->transcriptId = cloneString(row[0]);
ret->acc = cloneString(row[1]);
ret->name = cloneString(row[2]);
ret->dataset = sqlEnumParse(row[3], values_dataset, &valhash_dataset);
return ret;
}

struct wgEncodeGencodeUniProt *wgEncodeGencodeUniProtLoadAll(char *fileName) 
/* Load all wgEncodeGencodeUniProt from a whitespace-separated file.
 * Dispose of this with wgEncodeGencodeUniProtFreeList(). */
{
struct wgEncodeGencodeUniProt *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = wgEncodeGencodeUniProtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct wgEncodeGencodeUniProt *wgEncodeGencodeUniProtLoadAllByChar(char *fileName, char chopper) 
/* Load all wgEncodeGencodeUniProt from a chopper separated file.
 * Dispose of this with wgEncodeGencodeUniProtFreeList(). */
{
struct wgEncodeGencodeUniProt *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = wgEncodeGencodeUniProtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct wgEncodeGencodeUniProt *wgEncodeGencodeUniProtCommaIn(char **pS, struct wgEncodeGencodeUniProt *ret)
/* Create a wgEncodeGencodeUniProt out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wgEncodeGencodeUniProt */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->transcriptId = sqlStringComma(&s);
ret->acc = sqlStringComma(&s);
ret->name = sqlStringComma(&s);
ret->dataset = sqlEnumComma(&s, values_dataset, &valhash_dataset);
*pS = s;
return ret;
}

void wgEncodeGencodeUniProtFree(struct wgEncodeGencodeUniProt **pEl)
/* Free a single dynamically allocated wgEncodeGencodeUniProt such as created
 * with wgEncodeGencodeUniProtLoad(). */
{
struct wgEncodeGencodeUniProt *el;

if ((el = *pEl) == NULL) return;
freeMem(el->transcriptId);
freeMem(el->acc);
freeMem(el->name);
freez(pEl);
}

void wgEncodeGencodeUniProtFreeList(struct wgEncodeGencodeUniProt **pList)
/* Free a list of dynamically allocated wgEncodeGencodeUniProt's */
{
struct wgEncodeGencodeUniProt *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    wgEncodeGencodeUniProtFree(&el);
    }
*pList = NULL;
}

void wgEncodeGencodeUniProtOutput(struct wgEncodeGencodeUniProt *el, FILE *f, char sep, char lastSep) 
/* Print out wgEncodeGencodeUniProt.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->transcriptId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->acc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
sqlEnumPrint(f, el->dataset, values_dataset);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

