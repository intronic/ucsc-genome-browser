/* genePred.c was originally generated by the autoSql program, which also 
 * generated genePred.h and genePred.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "gff.h"
#include "jksql.h"
#include "psl.h"
#include "linefile.h"
#include "genePred.h"
#include "genbank.h"
#include "hdb.h"

static char const rcsid[] = "$Id: genePred.c,v 1.39 2004/03/08 05:20:30 markd Exp $";

/* SQL to create a genePred table */
static char *createSql = 
"CREATE TABLE %s ("
"   %s"                                 /* bin column goes here */
"   name varchar(255) not null,"	/* mrna accession of gene */
"   chrom varchar(255) not null,"	/* Chromosome name */
"   strand char(1) not null,"		/* + or - for strand */
"   txStart int unsigned not null,"	/* Transcription start position */
"   txEnd int unsigned not null,"	/* Transcription end position */
"   cdsStart int unsigned not null,"	/* Coding region start */
"   cdsEnd int unsigned not null,"	/* Coding region end */
"   exonCount int unsigned not null,"	/* Number of exons */
"   exonStarts longblob not null,"	/* Exon start positions */
"   exonEnds longblob not null,"	/* Exon end positions */
"   %s %s %s %s"                        /* Optional fields */
"   INDEX(name(10)),"
"   INDEX(chrom(12),txStart),"
"   INDEX(chrom(12),txEnd)"
")";

static char *binFieldSql = 
"    bin smallint unsigned not null,"
"    INDEX(tName(8),bin),";

static char *idFieldSql = 
"    id int unsigned PRIMARY KEY auto_increment,";   /* Numeric id of gene annotation. */

static char *name2FieldSql = 
"   name2 varchar(255) not null,"    /* Secondary name. (e.g. name of gene) or NULL if not available */
"   INDEX(name2(10)),";

static char *cdsStatFieldSql = 
"   cdsStartStat enum('none', 'unk', 'incmpl', 'cmpl') not null,"    /* Status of cdsStart annotation */
"   cdsEndStat enum('none', 'unk', 'incmpl', 'cmpl') not null,";     /* Status of cdsEnd annotation */

static char *exonFramesFieldSql = 
"    exonFrames longblob not null,";    /* List of frame for each exon, or -1 if no frame or not known. NULL if not available. */

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

