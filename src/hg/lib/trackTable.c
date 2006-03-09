/* trackTable.c was originally generated by the autoSql program, which also 
 * generated trackTable.h and trackTable.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "jksql.h"
#include "trackTable.h"
#include "hdb.h"

static char const rcsid[] = "$Id: trackTable.c,v 1.6 2006/03/09 18:26:58 angie Exp $";

void trackTableStaticLoad(char **row, struct trackTable *ret)
/* Load a row from trackTable table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->mapName = row[0];
ret->tableName = row[1];
ret->shortLabel = row[2];
ret->longLabel = row[3];
ret->visibility = sqlUnsigned(row[4]);
ret->colorR = sqlUnsigned(row[5]);
ret->colorG = sqlUnsigned(row[6]);
ret->colorB = sqlUnsigned(row[7]);
ret->altColorR = sqlUnsigned(row[8]);
ret->altColorG = sqlUnsigned(row[9]);
ret->altColorB = sqlUnsigned(row[10]);
ret->useScore = sqlUnsigned(row[11]);
ret->isSplit = sqlUnsigned(row[12]);
ret->private = sqlUnsigned(row[13]);
}

struct trackTable *trackTableLoad(char **row)
/* Load a trackTable from row fetched with select * from trackTable
 * from database.  Dispose of this with trackTableFree(). */
{
struct trackTable *ret;

AllocVar(ret);
ret->mapName = cloneString(row[0]);
ret->tableName = cloneString(row[1]);
ret->shortLabel = cloneString(row[2]);
ret->longLabel = cloneString(row[3]);
ret->visibility = sqlUnsigned(row[4]);
ret->colorR = sqlUnsigned(row[5]);
ret->colorG = sqlUnsigned(row[6]);
ret->colorB = sqlUnsigned(row[7]);
ret->altColorR = sqlUnsigned(row[8]);
ret->altColorG = sqlUnsigned(row[9]);
ret->altColorB = sqlUnsigned(row[10]);
ret->useScore = sqlUnsigned(row[11]);
ret->isSplit = sqlUnsigned(row[12]);
ret->private = sqlUnsigned(row[13]);
return ret;
}

