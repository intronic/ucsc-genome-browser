/* ggMrnaAli.c was originally generated by the autoSql program, which also 
 * generated ggMrnaAli.h and ggMrnaAli.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "ggMrnaAli.h"
#include "dnautil.h"

static char const rcsid[] = "$Id: ggMrnaAli.c,v 1.7 2004/04/29 20:50:20 sugnet Exp $";

void ggMrnaBlockStaticLoad(char **row, struct ggMrnaBlock *ret)
/* Load a row from ggMrnaBlock table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->qStart = sqlSigned(row[0]);
ret->qEnd = sqlSigned(row[1]);
ret->tStart = sqlSigned(row[2]);
ret->tEnd = sqlSigned(row[3]);
}

struct ggMrnaBlock *ggMrnaBlockLoad(char **row)
/* Load a ggMrnaBlock from row fetched with select * from ggMrnaBlock
 * from database.  Dispose of this with ggMrnaBlockFree(). */
{
struct ggMrnaBlock *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->qStart = sqlSigned(row[0]);
ret->qEnd = sqlSigned(row[1]);
ret->tStart = sqlSigned(row[2]);
ret->tEnd = sqlSigned(row[3]);
return ret;
}

struct ggMrnaBlock *ggMrnaBlockLoadAll(char *fileName) 
/* Load all ggMrnaBlock from a tab-separated file.
 * Dispose of this with ggMrnaBlockFreeList(). */
{
struct ggMrnaBlock *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = ggMrnaBlockLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ggMrnaBlock *ggMrnaBlockLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all ggMrnaBlock from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with ggMrnaBlockFreeList(). */
{
struct ggMrnaBlock *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = ggMrnaBlockLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void ggMrnaBlockSaveToDb(struct sqlConnection *conn, struct ggMrnaBlock *el, char *tableName, int updateSize)
/* Save ggMrnaBlock as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use ggMrnaBlockSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( %d,%d,%d,%d)", 
	tableName,  el->qStart,  el->qEnd,  el->tStart,  el->tEnd);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void ggMrnaBlockSaveToDbEscaped(struct sqlConnection *conn, struct ggMrnaBlock *el, char *tableName, int updateSize)
/* Save ggMrnaBlock as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than ggMrnaBlockSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( %d,%d,%d,%d)", 
	tableName, el->qStart , el->qEnd , el->tStart , el->tEnd );
sqlUpdate(conn, update->string);
freeDyString(&update);
}

struct ggMrnaBlock *ggMrnaBlockCommaIn(char **pS, struct ggMrnaBlock *ret)
/* Create a ggMrnaBlock out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ggMrnaBlock */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->qStart = sqlSignedComma(&s);
ret->qEnd = sqlSignedComma(&s);
ret->tStart = sqlSignedComma(&s);
ret->tEnd = sqlSignedComma(&s);
*pS = s;
return ret;
}

void ggMrnaBlockFree(struct ggMrnaBlock **pEl)
/* Free a single dynamically allocated ggMrnaBlock such as created
 * with ggMrnaBlockLoad(). */
{
struct ggMrnaBlock *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void ggMrnaBlockFreeList(struct ggMrnaBlock **pList)
/* Free a list of dynamically allocated ggMrnaBlock's */
{
struct ggMrnaBlock *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ggMrnaBlockFree(&el);
    }
*pList = NULL;
}

void ggMrnaBlockOutput(struct ggMrnaBlock *el, FILE *f, char sep, char lastSep) 
/* Print out ggMrnaBlock.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%d", el->qStart);
fputc(sep,f);
fprintf(f, "%d", el->qEnd);
fputc(sep,f);
fprintf(f, "%d", el->tStart);
fputc(sep,f);
fprintf(f, "%d", el->tEnd);
fputc(lastSep,f);
}

struct ggMrnaAli *ggMrnaAliLoad(char **row)
/* Load a ggMrnaAli from row fetched with select * from ggMrnaAli
 * from database.  Dispose of this with ggMrnaAliFree(). */
{
struct ggMrnaAli *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->blockCount = sqlSigned(row[11]);
ret->tName = cloneString(row[0]);
ret->tStart = sqlSigned(row[1]);
ret->tEnd = sqlSigned(row[2]);
strcpy(ret->strand, row[3]);
ret->qName = cloneString(row[4]);
ret->qStart = sqlSigned(row[5]);
ret->qEnd = sqlSigned(row[6]);
ret->baseCount = sqlUnsigned(row[7]);
ret->orientation = sqlSigned(row[8]);
ret->hasIntrons = sqlSigned(row[9]);
ret->milliScore = sqlSigned(row[10]);
s = row[12];
for (i=0; i<ret->blockCount; ++i)
    {
    s = sqlEatChar(s, '{');
    slSafeAddHead(&ret->blocks, ggMrnaBlockCommaIn(&s, NULL));
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
slReverse(&ret->blocks);
return ret;
}

struct ggMrnaAli *ggMrnaAliLoadAll(char *fileName) 
/* Load all ggMrnaAli from a tab-separated file.
 * Dispose of this with ggMrnaAliFreeList(). */
{
struct ggMrnaAli *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[13];

while (lineFileRow(lf, row))
    {
    el = ggMrnaAliLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct ggMrnaAli *ggMrnaAliLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all ggMrnaAli from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with ggMrnaAliFreeList(). */
{
struct ggMrnaAli *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = ggMrnaAliLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void ggMrnaAliSaveToDb(struct sqlConnection *conn, struct ggMrnaAli *el, char *tableName, int updateSize)
/* Save ggMrnaAli as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use ggMrnaAliSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( '%s',%d,%d,'%s','%s',%d,%d,%u,%d,%d,%d,%d, NULL )", 
	tableName,  el->tName,  el->tStart,  el->tEnd,  el->strand,  el->qName,  el->qStart,  el->qEnd,  el->baseCount,  el->orientation,  el->hasIntrons,  el->milliScore,  el->blockCount);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void ggMrnaAliSaveToDbEscaped(struct sqlConnection *conn, struct ggMrnaAli *el, char *tableName, int updateSize)
/* Save ggMrnaAli as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than ggMrnaAliSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *tName, *strand, *qName;
tName = sqlEscapeString(el->tName);
strand = sqlEscapeString(el->strand);
qName = sqlEscapeString(el->qName);

dyStringPrintf(update, "insert into %s values ( '%s',%d,%d,'%s','%s',%d,%d,%u,%d,%d,%d,%d, NULL )", 
	tableName,  tName, el->tStart , el->tEnd ,  strand,  qName, el->qStart , el->qEnd , el->baseCount , el->orientation , el->hasIntrons , el->milliScore , el->blockCount );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&tName);
freez(&strand);
freez(&qName);
}

struct ggMrnaAli *ggMrnaAliCommaIn(char **pS, struct ggMrnaAli *ret)
/* Create a ggMrnaAli out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ggMrnaAli */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->tName = sqlStringComma(&s);
ret->tStart = sqlSignedComma(&s);
ret->tEnd = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->qName = sqlStringComma(&s);
ret->qStart = sqlSignedComma(&s);
ret->qEnd = sqlSignedComma(&s);
ret->baseCount = sqlUnsignedComma(&s);
ret->orientation = sqlSignedComma(&s);
ret->hasIntrons = sqlSignedComma(&s);
ret->milliScore = sqlSignedComma(&s);
ret->blockCount = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
for (i=0; i<ret->blockCount; ++i)
    {
    s = sqlEatChar(s, '{');
    slSafeAddHead(&ret->blocks, ggMrnaBlockCommaIn(&s,NULL));
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
slReverse(&ret->blocks);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void ggMrnaAliFree(struct ggMrnaAli **pEl)
/* Free a single dynamically allocated ggMrnaAli such as created
 * with ggMrnaAliLoad(). */
{
struct ggMrnaAli *el;

if ((el = *pEl) == NULL) return;
freeMem(el->tName);
freeMem(el->qName);
ggMrnaBlockFreeList(&el->blocks);
freez(pEl);
}

void ggMrnaAliFreeList(struct ggMrnaAli **pList)
/* Free a list of dynamically allocated ggMrnaAli's */
{
struct ggMrnaAli *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    ggMrnaAliFree(&el);
    }
*pList = NULL;
}

void ggMrnaAliOutput(struct ggMrnaAli *el, FILE *f, char sep, char lastSep) 
/* Print out ggMrnaAli.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->tStart);
fputc(sep,f);
fprintf(f, "%d", el->tEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->qStart);
fputc(sep,f);
fprintf(f, "%d", el->qEnd);
fputc(sep,f);
fprintf(f, "%u", el->baseCount);
fputc(sep,f);
fprintf(f, "%d", el->orientation);
fputc(sep,f);
fprintf(f, "%d", el->hasIntrons);
fputc(sep,f);
fprintf(f, "%d", el->milliScore);
fputc(sep,f);
fprintf(f, "%d", el->blockCount);
fputc(sep,f);
/* Loading ggMrnaBlock list. */
    {
    struct ggMrnaBlock *it = el->blocks;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->blockCount; ++i)
        {
        fputc('{',f);
        ggMrnaBlockCommaOut(it,f);
        it = it->next;
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */



struct ggMrnaAli *pslToGgMrnaAli(struct psl *psl, char *chrom, unsigned int chromStart,
				 unsigned int chromEnd, struct dnaSeq *genoSeq)
/* Convert from psl format of alignment to ma format.  Return
 * NULL if no introns in psl. */
{
struct ggMrnaAli *ma;
int localIx;
int i;
int blockCount;
struct ggMrnaBlock *blocks, *block;
int iOrientation; 
char *strand;

/* convert psl to our local genoSeq coordinates */
pslTargetOffset(psl, -1*chromStart);

/* Figure out orientation and direction based on introns. */
iOrientation = pslIntronOrientation(psl, genoSeq, 0);
strand = psl->strand;
if (iOrientation < 0)
    strand = "-";
else if (iOrientation > 0)
    strand = "+";

AllocVar(ma);
ma->orientation = iOrientation;
ma->qName = cloneString(psl->qName);
ma->qStart = psl->qStart;
ma->qEnd = psl->qEnd;
ma->baseCount = psl->qSize;
ma->milliScore = psl->match + psl->repMatch - psl->misMatch - (psl->blockCount-1)*2;
snprintf(ma->strand, sizeof(ma->strand), "%s", strand);
ma->hasIntrons = (iOrientation == 0 ? FALSE : TRUE);
ma->tName = cloneString(psl->tName);
ma->tStart = psl->tStart;
ma->tEnd = psl->tEnd;
ma->blockCount = blockCount = psl->blockCount;
ma->blocks = AllocArray(blocks, blockCount);

for (i = 0; i<blockCount; ++i)
    {
    int bSize = psl->blockSizes[i];
    int qStart = psl->qStarts[i];
    int tStart = psl->tStarts[i];
    block = blocks+i;
    block->qStart = qStart;
    block->qEnd = qStart + bSize;
    block->tStart = tStart;
    block->tEnd = tStart + bSize;
    }
pslTargetOffset(psl, chromStart);
return ma;
}

boolean ggMrnaAliMergeBlocks(struct ggMrnaAli *ma, int maxGap)
/* Merge blocks that looks to be separated by small amounts
 * of sequencing noise only. 2 is a good value for maxGap */
{
struct ggMrnaBlock *readBlock, *writeBlock, *nextBlock;
int mergedCount = 1;
int i;
boolean mergedSome = FALSE;

readBlock = writeBlock = ma->blocks;
for (i=1; i<ma->blockCount; ++i)
    {
    ++readBlock;
    if (intAbs(readBlock->qStart - writeBlock->qEnd) <= maxGap &&
        intAbs(readBlock->tStart - writeBlock->tEnd) <= maxGap)
	{
	ma->baseCount += readBlock->tStart - writeBlock->tEnd; /* If we've added bases keep track. */
	writeBlock->qEnd = readBlock->qEnd;
	writeBlock->tEnd = readBlock->tEnd;
	mergedSome = TRUE;
	}
    else
	{
	++writeBlock;
	*writeBlock = *readBlock;
	++mergedCount;
	}
    }
ma->blockCount = mergedCount;
return mergedSome;
}

int cmpGgMrnaAliTargetStart(const void *va, const void *vb)
/* Compare two ggMrnaAli based on their  strand, tEnd. */
{
const struct ggMrnaAli *a = *((struct ggMrnaAli **)va);
const struct ggMrnaAli *b = *((struct ggMrnaAli **)vb);
if(differentString(a->tName, b->tName))
    return strcmp(a->tName, b->tName);
if(differentString(a->strand, b->strand))
    return strcmp(a->strand, b->strand);
return (a->tStart - b->tStart);
}

void ggMrnaAliBedOut(struct ggMrnaAli *ma, FILE *f)
/* write out the target blocks in simple bed format, one bed per block */
{
int i;
for(i=0; i < ma->blockCount; i++)
    {
    fprintf(f, "%s\t%d\t%d\t%s\t%d\t%s\n", ma->tName, ma->blocks[i].tStart + ma->tStart, ma->blocks[i].tEnd + ma->tStart, 
	    ma->qName, (ma->hasIntrons == TRUE ? 1000 : 500 ), ma->strand);
    }
}

void ggMrnaAliBed12Out(struct ggMrnaAli *ma, FILE *f)
/* Write out the target blocks as a linked feature bed format. */
{
int i;
fprintf(f, "%s\t%d\t%d\t%s\t%d\t%s\t%d\t%d\t0\t%d\t", 
	ma->tName, ma->tStart, ma->tEnd, ma->qName, 1000, ma->strand,
	ma->tStart, ma->tEnd, ma->blockCount);
for(i=0; i< ma->blockCount; i++)
    fprintf(f, "%d,", (ma->blocks[i].tEnd - ma->blocks[i].tStart));
fprintf(f,"\t");
for(i=0; i< ma->blockCount; i++)
    fprintf(f, "%d,", (ma->blocks[i].tStart - ma->tStart));
fprintf(f,"\n");
}

struct ggMrnaAli *pslListToGgMrnaAliList(struct psl *pslList, char *chrom, unsigned int chromStart, 
					 unsigned int chromEnd, struct dnaSeq *genoSeq, int maxGap)
/* create a ggMrnaAli list from a psl list merging gaps below size maxGap */
{
struct psl *psl = NULL;
struct ggMrnaAli *maList=NULL, *ma=NULL;
for(psl = pslList; psl != NULL; psl = psl->next)
    {
    ma = pslToGgMrnaAli(psl, chrom, chromStart, chromEnd, genoSeq);
    if(ma != NULL)
	{
	ggMrnaAliMergeBlocks(ma, maxGap);
	slAddHead(&maList, ma);
	}
    }
slReverse(&maList);
return maList;
}

struct ggMrnaInput *ggMrnaInputFromAlignments(struct ggMrnaAli *maList, struct dnaSeq *genoSeq)
/* wrap a ggMrnaInput around some alignments */
{
struct ggMrnaInput *mi = NULL;
assert(maList);
AllocVar(mi);
mi->tName = maList->tName;
mi->tStart = maList->tStart;
mi->tEnd = maList->tEnd;
snprintf(mi->strand, sizeof(mi->strand), "%s", maList->strand);
mi->genoSeq = genoSeq;
mi->maList = maList;
return mi;
}
