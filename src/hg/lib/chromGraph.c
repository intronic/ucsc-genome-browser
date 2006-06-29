/* chromGraph.c was originally generated by the autoSql program, which also 
 * generated chromGraph.h and chromGraph.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "obscure.h"
#include "jksql.h"
#include "sig.h"
#include "trackDb.h"
#include "chromGraph.h"

static char const rcsid[] = "$Id: chromGraph.c,v 1.9 2006/06/29 01:26:20 kent Exp $";

void chromGraphStaticLoad(char **row, struct chromGraph *ret)
/* Load a row from chromGraph table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlSigned(row[1]);
ret->val = atof(row[2]);
}

struct chromGraph *chromGraphLoad(char **row)
/* Load a chromGraph from row fetched with select * from chromGraph
 * from database.  Dispose of this with chromGraphFree(). */
{
struct chromGraph *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlSigned(row[1]);
ret->val = atof(row[2]);
return ret;
}

struct chromGraph *chromGraphLoadAll(char *fileName) 
/* Load all chromGraph from a whitespace-separated file.
 * Dispose of this with chromGraphFreeList(). */
{
struct chromGraph *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = chromGraphLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct chromGraph *chromGraphLoadAllByChar(char *fileName, char chopper) 
/* Load all chromGraph from a chopper separated file.
 * Dispose of this with chromGraphFreeList(). */
{
struct chromGraph *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = chromGraphLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct chromGraph *chromGraphCommaIn(char **pS, struct chromGraph *ret)
/* Create a chromGraph out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chromGraph */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlSignedComma(&s);
ret->val = sqlFloatComma(&s);
*pS = s;
return ret;
}

void chromGraphFree(struct chromGraph **pEl)
/* Free a single dynamically allocated chromGraph such as created
 * with chromGraphLoad(). */
{
struct chromGraph *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freez(pEl);
}

void chromGraphFreeList(struct chromGraph **pList)
/* Free a list of dynamically allocated chromGraph's */
{
struct chromGraph *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    chromGraphFree(&el);
    }
*pList = NULL;
}

