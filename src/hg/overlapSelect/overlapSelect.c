/* overlapSelect - select records based on overlap of chromosome ranges */

#include "common.h"
#include "linefile.h"
#include "selectTable.h"
#include "psl.h"
#include "bed.h"
#include "genePred.h"
#include "options.h"

/* FIXME: would be nice to be able to specify ranges in the same manner
 * as featureBits */

static struct optionSpec optionSpecs[] = {
    {"selectFmt", OPTION_STRING},
    {"inFmt", OPTION_STRING},
    {"nonOverlaping", OPTION_BOOLEAN},
    {"strand", OPTION_BOOLEAN},
    {NULL, 0}
};

/* file format constants */
#define UNKNOWN_FMT   0
#define PSL_FMT       1
#define GENEPRED_FMT  2
#define BED_FMT       3

/* Options parsed from the command line */
unsigned selectFmt = UNKNOWN_FMT;
unsigned inFmt = UNKNOWN_FMT;
boolean nonOverlaping = FALSE;
boolean useStrand = FALSE;

unsigned parseFormatSpec(char *fmt)
/* parse a format specification */
{
if (sameString(fmt, "psl"))
    return PSL_FMT;
if (sameString(fmt, ".genePred"))
    return GENEPRED_FMT;
if (sameString(fmt, ".bed"))
    return BED_FMT;
errAbort("invalid file format: %s", fmt);
return UNKNOWN_FMT;
}

unsigned getFileFormat(char *path)
/* determine the file format from the specified file extension */
{
if (endsWith(path, ".psl"))
    return PSL_FMT;
if (endsWith(path, ".genePred") || endsWith(path, ".gp"))
    return GENEPRED_FMT;
if (endsWith(path, ".bed"))
    return BED_FMT;
errAbort("can't determine file format of %s", path);
return UNKNOWN_FMT;
}

static void outputRow(char** row, int numCols, FILE* outFh)
/* output a row */
{
int iCol;
for (iCol = 0; iCol < numCols; iCol++)
    {
    if (iCol > 0)
        fputc('\t', outFh);
    fputs(row[iCol], outFh);
    }
fputc('\n', outFh);
}

static boolean pslSelected(struct psl* psl)
/* Check if a PSL should be selected for output */
{
int iBlk;
char strand;
boolean overlaps = FALSE;
if (psl->strand[1] == '\0')
    strand = psl->strand[0];
else
    strand = (psl->strand[0] != psl->strand[1]) ? '-' : '+';

for (iBlk = 0; (iBlk < psl->blockCount) && !overlaps; iBlk++)
    {
    int start = psl->tStarts[iBlk];
    int end = start + psl->blockSizes[iBlk];
    if (psl->strand[1] == '-')
        reverseIntRange(&start, &end, psl->tSize);
    if (useStrand)
        overlaps = selectOverlapsStrand(psl->tName, start, end, strand, psl->qName);
    else
        overlaps = selectOverlapsGenomic(psl->tName, start, end, psl->qName);
    }
if (nonOverlaping)
    return !overlaps;
else
    return overlaps;
}

void pslSelect(char* inFile, char* outFile)
/* copy psl records that matches the selection criteria */
{
struct lineFile *lf = pslFileOpen(inFile);
FILE* outFh = mustOpen(outFile, "w");
char* row[PSL_NUM_COLS];

while (lineFileNextRowTab(lf, row, PSL_NUM_COLS))
    {
    struct psl* psl = pslLoad(row);
    if (pslSelected(psl))
        outputRow(row, PSL_NUM_COLS, outFh);
    pslFree(&psl);
    }
carefulClose(&outFh);
lineFileClose(&lf);
}

static boolean genePredSelected(struct genePred* gp)
/* Check if a genePred should be selected for output */
{
int iExon;
boolean overlaps = FALSE;
for (iExon = 0; (iExon < gp->exonCount) && !overlaps; iExon++)
    {
    if (useStrand)
        overlaps = selectOverlapsStrand(gp->chrom, gp->exonStarts[iExon],
                                        gp->exonEnds[iExon], gp->strand[0], gp->name);
    else
        overlaps = selectOverlapsGenomic(gp->chrom, gp->exonStarts[iExon],
                                         gp->exonEnds[iExon], gp->name);
    }
if (nonOverlaping)
    return !overlaps;
else
    return overlaps;
}

void genePredSelect(char* inFile, char* outFile)
/* copy genePred records that matches the selection criteria */
{
struct lineFile *lf = lineFileOpen(inFile, TRUE);
FILE* outFh = mustOpen(outFile, "w");
char* row[GENEPRED_NUM_COLS];
while (lineFileNextRowTab(lf, row, GENEPRED_NUM_COLS))
    {
    struct genePred *gp = genePredLoad(row);
    if (genePredSelected(gp))
        outputRow(row, GENEPRED_NUM_COLS, outFh);
    genePredFree(&gp);
    }
carefulClose(&outFh);
lineFileClose(&lf);
}

