/* stsMap.c was originally generated by the autoSql program, which also 
 * generated stsMap.h and stsMap.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "stsMarker.h"
#include "stsMap.h"

void stsMapStaticLoad(char **row, struct stsMap *ret)
/* Load a row from stsMap table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->identNo = sqlUnsigned(row[5]);
ret->ctgAcc = row[6];
ret->otherAcc = row[7];
ret->genethonChrom = row[8];
ret->genethonPos = atof(row[9]);
ret->marshfieldChrom = row[10];
ret->marshfieldPos = atof(row[11]);
ret->gm99Gb4Chrom = row[12];
ret->gm99Gb4Pos = atof(row[13]);
ret->shgcTngChrom = row[14];
ret->shgcTngPos = atof(row[15]);
ret->shgcG3Chrom = row[16];
ret->shgcG3Pos = atof(row[17]);
ret->wiYacChrom = row[18];
ret->wiYacPos = atof(row[19]);
ret->wiRhChrom = row[20];
ret->wiRhPos = atof(row[21]);
ret->fishChrom = row[22];
ret->beginBand = row[23];
ret->endBand = row[24];
ret->lab = row[25];
ret->decodeChrom = "0";
ret->decodePos = 0;
}

void stsMapStaticLoad28(char **row, struct stsMap *ret)
/* Load a row from stsMap table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->identNo = sqlUnsigned(row[5]);
ret->ctgAcc = row[6];
ret->otherAcc = row[7];
ret->genethonChrom = row[8];
ret->genethonPos = atof(row[9]);
ret->marshfieldChrom = row[10];
ret->marshfieldPos = atof(row[11]);
ret->gm99Gb4Chrom = row[12];
ret->gm99Gb4Pos = atof(row[13]);
ret->shgcTngChrom = row[14];
ret->shgcTngPos = atof(row[15]);
ret->shgcG3Chrom = row[16];
ret->shgcG3Pos = atof(row[17]);
ret->wiYacChrom = row[18];
ret->wiYacPos = atof(row[19]);
ret->wiRhChrom = row[20];
ret->wiRhPos = atof(row[21]);
ret->fishChrom = row[22];
ret->beginBand = row[23];
ret->endBand = row[24];
ret->lab = row[25];
ret->decodeChrom = row[26];
ret->decodePos = atof(row[27]);
}

struct stsMap *stsMapLoad(char **row)
/* Load a stsMap from row fetched with select * from stsMap
 * from database.  Dispose of this with stsMapFree(). */
{
struct stsMap *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->identNo = sqlUnsigned(row[5]);
ret->ctgAcc = cloneString(row[6]);
ret->otherAcc = cloneString(row[7]);
ret->genethonChrom = cloneString(row[8]);
ret->genethonPos = atof(row[9]);
ret->marshfieldChrom = cloneString(row[10]);
ret->marshfieldPos = atof(row[11]);
ret->gm99Gb4Chrom = cloneString(row[12]);
ret->gm99Gb4Pos = atof(row[13]);
ret->shgcTngChrom = cloneString(row[14]);
ret->shgcTngPos = atof(row[15]);
ret->shgcG3Chrom = cloneString(row[16]);
ret->shgcG3Pos = atof(row[17]);
ret->wiYacChrom = cloneString(row[18]);
ret->wiYacPos = atof(row[19]);
ret->wiRhChrom = cloneString(row[20]);
ret->wiRhPos = atof(row[21]);
ret->fishChrom = cloneString(row[22]);
ret->beginBand = cloneString(row[23]);
ret->endBand = cloneString(row[24]);
ret->lab = cloneString(row[25]);
ret->decodeChrom = cloneString("0");
ret->decodePos = 0;

return ret;
}

struct stsMap *stsMapLoad28(char **row)
/* Load a stsMap from row fetched with select * from stsMap
 * from database.  Dispose of this with stsMapFree(). */
{
struct stsMap *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlSigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->identNo = sqlUnsigned(row[5]);
ret->ctgAcc = cloneString(row[6]);
ret->otherAcc = cloneString(row[7]);
ret->genethonChrom = cloneString(row[8]);
ret->genethonPos = atof(row[9]);
ret->marshfieldChrom = cloneString(row[10]);
ret->marshfieldPos = atof(row[11]);
ret->gm99Gb4Chrom = cloneString(row[12]);
ret->gm99Gb4Pos = atof(row[13]);
ret->shgcTngChrom = cloneString(row[14]);
ret->shgcTngPos = atof(row[15]);
ret->shgcG3Chrom = cloneString(row[16]);
ret->shgcG3Pos = atof(row[17]);
ret->wiYacChrom = cloneString(row[18]);
ret->wiYacPos = atof(row[19]);
ret->wiRhChrom = cloneString(row[20]);
ret->wiRhPos = atof(row[21]);
ret->fishChrom = cloneString(row[22]);
ret->beginBand = cloneString(row[23]);
ret->endBand = cloneString(row[24]);
ret->lab = cloneString(row[25]);
ret->decodeChrom = cloneString(row[26]);
ret->decodePos = atof(row[27]);
return ret;
}