void chromGraphOutput(struct chromGraph *el, FILE *f, char sep, char lastSep) 
/* Print out chromGraph.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->chromStart);
fputc(sep,f);
fprintf(f, "%g", el->val);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

int chromGraphCmp(const void *va, const void *vb)
/* Compare to sort based on query chromStart. */
{
const struct chromGraph *a = *((struct chromGraph **)va);
const struct chromGraph *b = *((struct chromGraph **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}

void chromGraphGetMinMax(struct chromGraph *list, double *pMin, double *pMax)
/* Figure out min/max values in list. */
{
double minVal = 0, maxVal = 0;
if (list != NULL)
    {
    struct chromGraph *el;
    minVal = maxVal = list->val;
    for (el = list->next; el != NULL; el = el->next)
        {
	double val = el->val;
	if (val < minVal)
	    minVal = val;
	if (val > maxVal)
	    maxVal = val;
	}
    }
*pMin = minVal;
*pMax = maxVal;
}

void chromGraphVarName(char *track, char *var, 
	char output[chromGraphVarNameMaxSize])
/* Fill in output with name of cart variable. */
{
safef(output, chromGraphVarNameMaxSize, "%s_%s_%s", "cgs", track, var);
}

void chromGraphDataRange(char *trackName, struct sqlConnection *conn,
	double *retMin, double *retMax)
/* Get min/max values observed from metaChromGraph table */
{
char query[256];
struct sqlResult *sr;
char **row;
safef(query, sizeof(query), 
    "select minVal,maxVal from metaChromGraph where name='%s'",
    trackName);
sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row == NULL)
    errAbort("%s is not in metaChromGraph", trackName);
*retMin = atof(row[0]);
*retMax = atof(row[1]);
sqlFreeResult(&sr);
}

struct slName *chromGraphListAll(struct sqlConnection *conn)
/* Return list of all chrom graph tables. */
{
if (!sqlTableExists(conn, "metaChromGraph"))
    return NULL;
else
    return sqlQuickList(conn, "select name from metaChromGraph");
}

char *chromGraphBinaryFileName(char *trackName, struct sqlConnection *conn)
/* Get binary file name associated with chromGraph track. Returns NULL
 * if no such file or track. FreeMem result when done. */
{
char query[256];
safef(query, sizeof(query), 
	"select binaryFile from metaChromGraph where name='%s'", trackName);
return sqlQuickString(conn, query);
}

void chromGraphParseMinMax(char *trackName, char *text, 
	double *pMin, double *pMax)
/* Parse out min,max from text.  TrackName is just for error reporting */
{
struct slName *list = commaSepToSlNames(text);
if (slCount(list) != 2)
    errAbort("minMax must have two values in %s", trackName);
*pMin = atof(list->name);
*pMax = atof(list->next->name);
slFreeList(&list);
}

void chromGraphSettingsFillFromHash(struct chromGraphSettings *cgs, 
	struct hash *hash, char *trackName)
/* Fill in settings from hash table. TrackName is just for error reporting. */
{
char *setting;
if ((setting = hashFindVal(hash, "maxGapToFill")) != NULL)
    cgs->maxGapToFill = sqlUnsigned(setting);
if ((setting = hashFindVal(hash, "minMax")) != NULL)
    chromGraphParseMinMax(trackName, setting, &cgs->minVal, &cgs->maxVal);
/* They can store min/max separately as well as together... */
if ((setting = hashFindVal(hash, "maxVal")) != NULL)
    cgs->maxVal = atof(setting);
if ((setting = hashFindVal(hash, "minVal")) != NULL)
    cgs->minVal = atof(setting);
if ((setting = hashFindVal(hash, "linesAt")) != NULL)
    {
    struct slName *el, *list = commaSepToSlNames(setting);
    int i;
    cgs->linesAtCount = slCount(list);
    if (cgs->linesAtCount <= 0)
	errAbort("Missing linesAt data in %s", trackName);
    AllocArray(cgs->linesAt, cgs->linesAtCount);
    for (i=0,el=list; el!=NULL; ++i,el=el->next)
	cgs->linesAt[i] = atof(el->name);
    slFreeList(&list);
    }
if ((setting = hashFindVal(hash, "maxHeightPixels")) != NULL)
    {
    struct slName *list = charSepToSlNames(setting, ':');
    if (slCount(list) != 3)
	errAbort("maxHeightPixels in %s must have 3 : separated fields", 
	    trackName);
    cgs->maxPixels = sqlUnsigned(list->name);
    cgs->pixels = sqlUnsigned(list->next->name);
    cgs->minPixels = sqlUnsigned(list->next->next->name);
    }

}

struct chromGraphSettings *chromGraphSettingsGet(char *trackName,
	struct sqlConnection *conn, struct trackDb *tdb, struct cart *cart)
/* Get settings for chromGraph track.  If you pass in all NULLs
 * you'll get a reasonable default. */
{
struct chromGraphSettings *cgs;

/* Allocate in memory with default settings */
AllocVar(cgs);
cgs->maxGapToFill = chromGraphDefaultGapToFill;
cgs->minVal = 0;
cgs->maxVal = 100;
cgs->minPixels = 8;
cgs->pixels = 32;
cgs->maxPixels = 128;

/* Try and fill in max/min from database. */
if (conn != NULL)
    {
    chromGraphDataRange(trackName, conn, &cgs->minVal, &cgs->maxVal);
    }

/* Try and fill in settings from tdb */
if (tdb != NULL)
    {
    struct hash *hash = trackDbHashSettings(tdb);
    chromGraphSettingsFillFromHash(cgs, hash, trackName);
    }

/* Finally try and fill in settings from cart */
if (cart != NULL)
    {
    char varName[chromGraphVarNameMaxSize];
    chromGraphVarName(trackName, "maxGapToFill", varName);
    cgs->maxGapToFill = cartUsualInt(cart, varName, cgs->maxGapToFill);
    chromGraphVarName(trackName, "minVal", varName);
    cgs->minVal = cartUsualDouble(cart, varName, cgs->minVal);
    chromGraphVarName(trackName, "maxVal", varName);
    cgs->maxVal = cartUsualDouble(cart, varName, cgs->maxVal);
    chromGraphVarName(trackName, "pixels", varName);
    cgs->pixels = cartUsualInt(cart, varName, cgs->pixels);
    if (cgs->pixels < cgs->minPixels)
        cgs->pixels = cgs->minPixels;
    if (cgs->pixels > cgs->maxPixels)
        cgs->pixels = cgs->maxPixels;
    }
return cgs;
}

void cgbChromFree(struct cgbChrom **pChrom)
/* Free up one cgbChrom */
{
struct cgbChrom *chrom = *pChrom;
if (chrom != NULL)
    {
    freeMem(chrom->name);
    freez(pChrom);
    }
}

void cgbChromFreeList(struct cgbChrom **pList)
/* Free a list of dynamically allocated cgbChrom's */
{
struct cgbChrom *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    cgbChromFree(&el);
    }
*pList = NULL;
}

