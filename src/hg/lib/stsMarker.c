/* stsMarker.c was originally generated by the autoSql program, which also 
 * generated stsMarker.h and stsMarker.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "stsMarker.h"

static char const rcsid[] = "$Id: stsMarker.c,v 1.3 2003/05/06 07:22:23 kate Exp $";

void stsMarkerStaticLoad(char **row, struct stsMarker *ret)
/* Load a row from stsMarker table into ret.  The contents of ret will
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
ret->shgcG3Chrom = row[14];
ret->shgcG3Pos = atof(row[15]);
ret->wiYacChrom = row[16];
ret->wiYacPos = atof(row[17]);
ret->shgcTngChrom = row[18];
ret->shgcTngPos = atof(row[19]);
ret->fishChrom = row[20];
ret->beginBand = row[21];
ret->endBand = row[22];
}

struct stsMarker *stsMarkerLoad(char **row)
/* Load a stsMarker from row fetched with select * from stsMarker
 * from database.  Dispose of this with stsMarkerFree(). */
{
struct stsMarker *ret;
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
ret->shgcG3Chrom = cloneString(row[14]);
ret->shgcG3Pos = atof(row[15]);
ret->wiYacChrom = cloneString(row[16]);
ret->wiYacPos = atof(row[17]);
ret->shgcTngChrom = cloneString(row[18]);
ret->shgcTngPos = atof(row[19]);
ret->fishChrom = cloneString(row[20]);
ret->beginBand = cloneString(row[21]);
ret->endBand = cloneString(row[22]);
return ret;
}

struct stsMarker *stsMarkerCommaIn(char **pS, struct stsMarker *ret)
/* Create a stsMarker out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stsMarker */
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
ret->shgcG3Chrom = sqlStringComma(&s);
ret->shgcG3Pos = sqlSignedComma(&s);
ret->wiYacChrom = sqlStringComma(&s);
ret->wiYacPos = sqlSignedComma(&s);
ret->shgcTngChrom = sqlStringComma(&s);
ret->shgcTngPos = sqlSignedComma(&s);
ret->fishChrom = sqlStringComma(&s);
ret->beginBand = sqlStringComma(&s);
ret->endBand = sqlStringComma(&s);
*pS = s;
return ret;
}

void stsMarkerFree(struct stsMarker **pEl)
/* Free a single dynamically allocated stsMarker such as created
 * with stsMarkerLoad(). */
{
struct stsMarker *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->ctgAcc);
freeMem(el->otherAcc);
freeMem(el->genethonChrom);
freeMem(el->marshfieldChrom);
freeMem(el->gm99Gb4Chrom);
freeMem(el->shgcG3Chrom);
freeMem(el->wiYacChrom);
freeMem(el->shgcTngChrom);
freeMem(el->fishChrom);
freeMem(el->beginBand);
freeMem(el->endBand);
freez(pEl);
}

void stsMarkerFreeList(struct stsMarker **pList)
/* Free a list of dynamically allocated stsMarker's */
{
struct stsMarker *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    stsMarkerFree(&el);
    }
*pList = NULL;
}

void stsMarkerOutput(struct stsMarker *el, FILE *f, char sep, char lastSep) 
/* Print out stsMarker.  Separate fields with sep. Follow last field with lastSep. */
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
fprintf(f, "%s", el->shgcTngChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->shgcTngPos);
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
fputc(lastSep,f);
}

