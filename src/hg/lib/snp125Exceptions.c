/* snp125Exceptions.c was originally generated by the autoSql program, which also 
 * generated snp125Exceptions.h and snp125Exceptions.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "snp125Exceptions.h"

static char const rcsid[] = "$Id: snp125Exceptions.c,v 1.2 2006/01/27 22:43:08 heather Exp $";

void snp125ExceptionsStaticLoad(char **row, struct snp125Exceptions *ret)
/* Load a row from snp125Exceptions table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->exception = row[4];
}

struct snp125Exceptions *snp125ExceptionsLoad(char **row)
/* Load a snp125Exceptions from row fetched with select * from snp125Exceptions
 * from database.  Dispose of this with snp125ExceptionsFree(). */
{
struct snp125Exceptions *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->exception = cloneString(row[4]);
return ret;
}

struct snp125Exceptions *snp125ExceptionsLoadAll(char *fileName) 
/* Load all snp125Exceptions from a whitespace-separated file.
 * Dispose of this with snp125ExceptionsFreeList(). */
{
struct snp125Exceptions *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileRow(lf, row))
    {
    el = snp125ExceptionsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snp125Exceptions *snp125ExceptionsLoadAllByChar(char *fileName, char chopper) 
/* Load all snp125Exceptions from a chopper separated file.
 * Dispose of this with snp125ExceptionsFreeList(). */
{
struct snp125Exceptions *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = snp125ExceptionsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct snp125Exceptions *snp125ExceptionsCommaIn(char **pS, struct snp125Exceptions *ret)
/* Create a snp125Exceptions out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snp125Exceptions */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->exception = sqlStringComma(&s);
*pS = s;
return ret;
}

void snp125ExceptionsFree(struct snp125Exceptions **pEl)
/* Free a single dynamically allocated snp125Exceptions such as created
 * with snp125ExceptionsLoad(). */
{
struct snp125Exceptions *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->exception);
freez(pEl);
}

void snp125ExceptionsFreeList(struct snp125Exceptions **pList)
/* Free a list of dynamically allocated snp125Exceptions's */
{
struct snp125Exceptions *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    snp125ExceptionsFree(&el);
    }
*pList = NULL;
}

void snp125ExceptionsOutput(struct snp125Exceptions *el, FILE *f, char sep, char lastSep) 
/* Print out snp125Exceptions.  Separate fields with sep. Follow last field with lastSep. */
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
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->exception);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


void snp125ExceptionsTableCreate(struct sqlConnection *conn)
/* create a snp125Exceptions table */
{
char *createString =
"CREATE TABLE snp125Exceptions (\n"
"   chrom varchar(255) not null,\n"
"    chromStart int unsigned not null,\n"
"    chromEnd int unsigned not null,\n"
"    name varchar(255) not null,\n"
"    exception enum ('NegativeSize',\n"

"                    'ExactLocTypeWrongSize','BetweenLocTypeWrongSize',\n"
"		     'RangeLocTypeWrongSize','DeletionClassWrongSize',\n"
"                    'SingleClassWrongLocType',\n"
"                    'SingleClassTriAllelic', 'SingleClassQuadAllelic',\n"
"                    'SingleClassWrongObserved','DeletionClassWrongObserved',\n"
"                    'RefNCBINotInObserved','RefUCSCNotInObserved',\n"
"                    'AlignTwoPlaces','AlignThreePlaces','AlignFourPlusPlaces',\n"
"                    'BadAlignmentFlanks', 'StrandIssue')\n"

")\n";

sqlRemakeTable(conn, "snp125Exceptions", createString);
}
