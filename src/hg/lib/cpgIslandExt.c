/* cpgIslandExt.c was originally generated by the autoSql program, which also 
 * generated cpgIslandExt.h and cpgIslandExt.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "cpgIslandExt.h"

static char const rcsid[] = "$Id: cpgIslandExt.c,v 1.1 2004/03/24 18:54:28 angie Exp $";

void cpgIslandExtStaticLoad(char **row, struct cpgIslandExt *ret)
/* Load a row from cpgIslandExt table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->length = sqlUnsigned(row[4]);
ret->cpgNum = sqlUnsigned(row[5]);
ret->gcNum = sqlUnsigned(row[6]);
ret->perCpg = atof(row[7]);
ret->perGc = atof(row[8]);
ret->obsExp = atof(row[9]);
}

struct cpgIslandExt *cpgIslandExtLoad(char **row)
/* Load a cpgIslandExt from row fetched with select * from cpgIslandExt
 * from database.  Dispose of this with cpgIslandExtFree(). */
{
struct cpgIslandExt *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->length = sqlUnsigned(row[4]);
ret->cpgNum = sqlUnsigned(row[5]);
ret->gcNum = sqlUnsigned(row[6]);
ret->perCpg = atof(row[7]);
ret->perGc = atof(row[8]);
ret->obsExp = atof(row[9]);
return ret;
}

struct cpgIslandExt *cpgIslandExtLoadAll(char *fileName) 
/* Load all cpgIslandExt from a whitespace-separated file.
 * Dispose of this with cpgIslandExtFreeList(). */
{
struct cpgIslandExt *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileRow(lf, row))
    {
    el = cpgIslandExtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cpgIslandExt *cpgIslandExtLoadAllByChar(char *fileName, char chopper) 
/* Load all cpgIslandExt from a chopper separated file.
 * Dispose of this with cpgIslandExtFreeList(). */
{
struct cpgIslandExt *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = cpgIslandExtLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct cpgIslandExt *cpgIslandExtCommaIn(char **pS, struct cpgIslandExt *ret)
/* Create a cpgIslandExt out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cpgIslandExt */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->length = sqlUnsignedComma(&s);
ret->cpgNum = sqlUnsignedComma(&s);
ret->gcNum = sqlUnsignedComma(&s);
ret->perCpg = sqlFloatComma(&s);
ret->perGc = sqlFloatComma(&s);
ret->obsExp = sqlFloatComma(&s);
*pS = s;
return ret;
}

void cpgIslandExtFree(struct cpgIslandExt **pEl)
/* Free a single dynamically allocated cpgIslandExt such as created
 * with cpgIslandExtLoad(). */
{
struct cpgIslandExt *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void cpgIslandExtFreeList(struct cpgIslandExt **pList)
/* Free a list of dynamically allocated cpgIslandExt's */
{
struct cpgIslandExt *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cpgIslandExtFree(&el);
    }
*pList = NULL;
}

void cpgIslandExtOutput(struct cpgIslandExt *el, FILE *f, char sep, char lastSep) 
/* Print out cpgIslandExt.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
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
fprintf(f, "%u", el->length);
fputc(sep,f);
fprintf(f, "%u", el->cpgNum);
fputc(sep,f);
fprintf(f, "%u", el->gcNum);
fputc(sep,f);
fprintf(f, "%f", el->perCpg);
fputc(sep,f);
fprintf(f, "%f", el->perGc);
fputc(sep,f);
fprintf(f, "%f", el->obsExp);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

