/* chainDb.c was originally generated by the autoSql program, which also 
 * generated chainDb.h and chainDb.sql.  This module links the database and
 * the RAM representation of objects. 
 *
 * This module was subsequently modified to blend better with the non-database
 * representation of chains.  This module really only deals with the chain
 * header in the database.  The blocks of the chain are loaded elsewhere,
 * and indeed in another table. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "chain.h"
#include "chainDb.h"
#include "hdb.h"

static char const rcsid[] = "$Id: chainDb.c,v 1.5 2003/06/27 21:38:44 braney Exp $";

void chainHeadStaticLoad(char **row, struct chain *ret)
/* Load a row from chain table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->score = atof(row[0]);
ret->tName = row[1];
ret->tSize = sqlUnsigned(row[2]);
ret->tStart = sqlUnsigned(row[3]);
ret->tEnd = sqlUnsigned(row[4]);
ret->qName = row[5];
ret->qSize = sqlUnsigned(row[6]);
ret->qStrand = row[7][0];
ret->qStart = sqlUnsigned(row[8]);
ret->qEnd = sqlUnsigned(row[9]);
ret->id = sqlUnsigned(row[10]);
}

struct chain *chainHeadLoad(char **row)
/* Load a chain from row fetched with select * from chain
 * from database.  Dispose of this with chainFree(). */
{
struct chain *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->score = atof(row[0]);
ret->tName = cloneString(row[1]);
ret->tSize = sqlUnsigned(row[2]);
ret->tStart = sqlUnsigned(row[3]);
ret->tEnd = sqlUnsigned(row[4]);
ret->qName = cloneString(row[5]);
ret->qSize = sqlUnsigned(row[6]);
ret->qStrand = row[7][0];
ret->qStart = sqlUnsigned(row[8]);
ret->qEnd = sqlUnsigned(row[9]);
ret->id = sqlUnsigned(row[10]);
return ret;
}

struct chain *chainHeadLoadAll(char *fileName) 
/* Load all chain from a tab-separated file.
 * Dispose of this with chainFreeList(). */
{
struct chain *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[11];

while (lineFileRow(lf, row))
    {
    el = chainHeadLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct chain *chainHeadCommaIn(char **pS, struct chain *ret)
/* Create a chain out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chain */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->score = sqlDoubleComma(&s);
ret->tName = sqlStringComma(&s);
ret->tSize = sqlUnsignedComma(&s);
ret->tStart = sqlUnsignedComma(&s);
ret->tEnd = sqlUnsignedComma(&s);
ret->qName = sqlStringComma(&s);
ret->qSize = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->qStrand), sizeof(ret->qStrand));
ret->qStart = sqlUnsignedComma(&s);
ret->qEnd = sqlUnsignedComma(&s);
ret->id = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void chainHeadOutput(struct chain *el, FILE *f, char sep, char lastSep) 
/* Print out chain.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%f", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->tSize);
fputc(sep,f);
fprintf(f, "%u", el->tStart);
fputc(sep,f);
fprintf(f, "%u", el->tEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qSize);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->qStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->qStart);
fputc(sep,f);
fprintf(f, "%u", el->qEnd);
fputc(sep,f);
fprintf(f, "%u", el->id);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void chainDbAddBlocks(struct chain *chain, char *track, struct sqlConnection *conn)
/* Add blocks to chain header. */
{
struct dyString *query = newDyString(1024);
struct sqlResult *sr = NULL;
char **row;
struct boxIn *b;
char fullName[64];

sprintf(fullName, "%s_%s", chain->tName, track);
if (!hTableExistsDb(hGetDb(), fullName))
    strcpy(fullName, track);
dyStringPrintf(query, 
	"select tStart,tEnd,qStart from %s_%sLink where chainId = %d",
	fullName);

sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    AllocVar(b);
    b->tStart = sqlUnsigned(row[0]);
    b->tEnd = sqlUnsigned(row[1]);
    b->qStart = sqlUnsigned(row[2]);
    b->qEnd = b->qStart + (b->tEnd - b->tStart);
    slAddHead(&chain->blockList, b);
    }
slReverse(&chain->blockList);
sqlFreeResult(&sr);
dyStringFree(&query);
}

