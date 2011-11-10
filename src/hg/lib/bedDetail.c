/* bedDetail.c was originally generated by the autoSql program, which also
 * generated bedDetail.h and bedDetail.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "basicBed.h"
#include "bedDetail.h"

static char const rcsid[] = "$Id:$";

struct bedDetail *bedDetailLoad(char **row)
/* Load a bedDetail from row fetched with select * from bedDetail
 * from database.  Dispose of this with bedDetailFree(). */
{
struct bedDetail *ret;

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
ret->id = cloneString(row[12]);
ret->description = cloneString(row[13]);
return ret;
}

struct bedDetail *bedDetailLoadAll(char *fileName)
/* Load all bedDetail from a whitespace-separated file.
 * Dispose of this with bedDetailFreeList(). */
{
struct bedDetail *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileRow(lf, row))
    {
    el = bedDetailLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bedDetail *bedDetailLoadAllByChar(char *fileName, char chopper)
/* Load all bedDetail from a chopper separated file.
 * Dispose of this with bedDetailFreeList(). */
{
struct bedDetail *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[14];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = bedDetailLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bedDetail *bedDetailCommaIn(char **pS, struct bedDetail *ret)
/* Create a bedDetail out of a comma separated string.
 * This will fill in ret if non-null, otherwise will
 * return a new bedDetail */
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
ret->id = sqlStringComma(&s);
ret->description = sqlStringComma(&s);
*pS = s;
return ret;
}

void bedDetailFree(struct bedDetail **pEl)
/* Free a single dynamically allocated bedDetail such as created
 * with bedDetailLoad(). */
{
struct bedDetail *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freeMem(el->id);
freeMem(el->description);
freez(pEl);
}

void bedDetailFreeList(struct bedDetail **pList)
/* Free a list of dynamically allocated bedDetail's */
{
struct bedDetail *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bedDetailFree(&el);
    }
*pList = NULL;
}

void bedDetailOutput(struct bedDetail *el, FILE *f, char sep, char lastSep, int size)
/* Print out bedDetail.  Separate fields with sep. Follow last field with lastSep. */
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
if (size > 6)
    {
    fprintf(f, "%u", el->score);
    fputc(sep,f);
    }
if (size > 7)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->strand);
    if (sep == ',') fputc('"',f);
    fputc(sep,f);
    }
if (size > 8)
    {
    fprintf(f, "%u", el->thickStart);
    fputc(sep,f);
    }
if (size > 9)
    {
    fprintf(f, "%u", el->thickEnd);
    fputc(sep,f);
    }
if (size > 10)
    {
    fprintf(f, "%u", el->reserved);
    fputc(sep,f);
    }
if (size > 11)
    {
    fprintf(f, "%d", el->blockCount);
    fputc(sep,f);
    }
if (size > 12)
    {
    int i;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->blockCount; ++i)
        {
        fprintf(f, "%d", el->blockSizes[i]);
        fputc(',', f);
        }
    if (sep == ',') fputc('}',f);
    fputc(sep,f);
    }
if (size > 13)
    {
    int i;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->blockCount; ++i)
        {
        fprintf(f, "%d", el->chromStarts[i]);
        fputc(',', f);
        }
    if (sep == ',') fputc('}',f);
    fputc(sep,f);
    }
/* last 2 should always be present */
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->description);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

static char *bedDetailAutoSqlString =
"table bedDetail\n"
"\"Browser extensible data, with extended fields for detail page\"\n"
"   (\n"
"   string chrom;      \"Reference sequence chromosome or scaffold\"\n"
"   uint   chromStart; \"Start position in chromosome\"\n"
"   uint   chromEnd;   \"End position in chromosome\"\n"
"   string name;       \"Short Name of item\"\n"
"   uint   score;      \"Score from 0-1000\"\n"
"   char[1] strand;    \"+ or -\"\n"
"   uint thickStart;   \"Start of where display should be thick (start codon)\"\n"
"   uint thickEnd;     \"End of where display should be thick (stop codon)\"\n"
"   uint reserved;     \"Used as itemRgb as of 2004-11-22\"\n"
"   int blockCount;    \"Number of blocks\"\n"
"   int[blockCount] blockSizes; \"Comma separated list of block sizes\"\n"
"   int[blockCount] chromStarts; \"Start positions relative to chromStart\"\n"
"   uint expCount;     \"Number of experiment values\"\n"
"   int[expCount] expIds; \"Comma separated list of experiment IDs\"\n"
"   float[expCount] expScores; \"Comma separated list of experiment scores\"\n"
"   string id;         \"ID to bed used in URL to link back\"\n"
"   lstring description; \"Long description of item for the details page\"\n"
"   )\n"
;

struct asObject *bedDetailAsObj()
// Return asObject describing fields of bedDetail
{
return asParseText(bedDetailAutoSqlString);
}

struct bedDetail *bedDetailLoadWithGaps(char **row, int size)
/* Load a bedDetail from row split from a file or from database,
   may have parts of BED not included.  Dispose of this with bedDetailFree().
   Size is total number of fields
*/
{
struct bedDetail *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
if (size > 6 && row[4] != NULL)
    ret->score = sqlUnsigned(row[4]);
if (size > 7 && row[5] != NULL)
    safecpy(ret->strand, sizeof(ret->strand), row[5]);
if (size > 8 && row[6] != NULL)
    ret->thickStart = sqlUnsigned(row[6]);
if (size > 9 && row[7] != NULL)
    ret->thickEnd = sqlUnsigned(row[7]);
if (size > 10 && row[8] != NULL)
    {
    char *comma = strchr(row[8], ',');
    if (comma)
        ret->reserved = bedParseRgb(row[8]);
    else
        ret->reserved = 0;
    }
if (size > 11 && row[9] != NULL)
    ret->blockCount = sqlSigned(row[9]);
if (size > 12 && row[10] != NULL)
    {
    int sizeOne;
    sqlSignedDynamicArray(row[10], &ret->blockSizes, &sizeOne);
    assert(sizeOne == ret->blockCount);
    }
if (size > 13 && row[11] != NULL)
    {
    int sizeOne;
    sqlSignedDynamicArray(row[11], &ret->chromStarts, &sizeOne);
    assert(sizeOne == ret->blockCount);
    }
/* these 2 should not be null, but always last 2 */
ret->id = cloneString(row[size-2]);
ret->description = cloneString(row[size-1]);
return ret;
}

