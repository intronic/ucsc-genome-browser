/* encodePeak.c was originally generated by the autoSql program, which also 
 * generated encodePeak.h and encodePeak.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hdb.h"
#include "encodePeak.h"

static char const rcsid[] = "$Id: encodePeak.c,v 1.2 2008/08/25 21:35:48 aamp Exp $";

struct encodePeak *encodePeakLoad(char **row)
/* Load a encodePeak from row fetched with select * from encodePeak
 * from database.  Dispose of this with encodePeakFree(). */
{
struct encodePeak *ret;

AllocVar(ret);
ret->blockCount = sqlUnsigned(row[6]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->signalValue = sqlFloat(row[3]);
ret->pValue = sqlFloat(row[4]);
ret->peak = sqlSigned(row[5]);
{
int sizeOne;
sqlUnsignedDynamicArray(row[7], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
}
{
int sizeOne;
sqlUnsignedDynamicArray(row[8], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
}
return ret;
}

struct encodePeak *encodePeakLoadAll(char *fileName) 
/* Load all encodePeak from a whitespace-separated file.
 * Dispose of this with encodePeakFreeList(). */
{
struct encodePeak *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = encodePeakLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodePeak *encodePeakLoadAllByChar(char *fileName, char chopper) 
/* Load all encodePeak from a chopper separated file.
 * Dispose of this with encodePeakFreeList(). */
{
struct encodePeak *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodePeakLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodePeak *encodePeakCommaIn(char **pS, struct encodePeak *ret)
/* Create a encodePeak out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodePeak */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->signalValue = sqlFloatComma(&s);
ret->pValue = sqlFloatComma(&s);
ret->peak = sqlSignedComma(&s);
ret->blockCount = sqlUnsignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->blockSizes, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->blockSizes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->chromStarts, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->chromStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void encodePeakFree(struct encodePeak **pEl)
/* Free a single dynamically allocated encodePeak such as created
 * with encodePeakLoad(). */
{
struct encodePeak *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freez(pEl);
}

void encodePeakFreeList(struct encodePeak **pList)
/* Free a list of dynamically allocated encodePeak's */
{
struct encodePeak *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodePeakFree(&el);
    }
*pList = NULL;
}

void encodePeakOutput(struct encodePeak *el, FILE *f, char sep, char lastSep) 
/* Print out encodePeak.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
fprintf(f, "%g", el->signalValue);
fputc(sep,f);
fprintf(f, "%g", el->pValue);
fputc(sep,f);
fprintf(f, "%d", el->peak);
fputc(sep,f);
fprintf(f, "%u", el->blockCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->blockSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%u", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

int encodePeakNumFields(char *trackName)
/* Just quickly count th number of fields. */
{
struct sqlConnection *conn = hAllocConn();
struct slName *fieldNames = sqlFieldNames(conn, trackName);
int numFields = slCount(fieldNames);
hFreeConn(&conn);
if (sameString(fieldNames->name, "bin"))
    numFields--;
slFreeList(&fieldNames);
return numFields;
}

