/* stsInfo.c was originally generated by the autoSql program, which also 
 * generated stsInfo.h and stsInfo.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "stsInfo.h"

struct stsInfo *stsInfoLoad(char **row)
/* Load a stsInfo from row fetched with select * from stsInfo
 * from database.  Dispose of this with stsInfoFree(). */
{
struct stsInfo *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->gbCount = sqlUnsigned(row[2]);
ret->gdbCount = sqlUnsigned(row[4]);
ret->nameCount = sqlUnsigned(row[6]);
ret->otherDbstsCount = sqlUnsigned(row[9]);
ret->otherUCSCcount = sqlUnsigned(row[16]);
ret->otherUCSCcount = sqlUnsigned(row[18]);
ret->identNo = sqlUnsigned(row[0]);
ret->name = cloneString(row[1]);
sqlStringDynamicArray(row[3], &ret->genbank, &sizeOne);
assert(sizeOne == ret->gbCount);
sqlStringDynamicArray(row[5], &ret->gdb, &sizeOne);
assert(sizeOne == ret->gdbCount);
sqlStringDynamicArray(row[7], &ret->otherNames, &sizeOne);
assert(sizeOne == ret->nameCount);
ret->dbSTSid = sqlUnsigned(row[8]);
sqlUnsignedDynamicArray(row[10], &ret->otherDbSTS, &sizeOne);
assert(sizeOne == ret->otherDbstsCount);
ret->leftPrimer = cloneString(row[11]);
ret->rightPrimer = cloneString(row[12]);
ret->distance = cloneString(row[13]);
ret->organism = cloneString(row[14]);
ret->sequence = sqlUnsigned(row[15]);
sqlUnsignedDynamicArray(row[17], &ret->otherUCSC, &sizeOne);
assert(sizeOne == ret->otherUCSCcount);
sqlUnsignedDynamicArray(row[19], &ret->otherUCSC, &sizeOne);
assert(sizeOne == ret->otherUCSCcount);
ret->genethonName = cloneString(row[20]);
ret->genethonChr = cloneString(row[21]);
ret->genethonPos = atof(row[22]);
ret->genethonLOD = atof(row[23]);
ret->marshfieldName = cloneString(row[24]);
ret->marshfieldChr = cloneString(row[25]);
ret->marshfieldPos = atof(row[26]);
ret->marshfieldLOD = atof(row[27]);
ret->wiyacName = cloneString(row[28]);
ret->wiyacChr = cloneString(row[29]);
ret->wiyacPos = atof(row[30]);
ret->wiyacLOD = atof(row[31]);
ret->wirhName = cloneString(row[32]);
ret->wirhChr = cloneString(row[33]);
ret->wirhPos = atof(row[34]);
ret->wirhLOD = atof(row[35]);
ret->gm99gb4Name = cloneString(row[36]);
ret->gm99gb4Chr = cloneString(row[37]);
ret->gm99gb4Pos = atof(row[38]);
ret->gm99gb4LOD = atof(row[39]);
ret->gm99g3Name = cloneString(row[40]);
ret->gm99g3Chr = cloneString(row[41]);
ret->gm99g3Pos = atof(row[42]);
ret->gm99g3LOD = atof(row[43]);
ret->tngName = cloneString(row[44]);
ret->tngChr = cloneString(row[45]);
ret->tngPos = atof(row[46]);
ret->tngLOD = atof(row[47]);
return ret;
}

