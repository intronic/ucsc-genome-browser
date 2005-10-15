/* mafFrames.c was originally generated by the autoSql program, which also 
 * generated mafFrames.h and mafFrames.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "mafFrames.h"

static char const rcsid[] = "$Id: mafFrames.c,v 1.3 2005/10/15 00:34:21 markd Exp $";

void mafFramesStaticLoad(char **row, struct mafFrames *ret)
/* Load a row from mafFrames table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->src = row[3];
ret->frame = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->name = row[6];
ret->prevFramePos = sqlSigned(row[7]);
ret->nextFramePos = sqlSigned(row[8]);
}

struct mafFrames *mafFramesLoad(char **row)
/* Load a mafFrames from row fetched with select * from mafFrames
 * from database.  Dispose of this with mafFramesFree(). */
{
struct mafFrames *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->src = cloneString(row[3]);
ret->frame = sqlUnsigned(row[4]);
strcpy(ret->strand, row[5]);
ret->name = cloneString(row[6]);
ret->prevFramePos = sqlSigned(row[7]);
ret->nextFramePos = sqlSigned(row[8]);
return ret;
}

struct mafFrames *mafFramesLoadAll(char *fileName) 
/* Load all mafFrames from a whitespace-separated file.
 * Dispose of this with mafFramesFreeList(). */
{
struct mafFrames *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = mafFramesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct mafFrames *mafFramesLoadAllByChar(char *fileName, char chopper) 
/* Load all mafFrames from a chopper separated file.
 * Dispose of this with mafFramesFreeList(). */
{
struct mafFrames *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = mafFramesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct mafFrames *mafFramesCommaIn(char **pS, struct mafFrames *ret)
/* Create a mafFrames out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mafFrames */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->src = sqlStringComma(&s);
ret->frame = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->name = sqlStringComma(&s);
ret->prevFramePos = sqlSignedComma(&s);
ret->nextFramePos = sqlSignedComma(&s);
*pS = s;
return ret;
}

void mafFramesFree(struct mafFrames **pEl)
/* Free a single dynamically allocated mafFrames such as created
 * with mafFramesLoad(). */
{
struct mafFrames *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->src);
freeMem(el->name);
freez(pEl);
}

void mafFramesFreeList(struct mafFrames **pList)
/* Free a list of dynamically allocated mafFrames's */
{
struct mafFrames *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    mafFramesFree(&el);
    }
*pList = NULL;
}

void mafFramesOutput(struct mafFrames *el, FILE *f, char sep, char lastSep) 
/* Print out mafFrames.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->src);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->frame);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->prevFramePos);
fputc(sep,f);
fprintf(f, "%d", el->nextFramePos);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

static char *createSql = 
    "CREATE TABLE %s ("
    "    bin smallint unsigned not null,"   /* bin column */
    "    chrom varchar(255) not null,"      /* Chromosome */
    "    chromStart int unsigned not null," /* Start range in chromosome */
    "    chromEnd int unsigned not null,"   /* End range in chromosome */
    "    src varchar(255) not null,"        /* Name of sequence source in MAF */
    "    frame tinyint unsigned not null,"  /* frame (0,1,2) for first base(+) or last bast(-) */
    "    strand char(1) not null,"          /* + or - */
    "    name varchar(255) not null,"       /* Name of gene used to define frame */
    "    prevEnd int not null,"             /* chromEnd of previous part of gene, or -1 if none */
    "    nextStart int not null,"           /* chromStart of next part of gene, or -1 if none */
    "    INDEX(chrom(%d),bin)"              /* used by range query */
    ")";
char *mafFramesGetSql(char *table, unsigned options, int chromIdxLen)
/* Get sql to create the table.  Returned string should be freed.  No options
 * are currently defined.*/
{
char sql[512];
safef(sql, sizeof(sql), createSql, table, chromIdxLen, chromIdxLen);
return cloneString(sql);
}