struct stsMap *stsMapLoadAll(char *fileName) 
/* Load all stsMap from a tab-separated file.
 * Dispose of this with stsMapFreeList(). */
{
struct stsMap *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[26];

while (lineFileRow(lf, row))
    {
    el = stsMapLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct stsMap *stsMapCommaIn(char **pS, struct stsMap *ret)
/* Create a stsMap out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stsMap */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlSignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
ret->identNo = sqlUnsignedComma(&s);
ret->ctgAcc = sqlStringComma(&s);
ret->otherAcc = sqlStringComma(&s);
ret->genethonChrom = sqlStringComma(&s);
ret->genethonPos = sqlSignedComma(&s);
ret->marshfieldChrom = sqlStringComma(&s);
ret->marshfieldPos = sqlSignedComma(&s);
ret->gm99Gb4Chrom = sqlStringComma(&s);
ret->gm99Gb4Pos = sqlSignedComma(&s);
ret->shgcTngChrom = sqlStringComma(&s);
ret->shgcTngPos = sqlSignedComma(&s);
ret->shgcG3Chrom = sqlStringComma(&s);
ret->shgcG3Pos = sqlSignedComma(&s);
ret->wiYacChrom = sqlStringComma(&s);
ret->wiYacPos = sqlSignedComma(&s);
ret->wiRhChrom = sqlStringComma(&s);
ret->wiRhPos = sqlSignedComma(&s);
ret->fishChrom = sqlStringComma(&s);
ret->beginBand = sqlStringComma(&s);
ret->endBand = sqlStringComma(&s);
ret->lab = sqlStringComma(&s);
*pS = s;
return ret;
}

void stsMapFree(struct stsMap **pEl)
/* Free a single dynamically allocated stsMap such as created
 * with stsMapLoad(). */
{
struct stsMap *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->ctgAcc);
freeMem(el->otherAcc);
freeMem(el->genethonChrom);
freeMem(el->marshfieldChrom);
freeMem(el->gm99Gb4Chrom);
freeMem(el->shgcTngChrom);
freeMem(el->shgcG3Chrom);
freeMem(el->wiYacChrom);
freeMem(el->wiRhChrom);
freeMem(el->fishChrom);
freeMem(el->beginBand);
freeMem(el->endBand);
freeMem(el->decodeChrom);
freez(pEl);
}

void stsMapFreeList(struct stsMap **pList)
/* Free a list of dynamically allocated stsMap's */
{
struct stsMap *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    stsMapFree(&el);
    }
*pList = NULL;
}

void stsMapOutput(struct stsMap *el, FILE *f, char sep, char lastSep) 
/* Print out stsMap.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
fprintf(f, "%u", el->identNo);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->ctgAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->otherAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genethonChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->genethonPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->marshfieldChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->marshfieldPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99Gb4Chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->gm99Gb4Pos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shgcTngChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->shgcTngPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shgcG3Chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->shgcG3Pos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wiYacChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->wiYacPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wiRhChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->wiRhPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fishChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->beginBand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->endBand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->lab);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->decodeChrom);
if (sep == ',') fputc('"',f);
fprintf(f, "%f", el->decodePos);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* ------ End AutoSQL generated Code ------- */

void stsMapFromStsMarker(struct stsMarker *oldEl, struct stsMap *el)
/* Convert from older stsMarker format to stsMap format. */
{
ZeroVar(el);
el->chrom = oldEl->chrom;
el->chromStart = oldEl->chromStart;
el->chromEnd = oldEl->chromEnd;
el->name = oldEl->name;
el->score = oldEl->score;
el->identNo = oldEl->identNo;
el->ctgAcc = oldEl->ctgAcc;
el->otherAcc = oldEl->otherAcc;
el->genethonChrom = oldEl->genethonChrom;
el->genethonPos = oldEl->genethonPos;
el->marshfieldChrom = oldEl->marshfieldChrom;
el->marshfieldPos = oldEl->marshfieldPos;
el->gm99Gb4Chrom = oldEl->gm99Gb4Chrom;
el->gm99Gb4Pos = oldEl->gm99Gb4Pos;
el->shgcG3Chrom = oldEl->shgcG3Chrom;
el->shgcG3Pos = oldEl->shgcG3Pos;
el->wiYacChrom = oldEl->wiYacChrom;
el->wiYacPos = oldEl->wiYacPos;
el->shgcTngChrom = oldEl->shgcTngChrom;
el->shgcTngPos = oldEl->shgcTngPos;
el->fishChrom = oldEl->fishChrom;
el->beginBand = oldEl->beginBand;
el->endBand = oldEl->endBand;
el->wiRhChrom = "0";
el->wiRhPos = 0;
el->decodeChrom = "0";
el->decodePos = 0;
el->lab = "-";
}
