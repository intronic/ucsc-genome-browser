/* snp126.c was originally generated by the autoSql program, which also 
 * generated snp126.h and snp126.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "snp126.h"

static char const rcsid[] = "$Id: snp126.c,v 1.1 2006/06/01 18:58:50 heather Exp $";

void snp126StaticLoad(char **row, struct snp126 *ret)
/* Load a row from snp126 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->strand = row[5];
ret->refNCBI = row[6];
ret->refUCSC = row[7];
ret->observed = row[8];
ret->molType = row[9];
ret->class = row[10];
ret->valid = row[11];
ret->avHet = atof(row[12]);
ret->avHetSE = atof(row[13]);
ret->func = row[14];
ret->locType = row[15];
ret->weight = sqlUnsigned(row[16]);
}

struct snp126 *snp126Load(char **row)
/* Load a snp126 from row fetched with select * from snp126
 * from database.  Dispose of this with snp126Free(). */
{
struct snp126 *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->strand = cloneString(row[5]);
ret->refNCBI = cloneString(row[6]);
ret->refUCSC = cloneString(row[7]);
ret->observed = cloneString(row[8]);
ret->molType = cloneString(row[9]);
ret->class = cloneString(row[10]);
ret->valid = cloneString(row[11]);
ret->avHet = atof(row[12]);
ret->avHetSE = atof(row[13]);
ret->func = cloneString(row[14]);
ret->locType = cloneString(row[15]);
ret->weight = sqlUnsigned(row[16]);
return ret;
}

struct snp126 *snp126LoadAll(char *fileName) 
/* Load all snp126 from a whitespace-separated file.
 * Dispose of this with snp126FreeList(). */
{
struct snp126 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileRow(lf, row))
    {
    el = snp126Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snp126 *snp126LoadAllByChar(char *fileName, char chopper) 
/* Load all snp126 from a chopper separated file.
 * Dispose of this with snp126FreeList(). */
{
struct snp126 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = snp126Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snp126 *snp126CommaIn(char **pS, struct snp126 *ret)
/* Create a snp126 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snp126 */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
ret->strand = sqlStringComma(&s);
ret->refNCBI = sqlStringComma(&s);
ret->refUCSC = sqlStringComma(&s);
ret->observed = sqlStringComma(&s);
ret->molType = sqlStringComma(&s);
ret->class = sqlStringComma(&s);
ret->valid = sqlStringComma(&s);
ret->avHet = sqlFloatComma(&s);
ret->avHetSE = sqlFloatComma(&s);
ret->func = sqlStringComma(&s);
ret->locType = sqlStringComma(&s);
ret->weight = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void snp126Free(struct snp126 **pEl)
/* Free a single dynamically allocated snp126 such as created
 * with snp126Load(). */
{
struct snp126 *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->strand);
freeMem(el->refNCBI);
freeMem(el->refUCSC);
freeMem(el->observed);
freeMem(el->molType);
freeMem(el->class);
freeMem(el->valid);
freeMem(el->func);
freeMem(el->locType);
freez(pEl);
}

void snp126FreeList(struct snp126 **pList)
/* Free a list of dynamically allocated snp126's */
{
struct snp126 *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    snp126Free(&el);
    }
*pList = NULL;
}

void snp126Output(struct snp126 *el, FILE *f, char sep, char lastSep) 
/* Print out snp126.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refNCBI);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refUCSC);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->observed);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->molType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->class);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->valid);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%g", el->avHet);
fputc(sep,f);
fprintf(f, "%g", el->avHetSE);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->func);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->locType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->weight);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


void snp126TableCreate(struct sqlConnection *conn, char *tableName)
/* create a snp126 table */
{
char *createString =
"CREATE TABLE %s (\n"
"    bin           smallint(5) unsigned not null,\n"
"    chrom         varchar(15) not null,\n"
"    chromStart    int(10) unsigned not null,\n"
"    chromEnd      int(10) unsigned not null,\n"
"    name          varchar(15) not null,\n"
"    score         smallint(5) unsigned not null,\n"
"    strand        enum('?','+','-') default '?' not null,\n"
"    refNCBI       blob not null,\n"
"    refUCSC       blob not null,\n"
"    observed      varchar(255) not null,\n"
"    molType       enum('unknown', 'genomic', 'cDNA') DEFAULT 'unknown' not null,\n"
"    class         enum('unknown', 'single', 'in-del', 'het', 'microsatellite',"
"                  'named', 'no var', 'mixed', 'mnp', 'insertion', 'deletion') \n"
"                  DEFAULT 'unknown' NOT NULL,\n"
"    valid         set('unknown', 'by-frequency', 'by-cluster', 'by-submitter', \n"
"                  'by-2hit-2allele', 'by-hapmap') \n"
"                  DEFAULT 'unknown' NOT NULL,\n"
"    avHet 	   float not null,\n"
"    avHetSE 	   float not null,\n"
"    func          set( 'unknown', 'locus', 'coding', 'coding-synon', 'coding-nonsynon', \n"
"                  'untranslated', 'intron','splice-site', 'cds-reference') \n"
"                  DEFAULT 'unknown' NOT NULL,\n"
"    locType       enum ('unknown', 'range', 'exact', 'between',\n"
"                  'rangeInsertion', 'rangeSubstitution', 'rangeDeletion') DEFAULT 'unknown' NOT NULL\n,"
"    weight        int not null\n"
")\n";

struct dyString *dy = newDyString(1024);
dyStringPrintf(dy, createString, tableName);
sqlRemakeTable(conn, tableName, dy->string);
dyStringFree(&dy);
}

int snp126Cmp(const void *va, const void *vb)
{
const struct snp126 *a = *((struct snp126 **)va);
const struct snp126 *b = *((struct snp126 **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}