static boolean bedSelected(struct bed* bed)
/* Check if a bed should be selected for output */
{
boolean overlaps = FALSE;
if (bed->blockCount == 0)
    {
    if (useStrand)
        overlaps = selectOverlapsStrand(bed->chrom, bed->chromStart,
                                        bed->chromEnd, bed->strand[0],
                                        bed->name);
    else
        overlaps = selectOverlapsGenomic(bed->chrom, bed->chromStart,
                                         bed->chromEnd, bed->name);
    }
else
    {
    int iBlk;
    for (iBlk = 0; (iBlk < bed->blockCount) && !overlaps; iBlk++)
        {
        int start = bed->chromStart + bed->chromStarts[iBlk];
        if (useStrand)
            overlaps = selectOverlapsStrand(bed->chrom, start,
                                            start + bed->blockSizes[iBlk],
                                            bed->strand[0], bed->name);
        else
            overlaps = selectOverlapsGenomic(bed->chrom, start,
                                             start + bed->blockSizes[iBlk],
                                             bed->name);
        }
    }
if (nonOverlaping)
    return !overlaps;
else
    return overlaps;
}

void bedSelect(char* inFile, char* outFile)
/* copy bed records that matches the selection criteria */
{
struct lineFile *lf = lineFileOpen(inFile, TRUE);
FILE* outFh = mustOpen(outFile, "w");
char* row[12];
int numCols;
while ((numCols = lineFileChopNextTab(lf, row, ArraySize(row))) > 0)
    {
    struct bed *bed = bedLoadN(row, numCols);
    if (bedSelected(bed))
        outputRow(row, numCols, outFh);
    bedFree(&bed);
    }
carefulClose(&outFh);
lineFileClose(&lf);
}


void loadSelectTable(char *selectFile)
/* load the select table from a file */
{
switch (selectFmt)
    {
    case PSL_FMT:
        selectAddPsls(selectFile);
        break;
    case GENEPRED_FMT:
        selectAddGenePreds(selectFile);
        break;
    case BED_FMT:
        selectAddBeds(selectFile);
        break;
    }
}

void overlapSelect(char *selectFile, char *inFile, char *outFile)
/* select records based on overlap of chromosome ranges */
{
loadSelectTable(selectFile);

switch (inFmt)
    {
    case PSL_FMT:
        pslSelect(inFile, outFile);
        break;
    case GENEPRED_FMT:
        genePredSelect(inFile, outFile);
        break;
    case BED_FMT:
        bedSelect(inFile, outFile);
        break;
    }
}

void usage(char *msg)
/* usage message and abort */
{
errAbort("%s:\n"
         "overlapSelect [options] selectFile inFile outFile\n"
         "\n"
         "Select records bases on overlaping chromosome ranges.\n"
         "The ranges are specified in the selectFile, with each block\n"
         "specifying a range.  Records are copied from the inFile to outFile\n"
         "based on the selection criteria.  Selection is based on blocks or\n"
         "exons rather than entire range.\n"
         "\n"
         "Options:\n"
         "  -selectFmt=fmt - specify selectFile format:\n"
         "          psl - PSL format (default for *.psl files).\n"
         "          genePred - gepePred format (default for *.gp or\n"
         "                     *.genePred files).\n"
         "          bed - BED format (default for *.bed files).\n"
         "                If BED doesn't have blocks, the bed range is used.\n" 
         "  -inFmt=fmt - specify inFile format, same values as -selectFmt.\n"
         "  -nonOverlaping - select non-overlaping instead of overlaping records\n"
         "  -strand - must be on the same strand to be considered overlaping\n",
         msg);
}

/* entry */
int main(int argc, char** argv) {
char *selectFile, *inFile, *outFile;
optionInit(&argc, argv, optionSpecs);
if (argc != 4)
    usage("wrong # args");
selectFile = argv[1];
inFile = argv[2];
outFile = argv[3];

if (optionExists("selectFmt"))
    selectFmt = parseFormatSpec(optionVal("selectFmt", NULL));
else
    selectFmt = getFileFormat(selectFile);
if (optionExists("inFmt"))
    inFmt = parseFormatSpec(optionVal("inFmt", NULL));
else
    inFmt = getFileFormat(inFile);

nonOverlaping = optionExists("nonOverlaping");
useStrand = optionExists("strand");

overlapSelect(selectFile, inFile, outFile);
return 0;
}
