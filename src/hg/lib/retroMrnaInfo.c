/* retroMrnaInfo.c was originally generated by the autoSql program, which also 
 * generated retroMrnaInfo.h and retroMrnaInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "retroMrnaInfo.h"

static char const rcsid[] = "$Id: retroMrnaInfo.c,v 1.1 2007/03/01 00:01:41 baertsch Exp $";

struct retroMrnaInfo *retroMrnaInfoLoad(char **row)
/* Load a retroMrnaInfo from row fetched with select * from retroMrnaInfo
 * from database.  Dispose of this with retroMrnaInfoFree(). */
{
struct retroMrnaInfo *ret;

AllocVar(ret);
ret->blockCount = sqlSigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
{
int sizeOne;
sqlSignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
}
{
int sizeOne;
sqlSignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
}
ret->retroExonCount = sqlSigned(row[12]);
ret->axtScore = sqlSigned(row[13]);
ret->type = cloneString(row[14]);
ret->gChrom = cloneString(row[15]);
ret->gStart = sqlSigned(row[16]);
ret->gEnd = sqlSigned(row[17]);
safecpy(ret->gStrand, sizeof(ret->gStrand), row[18]);
ret->parentSpliceCount = sqlUnsigned(row[19]);
ret->geneOverlap = sqlUnsigned(row[20]);
ret->polyA = sqlUnsigned(row[21]);
ret->polyAstart = sqlSigned(row[22]);
ret->exonCover = sqlSigned(row[23]);
ret->intronCount = sqlUnsigned(row[24]);
ret->bestAliCount = sqlUnsigned(row[25]);
ret->matches = sqlUnsigned(row[26]);
ret->qSize = sqlUnsigned(row[27]);
ret->qEnd = sqlUnsigned(row[28]);
ret->tReps = sqlUnsigned(row[29]);
ret->overlapRhesus = sqlSigned(row[30]);
ret->overlapMouse = sqlSigned(row[31]);
ret->coverage = sqlUnsigned(row[32]);
ret->label = sqlSigned(row[33]);
ret->milliBad = sqlUnsigned(row[34]);
ret->oldScore = sqlUnsigned(row[35]);
ret->oldIntronCount = sqlSigned(row[36]);
ret->processedIntrons = sqlSigned(row[37]);
ret->conservedSpliceSites = sqlSigned(row[38]);
ret->maxOverlap = sqlSigned(row[39]);
ret->refSeq = cloneString(row[40]);
ret->rStart = sqlSigned(row[41]);
ret->rEnd = sqlSigned(row[42]);
ret->mgc = cloneString(row[43]);
ret->mStart = sqlSigned(row[44]);
ret->mEnd = sqlSigned(row[45]);
ret->kgName = cloneString(row[46]);
ret->kStart = sqlSigned(row[47]);
ret->kEnd = sqlSigned(row[48]);
ret->overName = cloneString(row[49]);
ret->overStart = sqlSigned(row[50]);
ret->overExonCover = sqlSigned(row[51]);
safecpy(ret->overStrand, sizeof(ret->overStrand), row[52]);
ret->overlapDog = sqlSigned(row[53]);
ret->posConf = sqlFloat(row[54]);
ret->polyAlen = sqlUnsigned(row[55]);
ret->kaku = sqlFloat(row[56]);
return ret;
}
struct retroMrnaInfo *retroMrnaInfo56Load(char **row)
/* Load a retroMrnaInfo from row fetched with select * from retroMrnaInfo
 * from database.  Dispose of this with retroMrnaInfoFree(). */
{
struct retroMrnaInfo *ret;

AllocVar(ret);
ret->blockCount = sqlSigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
safecpy(ret->strand, sizeof(ret->strand), row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
{
int sizeOne;
sqlSignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
}
{
int sizeOne;
sqlSignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
}
ret->retroExonCount = sqlSigned(row[12]);
ret->axtScore = sqlSigned(row[13]);
ret->type = cloneString(row[14]);
ret->gChrom = cloneString(row[15]);
ret->gStart = sqlSigned(row[16]);
ret->gEnd = sqlSigned(row[17]);
safecpy(ret->gStrand, sizeof(ret->gStrand), row[18]);
ret->parentSpliceCount = sqlUnsigned(row[19]);
ret->geneOverlap = sqlUnsigned(row[20]);
ret->polyA = sqlUnsigned(row[21]);
ret->polyAstart = sqlSigned(row[22]);
ret->exonCover = sqlSigned(row[23]);
ret->intronCount = sqlUnsigned(row[24]);
ret->bestAliCount = sqlUnsigned(row[25]);
ret->matches = sqlUnsigned(row[26]);
ret->qSize = sqlUnsigned(row[27]);
ret->qEnd = sqlUnsigned(row[28]);
ret->tReps = sqlUnsigned(row[29]);
ret->overlapRhesus = sqlSigned(row[30]);
ret->overlapMouse = sqlSigned(row[31]);
ret->coverage = sqlUnsigned(row[32]);
ret->label = sqlSigned(row[33]);
ret->milliBad = sqlUnsigned(row[34]);
ret->oldScore = sqlUnsigned(row[35]);
ret->oldIntronCount = sqlSigned(row[36]);
ret->processedIntrons = sqlSigned(row[37]);
ret->conservedSpliceSites = sqlSigned(row[38]);
ret->maxOverlap = sqlSigned(row[39]);
ret->refSeq = cloneString(row[40]);
ret->rStart = sqlSigned(row[41]);
ret->rEnd = sqlSigned(row[42]);
ret->mgc = cloneString(row[43]);
ret->mStart = sqlSigned(row[44]);
ret->mEnd = sqlSigned(row[45]);
ret->kgName = cloneString(row[46]);
ret->kStart = sqlSigned(row[47]);
ret->kEnd = sqlSigned(row[48]);
ret->overName = cloneString(row[49]);
ret->overStart = sqlSigned(row[50]);
ret->overExonCover = sqlSigned(row[51]);
safecpy(ret->overStrand, sizeof(ret->overStrand), row[52]);
ret->overlapDog = sqlSigned(row[53]);
ret->posConf = sqlFloat(row[54]);
ret->polyAlen = sqlUnsigned(row[55]);
return ret;
}


struct retroMrnaInfo *retroMrnaInfoLoadAll(char *fileName) 
/* Load all retroMrnaInfo from a whitespace-separated file.
 * Dispose of this with retroMrnaInfoFreeList(). */
{
struct retroMrnaInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[57];

while (lineFileRow(lf, row))
    {
    el = retroMrnaInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct retroMrnaInfo *retroMrnaInfoLoadAllByChar(char *fileName, char chopper) 
/* Load all retroMrnaInfo from a chopper separated file.
 * Dispose of this with retroMrnaInfoFreeList(). */
{
struct retroMrnaInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[57];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = retroMrnaInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct retroMrnaInfo *retroMrnaInfoLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all retroMrnaInfo from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with retroMrnaInfoFreeList(). */
{
struct retroMrnaInfo *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = retroMrnaInfoLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void retroMrnaInfoSaveToDb(struct sqlConnection *conn, struct retroMrnaInfo *el, char *tableName, int updateSize)
/* Save retroMrnaInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use retroMrnaInfoSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
char  *blockSizesArray, *chromStartsArray;
blockSizesArray = sqlSignedArrayToString(el->blockSizes, el->blockCount);
chromStartsArray = sqlSignedArrayToString(el->chromStarts, el->blockCount);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s',%u,%u,%u,%d,'%s','%s',%d,%d,'%s','%s',%d,%d,'%s',%u,%u,%u,%d,%d,%u,%u,%u,%u,%u,%u,%d,%d,%u,%d,%u,%u,%d,%d,%d,%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,%g,%u,%g)", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->thickStart,  el->thickEnd,  el->reserved,  el->blockCount,  blockSizesArray ,  chromStartsArray ,  el->retroExonCount,  el->axtScore,  el->type,  el->gChrom,  el->gStart,  el->gEnd,  el->gStrand,  el->parentSpliceCount,  el->geneOverlap,  el->polyA,  el->polyAstart,  el->exonCover,  el->intronCount,  el->bestAliCount,  el->matches,  el->qSize,  el->qEnd,  el->tReps,  el->overlapRhesus,  el->overlapMouse,  el->coverage,  el->label,  el->milliBad,  el->oldScore,  el->oldIntronCount,  el->processedIntrons,  el->conservedSpliceSites,  el->maxOverlap,  el->refSeq,  el->rStart,  el->rEnd,  el->mgc,  el->mStart,  el->mEnd,  el->kgName,  el->kStart,  el->kEnd,  el->overName,  el->overStart,  el->overExonCover,  el->overStrand,  el->overlapDog,  el->posConf,  el->polyAlen,  el->kaku);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&blockSizesArray);
freez(&chromStartsArray);
}

void retroMrnaInfoSaveToDbEscaped(struct sqlConnection *conn, struct retroMrnaInfo *el, char *tableName, int updateSize)
/* Save retroMrnaInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than retroMrnaInfoSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *strand, *blockSizesArray, *chromStartsArray, *type, *gChrom, *gStrand, *refSeq, *mgc, *kgName, *overName, *overStrand;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
strand = sqlEscapeString(el->strand);
type = sqlEscapeString(el->type);
gChrom = sqlEscapeString(el->gChrom);
gStrand = sqlEscapeString(el->gStrand);
refSeq = sqlEscapeString(el->refSeq);
mgc = sqlEscapeString(el->mgc);
kgName = sqlEscapeString(el->kgName);
overName = sqlEscapeString(el->overName);
overStrand = sqlEscapeString(el->overStrand);

blockSizesArray = sqlSignedArrayToString(el->blockSizes, el->blockCount);
chromStartsArray = sqlSignedArrayToString(el->chromStarts, el->blockCount);
dyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%s',%u,%u,%u,%d,'%s','%s',%d,%d,'%s','%s',%d,%d,'%s',%u,%u,%u,%d,%d,%u,%u,%u,%u,%u,%u,%d,%d,%u,%d,%u,%u,%d,%d,%d,%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,%d,'%s',%d,%g,%u,%g)", 
	tableName,  chrom, el->chromStart , el->chromEnd ,  name, el->score ,  strand, el->thickStart , el->thickEnd , el->reserved , el->blockCount ,  blockSizesArray ,  chromStartsArray , el->retroExonCount , el->axtScore ,  type,  gChrom, el->gStart , el->gEnd ,  gStrand, el->parentSpliceCount , el->geneOverlap , el->polyA , el->polyAstart , el->exonCover , el->intronCount , el->bestAliCount , el->matches , el->qSize , el->qEnd , el->tReps , el->overlapRhesus , el->overlapMouse , el->coverage , el->label , el->milliBad , el->oldScore , el->oldIntronCount , el->processedIntrons , el->conservedSpliceSites , el->maxOverlap ,  refSeq, el->rStart , el->rEnd ,  mgc, el->mStart , el->mEnd ,  kgName, el->kStart , el->kEnd ,  overName, el->overStart , el->overExonCover ,  overStrand, el->overlapDog , el->posConf , el->polyAlen , el->kaku );
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&strand);
freez(&blockSizesArray);
freez(&chromStartsArray);
freez(&type);
freez(&gChrom);
freez(&gStrand);
freez(&refSeq);
freez(&mgc);
freez(&kgName);
freez(&overName);
freez(&overStrand);
}

struct retroMrnaInfo *retroMrnaInfoCommaIn(char **pS, struct retroMrnaInfo *ret)
/* Create a retroMrnaInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new retroMrnaInfo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->thickStart = sqlUnsignedComma(&s);
ret->thickEnd = sqlUnsignedComma(&s);
ret->reserved = sqlUnsignedComma(&s);
ret->blockCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->blockSizes, ret->blockCount);
for (i=0; i<ret->blockCount; ++i)
    {
    ret->blockSizes[i] = sqlSignedComma(&s);
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
    ret->chromStarts[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
ret->retroExonCount = sqlSignedComma(&s);
ret->axtScore = sqlSignedComma(&s);
ret->type = sqlStringComma(&s);
ret->gChrom = sqlStringComma(&s);
ret->gStart = sqlSignedComma(&s);
ret->gEnd = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->gStrand, sizeof(ret->gStrand));
ret->parentSpliceCount = sqlUnsignedComma(&s);
ret->geneOverlap = sqlUnsignedComma(&s);
ret->polyA = sqlUnsignedComma(&s);
ret->polyAstart = sqlSignedComma(&s);
ret->exonCover = sqlSignedComma(&s);
ret->intronCount = sqlUnsignedComma(&s);
ret->bestAliCount = sqlUnsignedComma(&s);
ret->matches = sqlUnsignedComma(&s);
ret->qSize = sqlUnsignedComma(&s);
ret->qEnd = sqlUnsignedComma(&s);
ret->tReps = sqlUnsignedComma(&s);
ret->overlapRhesus = sqlSignedComma(&s);
ret->overlapMouse = sqlSignedComma(&s);
ret->coverage = sqlUnsignedComma(&s);
ret->label = sqlSignedComma(&s);
ret->milliBad = sqlUnsignedComma(&s);
ret->oldScore = sqlUnsignedComma(&s);
ret->oldIntronCount = sqlSignedComma(&s);
ret->processedIntrons = sqlSignedComma(&s);
ret->conservedSpliceSites = sqlSignedComma(&s);
ret->maxOverlap = sqlSignedComma(&s);
ret->refSeq = sqlStringComma(&s);
ret->rStart = sqlSignedComma(&s);
ret->rEnd = sqlSignedComma(&s);
ret->mgc = sqlStringComma(&s);
ret->mStart = sqlSignedComma(&s);
ret->mEnd = sqlSignedComma(&s);
ret->kgName = sqlStringComma(&s);
ret->kStart = sqlSignedComma(&s);
ret->kEnd = sqlSignedComma(&s);
ret->overName = sqlStringComma(&s);
ret->overStart = sqlSignedComma(&s);
ret->overExonCover = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->overStrand, sizeof(ret->overStrand));
ret->overlapDog = sqlSignedComma(&s);
ret->posConf = sqlFloatComma(&s);
ret->polyAlen = sqlUnsignedComma(&s);
ret->kaku = sqlFloatComma(&s);
*pS = s;
return ret;
}

void retroMrnaInfoFree(struct retroMrnaInfo **pEl)
/* Free a single dynamically allocated retroMrnaInfo such as created
 * with retroMrnaInfoLoad(). */
{
struct retroMrnaInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->type);
freeMem(el->gChrom);
freeMem(el->refSeq);
freeMem(el->mgc);
freeMem(el->kgName);
freeMem(el->overName);
freez(pEl);
}

void retroMrnaInfoFreeList(struct retroMrnaInfo **pList)
/* Free a list of dynamically allocated retroMrnaInfo's */
{
struct retroMrnaInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    retroMrnaInfoFree(&el);
    }
*pList = NULL;
}

void retroMrnaInfoOutput(struct retroMrnaInfo *el, FILE *f, char sep, char lastSep) 
/* Print out retroMrnaInfo.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%u", el->thickStart);
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
fputc(sep,f);
fprintf(f, "%u", el->reserved);
fputc(sep,f);
fprintf(f, "%d", el->blockCount);
fputc(sep,f);
{
int i;
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%d", el->blockSizes[i]);
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
    fprintf(f, "%d", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
}
fputc(sep,f);
fprintf(f, "%d", el->retroExonCount);
fputc(sep,f);
fprintf(f, "%d", el->axtScore);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->gStart);
fputc(sep,f);
fprintf(f, "%d", el->gEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->parentSpliceCount);
fputc(sep,f);
fprintf(f, "%u", el->geneOverlap);
fputc(sep,f);
fprintf(f, "%u", el->polyA);
fputc(sep,f);
fprintf(f, "%d", el->polyAstart);
fputc(sep,f);
fprintf(f, "%d", el->exonCover);
fputc(sep,f);
fprintf(f, "%u", el->intronCount);
fputc(sep,f);
fprintf(f, "%u", el->bestAliCount);
fputc(sep,f);
fprintf(f, "%u", el->matches);
fputc(sep,f);
fprintf(f, "%u", el->qSize);
fputc(sep,f);
fprintf(f, "%u", el->qEnd);
fputc(sep,f);
fprintf(f, "%u", el->tReps);
fputc(sep,f);
fprintf(f, "%d", el->overlapRhesus);
fputc(sep,f);
fprintf(f, "%d", el->overlapMouse);
fputc(sep,f);
fprintf(f, "%u", el->coverage);
fputc(sep,f);
fprintf(f, "%d", el->label);
fputc(sep,f);
fprintf(f, "%u", el->milliBad);
fputc(sep,f);
fprintf(f, "%u", el->oldScore);
fputc(sep,f);
fprintf(f, "%d", el->oldIntronCount);
fputc(sep,f);
fprintf(f, "%d", el->processedIntrons);
fputc(sep,f);
fprintf(f, "%d", el->conservedSpliceSites);
fputc(sep,f);
fprintf(f, "%d", el->maxOverlap);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->refSeq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->rStart);
fputc(sep,f);
fprintf(f, "%d", el->rEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mgc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->mStart);
fputc(sep,f);
fprintf(f, "%d", el->mEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->kgName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->kStart);
fputc(sep,f);
fprintf(f, "%d", el->kEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->overName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->overStart);
fputc(sep,f);
fprintf(f, "%d", el->overExonCover);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->overStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->overlapDog);
fputc(sep,f);
fprintf(f, "%g", el->posConf);
fputc(sep,f);
fprintf(f, "%u", el->polyAlen);
fputc(sep,f);
fprintf(f, "%g", el->kaku);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