struct genePred *genePredLoadAll(char *fileName) 
/* Load all genePred from a whitespace-separated file.
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

struct genePred *genePredLoadAllByChar(char *fileName, char chopper) 
/* Load all genePred from a chopper separated file.
 * Dispose of this with genePredFreeList(). */
{
struct genePred *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[10];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = genePredLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
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
freeMem(el->name2);
freeMem(el->exonFrames);
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

/* optional fields, >= test is used so unspecified coumns can be filled in */
if (el->optFields >= genePredIdFld)
    {
    fputc(sep,f);
    fprintf(f, "%u", el->id);
    }
if (el->optFields >= genePredName2Fld)
    {
    fputc(sep,f);
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", ((el->name2 != NULL) ? el->name2 : ""));
    if (sep == ',') fputc('"',f);
    }
if (el->optFields >= genePredCdsStatFld)
    {
    fputc(sep,f);
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", genePredCdsStatStr(el->cdsStartStat));
    if (sep == ',') fputc('"',f);
    fputc(sep,f);
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", genePredCdsStatStr(el->cdsEndStat));
    if (sep == ',') fputc('"',f);
    }
if (el->optFields >= genePredExonFramesFld)
    {
    fputc(sep,f);
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->exonCount; ++i)
        {
        fprintf(f, "%d", el->exonFrames[i]);
        fputc(',', f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(lastSep,f);
}

/* ---------  Start of hand generated code. ---------------------------- */

char *genePredCdsStatStr(enum cdsStatus stat)
/* get string value of a cdsStatus */
{
switch (stat) {
case cdsNone:
    return "none";
case cdsUnknown:
    return "unk";
case cdsIncomplete:
    return "incmpl";
case cdsComplete:
    return "cmpl";
default:
    errAbort("invalid cdsStatus enum value: %d", stat);
    return NULL;  /* make compiler happy */
}
}

static enum cdsStatus parseCdsStat(char *statStr)
/* parse a cdsStatus string */
{
if ((statStr == NULL) || sameString(statStr, "none"))
    return cdsNone;
if (sameString(statStr, "unk"))
    return cdsUnknown;
if (sameString(statStr, "incmpl"))
    return cdsIncomplete;
if (sameString(statStr, "cmpl"))
    return cdsComplete;

errAbort("invalid genePred cdsStatus: \"%s\"", statStr);
return cdsNone;  /* make compiler happy */
}

struct genePred *genePredExtLoad(char **row, int numCols)
/* Load a genePred with from a row, with optional fields.  The row must
 * contain columns in the order in the struct, and they must be present up to
 * the last specfied optional field.  Missing intermediate fields must have
 * zero or empty columns, they may not be omitted.  Fields at the end can be
 * omitted. Dispose of this with genePredFree(). */
{
struct genePred *ret;
int sizeOne, iCol;

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

iCol=GENEPRED_NUM_COLS;
if (iCol < numCols)
    {
    ret->id = sqlUnsigned(row[iCol++]);
    ret->optFields |= genePredIdFld;
    }
if (iCol < numCols)
    {
    ret->name2 = cloneString(row[iCol++]);
    ret->optFields |= genePredName2Fld;
    }
if (iCol < numCols)
    {
    ret->cdsStartStat = parseCdsStat(row[iCol++]);
    ret->optFields |= genePredCdsStatFld;
    }
if (iCol < numCols)
    {
    ret->cdsEndStat = parseCdsStat(row[iCol++]);
    ret->optFields |= genePredCdsStatFld;
    }
if (iCol < numCols)
    {
    sqlSignedDynamicArray(row[iCol++], &ret->exonFrames, &sizeOne);
    assert(sizeOne == ret->exonCount);
    ret->optFields |= genePredExonFramesFld;
    }
return ret;
}

struct genePred *genePredExtLoadAll(char *fileName)
/* Load all genePreds with from tab-separated file, possibly with optional
 * fields. Dispose of this with genePredFreeList(). */
{
struct genePred *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[GENEPREDX_NUM_COLS];
int numCols;

while ((numCols = lineFileChopNextTab(lf, row, ArraySize(row))) > 0)
    {
    lineFileExpectAtLeast(lf, GENEPRED_NUM_COLS, numCols);
    el = genePredExtLoad(row, numCols);
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

static boolean isExon(char *feat, boolean isGtf, char *exonSelectWord)
/* determine if a feature is an exon; different criteria for GFF ane GTF */
{
/* FIXME: shouldn't actually need to allow CDS here for GTF */
if (isGtf)
    return (sameWord(feat, "CDS") || sameWord(feat, "exon"));
else
    return ((exonSelectWord == NULL) || sameWord(feat, exonSelectWord));
}

static void chkGroupLine(struct gffGroup *group, struct gffLine *gl, struct genePred *gp)
/* check that a gffLine is consistent with the genePred being built.  this
 * helps detect some problems that lead to corrupt genePreds */
{
if (!sameString(gl->seq, gp->chrom) && (gl->strand == gp->strand[0]))
    {
    fprintf(stderr, "invalid gffGroup detected on line: ");
    gffTabOut(gl, stderr);
    errAbort("GFF/GTF group %s on %s%c, this line is on %s%c, all group members must be on same seq and strand",
             group->name, gp->chrom, gp->strand[0],
                     gl->seq, gl->strand);
    }
}

static boolean isCds(char *feat, boolean isGtf)
/* determine if a feature is CDS */
{
return sameWord(feat, "CDS");
}

static struct genePred *mkFromGroupedGxf(struct gffFile *gff, struct gffGroup *group, char *name,
                                         boolean isGtf, char *exonSelectWord, unsigned optFields)
/* common function to create genePreds from GFFs or GTFs.  This is a little
 * ugly with to many check of isGtf, however the was way to much identical
 * code the other way.*/
{
struct genePred *gp;
int stopCodonStart = -1, stopCodonEnd = -1;
int cdsStart = BIGNUM, cdsEnd = -BIGNUM;
int exonCount = 0;
boolean haveCds = FALSE, haveStartCodon = FALSE, haveStopCodon = FALSE;
struct gffLine *gl;
unsigned *eStarts, *eEnds, *eFrames;
int i;

/* Count up exons and figure out cdsStart and cdsEnd. */
for (gl = group->lineList; gl != NULL; gl = gl->next)
    {
    char *feat = gl->feature;
    if (isExon(feat, isGtf, exonSelectWord))
	++exonCount;
    if (isCds(feat, isGtf))
        {
	if (gl->start < cdsStart)
            cdsStart = gl->start;
	if (gl->end > cdsEnd)
            cdsEnd = gl->end;
        haveCds = TRUE;
	}
    if (sameWord(feat, "start_codon"))
        haveStartCodon = TRUE;
    if (sameWord(feat, "stop_codon"))
        {
        stopCodonStart = gl->start;
        stopCodonEnd = gl->end;
        haveStopCodon = TRUE;
        }
    }
if (exonCount == 0)
    return NULL;
if (cdsStart > cdsEnd)
    {
    /* no cds annotated */
    cdsStart = 0;
    cdsEnd = 0;
    }
else if (isGtf && (stopCodonStart >= 0))
    {
    /* adjust CDS to include stop codon in GTF */
    if (group->strand == '+')
        cdsEnd = stopCodonEnd;
    else
        cdsStart = stopCodonStart;
    }

/* Allocate genePred and fill in values. */
AllocVar(gp);
gp->name = cloneString(name);
gp->chrom = cloneString(group->seq);
gp->strand[0] = group->strand;
gp->txStart = group->start;
gp->txEnd = group->end;
gp->cdsStart = cdsStart;
gp->cdsEnd = cdsEnd;
gp->exonStarts = AllocArray(eStarts, exonCount);
gp->exonEnds = AllocArray(eEnds, exonCount);
gp->optFields = optFields;

if (optFields & genePredName2Fld)
    {
    if (group->lineList->geneId != NULL)
        gp->name2 = cloneString(group->lineList->geneId);
    else
        gp->name2 = cloneString("");
    }
if (optFields & genePredCdsStatFld)
    {
    if (haveCds)
        {
        if (group->strand == '+')
            {
            gp->cdsStartStat = (haveStartCodon ? cdsComplete : cdsIncomplete);
            gp->cdsEndStat = (haveStopCodon ? cdsComplete : cdsIncomplete);;
            }
        else
            {
            gp->cdsEndStat = (haveStartCodon ? cdsComplete : cdsIncomplete);
            gp->cdsStartStat = (haveStopCodon ? cdsComplete : cdsIncomplete);;
            }
        }
    else
        {
        gp->cdsStartStat = cdsNone;
        gp->cdsEndStat = cdsNone;
        }
    }
if (optFields & genePredExonFramesFld)
    {
    gp->exonFrames = AllocArray(eFrames, exonCount);
    for (i = 0; i < exonCount; i++)
        gp->exonFrames[i] = -1;
    }
eFrames = gp->exonFrames;


/* adjust tx range to include stop codon on */
if ((group->strand == '+') && (gp->txEnd == stopCodonStart))
     gp->txEnd = stopCodonEnd;
else if ((group->strand == '-') && (gp->txStart == stopCodonEnd))
    gp->txStart = stopCodonStart;

i = 0;
/* fill in exons, merging overlaping and adjacent exons */
for (gl = group->lineList; gl != NULL; gl = gl->next)
    {
    if ((optFields & genePredExonFramesFld) && isCds(gl->feature, isGtf))
        {
        /* set frame if this is a CDS */
        assert(i < exonCount);
        if (isdigit(gl->frame))
            eFrames[i] = (int)gl->frame - '0';
        }
    if (isExon(gl->feature, isGtf, exonSelectWord))
        {
        chkGroupLine(group, gl, gp);
        if ((i == 0) || (gl->start > eEnds[i-1]))
            {
            eStarts[i] = gl->start;
            eEnds[i] = gl->end;
            ++i;
            }
        else
            {
            /* overlap, extend exon, picking the largest of ends */
            assert(gl->start >= eStarts[i-1]);
            if (gl->end > eEnds[i-1])
                eEnds[i-1] = gl->end;
            }
        /* extend exon for stop codon in GTF if needed */
        if (isGtf)
            {
            if ((group->strand == '+') && (eEnds[i-1] == stopCodonStart))
                eEnds[i-1] = stopCodonEnd;
            else if ((group->strand == '-') && (eStarts[i-1] == stopCodonEnd))
                eStarts[i-1] = stopCodonStart;
            }
        }
    }
gp->exonCount = i;
return gp;
}

struct genePred *genePredFromGroupedGff(struct gffFile *gff, struct gffGroup *group, char *name,
	char *exonSelectWord, unsigned optFields)
/* Convert gff->groupList to genePred list. */
{
struct genePred *gp;
int cdsStart = BIGNUM, cdsEnd = -BIGNUM;
int exonCount = 0;
boolean haveCds = FALSE, haveStartCodon = FALSE, haveStopCodon = FALSE;
struct gffLine *gl;
unsigned *eStarts, *eEnds, *eFrames;
int i;
boolean anyExon = FALSE;

/* Look to see if any exons.  If not allow CDS to be used instead. */
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

return mkFromGroupedGxf(gff, group, name, FALSE, exonSelectWord, optFields);
}

struct genePred *genePredFromGroupedGtf(struct gffFile *gff, struct gffGroup *group, char *name, unsigned optFields)
/* Convert gff->groupList to genePred list, using GTF feature conventions;
 * including the stop codon in the 3' UTR, not the CDS (grr).  Assumes
 * gffGroup is sorted in assending coords, with overlaping starts sorted by
 * end coords, which is true if it was created by gffGroupLines().  If
 * optFields contains the bit set of optional fields to add to the genePred.
 * If genePredName2Fld is specified, then the gene_id is used for the name2
 * field.  If genePredCdsStatFld is set, then the CDS status information is
 * set based on the presences of start_codon, stop_codon, and CDS features.
 * If genePredExonFramesFld is set, then frame is set as specified in the GTF.
 */
{
return mkFromGroupedGxf(gff, group, name, TRUE, NULL, optFields);
}

static void mapCdsToGenome(struct psl *psl, struct genbankCds* cds,
                           struct genePred* gene)
/* Convert set cdsStart/end from mrna to genomic coordinates. */
{
int rnaCdsStart = cds->start,  rnaCdsEnd = cds->end;
int cdsStart = -1, cdsEnd = -1;
int iBlk;

if (psl->strand[0] == '-')
    reverseIntRange(&rnaCdsStart, &rnaCdsEnd, psl->qSize);


/* find query block or gap containing start and map to target */
for (iBlk = 0; (iBlk < psl->blockCount) && (cdsStart < 0); iBlk++)
    {
    if (rnaCdsStart < psl->qStarts[iBlk])
        {
        /* in gap before block, set to start of block */
        cdsStart = psl->tStarts[iBlk];
        }
    else if (rnaCdsStart < (psl->qStarts[iBlk] + psl->blockSizes[iBlk]))
        {
        /* in this block, map to target */
        cdsStart = psl->tStarts[iBlk] + (rnaCdsStart - psl->qStarts[iBlk]);
        }
    }
if (cdsStart < 0)
    {
    /* after last block, set after end of that block */
    cdsStart = psl->tStarts[iBlk-1] + psl->blockSizes[iBlk-1];
    }

/* find query block or gap containing end and map to target */
for (iBlk = 0; (iBlk < psl->blockCount) && (cdsEnd < 0); iBlk++)
    {
    if (rnaCdsEnd < psl->qStarts[iBlk])
        {
        /* in gap before block, set to end of gap */
        cdsEnd = psl->tStarts[iBlk];
        }
    else if (rnaCdsEnd < (psl->qStarts[iBlk] + psl->blockSizes[iBlk]))
        {
        /* in this block, map to target */
        cdsEnd = psl->tStarts[iBlk] + (rnaCdsEnd - psl->qStarts[iBlk]);
        }
    }
if (cdsEnd < 0)
    {
    /* after last block, set to end of that block */
    cdsEnd = psl->tStarts[iBlk-1] + psl->blockSizes[iBlk-1];
    }

if (psl->strand[1] == '-')
    reverseIntRange(&cdsStart, &cdsEnd, psl->tSize);

assert(cdsStart <= cdsEnd);
gene->cdsStart = cdsStart;
gene->cdsEnd = cdsEnd;
}

static void annotateCds(struct psl *psl, struct genbankCds* cds,
                        struct genePred* gene)
/* Convert cdsStart/End from mrna to genomic coordinates. */
{
if (cds == NULL)
    {
    /* no CDS, set to end */
    gene->cdsStart = psl->tEnd;
    gene->cdsEnd = psl->tEnd;
    if (gene->optFields & genePredCdsStatFld)
        {
        gene->cdsStartStat = cdsNone;
        gene->cdsEndStat = cdsNone;
        }
    }
else if ((cds->start < 0) || (cds->end <= 0))
    {
    /* unknown CDS, set to end */
    gene->cdsStart = psl->tEnd;
    gene->cdsEnd = psl->tEnd;
    if (gene->optFields & genePredCdsStatFld)
        {
        gene->cdsStartStat = cdsUnknown;
        gene->cdsEndStat = cdsUnknown;
        }
    }
else 
    {
    /* have CDS annotation, make sure it's in bounds */
    struct genbankCds adjCds = *cds;
    if (adjCds.end > psl->qSize)
        {
        adjCds.end = psl->qSize;
        adjCds.endComplete = FALSE;
        }
    mapCdsToGenome(psl, &adjCds, gene);
    if (gene->optFields & genePredCdsStatFld)
        {
        gene->cdsStartStat = (adjCds.startComplete) ? cdsComplete : cdsIncomplete;
        gene->cdsEndStat = (adjCds.endComplete) ? cdsComplete : cdsIncomplete;;
        }
    }
}

static int getFrame(struct psl *psl, int mrnaStart, int mrnaEnd,
                    struct genbankCds* cds)
/* get the starting frame for a range of mRNA.  This handles strand stuff.
 * A range is passed due to block merging. */
{
int frame = -1;
if ((cds != NULL) && cds->startComplete)
    {
    if (psl->strand[0] == '-')
        reverseIntRange(&mrnaStart, &mrnaEnd, psl->qSize);
    if ((cds->start >= mrnaStart) && (cds->start < mrnaEnd))
        {
        /* exon contains CDS start */
        frame = 0;
        }
    else if ((cds->end > mrnaStart) && (cds->start < mrnaEnd))
        {
        /* exon overlaps CDS */
        frame = (mrnaStart-cds->start) % 3;
        }
    }
return frame;
}

static void pslToExons(struct psl *psl, struct genePred *gene,
                       struct genbankCds* cds, int insertMergeSize)
/* Convert psl alignment blocks to genePred exons, merging together blocks
 * separated by small inserts as necessary.  Optionally add frame
 * information. */
{
int iBlk, iExon = -1;
int startIdx, stopIdx, idxIncr;
int mrnaStart = 0;

gene->exonStarts = needMem(psl->blockCount*sizeof(unsigned));
gene->exonEnds = needMem(psl->blockCount*sizeof(unsigned));
if (gene->optFields & genePredExonFramesFld)
    gene->exonFrames = needMem(psl->blockCount*sizeof(unsigned));

if (psl->strand[1] == '-')
    {
    startIdx = psl->blockCount-1;
    stopIdx = -1;
    idxIncr = -1;
    }
else
    {
    startIdx = 0;
    stopIdx = psl->blockCount;
    idxIncr = 1;
    }

for (iBlk = startIdx; iBlk != stopIdx; iBlk += idxIncr)
    {
    unsigned tStart = psl->tStarts[iBlk];
    unsigned tEnd = tStart + psl->blockSizes[iBlk];
    if (psl->strand[1] == '-')
        reverseIntRange(&tStart, &tEnd, psl->tSize);
    if ((iExon < 0) || (insertMergeSize < 0)
        || ((tStart - gene->exonEnds[iExon]) > insertMergeSize))
        {
        iExon++;
        gene->exonStarts[iExon] = tStart;
        mrnaStart = psl->qStarts[iBlk];
	}
    gene->exonEnds[iExon] = tEnd;
    if (gene->optFields & genePredExonFramesFld)
        gene->exonFrames[iExon] = getFrame(psl, mrnaStart, mrnaStart+psl->blockSizes[iBlk], cds);
    }
gene->exonCount = iExon+1;
}

struct genePred *genePredFromPsl2(struct psl *psl, unsigned optFields,
                                  struct genbankCds* cds, int insertMergeSize)
/* Convert a PSL of an RNA alignment to a genePred, converting a genbank CDS
 * specification string to genomic coordinates. Small inserts, no more than
 * insertMergeSize, will be dropped and the blocks merged. A negative
 * insertMergeSize disables merging of blocks. optFields is a set from
 * genePredFields, indicated what fields to create.  Zero-length CDS, or null
 * cds, creates without CDS annotation.  If cds is null, it will set status
 * fields to cdsNone.  */
{
struct genePred *gene;
AllocVar(gene);
gene->name = cloneString(psl->qName);
gene->chrom = cloneString(psl->tName);
gene->txStart = psl->tStart;
gene->txEnd = psl->tEnd;
gene->optFields = optFields;

/* get strand in genome that the positive version mRNA aligns to */
if (psl->strand[1] == '\0')
    {
    /* assumed pos target strand, so neg query would be pos target */
    gene->strand[0] = psl->strand[0];
    }
else 
    {
    /* query and target strand are different; will be neg when query pos */
    gene->strand[0] = ((psl->strand[0] != psl->strand[1]) ? '-' : '+');
    }

annotateCds(psl, cds, gene);
pslToExons(psl, gene, cds, insertMergeSize);
return gene;
}

struct genePred *genePredFromPsl(struct psl *psl, int cdsStart, int cdsEnd,
                                 int insertMergeSize)
/* Compatibility function, genePredFromPsl2 is prefered. Convert a PSL of an
 * RNA alignment to a genePred, converting a genbank CDS specification string
 * to genomic coordinates. Small inserts, no more than insertMergeSize, will
 * be dropped and the blocks merged.  CDS start or end of -1 creates without
 * CDS annotation*/
{
struct genbankCds cds;
ZeroVar(&cds);
cds.start = cdsStart;
cds.end = cdsEnd;
return genePredFromPsl2(psl, 0, &cds, insertMergeSize);

}


char* genePredGetCreateSql(char* table, unsigned optFields, unsigned options)
/* Get SQL required to create a genePred table. optFields is a bit set
 * consisting of the genePredFields values. Options are a bit set of
 * genePredCreateOpts. Returned string should be freed.
 * This will create all optional fields that preceed the highest optFields
 * column. */
{
/* the >= is used so that we create preceeding fields. */
char sqlCmd[1024];
char *binFld = (options & genePredWithBin) ? binFieldSql : "";
char *idFld = (optFields >= genePredIdFld) ? idFieldSql : "";
char *name2Fld = (optFields >= genePredName2Fld) ? name2FieldSql : "";
char *cdsStatFld = (optFields >= genePredCdsStatFld) ? cdsStatFieldSql : "";
char *exonFramesFld = (optFields >= genePredExonFramesFld) ? exonFramesFieldSql : "";

safef(sqlCmd, sizeof(sqlCmd), createSql, table,
      binFld, idFld, name2Fld, cdsStatFld, exonFramesFld);

return cloneString(sqlCmd);
}

struct genePred *getOverlappingGene(struct genePred **list, char *table, char *chrom, int cStart, int cEnd, char *name, int *retOverlap)
{
/* read all genes from a table find the gene with the biggest overlap. 
   Cache the list of genes to so we only read it once */

char query[256];
struct genePred *gene;
struct sqlConnection *conn;
struct sqlResult *sr;
boolean hasBin = 0;
char **row;
struct genePred *el = NULL, *bestMatch = NULL, *gp = NULL;
int overlap = 0 , bestOverlap = 0, i;
struct psl *psl;
int selfHit = 0;
unsigned *eFrames;


if (*list == NULL)
    {
    printf("Loading Predictions from %s\n",table);
    AllocVar(*list);
    conn = hAllocConn();
    AllocVar(gene);
    sprintf(query, "select * from %s \n",table);
    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL){
        if (!sameString(table,"all_mrna"))
            {
            el = genePredLoad(row);
            }
        else
            {
            psl = pslLoad(row);
            el = genePredFromPsl2(psl, 0, NULL, 10);
            }
        slAddHead(list, el);
        }
    slReverse(list);
    sqlFreeResult(&sr);
    hFreeConn(&conn);
    }
for (el = *list; el != NULL; el = el->next)
    {
    if (chrom != NULL && el->chrom != NULL)
        {
        overlap = 0;
        if ( sameString(chrom, el->chrom))
            {
            for (i = 0 ; i<(el->exonCount); i++)
                {
                overlap += positiveRangeIntersection(cStart,cEnd, el->exonStarts[i], el->exonEnds[i]) ;
                }
            if (overlap > 20 && sameString(name, el->name))
                {
                bestMatch = el;
                bestOverlap = overlap;
                *retOverlap = bestOverlap;
                }
            if (overlap > bestOverlap)
                {
                bestMatch = el;
                bestOverlap = overlap;
                *retOverlap = bestOverlap;
                }
            }
        }
    }
if (bestMatch != NULL)
    {
    /* Allocate genePred and fill in values. */
    AllocVar(gp);
    gp->name = cloneString(bestMatch->name);
    gp->chrom = cloneString(bestMatch->chrom);
    gp->strand[1] = bestMatch->strand[1];
    gp->strand[0] = bestMatch->strand[0];
    gp->txStart = bestMatch->txStart;
    gp->txEnd = bestMatch->txEnd;
    gp->cdsStart = bestMatch->cdsStart;
    gp->cdsEnd = bestMatch->cdsEnd;
    gp->exonCount = bestMatch->exonCount;
    AllocArray(gp->exonStarts, bestMatch->exonCount);
    AllocArray(gp->exonEnds, bestMatch->exonCount);
    for (i=0; i<bestMatch->exonCount; ++i)
        {
        gp->exonStarts[i] = bestMatch->exonStarts[i] ;
        gp->exonEnds[i] = bestMatch->exonEnds[i] ;
        }
    gp->optFields = bestMatch->optFields;
    gp->id = bestMatch->id;

    if (bestMatch->optFields & genePredName2Fld)
        gp->name2 = cloneString(bestMatch->name2);
    if (bestMatch->optFields & genePredCdsStatFld)
        {
        gp->cdsStartStat = bestMatch->cdsStartStat;
        gp->cdsEndStat = bestMatch->cdsEndStat;
        }
    if (bestMatch->optFields & genePredExonFramesFld)
        {
        gp->exonFrames = AllocArray(eFrames, bestMatch->exonCount);
        for (i = 0; i < bestMatch->exonCount; i++)
            gp->exonFrames[i] = bestMatch->exonFrames[i];
        }
    eFrames = gp->exonFrames;
    }

return gp;
}
int genePredBases(struct genePred *gp)
/* count coding and utr bases in a gene prediction */
{
int count = 0, i;

if (gp == NULL) return 0;

for (i=0; i<gp->exonCount; i++)
    {
    count += gp->exonEnds[i] - gp->exonStarts[i] ;
    }
return count;
}

int genePredCodingBases(struct genePred *gp)
/* Count up the number of coding bases in gene prediction. */
{
int i, exonCount = gp->exonCount;
int cdsStart = gp->cdsStart, cdsEnd = gp->cdsEnd;
int baseCount = 0;
for (i=0; i<exonCount; ++i)
    {
    baseCount += positiveRangeIntersection(cdsStart,cdsEnd,
    	gp->exonStarts[i], gp->exonEnds[i]);
    }
return baseCount;
}

