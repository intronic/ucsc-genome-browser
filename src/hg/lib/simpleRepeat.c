/* simpleRepeat.c was originally generated by the autoSql program, which also 
 * generated simpleRepeat.h and simpleRepeat.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "simpleRepeat.h"

void simpleRepeatStaticLoad(char **row, struct simpleRepeat *ret)
/* Load a row from simpleRepeat table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->period = sqlUnsigned(row[4]);
ret->copyNum = atof(row[5]);
ret->consensusSize = sqlUnsigned(row[6]);
ret->perMatch = sqlUnsigned(row[7]);
ret->perIndel = sqlUnsigned(row[8]);
ret->score = sqlUnsigned(row[9]);
ret->A = sqlUnsigned(row[10]);
ret->C = sqlUnsigned(row[11]);
ret->T = sqlUnsigned(row[12]);
ret->G = sqlUnsigned(row[13]);
ret->entropy = atof(row[14]);
ret->sequence = row[15];
}

struct simpleRepeat *simpleRepeatLoad(char **row)
/* Load a simpleRepeat from row fetched with select * from simpleRepeat
 * from database.  Dispose of this with simpleRepeatFree(). */
{
struct simpleRepeat *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->period = sqlUnsigned(row[4]);
ret->copyNum = atof(row[5]);
ret->consensusSize = sqlUnsigned(row[6]);
ret->perMatch = sqlUnsigned(row[7]);
ret->perIndel = sqlUnsigned(row[8]);
ret->score = sqlUnsigned(row[9]);
ret->A = sqlUnsigned(row[10]);
ret->C = sqlUnsigned(row[11]);
ret->T = sqlUnsigned(row[12]);
ret->G = sqlUnsigned(row[13]);
ret->entropy = atof(row[14]);
ret->sequence = cloneString(row[15]);
return ret;
}

struct simpleRepeat *simpleRepeatCommaIn(char **pS, struct simpleRepeat *ret)
/* Create a simpleRepeat out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new simpleRepeat */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->period = sqlUnsignedComma(&s);
ret->copyNum = sqlSignedComma(&s);
ret->consensusSize = sqlUnsignedComma(&s);
ret->perMatch = sqlUnsignedComma(&s);
ret->perIndel = sqlUnsignedComma(&s);
ret->score = sqlUnsignedComma(&s);
ret->A = sqlUnsignedComma(&s);
ret->C = sqlUnsignedComma(&s);
ret->T = sqlUnsignedComma(&s);
ret->G = sqlUnsignedComma(&s);
ret->entropy = sqlSignedComma(&s);
ret->sequence = sqlStringComma(&s);
*pS = s;
return ret;
}

void simpleRepeatFree(struct simpleRepeat **pEl)
/* Free a single dynamically allocated simpleRepeat such as created
 * with simpleRepeatLoad(). */
{
struct simpleRepeat *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->sequence);
freez(pEl);
}

void simpleRepeatFreeList(struct simpleRepeat **pList)
/* Free a list of dynamically allocated simpleRepeat's */
{
struct simpleRepeat *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    simpleRepeatFree(&el);
    }
*pList = NULL;
}

void simpleRepeatOutput(struct simpleRepeat *el, FILE *f, char sep, char lastSep) 
/* Print out simpleRepeat.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->period, sep);
fputc(sep,f);
fprintf(f, "%f", el->copyNum, sep);
fputc(sep,f);
fprintf(f, "%u", el->consensusSize, sep);
fputc(sep,f);
fprintf(f, "%u", el->perMatch, sep);
fputc(sep,f);
fprintf(f, "%u", el->perIndel, sep);
fputc(sep,f);
fprintf(f, "%u", el->score, sep);
fputc(sep,f);
fprintf(f, "%u", el->A, sep);
fputc(sep,f);
fprintf(f, "%u", el->C, sep);
fputc(sep,f);
fprintf(f, "%u", el->T, sep);
fputc(sep,f);
fprintf(f, "%u", el->G, sep);
fputc(sep,f);
fprintf(f, "%f", el->entropy, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->sequence, lastSep);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

