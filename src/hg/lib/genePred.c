/* genePred.c was originally generated by the autoSql program, which also 
 * generated genePred.h and genePred.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "gff.h"
#include "linefile.h"
#include "genePred.h"

struct genePred *genePredLoad(char **row)
/* Load a genePred from row fetched with select * from genePred
 * from database.  Dispose of this with genePredFree(). */
{
struct genePred *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->exonCount = sqlUnsigned(row[7]);
ret->name = cloneString(row[0]);
ret->chrom = cloneString(row[1]);
strcpy(ret->strand, row[2]);
ret->txStart = sqlUnsigned(row[3]);
ret->txEnd = sqlUnsigned(row[4]);
ret->cdsStart = sqlUnsigned(row[5]);
ret->cdsEnd = sqlUnsigned(row[6]);
sqlUnsignedDynamicArray(row[8], &ret->exonStarts, &sizeOne);
assert(sizeOne == ret->exonCount);
sqlUnsignedDynamicArray(row[9], &ret->exonEnds, &sizeOne);
assert(sizeOne == ret->exonCount);
return ret;
}

struct genePred *genePredCommaIn(char **pS, struct genePred *ret)
/* Create a genePred out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new genePred */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->chrom = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->txStart = sqlUnsignedComma(&s);
ret->txEnd = sqlUnsignedComma(&s);
ret->cdsStart = sqlUnsignedComma(&s);
ret->cdsEnd = sqlUnsignedComma(&s);
ret->exonCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->exonStarts, ret->exonCount);
for (i=0; i<ret->exonCount; ++i)
    {
    ret->exonStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->exonEnds, ret->exonCount);
for (i=0; i<ret->exonCount; ++i)
    {
    ret->exonEnds[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void genePredFree(struct genePred **pEl)
/* Free a single dynamically allocated genePred such as created
 * with genePredLoad(). */
{
struct genePred *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->chrom);
freeMem(el->exonStarts);
freeMem(el->exonEnds);
freez(pEl);
}

void genePredFreeList(struct genePred **pList)
/* Free a list of dynamically allocated genePred's */
{
struct genePred *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    genePredFree(&el);
    }
*pList = NULL;
}

void genePredOutput(struct genePred *el, FILE *f, char sep, char lastSep) 
/* Print out genePred.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->txStart);
fputc(sep,f);
fprintf(f, "%u", el->txEnd);
fputc(sep,f);
fprintf(f, "%u", el->cdsStart);
fputc(sep,f);
fprintf(f, "%u", el->cdsEnd);
fputc(sep,f);
fprintf(f, "%u", el->exonCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->exonCount; ++i)
    {
    fprintf(f, "%u", el->exonStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->exonCount; ++i)
    {
    fprintf(f, "%u", el->exonEnds[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* ---------  Start of hand generated code. ---------------------------- */

struct genePred *genePredLoadAll(char *fileName) 
/* Load all genePred from a tab-separated file.
 * Dispose of this with genePredFreeList(). */
{
struct genePred *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileRow(lf, row))
    {
    el = genePredLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

int genePredCmp(const void *va, const void *vb)
/* Compare to sort based on chromosome, txStart. */
{
const struct genePred *a = *((struct genePred **)va);
const struct genePred *b = *((struct genePred **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->txStart - b->txStart;
return dif;
}


struct genePred *genePredFromGroupedGff(struct gffFile *gff, struct gffGroup *group, char *name,
	char *exonSelectWord)
/* Convert gff->groupList to genePred list. */
{
struct genePred *gp;
int cdsStart = BIGNUM, cdsEnd = -BIGNUM;
int exonCount = 0;
struct gffLine *gl;
unsigned *eStarts, *eEnds;
int i;
boolean anyExon = FALSE;

/* Look to see if any exons.  If not allow CDS to be
 * used instead. */
if (exonSelectWord)
    {
    for (gl = group->lineList; gl != NULL; gl = gl->next)
	{
	if (sameWord(gl->feature, exonSelectWord))
	    {
	    anyExon = TRUE;
	    break;
	    }
	}
    }
else
    anyExon = TRUE;
if (!anyExon)
    exonSelectWord = "CDS";

/* Count up exons and figure out cdsStart and cdsEnd. */
for (gl = group->lineList; gl != NULL; gl = gl->next)
    {
    char *feat = gl->feature;
    if (exonSelectWord == NULL || sameWord(feat, exonSelectWord))
        {
	++exonCount;
	}
    if (sameWord(feat, "CDS") || sameWord(feat, "start_codon") 
        || sameWord(feat, "stop_codon"))
	{
	if (gl->start < cdsStart) cdsStart = gl->start;
	if (gl->end > cdsEnd) cdsEnd = gl->end;
	}
    }
if (cdsStart > cdsEnd)
    {
    cdsStart = group->start;
    cdsEnd = group->end;
    }
if (exonCount == 0)
    return NULL;

/* Allocate genePred and fill in values. */
AllocVar(gp);
gp->name = cloneString(name);
gp->chrom = cloneString(group->seq);
gp->strand[0] = group->strand;
gp->txStart = group->start;
gp->txEnd = group->end;
gp->cdsStart = cdsStart;
gp->cdsEnd = cdsEnd;
gp->exonCount = exonCount;
gp->exonStarts = AllocArray(eStarts, exonCount);
gp->exonEnds = AllocArray(eEnds, exonCount);
i = 0;
for (gl = group->lineList; gl != NULL; gl = gl->next)
    {
    if (exonSelectWord == NULL || sameWord(gl->feature, exonSelectWord))
        {
	eStarts[i] = gl->start;
	eEnds[i] = gl->end;
	++i;
	}
    }
return gp;
}

