/* goa.c was originally generated by the autoSql program, which also 
 * generated goa.h and goa.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "goa.h"

static char const rcsid[] = "$Id: goa.c,v 1.2 2005/04/13 06:25:53 markd Exp $";

void goaStaticLoad(char **row, struct goa *ret)
/* Load a row from goa table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->db = row[0];
ret->dbObjectId = row[1];
ret->dbObjectSymbol = row[2];
ret->notId = row[3];
ret->goId = row[4];
ret->dbReference = row[5];
ret->evidence = row[6];
ret->withFrom = row[7];
ret->aspect = row[8];
ret->dbObjectName = row[9];
ret->synonym = row[10];
ret->dbObjectType = row[11];
ret->taxon = row[12];
ret->date = row[13];
ret->assignedBy = row[14];
}

struct goa *goaLoad(char **row)
/* Load a goa from row fetched with select * from goa
 * from database.  Dispose of this with goaFree(). */
{
struct goa *ret;

AllocVar(ret);
ret->db = cloneString(row[0]);
ret->dbObjectId = cloneString(row[1]);
ret->dbObjectSymbol = cloneString(row[2]);
ret->notId = cloneString(row[3]);
ret->goId = cloneString(row[4]);
ret->dbReference = cloneString(row[5]);
ret->evidence = cloneString(row[6]);
ret->withFrom = cloneString(row[7]);
ret->aspect = cloneString(row[8]);
ret->dbObjectName = cloneString(row[9]);
ret->synonym = cloneString(row[10]);
ret->dbObjectType = cloneString(row[11]);
ret->taxon = cloneString(row[12]);
ret->date = cloneString(row[13]);
ret->assignedBy = cloneString(row[14]);
return ret;
}

struct goa *goaLoadAll(char *fileName) 
/* Load all goa from a whitespace-separated file.
 * Dispose of this with goaFreeList(). */
{
struct goa *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[15];

while (lineFileRow(lf, row))
    {
    el = goaLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct goa *goaLoadAllByChar(char *fileName, char chopper) 
/* Load all goa from a chopper separated file.
 * Dispose of this with goaFreeList(). */
{
struct goa *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[15];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = goaLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct goa *goaCommaIn(char **pS, struct goa *ret)
/* Create a goa out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new goa */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->db = sqlStringComma(&s);
ret->dbObjectId = sqlStringComma(&s);
ret->dbObjectSymbol = sqlStringComma(&s);
ret->notId = sqlStringComma(&s);
ret->goId = sqlStringComma(&s);
ret->dbReference = sqlStringComma(&s);
ret->evidence = sqlStringComma(&s);
ret->withFrom = sqlStringComma(&s);
ret->aspect = sqlStringComma(&s);
ret->dbObjectName = sqlStringComma(&s);
ret->synonym = sqlStringComma(&s);
ret->dbObjectType = sqlStringComma(&s);
ret->taxon = sqlStringComma(&s);
ret->date = sqlStringComma(&s);
ret->assignedBy = sqlStringComma(&s);
*pS = s;
return ret;
}

void goaFree(struct goa **pEl)
/* Free a single dynamically allocated goa such as created
 * with goaLoad(). */
{
struct goa *el;

if ((el = *pEl) == NULL) return;
freeMem(el->db);
freeMem(el->dbObjectId);
freeMem(el->dbObjectSymbol);
freeMem(el->notId);
freeMem(el->goId);
freeMem(el->dbReference);
freeMem(el->evidence);
freeMem(el->withFrom);
freeMem(el->aspect);
freeMem(el->dbObjectName);
freeMem(el->synonym);
freeMem(el->dbObjectType);
freeMem(el->taxon);
freeMem(el->date);
freeMem(el->assignedBy);
freez(pEl);
}

void goaFreeList(struct goa **pList)
/* Free a list of dynamically allocated goa's */
{
struct goa *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    goaFree(&el);
    }
*pList = NULL;
}

void goaOutput(struct goa *el, FILE *f, char sep, char lastSep) 
/* Print out goa.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->db);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dbObjectId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dbObjectSymbol);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->notId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->goId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dbReference);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->evidence);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->withFrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->aspect);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dbObjectName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->synonym);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dbObjectType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->taxon);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->date);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->assignedBy);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

