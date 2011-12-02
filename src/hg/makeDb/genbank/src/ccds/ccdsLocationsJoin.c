/* ccdsLocationsJoin.c was originally generated by the autoSql program, which also 
 * generated ccdsLocationsJoin.h and ccdsLocationsJoin.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "ccdsLocationsJoin.h"


void ccdsLocationsJoinStaticLoad(char **row, struct ccdsLocationsJoin *ret)
/* Load a row from ccdsLocationsJoin table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->ccds_uid = sqlSigned(row[0]);
ret->ccds_version = sqlSigned(row[1]);
strcpy(ret->chrom, row[2]);
strcpy(ret->strand, row[3]);
ret->start = sqlSigned(row[4]);
ret->stop = sqlSigned(row[5]);
}

struct ccdsLocationsJoin *ccdsLocationsJoinLoad(char **row)
/* Load a ccdsLocationsJoin from row fetched with select * from ccdsLocationsJoin
 * from database.  Dispose of this with ccdsLocationsJoinFree(). */
{
struct ccdsLocationsJoin *ret;

AllocVar(ret);
ret->ccds_uid = sqlSigned(row[0]);
ret->ccds_version = sqlSigned(row[1]);
strcpy(ret->chrom, row[2]);
strcpy(ret->strand, row[3]);
ret->start = sqlSigned(row[4]);
ret->stop = sqlSigned(row[5]);
return ret;
}

struct ccdsLocationsJoin *ccdsLocationsJoinLoadAll(char *fileName) 
/* Load all ccdsLocationsJoin from a whitespace-separated file.
 * Dispose of this with ccdsLocationsJoinFreeList(). */
{
struct ccdsLocationsJoin *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = ccdsLocationsJoinLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ccdsLocationsJoin *ccdsLocationsJoinLoadAllByChar(char *fileName, char chopper) 
/* Load all ccdsLocationsJoin from a chopper separated file.
 * Dispose of this with ccdsLocationsJoinFreeList(). */
{
struct ccdsLocationsJoin *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = ccdsLocationsJoinLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ccdsLocationsJoin *ccdsLocationsJoinCommaIn(char **pS, struct ccdsLocationsJoin *ret)
/* Create a ccdsLocationsJoin out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ccdsLocationsJoin */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->ccds_uid = sqlSignedComma(&s);
ret->ccds_version = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->chrom, sizeof(ret->chrom));
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->start = sqlSignedComma(&s);
ret->stop = sqlSignedComma(&s);
*pS = s;
return ret;
}

void ccdsLocationsJoinFree(struct ccdsLocationsJoin **pEl)
/* Free a single dynamically allocated ccdsLocationsJoin such as created
 * with ccdsLocationsJoinLoad(). */
{
struct ccdsLocationsJoin *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void ccdsLocationsJoinFreeList(struct ccdsLocationsJoin **pList)
/* Free a list of dynamically allocated ccdsLocationsJoin's */
{
struct ccdsLocationsJoin *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ccdsLocationsJoinFree(&el);
    }
*pList = NULL;
}

void ccdsLocationsJoinOutput(struct ccdsLocationsJoin *el, FILE *f, char sep, char lastSep) 
/* Print out ccdsLocationsJoin.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->ccds_uid);
fputc(sep,f);
fprintf(f, "%d", el->ccds_version);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->start);
fputc(sep,f);
fprintf(f, "%d", el->stop);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

