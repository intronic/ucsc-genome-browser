/* dnaMotif.c was originally generated by the autoSql program, which also 
 * generated dnaMotif.h and dnaMotif.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "sqlList.h"
#include "dystring.h"
#include "dnaMotif.h"
#include "portable.h"

static char const rcsid[] = "$Id: dnaMotif.c,v 1.4 2006/09/14 21:08:24 braney Exp $";

struct dnaMotif *dnaMotifCommaIn(char **pS, struct dnaMotif *ret)
/* Create a dnaMotif out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dnaMotif */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->columnCount = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->aProb, ret->columnCount);
for (i=0; i<ret->columnCount; ++i)
    {
    ret->aProb[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->cProb, ret->columnCount);
for (i=0; i<ret->columnCount; ++i)
    {
    ret->cProb[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->gProb, ret->columnCount);
for (i=0; i<ret->columnCount; ++i)
    {
    ret->gProb[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->tProb, ret->columnCount);
for (i=0; i<ret->columnCount; ++i)
    {
    ret->tProb[i] = sqlSignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void dnaMotifFree(struct dnaMotif **pEl)
/* Free a single dynamically allocated dnaMotif such as created
 * with dnaMotifLoad(). */
{
struct dnaMotif *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->aProb);
freeMem(el->cProb);
freeMem(el->gProb);
freeMem(el->tProb);
freez(pEl);
}

void dnaMotifFreeList(struct dnaMotif **pList)
/* Free a list of dynamically allocated dnaMotif's */
{
struct dnaMotif *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    dnaMotifFree(&el);
    }
*pList = NULL;
}

void dnaMotifOutput(struct dnaMotif *el, FILE *f, char sep, char lastSep) 
/* Print out dnaMotif.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->columnCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->columnCount; ++i)
    {
    fprintf(f, "%f", el->aProb[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->columnCount; ++i)
    {
    fprintf(f, "%f", el->cProb[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->columnCount; ++i)
    {
    fprintf(f, "%f", el->gProb[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->columnCount; ++i)
    {
    fprintf(f, "%f", el->tProb[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

float dnaMotifSequenceProb(struct dnaMotif *motif, DNA *dna)
/* Return probability of dna according to motif.  Make sure
 * motif is probabalistic (with call to dnaMotifMakeProbabalistic
 * if you're not sure) before calling this. */
{
float p = 1.0;
int i;
for (i=0; i<motif->columnCount; ++i)
    {
    switch (dna[i])
        {
	case 'a':
	case 'A':
	    p *= motif->aProb[i];
	    break;
	case 'c':
	case 'C':
	    p *= motif->cProb[i];
	    break;
	case 'g':
	case 'G':
	    p *= motif->gProb[i];
	    break;
	case 't':
	case 'T':
	    p *= motif->tProb[i];
	    break;
	case 0:
	    warn("dna shorter than motif");
	    internalErr();
	    break;
	default:
	    p *= 0.25;
	    break;
	}
    }
return p;
}

char dnaMotifBestStrand(struct dnaMotif *motif, DNA *dna)
/* Figure out which strand of DNA is better for probabalistic motif. */
{
float fScore, rScore;
fScore = dnaMotifSequenceProb(motif, dna);
reverseComplement(dna, motif->columnCount);
rScore = dnaMotifSequenceProb(motif, dna);
reverseComplement(dna, motif->columnCount);
if (fScore >= rScore)
    return '+';
else
    return '-';
}

double dnaMotifBitScore(struct dnaMotif *motif, DNA *dna)
/* Return logBase2-odds score of dna given a probabalistic motif. */
{
double p = dnaMotifSequenceProb(motif, dna);
double q = pow(0.25, motif->columnCount);
double odds = p/q;
return logBase2(odds);
}


void dnaMotifNormalize(struct dnaMotif *motif)
/* Make all columns of motif sum to one. */
{
int i;
for (i=0; i<motif->columnCount; ++i)
    {
    float sum = motif->aProb[i] + motif->cProb[i] + motif->gProb[i] + motif->tProb[i];
    if (sum < 0)
        errAbort("%s has negative numbers, perhaps it's score not probability based", 
		motif->name);
    if (sum == 0)
         motif->aProb[i] = motif->cProb[i] = motif->gProb[i] = motif->tProb[i] = 0.25;
    motif->aProb[i] /= sum;
    motif->cProb[i] /= sum;
    motif->gProb[i] /= sum;
    motif->tProb[i] /= sum;
    }
}

boolean dnaMotifIsScoreBased(struct dnaMotif *motif)
/* Return TRUE if dnaMotif is score-based (which we decide by
 * the presense of negative values. */
{
int i;
for (i=0; i<motif->columnCount; ++i)
    {
    if (motif->aProb[i] < 0) return TRUE;
    if (motif->cProb[i] < 0) return TRUE;
    if (motif->gProb[i] < 0) return TRUE;
    if (motif->tProb[i] < 0) return TRUE;
    }
return FALSE;
}

void dnaMotifScoreToProb(struct dnaMotif *motif)
/* Convert motif that is log-odds score based to motif
 * that is probability based.  This assumes that the
 * background distribution is simple: 25% for each base */
{
int i;
for (i=0; i<motif->columnCount; ++i)
    {
    motif->aProb[i] = exp(motif->aProb[i]);
    motif->cProb[i] = exp(motif->cProb[i]);
    motif->gProb[i] = exp(motif->gProb[i]);
    motif->tProb[i] = exp(motif->tProb[i]);
    }
dnaMotifNormalize(motif);
}

void dnaMotifMakeProbabalistic(struct dnaMotif *motif)
/* Change motif, which may be score or count based, to 
 * probabalistic one, where each column adds to 1.0 */
{
if (dnaMotifIsScoreBased(motif))
    dnaMotifScoreToProb(motif);
else
    dnaMotifNormalize(motif);
}

static void printProbRow(FILE *f, char *label, float *p, int pCount)
/* Print one row of a probability profile. */
{
int i;
fprintf(f, "%s ", label);
for (i=0; i < pCount; ++i)
    fprintf(f, "%5.2f ", p[i]);
printf("\n");
}

void dnaMotifPrintProb(struct dnaMotif *motif, FILE *f)
/* Print DNA motif probabilities. */
{
printProbRow(f, "A", motif->aProb, motif->columnCount);
printProbRow(f, "C", motif->cProb, motif->columnCount);
printProbRow(f, "G", motif->gProb, motif->columnCount);
printProbRow(f, "T", motif->tProb, motif->columnCount);
}


static double u1(double prob)
/* Calculate partial uncertainty for one base. */
{
if (prob == 0)
    return 0;
return prob * logBase2(prob);
}

static double uncertainty(struct dnaMotif *motif, int pos)
/* Return the uncertainty at pos of motif.  This corresponds
 * to the H function in logo.pm */
{
return -( u1(motif->aProb[pos]) + u1(motif->cProb[pos])
	+ u1(motif->gProb[pos]) +u1(motif->tProb[pos]) );
}

double dnaMotifBitsOfInfo(struct dnaMotif *motif, int pos)
/* Return bits of information at position. */
{
if (pos > motif->columnCount || pos < 0)
    internalErr();
return 2 - uncertainty(motif, pos);
}

struct letterProb
/* A letter tied to a probability. */
    {
    struct letterProb *next;
    double prob;	/* Probability for this letter. */
    char letter;	/* The letter (upper case) */
    };

static struct letterProb *letterProbNew(char letter, double prob)
/* Make a new letterProb. */
{
struct letterProb *lp;
AllocVar(lp);
lp->letter = letter;
lp->prob = prob;
return lp;
}

static int letterProbCmp(const void *va, const void *vb)
/* Compare to sort highest probability first. */
{
const struct letterProb *a = *((struct letterProb **)va);
const struct letterProb *b = *((struct letterProb **)vb);
double dif = a->prob - b->prob;
if (dif < 0)
   return -1;
else if (dif > 0)
   return 1;
else
   return 0;
}

static void addBaseProb(struct letterProb **pList, char letter, double prob)
/* If prob > 0 add letterProb to list. */
{
if (prob > 0)
    {
    struct letterProb *lp = letterProbNew(letter, prob);
    slAddHead(pList, lp);
    }
}

static struct letterProb *letterProbFromMotifColumn(struct dnaMotif *motif, int pos)
/* Return letterProb list corresponding to column of motif. */
{
struct letterProb *lpList = NULL;
addBaseProb(&lpList, 'A', motif->aProb[pos]);
addBaseProb(&lpList, 'C', motif->cProb[pos]);
addBaseProb(&lpList, 'G', motif->gProb[pos]);
addBaseProb(&lpList, 'T', motif->tProb[pos]);
slSort(&lpList, letterProbCmp);
return lpList;
}

static void psOneColumn(struct dnaMotif *motif, int pos,
    double xStart, double yStart, double width, double totalHeight,
    FILE *f)
/* Write one column of logo to postScript. */
{
struct letterProb *lp, *lpList = letterProbFromMotifColumn(motif, pos);
double x = xStart, y = yStart, w = width, h;
for (lp = lpList; lp != NULL; lp = lp->next)
    {
    h = totalHeight * lp->prob;
    if (h >= 1.0)
	{
	fprintf(f, "%cColor ", tolower(lp->letter));
	fprintf(f, "%3.2f ", x);
	fprintf(f, "%3.2f ", y);
	fprintf(f, "%3.2f ", x + w);
	fprintf(f, "%3.2f ", y + h);
	fprintf(f, "(%c) textInBox\n", lp->letter);
	}
    y += h;
    }
fprintf(f, "\n");
slFreeList(&lpList);
}

static void dnaMotifDims(struct dnaMotif *motif, double widthPerBase, double height, 
	int *retWidth, int *retHeight)
/* Calculate dimensions of motif when rendered. */
{
static int widthFudgeFactor = 2, heightFudgeFactor = 2;
*retWidth = ceil(widthPerBase * motif->columnCount) + widthFudgeFactor;
*retHeight = ceil(height) + heightFudgeFactor;
}

void dnaMotifToLogoPs(struct dnaMotif *motif, double widthPerBase, double height, char *fileName)
/* Write logo corresponding to motif to postScript file. */
{
FILE *f = mustOpen(fileName, "w");
int i;
int xStart = 0;
int w, h;
char *s = 
#include "dnaMotif.pss"
;

dnaMotifDims(motif, widthPerBase, height, &w, &h);
fprintf(f, "%%!PS-Adobe-3.1 EPSF-3.0\n");
fprintf(f, "%%%%BoundingBox: 0 0 %d %d\n\n", w, h);
fprintf(f, "%s", s);

fprintf(f, "%s", "% Start of code for this specific logo\n");

for (i=0; i<motif->columnCount; ++i)
    {
    double infoScale = dnaMotifBitsOfInfo(motif, i)/2.0;
    psOneColumn(motif, i, xStart, 0, widthPerBase, infoScale * height, f);
    xStart += widthPerBase;
    }
fprintf(f, "showpage\n");
carefulClose(&f);
}

void dnaMotifToLogoPng(
	struct dnaMotif *motif,	/* Motif to draw. */
	double widthPerBase, 	/* Width of each base. */
	double height, 		/* Max height. */
	char *gsExe, 		/* ghostscript executable, NULL for default */
	char *tempDir,          /* temp dir , NULL for default */
	char *fileName)		/* output png file name. */
/* Write logo corresponding to motif to png file. */
{
char *psName = rTempName(tempDir, "dnaMotif", ".ps");
struct dyString *dy = dyStringNew(0);
int w, h;
int sysRet;

if (gsExe == NULL) gsExe = "gs";
if (tempDir == NULL) tempDir = "/tmp";
dnaMotifToLogoPs(motif, widthPerBase, height, psName);
dnaMotifDims(motif, widthPerBase, height, &w, &h);
dyStringAppend(dy, gsExe);
dyStringAppend(dy, " -sDEVICE=png16m -sOutputFile=");
dyStringAppend(dy, fileName);
dyStringAppend(dy, " -dBATCH -dNOPAUSE -q ");
dyStringPrintf(dy, "-g%dx%d ", w, h);
dyStringAppend(dy, psName);
sysRet = system(dy->string);
if (sysRet != 0)
    errAbort("System call returned %d for:\n  %s", sysRet, dy->string);

/* Clean up. */
dyStringFree(&dy);

/* change permisssions so the webserver can access the file */
dy = newDyString(0);
dyStringPrintf(dy, "chmod 666 %s ", fileName);
sysRet = system(dy->string);

remove(psName);
}

void dnaMotifToLogoPsW(struct dnaMotif *motif, double widthPerBase, double width, double height, char *fileName)
/* Write logo corresponding to motif to postScript file. */
{
FILE *f = mustOpen(fileName, "w");
int i;
int xStart = 0;
int w, h;
char *s = 
#include "dnaMotif.pss"
;

dnaMotifDims(motif, widthPerBase, height, &w, &h);
fprintf(f, "%%!PS-Adobe-3.1 EPSF-3.0\n");
fprintf(f, "%%%%BoundingBox: 0 0 %d %d\n\n", w, h);
fprintf(f, "%s", s);

fprintf(f, "%s", "% Start of code for this specific logo\n");

for (i=0; i<motif->columnCount; ++i)
    {
    double infoScale = 0.9 ; 
    xStart = i * width / motif->columnCount;
    psOneColumn(motif, i, xStart, 0, widthPerBase, infoScale * height, f);
    }
fprintf(f, "showpage\n");
carefulClose(&f);
}

void dnaMotifToLogoPGM(
	struct dnaMotif *motif,	/* Motif to draw. */
	double widthPerBase, 	/* Width of each base. */
	double width, 		/* Max width. */
	double height, 		/* Max height. */
	char *gsExe, 		/* ghostscript executable, NULL for default */
	char *tempDir,          /* temp dir , NULL for default */
	char *fileName)		/* output png file name. */
/* Write logo corresponding to motif to pgm file. */
{
char *psName = rTempName(tempDir, "dnaMotif", ".ps");
struct dyString *dy = dyStringNew(0);
int w, h;
int sysRet;

if (gsExe == NULL) gsExe = "gs";
if (tempDir == NULL) tempDir = "/tmp";
dnaMotifToLogoPsW(motif, widthPerBase, width, height, psName);
dnaMotifDims(motif, widthPerBase, height, &w, &h);
dyStringAppend(dy, gsExe);
dyStringAppend(dy, " -sDEVICE=pgmraw -sOutputFile=");
dyStringAppend(dy, fileName);
dyStringAppend(dy, " -dBATCH -dNOPAUSE -q ");
dyStringPrintf(dy, "-g%dx%d ", (int) ceil(width), h);
dyStringAppend(dy, psName);
sysRet = system(dy->string);
if (sysRet != 0)
    errAbort("System call returned %d for:\n  %s", sysRet, dy->string);

/* Clean up. */
dyStringFree(&dy);

/* change permisssions so the webserver can access the file */
dy = newDyString(0);
dyStringPrintf(dy, "chmod 666 %s ", fileName);
sysRet = system(dy->string);

remove(psName);
}
