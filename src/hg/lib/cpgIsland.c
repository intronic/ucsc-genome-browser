/* cpgIsland.c was originally generated by the autoSql program, which also 
 * generated cpgIsland.h and cpgIsland.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "cpgIsland.h"


void cpgIslandStaticLoad(char **row, struct cpgIsland *ret)
/* Load a row from cpgIsland table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->length = sqlUnsigned(row[4]);
ret->cpgNum = sqlUnsigned(row[5]);
ret->gcNum = sqlUnsigned(row[6]);
ret->perCpg = atof(row[7]);
ret->perGc = atof(row[8]);
}

struct cpgIsland *cpgIslandLoad(char **row)
/* Load a cpgIsland from row fetched with select * from cpgIsland
 * from database.  Dispose of this with cpgIslandFree(). */
{
struct cpgIsland *ret;

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
return ret;
}

struct cpgIsland *cpgIslandCommaIn(char **pS, struct cpgIsland *ret)
/* Create a cpgIsland out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cpgIsland */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->length = sqlUnsignedComma(&s);
ret->cpgNum = sqlUnsignedComma(&s);
ret->gcNum = sqlUnsignedComma(&s);
ret->perCpg = sqlSignedComma(&s);
ret->perGc = sqlSignedComma(&s);
*pS = s;
return ret;
}

void cpgIslandFree(struct cpgIsland **pEl)
/* Free a single dynamically allocated cpgIsland such as created
 * with cpgIslandLoad(). */
{
struct cpgIsland *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freez(pEl);
}

void cpgIslandFreeList(struct cpgIsland **pList)
/* Free a list of dynamically allocated cpgIsland's */
{
struct cpgIsland *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cpgIslandFree(&el);
    }
*pList = NULL;
}

void cpgIslandOutput(struct cpgIsland *el, FILE *f, char sep, char lastSep) 
/* Print out cpgIsland.  Separate fields with sep. Follow last field with lastSep. */
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
fputc(lastSep,f);
}

