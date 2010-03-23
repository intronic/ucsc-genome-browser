/* gff3ToGenePred - convert a GFF3 file to a genePred file. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "gff3.h"
#include "genePred.h"

static char const rcsid[] = "$Id: gff3ToGenePred.c,v 1.5 2010/03/23 21:51:36 galt Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "gff3ToGenePred - convert a GFF3 file to a genePred file\n"
  "usage:\n"
  "   gff3ToGenePred inGff3 outGp\n"
  "options:\n"
  "  -honorStartStopCodons - only set CDS start/stop status to complete if there are\n"
  "   corresponding start_stop codon records\n"
  "This converts:\n"
  "   - top-level gene records with mRNA records\n"
  "   - top-level mRNA records\n"
  "   - mRNA records can contain exon and CDS, or only CDS, or only\n"
  "     exon for non--coding.\n"
  "The first step is to parse GFF3 file, up to 50 errors are reported before\n"
  "aborting.  If the GFF3 files is successfully parse, it is converted to gene,\n"
  "annotation.  Up to 50 conversion errors are reported before aborting.\n"
  );
}

static struct optionSpec options[] = {
    {"honorStartStopCodons", OPTION_BOOLEAN},
    {NULL, 0},
};
static boolean honorStartStopCodons = FALSE;
static int maxParseErrs = 50;  // maximum number of errors during parse
static int maxConvertErrs = 50;  // maximum number of errors during conversion
static int convertErrCnt = 0;  // number of convert errors


static void cnvError(char *format, ...)
/* print a convert error */
{
fputs("Error: ", stderr);
va_list args;
va_start(args, format);
vfprintf(stderr, format, args);
va_end(args);
fputc('\n', stderr);
convertErrCnt++;
}

static char *mkAnnAddrKey(struct gff3Ann *ann)
/* create a key for a gff3Ann from its address.  WARNING: static return */
{
static char buf[64];
safef(buf, sizeof(buf), "%lu", (unsigned long)ann);
return buf;
}

static boolean isProcessed(struct hash *processed, struct gff3Ann *ann)
/* has an ann record be processed? */
{
return hashLookup(processed, mkAnnAddrKey(ann)) != NULL;
}

static void recProcessed(struct hash *processed, struct gff3Ann *ann)
/* add an ann record to processed hash */
{
hashAdd(processed, mkAnnAddrKey(ann), ann);
}

static struct gff3File *loadGff3(char *inGff3File)
/* load GFF3 into memory */
{
struct gff3File *gff3File = gff3FileOpen(inGff3File, maxParseErrs, NULL);
if (gff3File->errCnt > 0)
    errAbort("%d errors parsing GFF3 file: %s", gff3File->errCnt, inGff3File); 
return gff3File;
}

static boolean haveChildFeature(struct gff3Ann *parent, char *featName)
/* does a child feature of the specified they exist as a child */
{
struct gff3AnnRef *child;
for (child = parent->children; child != NULL; child = child->next)
    {
    if (sameString(child->ann->type, featName))
        return TRUE;
    }
return FALSE;
}

static struct gff3AnnRef *getChildFeatures(struct gff3Ann *parent, char *featName)
/* build sorted list of the specified children */
{
struct gff3AnnRef *child, *feats = NULL;
for (child = parent->children; child != NULL; child = child->next)
    {
    if (sameString(child->ann->type, featName))
        slAddHead(&feats, gff3AnnRefNew(child->ann));
    }
slSort(&feats, gff3AnnRefLocCmp);
return feats;
}

static struct genePred *makeGenePred(struct gff3Ann *gene, struct gff3Ann *mrna, struct gff3AnnRef *exons, struct gff3AnnRef *cdsBlks)
/* construct the empty genePred, return NULL on a failure. */
{
if (exons == NULL)
    {
    cnvError("no exons defined for mRNA %s", mrna->id);
    return NULL;
    }

int txStart = exons->ann->start;
int txEnd = ((struct gff3AnnRef*)slLastEl(exons))->ann->end;
int cdsStart = (cdsBlks == NULL) ? txEnd : cdsBlks->ann->start;
int cdsEnd = (cdsBlks == NULL) ? txEnd : ((struct gff3AnnRef*)slLastEl(cdsBlks))->ann->end;

if ((mrna->strand == NULL) || (mrna->strand[0] == '?'))
    {
    cnvError("invalid strand for mRNA %s", mrna->id);
    return NULL;
    }

struct genePred *gp = genePredNew(mrna->id, mrna->seqid, mrna->strand[0],
                                  txStart, txEnd, cdsStart, cdsEnd,
                                  genePredAllFlds, slCount(exons));
gp->name2 = cloneString(gene->id);

// set start/end status based on codon features if requested
if (honorStartStopCodons)
    {
    if (gp->strand[0] == '+')
        {
        gp->cdsStartStat = haveChildFeature(mrna, gff3FeatStartCodon) ? cdsComplete : cdsIncomplete;
        gp->cdsEndStat =  haveChildFeature(mrna, gff3FeatStopCodon) ? cdsComplete : cdsIncomplete;
        }
    else
        {
        gp->cdsStartStat = haveChildFeature(mrna, gff3FeatStopCodon) ? cdsComplete : cdsIncomplete;
        gp->cdsEndStat =  haveChildFeature(mrna, gff3FeatStartCodon) ? cdsComplete : cdsIncomplete;
        }
    }
else
    {
    gp->cdsStartStat = cdsComplete;
    gp->cdsEndStat = cdsComplete;
    }
return gp;
}

