/* genomicDups.c was originally generated by the autoSql program, which also 
 * generated genomicDups.h and genomicDups.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "genomicDups.h"

void genomicDupsStaticLoad(char **row, struct genomicDups *ret)
/* Load a row from genomicDups table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->otherChrom = row[6];
ret->otherStart = sqlUnsigned(row[7]);
ret->otherEnd = sqlUnsigned(row[8]);
ret->alignB = sqlUnsigned(row[9]);
ret->matchB = sqlUnsigned(row[10]);
ret->mismatchB = sqlUnsigned(row[11]);
ret->fracMatch = atof(row[12]);
ret->jcK = atof(row[13]);
}

struct genomicDups *genomicDupsLoad(char **row)
/* Load a genomicDups from row fetched with select * from genomicDups
 * from database.  Dispose of this with genomicDupsFree(). */
{
struct genomicDups *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->otherChrom = cloneString(row[6]);
ret->otherStart = sqlUnsigned(row[7]);
ret->otherEnd = sqlUnsigned(row[8]);
ret->alignB = sqlUnsigned(row[9]);
ret->matchB = sqlUnsigned(row[10]);
ret->mismatchB = sqlUnsigned(row[11]);
ret->fracMatch = atof(row[12]);
ret->jcK = atof(row[13]);
return ret;
}

struct genomicDups *genomicDupsCommaIn(char **pS, struct genomicDups *ret)
/* Create a genomicDups out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new genomicDups */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->otherChrom = sqlStringComma(&s);
ret->otherStart = sqlUnsignedComma(&s);
ret->otherEnd = sqlUnsignedComma(&s);
ret->alignB = sqlUnsignedComma(&s);
ret->matchB = sqlUnsignedComma(&s);
ret->mismatchB = sqlUnsignedComma(&s);
ret->fracMatch = sqlSignedComma(&s);
ret->jcK = sqlSignedComma(&s);
*pS = s;
return ret;
}

void genomicDupsFree(struct genomicDups **pEl)
/* Free a single dynamically allocated genomicDups such as created
 * with genomicDupsLoad(). */
{
struct genomicDups *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->otherChrom);
freez(pEl);
}

void genomicDupsFreeList(struct genomicDups **pList)
/* Free a list of dynamically allocated genomicDups's */
{
struct genomicDups *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    genomicDupsFree(&el);
    }
*pList = NULL;
}

void genomicDupsOutput(struct genomicDups *el, FILE *f, char sep, char lastSep) 
/* Print out genomicDups.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->score, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->otherChrom, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->otherStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->otherEnd, sep);
fputc(sep,f);
fprintf(f, "%u", el->alignB, sep);
fputc(sep,f);
fprintf(f, "%u", el->matchB, sep);
fputc(sep,f);
fprintf(f, "%u", el->mismatchB, sep);
fputc(sep,f);
fprintf(f, "%f", el->fracMatch, sep);
fputc(sep,f);
fprintf(f, "%f", el->jcK, lastSep);
fputc(lastSep,f);
}