struct stsInfo *stsInfoLoadAll(char *fileName) 
/* Load all stsInfo from a tab-separated file.
 * Dispose of this with stsInfoFreeList(). */
{
struct stsInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[48];

while (lineFileRow(lf, row))
    {
    el = stsInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct stsInfo *stsInfoCommaIn(char **pS, struct stsInfo *ret)
/* Create a stsInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stsInfo */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->identNo = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->gbCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->genbank, ret->gbCount);
for (i=0; i<ret->gbCount; ++i)
    {
    ret->genbank[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->gdbCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->gdb, ret->gdbCount);
for (i=0; i<ret->gdbCount; ++i)
    {
    ret->gdb[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->nameCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherNames, ret->nameCount);
for (i=0; i<ret->nameCount; ++i)
    {
    ret->otherNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->dbSTSid = sqlUnsignedComma(&s);
ret->otherDbstsCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherDbSTS, ret->otherDbstsCount);
for (i=0; i<ret->otherDbstsCount; ++i)
    {
    ret->otherDbSTS[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->leftPrimer = sqlStringComma(&s);
ret->rightPrimer = sqlStringComma(&s);
ret->distance = sqlStringComma(&s);
ret->organism = sqlStringComma(&s);
ret->sequence = sqlUnsignedComma(&s);
ret->otherUCSCcount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherUCSC, ret->otherUCSCcount);
for (i=0; i<ret->otherUCSCcount; ++i)
    {
    ret->otherUCSC[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->otherUCSCcount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherUCSC, ret->otherUCSCcount);
for (i=0; i<ret->otherUCSCcount; ++i)
    {
    ret->otherUCSC[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->genethonName = sqlStringComma(&s);
ret->genethonChr = sqlStringComma(&s);
ret->genethonPos = sqlSignedComma(&s);
ret->genethonLOD = sqlSignedComma(&s);
ret->marshfieldName = sqlStringComma(&s);
ret->marshfieldChr = sqlStringComma(&s);
ret->marshfieldPos = sqlSignedComma(&s);
ret->marshfieldLOD = sqlSignedComma(&s);
ret->wiyacName = sqlStringComma(&s);
ret->wiyacChr = sqlStringComma(&s);
ret->wiyacPos = sqlSignedComma(&s);
ret->wiyacLOD = sqlSignedComma(&s);
ret->wirhName = sqlStringComma(&s);
ret->wirhChr = sqlStringComma(&s);
ret->wirhPos = sqlSignedComma(&s);
ret->wirhLOD = sqlSignedComma(&s);
ret->gm99gb4Name = sqlStringComma(&s);
ret->gm99gb4Chr = sqlStringComma(&s);
ret->gm99gb4Pos = sqlSignedComma(&s);
ret->gm99gb4LOD = sqlSignedComma(&s);
ret->gm99g3Name = sqlStringComma(&s);
ret->gm99g3Chr = sqlStringComma(&s);
ret->gm99g3Pos = sqlSignedComma(&s);
ret->gm99g3LOD = sqlSignedComma(&s);
ret->tngName = sqlStringComma(&s);
ret->tngChr = sqlStringComma(&s);
ret->tngPos = sqlSignedComma(&s);
ret->tngLOD = sqlSignedComma(&s);
*pS = s;
return ret;
}

void stsInfoFree(struct stsInfo **pEl)
/* Free a single dynamically allocated stsInfo such as created
 * with stsInfoLoad(). */
{
struct stsInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
/* All strings in genbank are allocated at once, so only need to free first. */
if (el->genbank != NULL)
    freeMem(el->genbank[0]);
freeMem(el->genbank);
/* All strings in gdb are allocated at once, so only need to free first. */
if (el->gdb != NULL)
    freeMem(el->gdb[0]);
freeMem(el->gdb);
/* All strings in otherNames are allocated at once, so only need to free first. */
if (el->otherNames != NULL)
    freeMem(el->otherNames[0]);
freeMem(el->otherNames);
freeMem(el->otherDbSTS);
freeMem(el->leftPrimer);
freeMem(el->rightPrimer);
freeMem(el->distance);
freeMem(el->organism);
freeMem(el->otherUCSC);
freeMem(el->otherUCSC);
freeMem(el->genethonName);
freeMem(el->genethonChr);
freeMem(el->marshfieldName);
freeMem(el->marshfieldChr);
freeMem(el->wiyacName);
freeMem(el->wiyacChr);
freeMem(el->wirhName);
freeMem(el->wirhChr);
freeMem(el->gm99gb4Name);
freeMem(el->gm99gb4Chr);
freeMem(el->gm99g3Name);
freeMem(el->gm99g3Chr);
freeMem(el->tngName);
freeMem(el->tngChr);
freez(pEl);
}

void stsInfoFreeList(struct stsInfo **pList)
/* Free a list of dynamically allocated stsInfo's */
{
struct stsInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    stsInfoFree(&el);
    }
*pList = NULL;
}

void stsInfoOutput(struct stsInfo *el, FILE *f, char sep, char lastSep) 
/* Print out stsInfo.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->identNo);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->gbCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->gbCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->genbank[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->gdbCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->gdbCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->gdb[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->nameCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->nameCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->otherNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->dbSTSid);
fputc(sep,f);
fprintf(f, "%u", el->otherDbstsCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->otherDbstsCount; ++i)
    {
    fprintf(f, "%u", el->otherDbSTS[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->leftPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rightPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->distance);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->organism);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->sequence);
fputc(sep,f);
fprintf(f, "%u", el->otherUCSCcount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->otherUCSCcount; ++i)
    {
    fprintf(f, "%u", el->otherUCSC[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->otherUCSCcount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->otherUCSCcount; ++i)
    {
    fprintf(f, "%u", el->otherUCSC[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genethonName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genethonChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->genethonPos);
fputc(sep,f);
fprintf(f, "%f", el->genethonLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->marshfieldName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->marshfieldChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->marshfieldPos);
fputc(sep,f);
fprintf(f, "%f", el->marshfieldLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wiyacName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wiyacChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->wiyacPos);
fputc(sep,f);
fprintf(f, "%f", el->wiyacLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wirhName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wirhChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->wirhPos);
fputc(sep,f);
fprintf(f, "%f", el->wirhLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99gb4Name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99gb4Chr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->gm99gb4Pos);
fputc(sep,f);
fprintf(f, "%f", el->gm99gb4LOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99g3Name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99g3Chr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->gm99g3Pos);
fputc(sep,f);
fprintf(f, "%f", el->gm99g3LOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tngName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tngChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->tngPos);
fputc(sep,f);
fprintf(f, "%f", el->tngLOD);
fputc(lastSep,f);
}