static void addExons(struct genePred *gp, struct gff3AnnRef *exons)
/* add exons */
{
struct gff3AnnRef *exon;
for (exon = exons; exon != NULL; exon = exon->next)
    {
    int i = gp->exonCount++;
    gp->exonStarts[i] = exon->ann->start;
    gp->exonEnds[i] = exon->ann->end;
    gp->exonFrames[i] = -1;
    }
}

static int findCdsExon(struct genePred *gp, struct gff3Ann *cds, int iExon)
/* search for the exon containing the CDS, starting with iExon+1, return -1 on error */
{
for (iExon++; iExon < gp->exonCount; iExon++)
    {
    if ((gp->exonStarts[iExon] <= cds->start) && (cds->end <= gp->exonEnds[iExon]))
        return iExon;
    }
cnvError("no exon in %s contains CDS %d-%d", gp->name, cds->start, cds->end);
return -1;
}

static boolean addCdsFrame(struct genePred *gp, struct gff3AnnRef *cdsBlks)
/* assign frame based on CDS regions.  Return FALSE error */
{
struct gff3AnnRef *cds;
int iExon = -1; // caches current position
for (cds = cdsBlks; cds != NULL; cds = cds->next)
    {
    iExon = findCdsExon(gp, cds->ann, iExon);
    if (iExon < 0)
        return FALSE; // error
    gp->exonFrames[iExon] = gff3PhaseToFrame(cds->ann->phase);
    }
return TRUE;
}

static void processMRna(FILE *gpFh, struct gff3Ann *gene, struct gff3Ann *mrna, struct hash *processed)
/* process a mRNA node in the tree; gene can be NULL. Error count increment on error and genePred discarded */
{
recProcessed(processed, mrna);

// allow for only having CDS children
struct gff3AnnRef *exons = getChildFeatures(mrna, gff3FeatExon);
struct gff3AnnRef *cdsBlks = getChildFeatures(mrna, gff3FeatCDS);
struct gff3AnnRef *useExons = (exons != NULL) ? exons : cdsBlks;

struct genePred *gp = makeGenePred((gene != NULL) ? gene : mrna, mrna, useExons, cdsBlks);
if (gp == NULL)
    return; // error

addExons(gp, useExons);
if (!addCdsFrame(gp, cdsBlks))
    return; // error

// output before checking so it can be examined
genePredTabOut(gp, gpFh);
if (genePredCheck("GFF3 convert to genePred", stderr, -1, gp) != 0)
    {
    cnvError("discarding invalid genePred created for: %s", gp->name);
    genePredFree(&gp);
    return; // error
    }

genePredFree(&gp);
slFreeList(&exons);
slFreeList(&cdsBlks);
}

static void processGene(FILE *gpFh, struct gff3Ann *gene, struct hash *processed)
/* process a gene node in the tree.  Stop process if maximum errors reached */
{
recProcessed(processed, gene);

struct gff3AnnRef *child;
for (child = gene->children; child != NULL; child = child->next)
    {
    if (sameString(child->ann->type, gff3FeatMRna) && !isProcessed(processed, child->ann))
        {
        processMRna(gpFh, gene, child->ann, processed);
        if (convertErrCnt >= maxConvertErrs)
            break;
        }
    }
}

static void processRoot(FILE *gpFh, struct gff3Ann *node, struct hash *processed)
/* process a root node in the tree */
{
recProcessed(processed, node);

if (sameString(node->type, gff3FeatGene))
    processGene(gpFh, node, processed);
else if (sameString(node->type, gff3FeatMRna))
    processMRna(gpFh, NULL, node, processed);
}

static void gff3ToGenePred(char *inGff3File, char *outGpFile)
/* gff3ToGenePred - convert a GFF3 file to a genePred file. */
{
// hash of nodes ptrs, prevents dup processing due to dup parents
struct hash *processed = hashNew(12);
struct gff3File *gff3File = loadGff3(inGff3File);
FILE *gpFh = mustOpen(outGpFile, "w");
struct gff3AnnRef *root;
for (root = gff3File->roots; root != NULL; root = root->next)
    {
    if (!isProcessed(processed, root->ann))
        {
        processRoot(gpFh, root->ann, processed);
        if (convertErrCnt >= maxConvertErrs)
            break;
        }
    }
carefulClose(&gpFh);
if (convertErrCnt > 0)
    errAbort("%d errors converting GFF3 file: %s", convertErrCnt, inGff3File); 

#if 0  // free memory for leak debugging if 1
gff3FileFree(&gff3File);
hashFree(&processed);
#endif
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
honorStartStopCodons = optionExists("honorStartStopCodons");
gff3ToGenePred(argv[1], argv[2]);
return 0;
}