struct cInfo
/* Local structure to help us keep track of chromosomes */
    {
    struct cInfo *next;
    char *name;	/* Not allocated here. */
    struct chromGraph *start;	/* First element in this chrom */
    struct chromGraph *end;	/* First element not in this chrom */
    bits64 offset;		/* Offset to start of chrom in file */
    };

struct cInfo *cInfoMake(struct chromGraph *cgList, char *fileName)
/* Go through cgList and create cInfo list pointing into
 * start and end of each chromosome.  slFreeList this when done. */
{
struct chromGraph *cg; 
int lastPos = -1;
struct hash *uniqHash = hashNew(0);
struct cInfo *ciList = NULL, *ci = NULL;
for (cg = cgList; cg != NULL; cg = cg->next)
    {
    if (ci == NULL || !sameString(ci->name, cg->chrom))
        {
	if (hashLookup(uniqHash, cg->chrom) != NULL)
	    errAbort("%s isn't sorted by chrom,start", fileName);
	hashAdd(uniqHash, cg->chrom, NULL);
	if (ci != NULL)
	    ci->end = cg;
	AllocVar(ci);
	ci->name = cg->chrom;
	ci->start = cg;
	slAddHead(&ciList, ci);
	lastPos = -1;
	}
    else
        {
	if (cg->chromStart < lastPos)
	    errAbort("%s isn't sorted by chrom,start", fileName);
	lastPos = cg->chromStart;
	}
    }
hashFree(&uniqHash);
slReverse(&ciList);
return ciList;
}

void chromGraphToBin(struct chromGraph *list, char *fileName)
/* Create binary representation of chromGraph list, which should
 * be sorted. */
{
struct chromGraph *el;
FILE *f = mustOpen(fileName, "wb");
bits32 sig = chromGraphSig;
bits32 endMarker = (bits32)(-1);
struct cInfo *ci, *ciList = cInfoMake(list, fileName);
bits32 chromCount = slCount(ciList);
fpos_t indexPos;
double minVal, maxVal;
bits32 reserved1=0, reserved2=0, reserved3=0, reserved4=0;

/* Start out with file signature and chromosome count */
writeOne(f, sig);
writeOne(f, chromCount);

/* Figure and write min/max values. */
chromGraphGetMinMax(list, &minVal, &maxVal);
writeOne(f, minVal);
writeOne(f, maxVal);

/* Write out reserved (currently zero) words */
writeOne(f, reserved1);
writeOne(f, reserved2);
writeOne(f, reserved3);
writeOne(f, reserved4);

/* Write preliminary version of index, with offsets not filled in */
fgetpos(f, &indexPos);
for (ci = ciList; ci != NULL; ci = ci->next)
    {
    writeString(f, ci->name);
    writeBits64(f, ci->offset);
    }

/* Write data. */
for (ci = ciList; ci  != NULL; ci = ci->next)
    {
    ci->offset = ftell(f);
    writeString(f, ci->name);
    for (el = ci->start; el != ci->end; el = el->next)
        {
	bits32 pos = el->chromStart;
	writeOne(f, pos);
	writeOne(f, el->val);
	}
    writeOne(f, endMarker);
    }

/* Go back and rewrite index. */
fsetpos(f, &indexPos);
for (ci = ciList; ci != NULL; ci = ci->next)
    {
    writeString(f, ci->name);
    writeBits64(f, ci->offset);
    }
carefulClose(&f);
slFreeList(&ciList);
}
	  

