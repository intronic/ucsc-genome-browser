/* bed.c was originally generated by the autoSql program, which also 
 * generated bed.h and bed.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "linefile.h"
#include "bed.h"
#include "binRange.h"

static char const rcsid[] = "$Id: bed.c,v 1.27 2004/07/19 17:46:20 kent Exp $";

void bedStaticLoad(char **row, struct bed *ret)
/* Load a row from bed table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
}

struct bed *bedLoad(char **row)
/* Load a bed from row fetched with select * from bed
 * from database.  Dispose of this with bedFree(). */
{
struct bed *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
return ret;
}

struct bed *bedCommaIn(char **pS, struct bed *ret)
/* Create a bed out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
*pS = s;
return ret;
}

void bedFree(struct bed **pEl)
/* Free a single dynamically allocated bed such as created
 * with bedLoad(). */
{
struct bed *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->expIds);
freeMem(el->expScores);
freez(pEl);
}

void bedFreeList(struct bed **pList)
/* Free a list of dynamically allocated bed's */
{
struct bed *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bedFree(&el);
    }
*pList = NULL;
}

void bedOutput(struct bed *el, FILE *f, char sep, char lastSep) 
/* Print out bed.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
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
fputc(lastSep,f);
}

/* --------------- End of AutoSQL generated code. --------------- */

int bedCmp(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}


int bedCmpScore(const void *va, const void *vb)
/* Compare to sort based on score - lowest first. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
return a->score - b->score;
}

int bedCmpPlusScore(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    {
    dif = (a->chromStart - b->chromStart) * 1000 +(a->score - b->score);
    }
return dif;
}
struct bedLine *bedLineNew(char *line)
/* Create a new bedLine based on tab-separated string s. */
{
struct bedLine *bl;
char *s, c;

AllocVar(bl);
bl->chrom = cloneString(line);
s = strchr(bl->chrom, '\t');
if (s == NULL)
    errAbort("Expecting tab in bed line %s", line);
*s++ = 0;
c = *s;
if (isdigit(c) || (c == '-' && isdigit(s[1])))
    {
    bl->chromStart = atoi(s);
    bl->line = s;
    return bl;
    }
else
    {
    errAbort("Expecting start position in second field of %s", line);
    return NULL;
    }
}

void bedLineFree(struct bedLine **pBl)
/* Free up memory associated with bedLine. */
{
struct bedLine *bl;

if ((bl = *pBl) != NULL)
    {
    freeMem(bl->chrom);
    freez(pBl);
    }
}

void bedLineFreeList(struct bedLine **pList)
/* Free a list of dynamically allocated bedLine's */
{
struct bedLine *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bedLineFree(&el);
    }
*pList = NULL;
}