struct bedDetail *bedDetailLineFileLoad (char **row, int size, struct lineFile *lf)
/* load from linefile line, with error checking */
{
struct bedDetail *item;
int count; /* block count */
int wordCount = size - 2; /* bed part of row */
AllocVar(item);
item->chrom = cloneString(row[0]);
item->chromStart = lineFileNeedNum(lf, row, 1);
item->chromEnd = lineFileNeedNum(lf, row, 2);
if (item->chromEnd < 1)
    lineFileAbort(lf, "chromEnd less than 1 (%d)", item->chromEnd);
if (item->chromEnd < item->chromStart)
    lineFileAbort(lf, "chromStart after chromEnd (%d > %d)",
        item->chromStart, item->chromEnd);
if (wordCount > 3)
     item->name = cloneString(row[3]);
if (wordCount > 4)
     item->score = lineFileNeedNum(lf, row, 4);
if (wordCount > 5)
     {
     strncpy(item->strand, row[5], sizeof(item->strand));
     if (item->strand[0] != '+' && item->strand[0] != '-' && item->strand[0] != '.')
          lineFileAbort(lf, "Expecting + or - in strand");
     }
if (wordCount > 6)
     item->thickStart = lineFileNeedNum(lf, row, 6);
else
     item->thickStart = item->chromStart;
if (wordCount > 7)
     {
     item->thickEnd = lineFileNeedNum(lf, row, 7);
     if (item->thickEnd < item->thickStart)
         lineFileAbort(lf, "thickStart after thickEnd");
     if ((item->thickStart != 0) &&
         ((item->thickStart < item->chromStart) ||
          (item->thickStart > item->chromEnd)))
         lineFileAbort(lf,
             "thickStart out of range (chromStart to chromEnd, or 0 if no CDS)");
     if ((item->thickEnd != 0) &&
         ((item->thickEnd < item->chromStart) ||
          (item->thickEnd > item->chromEnd)))
         lineFileAbort(lf,
             "thickEnd out of range for %s:%d-%d, thick:%d-%d (chromStart to chromEnd, or 0 if no CDS)",
                       item->name, item->chromStart, item->chromEnd,
                       item->thickStart, item->thickEnd);
     }
else
     item->thickEnd = item->chromEnd;
if (wordCount > 8)
    {
    char *comma;
    /*  Allow comma separated list of rgb values here   */
    comma = strchr(row[8], ',');
    if (comma)
        {
        int rgb = bedParseRgb(row[8]);
        if (rgb < 0)
            lineFileAbort(lf,
                "Expecting 3 comma separated numbers for r,g,b bed item color.");
        else
            item->reserved = rgb;
        }
    else
        item->reserved = lineFileNeedNum(lf, row, 8);
    }
if (wordCount > 9)
    item->blockCount = lineFileNeedNum(lf, row, 9);
if (wordCount > 10)
    {
    sqlSignedDynamicArray(row[10], &item->blockSizes, &count);
    if (count != item->blockCount)
        lineFileAbort(lf,  "expecting %d elements in array", item->blockCount);
    }
if (wordCount > 11)
    {
    int i;
    int lastEnd, lastStart;
    sqlSignedDynamicArray(row[11], &item->chromStarts, &count);
    if (count != item->blockCount)
        lineFileAbort(lf, "expecting %d elements in array", item->blockCount);
    // tell the user if they appear to be using absolute starts rather than
    // relative... easy to forget!  Also check block order, coord ranges...
    lastStart = -1;
    lastEnd = 0;
    for (i=0;  i < item->blockCount;  i++)
        {
        if (item->chromStarts[i]+item->chromStart >= item->chromEnd)
            {
            if (item->chromStarts[i] >= item->chromStart)
                lineFileAbort(lf,
                    "BED chromStarts offsets must be relative to chromStart, "
                    "not absolute.  Try subtracting chromStart from each offset "
                    "in chromStarts.");
            else
                lineFileAbort(lf,
                    "BED chromStarts[i]+chromStart must be less than chromEnd.");
            }
        lastStart = item->chromStarts[i];
        lastEnd = item->chromStart + item->chromStarts[i] + item->blockSizes[i];
        }
    if (item->chromStarts[0] != 0)
        lineFileAbort(lf,
            "BED blocks must span chromStart to chromEnd.  "
            "BED chromStarts[0] must be 0 (==%d) so that (chromStart + "
            "chromStarts[0]) equals chromStart.", item->chromStarts[0]);
    i = item->blockCount-1;
    if ((item->chromStart + item->chromStarts[i] + item->blockSizes[i]) !=
        item->chromEnd)
        {
        lineFileAbort(lf,
            "BED blocks must span chromStart to chromEnd.  (chromStart + "
            "chromStarts[last] + blockSizes[last]) must equal chromEnd.");
        }
    }
    /* these 2 should not be null (maybe empty string), but always last 2 */
    item->id = cloneString(row[size-2]);
    item->description = cloneString(row[size-1]);
return item;
}