struct chromGraphBin *chromGraphBinOpen(char *path)
/* Open up a chromGraphBin file */
{
struct chromGraphBin *cgb;
FILE *f = mustOpen(path, "rb");
bits32 sig;
bits32 chromCount, i;
boolean isSwapped = FALSE;
struct cgbChrom *chrom;
bits32 reserved1, reserved2, reserved3, reserved4;

/* Read in signature and use it to make sure it's the right type
 * of file, and to tell if we need to swap bytes on integers. */
if (!readOne(f, sig))
    errAbort("%s is empty", path);
if (sig == chromGraphSig)
    isSwapped = FALSE;
else if (sig == chromGraphSwapSig)
    isSwapped = TRUE;
else
    errAbort("%s is not a chromGraph binary file", path);

/* Allocate object and fill in several fields*/
AllocVar(cgb);
cgb->fileName = cloneString(path);
cgb->f = f;
cgb->isSwapped = isSwapped;
cgb->chromHash = hashNew(0);

/* Read in chromosome count, swapping if need be. */
mustReadOne(f, chromCount);
if (isSwapped)
    chromCount = byteSwap32(chromCount);

/* Read in min/max */
mustReadOne(f, cgb->minVal);
mustReadOne(f, cgb->maxVal);

/* Read in reserved (currently zero) words */
mustReadOne(f, reserved1);
mustReadOne(f, reserved2);
mustReadOne(f, reserved3);
mustReadOne(f, reserved4);

/* Read index into list/hash */
for (i=0; i<chromCount; ++i)
    {
    AllocVar(chrom);
    chrom->name =  readString(f);
    chrom->offset = readBits64(f);
    slAddHead(&cgb->chromList, chrom);
    hashAdd(cgb->chromHash, chrom->name, chrom);
    }
return cgb;
}

boolean chromGraphBinSeekToChrom(struct chromGraphBin *cgb, char *chromName)
/* Seek to chromosome if have data for it.  Otherwise return FALSE. */
{
struct cgbChrom *chrom = hashFindVal(cgb->chromHash, chromName);
if (chrom == NULL)
    return FALSE;
fseek(cgb->f, chrom->offset, SEEK_SET);
chromGraphBinNextChrom(cgb);
return TRUE;
}

void chromGraphBinFree(struct chromGraphBin **pCgb)
/* Close down and free up chromGraphBin. */
{
struct chromGraphBin *cgb = *pCgb;
if (cgb != NULL)
     {
     carefulClose(&cgb->f);
     freeMem(cgb->fileName);
     hashFree(&cgb->chromHash);
     cgbChromFreeList(&cgb->chromList);
     freez(pCgb);
     }
}

boolean chromGraphBinNextChrom(struct chromGraphBin *cgb)
/* Fetch next chromosome, or FALSE if at end of file. */
{
UBYTE size;
FILE *f = cgb->f;
if (!readOne(f, size))
    return FALSE;
mustRead(f, cgb->chrom, size);
cgb->chrom[size] = 0;
return TRUE;
}

boolean chromGraphBinNextVal(struct chromGraphBin *cgb)
/* Fetch next chromStart/val or FALSE if at end of chromosome. */
{
FILE *f = cgb->f;
mustReadOne(f, cgb->chromStart);
if (cgb->isSwapped)
    cgb->chromStart = byteSwap32(cgb->chromStart);
if (cgb->chromStart == (bits32)(-1))
    return FALSE;
mustReadOne(f, cgb->val);
return TRUE;
}