int bedLineCmp(const void *va, const void *vb)
/* Compare to sort based on query. */
{
const struct bedLine *a = *((struct bedLine **)va);
const struct bedLine *b = *((struct bedLine **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}


void bedSortFile(char *inFile, char *outFile)
/* Sort a bed file (in place, overwrites old file. */
{
struct lineFile *lf = NULL;
FILE *f = NULL;
struct bedLine *blList = NULL, *bl;
char *line;
int lineSize;

printf("Reading %s\n", inFile);
lf = lineFileOpen(inFile, TRUE);
while (lineFileNext(lf, &line, &lineSize))
    {
    if (line[0] == '#')
        continue;
    bl = bedLineNew(line);
    slAddHead(&blList, bl);
    }
lineFileClose(&lf);

printf("Sorting\n");
slSort(&blList, bedLineCmp);

printf("Writing %s\n", outFile);
f = mustOpen(outFile, "w");
for (bl = blList; bl != NULL; bl = bl->next)
    {
    fprintf(f, "%s\t%s\n", bl->chrom, bl->line);
    if (ferror(f))
        {
	perror("Writing error\n");
	errAbort("%s is truncated, sorry.", outFile);
	}
    }
fclose(f);
}

struct bed *bedLoad3(char **row)
/* Load first three fields of bed. */
{
struct bed *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
return ret;
}

struct bed *bedLoad5(char **row)
/* Load first five fields of bed. */
{
struct bed *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlSigned(row[4]);
return ret;
}

struct bed *bedLoad6(char **row)
/* Load first six fields of bed. */
{
struct bed *ret = bedLoad5(row);
strcpy(ret->strand, row[5]);
return ret;
}

struct bed *bedLoad12(char **row)
/* Load a bed from row fetched with select * from bed
 * from database.  Dispose of this with bedFree(). */
{
struct bed *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->blockCount = sqlSigned(row[9]);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlSigned(row[4]);
strcpy(ret->strand, row[5]);
ret->thickStart = sqlUnsigned(row[6]);
ret->thickEnd = sqlUnsigned(row[7]);
ret->reserved = sqlUnsigned(row[8]);
sqlSignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
assert(sizeOne == ret->blockCount);
sqlSignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
assert(sizeOne == ret->blockCount);
return ret;
}


struct bed *bedLoadN(char *row[], int wordCount)
/* Convert a row of strings to a bed. */
{
struct bed * bed;
int count;

AllocVar(bed);
bed->chrom = cloneString(row[0]);
bed->chromStart = sqlUnsigned(row[1]);
bed->chromEnd = sqlUnsigned(row[2]);
if (wordCount > 3)
     bed->name = cloneString(row[3]);
if (wordCount > 4)
     bed->score = sqlSigned(row[4]);
if (wordCount > 5)
     bed->strand[0] = row[5][0];
if (wordCount > 6)
     bed->thickStart = sqlUnsigned(row[6]);
else
     bed->thickStart = bed->chromStart;
if (wordCount > 7)
     bed->thickEnd = sqlUnsigned(row[7]);
else
     bed->thickEnd = bed->chromEnd;
if (wordCount > 8)
    bed->reserved = sqlUnsigned(row[8]);
if (wordCount > 9)
    bed->blockCount = sqlUnsigned(row[9]);
if (wordCount > 10)
    sqlSignedDynamicArray(row[10], &bed->blockSizes, &count);
if (wordCount > 11)
    sqlSignedDynamicArray(row[11], &bed->chromStarts, &count);
if (wordCount > 12)
    bed->expCount = sqlUnsigned(row[12]);
if (wordCount > 13)
    sqlSignedDynamicArray(row[13], &bed->expIds, &count);
if (wordCount > 14)
    sqlFloatDynamicArray(row[14], &bed->expScores, &count);
return bed;
}

struct bed *bedLoadNAllChrom(char *fileName, int numFields, char* chrom) 
/* Load bed entries from a tab-separated file that have the given chrom.
 * Dispose of this with bedFreeList(). */
{
struct bed *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[numFields];

while (lineFileRow(lf, row))
    {
    el = bedLoadN(row, numFields);
    if(chrom == NULL || sameString(el->chrom, chrom))
        slAddHead(&list, el);
    else
        freez(&el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bed *bedLoadNAll(char *fileName, int numFields) 
/* Load all bed from a tab-separated file.
 * Dispose of this with bedFreeList(). */
{
return bedLoadNAllChrom(fileName, numFields, NULL);
}

struct bed *bedLoadAll(char *fileName)
/* Determines how many fields are in a bedFile and load all beds from
 * a tab-separated file.  Dispose of this with bedFreeList(). */
{
struct bed *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
int numFields = 0;
char *line = NULL;
/* First peek to see how many columns in the bed file. */
lineFileNextReal(lf, &line);
numFields = chopByWhite(line, NULL, 0);
lineFileClose(&lf);
/* Now load them up with that number of fields. */
list = bedLoadNAll(fileName, numFields);
return list;
}


struct bed *bedLoadNBin(char *row[], int wordCount)
/* Convert a row of strings to a bed. */
{
struct bed * bed;
int count;

AllocVar(bed);
bed->chrom = cloneString(row[1]);
bed->chromStart = sqlUnsigned(row[2]);
bed->chromEnd = sqlUnsigned(row[3]);
if (wordCount > 3)
     bed->name = cloneString(row[4]);
if (wordCount > 4)
     bed->score = sqlSigned(row[5]);
if (wordCount > 5)
     bed->strand[0] = row[6][0];
if (wordCount > 6)
     bed->thickStart = sqlUnsigned(row[7]);
else
     bed->thickStart = bed->chromStart;
if (wordCount > 7)
     bed->thickEnd = sqlUnsigned(row[8]);
else
     bed->thickEnd = bed->chromEnd;
if (wordCount > 8)
    bed->reserved = sqlUnsigned(row[9]);
if (wordCount > 9)
    bed->blockCount = sqlUnsigned(row[10]);
if (wordCount > 10)
    sqlSignedDynamicArray(row[11], &bed->blockSizes, &count);
if (wordCount > 11)
    sqlSignedDynamicArray(row[12], &bed->chromStarts, &count);
if (wordCount > 12)
    bed->expCount = sqlUnsigned(row[13]);
if (wordCount > 13)
    sqlSignedDynamicArray(row[14], &bed->expIds, &count);
if (wordCount > 14)
    sqlFloatDynamicArray(row[15], &bed->expScores, &count);
return bed;
}

void bedOutputN(struct bed *el, int wordCount, FILE *f, char sep, char lastSep)
/* Write a bed of wordCount fields. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
if (wordCount <= 3)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
if (wordCount <= 4)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
fprintf(f, "%d", el->score);
if (wordCount <= 5)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
if (wordCount <= 6)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
fprintf(f, "%u", el->thickStart);
if (wordCount <= 7)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
fprintf(f, "%u", el->thickEnd);
if (wordCount <= 8)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
fprintf(f, "%u", el->reserved);
if (wordCount <= 9)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
fprintf(f, "%d", el->blockCount);
if (wordCount <= 10)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%d", el->blockSizes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
if (wordCount <= 11)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->blockCount; ++i)
    {
    fprintf(f, "%d", el->chromStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);

if (wordCount <= 12)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
fprintf(f, "%d", el->expCount);

if (wordCount <= 13)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->expCount; ++i)
    {
    fprintf(f, "%d", el->expIds[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);


if (wordCount <= 14)
    {
    fputc(lastSep, f);
    return;
    }
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->expCount; ++i)
    {
    fprintf(f, "%f", el->expScores[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);


fputc(lastSep,f);
}

struct genePred *bedToGenePred(struct bed *bed)
/* Convert a single bed to a genePred structure. */
{
struct genePred *gp = NULL;
int i;
assert(bed);
AllocVar(gp);
gp->name = cloneString(bed->name);
gp->chrom = cloneString(bed->chrom);
safef(gp->strand, sizeof(gp->strand), "%s", bed->strand);
gp->txStart = bed->chromStart;
gp->txEnd = bed->chromEnd;
gp->cdsStart = bed->thickStart;
gp->cdsEnd = bed->thickEnd;
gp->exonCount = bed->blockCount;
if(gp->exonCount != 0)
    {
    AllocArray(gp->exonStarts, gp->exonCount);
    AllocArray(gp->exonEnds, gp->exonCount);
    for(i=0; i<gp->exonCount; i++)
	{
	gp->exonStarts[i] = bed->chromStarts[i] + bed->chromStart;
	gp->exonEnds[i] = gp->exonStarts[i] + bed->blockSizes[i];
	}
    }
else 
    {
    gp->exonCount = 1;
    AllocArray(gp->exonStarts, gp->exonCount);
    AllocArray(gp->exonEnds, gp->exonCount);
    gp->exonStarts[0] = bed->chromStart;
    gp->exonEnds[0] = bed->chromEnd;
    }
return gp;
}

struct bed *bedFromGenePred(struct genePred *genePred)
/* Convert a single genePred to a bed structure */
{
struct bed *bed;
int i, blockCount, *chromStarts, *blockSizes, chromStart;

/* A tiny bit of error checking on the genePred. */
if (genePred->txStart >= genePred->txEnd || genePred->cdsStart > genePred->cdsEnd
    || genePred->exonCount < 0 )
    {
    errAbort("mangled genePred format for %s", genePred->name);
    }

/* Allocate bed and fill in from psl. */
AllocVar(bed);
bed->chrom = cloneString(genePred->chrom);
bed->chromStart = chromStart = genePred->txStart;
bed->chromEnd = genePred->txEnd;
bed->thickStart = genePred->cdsStart;
bed->thickEnd = genePred->cdsEnd;
bed->score = 0;
strncpy(bed->strand,  genePred->strand, sizeof(bed->strand));
bed->blockCount = blockCount = genePred->exonCount;
bed->blockSizes = blockSizes = (int *)cloneMem(genePred->exonEnds,(sizeof(int)*genePred->exonCount));
bed->chromStarts = chromStarts = (int *)cloneMem(genePred->exonStarts, (sizeof(int)*genePred->exonCount));
bed->name = cloneString(genePred->name);

/* Convert coordinates to relative and exnosEnds to blockSizes. */
for (i=0; i<blockCount; ++i)
    {
    blockSizes[i] -= chromStarts[i];
    chromStarts[i] -= chromStart;
    }
return bed;
}




struct bed *bedFromPsl(struct psl *psl)
/* Convert a single psl to a bed structure */
{
struct bed *bed;
int i, blockCount, *chromStarts, chromStart;

/* A tiny bit of error checking on the psl. */
if (psl->qStart >= psl->qEnd || psl->qEnd > psl->qSize 
    || psl->tStart >= psl->tEnd || psl->tEnd > psl->tSize)
    {
    errAbort("mangled psl format for %s", psl->qName);
    }

/* Allocate bed and fill in from psl. */
AllocVar(bed);
bed->chrom = cloneString(psl->tName);
bed->chromStart = bed->thickStart = chromStart = psl->tStart;
bed->chromEnd = bed->thickEnd = psl->tEnd;
bed->score = 1000 - 2*pslCalcMilliBad(psl, TRUE);
if (bed->score < 0) bed->score = 0;
strncpy(bed->strand,  psl->strand, sizeof(bed->strand));
bed->blockCount = blockCount = psl->blockCount;
bed->blockSizes = (int *)cloneMem(psl->blockSizes,(sizeof(int)*psl->blockCount));
bed->chromStarts = chromStarts = (int *)cloneMem(psl->tStarts, (sizeof(int)*psl->blockCount));
bed->name = cloneString(psl->qName);

/* Switch minus target strand to plus strand. */
if (psl->strand[1] == '-')
    {
    int chromSize = psl->tSize;
    reverseInts(bed->blockSizes, blockCount);
    reverseInts(chromStarts, blockCount);
    for (i=0; i<blockCount; ++i)
	chromStarts[i] = chromSize - chromStarts[i];
    }

/* Convert coordinates to relative. */
for (i=0; i<blockCount; ++i)
    chromStarts[i] -= chromStart;
return bed;
}


struct bed *cloneBed(struct bed *bed)
/* Make an all-newly-allocated copy of a single bed record. */
{
struct bed *newBed;
if (bed == NULL)
    return NULL;
AllocVar(newBed);
newBed->chrom = cloneString(bed->chrom);
newBed->chromStart = bed->chromStart;
newBed->chromEnd = bed->chromEnd;
newBed->name = cloneString(bed->name);
newBed->score = bed->score;
strncpy(newBed->strand, bed->strand, sizeof(bed->strand));
newBed->thickStart = bed->thickStart;
newBed->thickEnd = bed->thickEnd;
newBed->reserved = bed->reserved;
newBed->blockCount = bed->blockCount;
if (bed->blockCount > 0)
    {
    newBed->blockSizes = needMem(sizeof(int) * bed->blockCount);
    memcpy(newBed->blockSizes, bed->blockSizes,
	   sizeof(int) * bed->blockCount);
    newBed->chromStarts = needMem(sizeof(int) * bed->blockCount);
    memcpy(newBed->chromStarts, bed->chromStarts,
	   sizeof(int) * bed->blockCount);
    }
newBed->expCount = bed->expCount;
if (bed->expCount > 0)
    {
    newBed->expIds = needMem(sizeof(int) * bed->expCount);
    memcpy(newBed->expIds, bed->expIds,
	   sizeof(int) * bed->expCount);
    newBed->expScores = needMem(sizeof(float) * bed->expCount);
    memcpy(newBed->expScores, bed->expScores,
	   sizeof(float) * bed->expCount);
    }
return(newBed);
}


struct bed *cloneBedList(struct bed *bedList)
/* Make an all-newly-allocated list copied from bed. */
{
struct bed *bedListOut = NULL, *bed=NULL;

for (bed=bedList;  bed != NULL;  bed=bed->next)
    {
    struct bed *newBed = cloneBed(bed);
    slAddHead(&bedListOut, newBed);
    }

slReverse(&bedListOut);
return bedListOut;
}


static boolean filterChar(char value, enum charFilterType cft,
			  char *filterValues, boolean invert)
/* Return TRUE if value passes the filter. */
{
char thisVal;
if (filterValues == NULL)
    return(TRUE);
switch (cft)
    {
    case (cftIgnore):
	return(TRUE);
	break;
    case (cftSingleLiteral):
	return((value == *filterValues) ^ invert);
	break;
    case (cftMultiLiteral):
	while ((thisVal = *(filterValues++)) != 0)
	    {
	    if (value == thisVal)
		return(TRUE ^ invert);
	    }
	break;
    default:
	errAbort("illegal charFilterType: %d", cft);
	break;
    }
return(FALSE ^ invert);
}

static boolean filterString(char *value, enum stringFilterType sft,
			    char **filterValues, boolean invert)
/* Return TRUE if value passes the filter. */
{
char *thisVal;

if (filterValues == NULL)
    return(TRUE);
switch (sft)
    {
    case (sftIgnore):
	return(TRUE);
	break;
    case (sftSingleLiteral):
	return(sameString(value, *filterValues) ^ invert);
	break;
    case (sftMultiLiteral):
	while ((thisVal = *(filterValues++)) != NULL)
	    if (sameString(value, thisVal))
		return(TRUE ^ invert);
	break;
    case (sftSingleRegexp):
	return(wildMatch(*filterValues, value) ^ invert);
	break;
    case (sftMultiRegexp):
	while ((thisVal = *(filterValues++)) != NULL)
	    if (wildMatch(thisVal, value))
		return(TRUE ^ invert);
	break;
    default:
	errAbort("illegal stringFilterType: %d", sft);
	break;
    }
return(FALSE ^ invert);
}

static boolean filterInt(int value, enum numericFilterType nft,
			 int *filterValues)
/* Return TRUE if value passes the filter. */
/* This could probably be turned into a macro if performance is bad. */
{
if (filterValues == NULL)
    return(TRUE);
switch (nft)
    {
    case (nftIgnore):
	return(TRUE);
	break;
    case (nftLessThan):
	return(value < *filterValues);
	break;
    case (nftLTE):
	return(value <= *filterValues);
	break;
    case (nftEqual):
	return(value == *filterValues);
	break;
    case (nftNotEqual):
	return(value != *filterValues);
	break;
    case (nftGTE):
	return(value >= *filterValues);
	break;
    case (nftGreaterThan):
	return(value > *filterValues);
	break;
    case (nftInRange):
	return((value >= *filterValues) && (value < *(filterValues+1)));
	break;
    case (nftNotInRange):
	return(! ((value >= *filterValues) && (value < *(filterValues+1))));
	break;
    default:
	errAbort("illegal numericFilterType: %d", nft);
	break;
    }
return(FALSE);
}

boolean bedFilterOne(struct bedFilter *bf, struct bed *bed)
/* Return TRUE if bed passes filter. */
{
int cmpValues[2];
if (bf == NULL)
    return TRUE;
if (!filterString(bed->chrom, bf->chromFilter, bf->chromVals, bf->chromInvert))
    return FALSE;
if (!filterInt(bed->chromStart, bf->chromStartFilter, bf->chromStartVals))
    return FALSE;
if (!filterInt(bed->chromEnd, bf->chromEndFilter, bf->chromEndVals))
    return FALSE;
if (!filterString(bed->name, bf->nameFilter, bf->nameVals, bf->nameInvert))
    return FALSE;
if (!filterInt(bed->score, bf->scoreFilter, bf->scoreVals))
    return FALSE;
if (!filterChar(bed->strand[0], bf->strandFilter, bf->strandVals,
		    bf->strandInvert))
    return FALSE;
if (!filterInt(bed->thickStart, bf->thickStartFilter,
		    bf->thickStartVals))
    return FALSE;
if (!filterInt(bed->thickEnd, bf->thickEndFilter,
		    bf->thickEndVals))
    return FALSE;
if (!filterInt(bed->blockCount, bf->blockCountFilter,
		    bf->blockCountVals))
    return FALSE;
if (!filterInt((bed->chromEnd - bed->chromStart),
		    bf->chromLengthFilter, bf->chromLengthVals))
    return FALSE;
if (!filterInt((bed->thickEnd - bed->thickStart),
		    bf->thickLengthFilter, bf->thickLengthVals))
    return FALSE;
cmpValues[0] = cmpValues[1] = bed->thickStart;
if (!filterInt(bed->chromStart, bf->compareStartsFilter,
		    cmpValues))
    return FALSE;
cmpValues[0] = cmpValues[1] = bed->thickEnd;
if (!filterInt(bed->chromEnd, bf->compareEndsFilter, cmpValues))
    return FALSE;
return TRUE;
}

struct bed *bedFilterListInRange(struct bed *bedListIn, struct bedFilter *bf,
				 char *chrom, int winStart, int winEnd)
/* Given a bed list, a position range, and a bedFilter which specifies
 * constraints on bed fields, return the list of bed items that meet
 * the constraints.  If chrom is NULL, position range is ignored. */
{
struct bed *bedListOut = NULL, *bed;

for (bed=bedListIn;  bed != NULL;  bed=bed->next)
    {
    boolean passes = TRUE;
    if (chrom != NULL)
	{
	passes &= (sameString(bed->chrom, chrom) &&
		   (bed->chromStart < winEnd) &&
		   (bed->chromEnd   > winStart));
        }
    if (bf != NULL && passes)
	{
	passes &= bedFilterOne(bf, bed);
	}
    if (passes)
	{
	struct bed *newBed = cloneBed(bed);
	slAddHead(&bedListOut, newBed);
	}
    }
slReverse(&bedListOut);
return(bedListOut);
}

struct bed *bedFilterList(struct bed *bedListIn, struct bedFilter *bf)
/* Given a bed list and a bedFilter which specifies constraints on bed 
 * fields, return the list of bed items that meet the constraints. */
{
return bedFilterListInRange(bedListIn, bf, NULL, 0, 0);
}

struct bed *bedFilterByNameHash(struct bed *bedList, struct hash *nameHash)
/* Given a bed list and a hash of names to keep, return the list of bed 
 * items whose name is in nameHash. */
{
struct bed *bedListOut = NULL, *bed=NULL;

for (bed=bedList;  bed != NULL;  bed=bed->next)
    {
    if (hashLookup(nameHash, bed->name) != NULL)
	{
	struct bed *newBed = cloneBed(bed);
	slAddHead(&bedListOut, newBed);
	}
    }

slReverse(&bedListOut);
return bedListOut;
}

struct bed *bedFilterByWildNames(struct bed *bedList, struct slName *wildNames)
/* Given a bed list and a list of names that may include wildcard characters,
 * return the list of bed items whose name matches at least one wildName. */
{
struct bed *bedListOut = NULL, *bed=NULL;
struct slName *wildName=NULL;

for (bed=bedList;  bed != NULL;  bed=bed->next)
    {
    for (wildName=wildNames;  wildName != NULL;  wildName=wildName->next)
	{
	if (wildMatch(wildName->name, bed->name))
	    {
	    struct bed *newBed = cloneBed(bed);
	    slAddHead(&bedListOut, newBed);
	    break;
	    }
	}
    }

slReverse(&bedListOut);
return bedListOut;
}


struct bed *bedCommaInN(char **pS, struct bed *ret, int fieldCount)
/* Create a bed out of a comma separated string looking for fieldCount
 * fields. This will fill in ret if non-null, otherwise will return a
 * new bed */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
if (fieldCount > 3)
    ret->name = sqlStringComma(&s);
if (fieldCount > 4)
    ret->score = sqlUnsignedComma(&s);
if (fieldCount > 5)
    sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
if (fieldCount > 6)
    ret->thickStart = sqlUnsignedComma(&s);
else
    ret->thickStart = ret->chromStart;
if (fieldCount > 7)
    ret->thickEnd = sqlUnsignedComma(&s);
else
     ret->thickEnd = ret->chromEnd;
if (fieldCount > 8)
    ret->reserved = sqlUnsignedComma(&s);
if (fieldCount > 9)
    ret->blockCount = sqlUnsignedComma(&s);
if (fieldCount > 10)
    {
    s = sqlEatChar(s, '{');
    AllocArray(ret->blockSizes, ret->blockCount);
    for (i=0; i<ret->blockCount; ++i)
	{
	ret->blockSizes[i] = sqlSignedComma(&s);
	}
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
if(fieldCount > 11)
    {
    s = sqlEatChar(s, '{');
    AllocArray(ret->chromStarts, ret->blockCount);
    for (i=0; i<ret->blockCount; ++i)
	{
	ret->chromStarts[i] = sqlSignedComma(&s);
	}
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
if (fieldCount > 12)
    ret->expCount = sqlSignedComma(&s);
if (fieldCount > 13)
    {
    s = sqlEatChar(s, '{');
    AllocArray(ret->expIds, ret->expCount);
    for (i=0; i<ret->expCount; ++i)
	{
	ret->expIds[i] = sqlSignedComma(&s);
	}
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
if (fieldCount > 14)
    {
    s = sqlEatChar(s, '{');
    AllocArray(ret->expScores, ret->expCount);
    for (i=0; i<ret->expCount; ++i)
	{
	ret->expScores[i] = sqlFloatComma(&s);
	}
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
*pS = s;
return ret;
}

struct hash *readBedToBinKeeper(char *sizeFileName, char *bedFileName, int wordCount)
/* read a list of beds and return results in hash of binKeeper structure for fast query*/
{
struct binKeeper *bk; 
int size;
struct bed *bed;
struct lineFile *lf = lineFileOpen(sizeFileName, TRUE);
struct lineFile *bf = lineFileOpen(bedFileName , TRUE);
struct hash *hash = newHash(0);
char *chromRow[2];
char *row[3] ;

assert (wordCount == 3);
while (lineFileRow(lf, chromRow))
    {
    char *name = chromRow[0];
    int size = lineFileNeedNum(lf, chromRow, 1);

    if (hashLookup(hash, name) != NULL)
        warn("Duplicate %s, ignoring all but first\n", name);
    else
        {
        bk = binKeeperNew(0, size);
        assert(size > 1);
	hashAdd(hash, name, bk);
        }
    }
while (lineFileNextRow(bf, row, ArraySize(row)))
    {
    bed = bedLoadN(row, wordCount);
    bk = hashMustFindVal(hash, bed->chrom);
    binKeeperAdd(bk, bed->chromStart, bed->chromEnd, bed);
    }
lineFileClose(&bf);
lineFileClose(&lf);
return hash;
}