struct trackTable *trackTableLoadAll(char *fileName) 
/* Load all trackTable from a tab-separated file.
 * Dispose of this with trackTableFreeList(). */
{
struct trackTable *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[15];

while (lineFileRow(lf, row))
    {
    el = trackTableLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct trackTable *trackTableCommaIn(char **pS, struct trackTable *ret)
/* Create a trackTable out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new trackTable */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->mapName = sqlStringComma(&s);
ret->tableName = sqlStringComma(&s);
ret->shortLabel = sqlStringComma(&s);
ret->longLabel = sqlStringComma(&s);
ret->visibility = sqlUnsignedComma(&s);
ret->colorR = sqlUnsignedComma(&s);
ret->colorG = sqlUnsignedComma(&s);
ret->colorB = sqlUnsignedComma(&s);
ret->altColorR = sqlUnsignedComma(&s);
ret->altColorG = sqlUnsignedComma(&s);
ret->altColorB = sqlUnsignedComma(&s);
ret->useScore = sqlUnsignedComma(&s);
ret->isSplit = sqlUnsignedComma(&s);
ret->private = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void trackTableFree(struct trackTable **pEl)
/* Free a single dynamically allocated trackTable such as created
 * with trackTableLoad(). */
{
struct trackTable *el;

if ((el = *pEl) == NULL) return;
freeMem(el->mapName);
freeMem(el->tableName);
freeMem(el->shortLabel);
freeMem(el->longLabel);
freez(pEl);
}

void trackTableFreeList(struct trackTable **pList)
/* Free a list of dynamically allocated trackTable's */
{
struct trackTable *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    trackTableFree(&el);
    }
*pList = NULL;
}

void trackTableOutput(struct trackTable *el, FILE *f, char sep, char lastSep) 
/* Print out trackTable.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mapName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tableName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shortLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->longLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->visibility);
fputc(sep,f);
fprintf(f, "%u", el->colorR);
fputc(sep,f);
fprintf(f, "%u", el->colorG);
fputc(sep,f);
fprintf(f, "%u", el->colorB);
fputc(sep,f);
fprintf(f, "%u", el->altColorR);
fputc(sep,f);
fprintf(f, "%u", el->altColorG);
fputc(sep,f);
fprintf(f, "%u", el->altColorB);
fputc(sep,f);
fprintf(f, "%u", el->useScore);
fputc(sep,f);
fprintf(f, "%u", el->isSplit);
fputc(sep,f);
fprintf(f, "%u", el->private);
fputc(sep,f);
}

/* ---------------- End of AutoSQL generated code. ------------------ */

static struct trackTable builtIns[] = 
{
    {
     NULL,
     "hgEst",	/* mapName */
     "est",	/* tableName */
     "Human ESTs",	/* shortLabel */
     "Human ESTs",	/* longLabel */
     0,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgMrna",	/* mapName */
     "mrna",	/* tableName */
     "Full MGC mRNAs",	/* shortLabel */
     "Full Length MGC mRNAs",	/* longLabel */
     2,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "BACends",	/* mapName */
     "bacEnds",	/* tableName */
     "BAC ends",	/* shortLabel */
     "BAC end pairs",	/* longLabel */
     0,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgEst",	/* mapName */
     "est",	/* tableName */
     "Human ESTs",	/* shortLabel */
     "Human ESTs",	/* longLabel */
     0,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     1,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgIntronEst",	/* mapName */
     "intronEst",	/* tableName */
     "Spliced ESTs",	/* shortLabel */
     "Human ESTs That Have Been Spliced",	/* longLabel */
     1,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     1,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgMrna",	/* mapName */
     "mrna",	/* tableName */
     "Sequenced mRNAs",	/* shortLabel */
     "Sequenced mRNAs from Genbank",	/* longLabel */
     2,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     1,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgRepeat",	/* mapName */
     "rmsk",	/* tableName */
     "RepeatMasker",	/* shortLabel */
     "Repeating Elements by RepeatMasker",	/* longLabel */
     1,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     1,	/* useScore */
     1,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgBlatMouse",	/* mapName */
     "blatMouse",	/* tableName */
     "Mouse Blat",	/* shortLabel */
     "Mouse Translated Blat Alignments",	/* longLabel */
     0,	/* visibility */
     100,50,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     1,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgContig",	/* mapName */
     "ctgPos",	/* tableName */
     "Chromosome Band",	/* shortLabel */
     "Chromosome Bands Localized by FISH Mapping Clones",	/* longLabel */
     1,	/* visibility */
     150,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgCytoBands",	/* mapName */
     "cytoBand",	/* tableName */
     "Chromosome Band",	/* shortLabel */
     "Chromosome Bands Localized by FISH Mapping Clones",	/* longLabel */
     1,	/* visibility */
     0,0,0,	/* color */
     150,50,50,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgExoFish",	/* mapName */
     "exoFish",	/* tableName */
     "Exofish ecores",	/* shortLabel */
     "Exofish Tetraodon/Human Conserved Regions (ecores)",	/* longLabel */
     1,	/* visibility */
     153,21,153,	/* color */
     0,0,255,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgGcPercent",	/* mapName */
     "gcPercent",	/* tableName */
     "GC Percent",	/* shortLabel */
     "Percentage GC in 20,000 Base Windows",	/* longLabel */
     0,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgMusTest1",	/* mapName */
     "musTest1",	/* tableName */
     "Mouse Test 40",	/* shortLabel */
     "Mouse Translated Blat Alignments Score > 40",	/* longLabel */
     0,	/* visibility */
     0,0,0,	/* color */
     0,0,0,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     1,	/* private */
    },
    {
     NULL,
     "hgRefGene",	/* mapName */
     "refGene",	/* tableName */
     "RefSeq Genes",	/* shortLabel */
     "GenBank RefSeq Genes",	/* longLabel */
     2,	/* visibility */
     20,20,170,	/* color */
     137,137,212,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgRnaGene",	/* mapName */
     "rnaGene",	/* tableName */
     "RNA Genes",	/* shortLabel */
     "Non-coding RNA Genes (dark) and Pseudogenes (light)",	/* longLabel */
     2,	/* visibility */
     170,80,130,	/* color */
     230,180,130,	/* altColor */
     0,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
    {
     NULL,
     "hgStsMarker",	/* mapName */
     "stsMarker",	/* tableName */
     "STS Markers",	/* shortLabel */
     "STS Markers on Genetic (blue), FISH (green) and Radiation Hybrid (black) Maps",	/* longLabel */
     1,	/* visibility */
     0,0,0,	/* color */
     128,128,255,	/* altColor */
     1,	/* useScore */
     0,	/* isSplit */
     0,	/* private */
    },
};

struct trackTable *hGetTracks()
/* Get track table for current database. */
{
struct trackTable *ttList = NULL, *tt;
int i;
char table[256];

for (i=0; i<ArraySize(builtIns); ++i)
    {
    tt = builtIns+i;
    if (tt->isSplit)
       safef(table, sizeof(table), "chr22_%s", tt->tableName);
    else
       safef(table, sizeof(table), "%s", tt->tableName);
    if (hTableExists(table))
        {
	slAddHead(&ttList, tt);
	}
    }
slReverse(&ttList);
return ttList;
}

