/* gtfToGenePred - convert a GTF file to a genePred. */
#include "common.h"
#include "linefile.h"
#include "gff.h"
#include "genePred.h"
#include "errCatch.h"
#include "options.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "gtfToGenePred - convert a GTF file to a genePred\n"
  "usage:\n"
  "   gtfToGenePred gff genePred\n"
  "\n"
  "options:\n"
  "     -genePredExt - create a extended genePred, including frame\n"
  "      information and gene name\n"
  "     -allErrors - skip groups with errors rather than aborting.\n"
  "      Useful for getting infomation about as many errors as possible.\n"
  "     -infoOut=file - write a file with informaton on each transcript\n"
  "     -sourcePrefix=pre - only process entries where the source name has the\n"
  "      specified prefix.  Maybe repeated.\n"
  );
}

static struct optionSpec options[] = {
    {"genePredExt", OPTION_BOOLEAN},
    {"allErrors", OPTION_BOOLEAN},
    {"infoOut", OPTION_STRING},
    {"sourcePrefix", OPTION_STRING|OPTION_MULTI},
    {NULL, 0},
};
boolean clGenePredExt = FALSE;  /* include frame and geneName */
boolean clAllErrors = FALSE;    /* report as many errors as possible */
struct slName *clSourcePrefixes; /* list of source prefixes to match */

int badGroupCount = 0;  /* count of inconsistent groups found */


/* header for info file */
static char *infoHeader = "#transId\tgeneId\tsource\tchrom\tstart\tend\tstrand\n";

static void writeInfo(FILE *infoFh, struct gffGroup *group)
/* write a row for a GTF group from the info file */
{
char *geneId = group->lineList->geneId;
if (geneId == NULL)
    geneId = "";
fprintf(infoFh, "%s\t%s\t%s\t%s\t%d\t%d\t%c\n",
        group->name, geneId, group->source,
        group->seq, group->start, group->end, group->strand);
}

static void gtfGroupToGenePred(struct gffFile *gtf, struct gffGroup *group, FILE *gpFh,
                               FILE *infoFh)
/* convert one gtf group to a genePred */
{
unsigned optFields = (clGenePredExt ? genePredAllFlds : 0);
struct genePred *gp;
struct errCatch *errCatch = errCatchNew();

if (errCatchStart(errCatch))
    {
    gp = genePredFromGroupedGtf(gtf, group, group->name, optFields);
    genePredTabOut(gp, gpFh);
    genePredFree(&gp);
    }
errCatchEnd(errCatch);
if (errCatch->gotError)
    {
    if (clAllErrors)
        warn("%s", errCatch->message->string);
    else
        errAbort("%s", errCatch->message->string);
    badGroupCount++;
    }
else
    {
    if (infoFh != NULL)
        writeInfo(infoFh, group);
    }
errCatchFree(&errCatch); 
}

static bool sourceMatches(struct gffGroup *group)
/* see if the source matches on on the list */
{
struct slName *pre = NULL;
for (pre = clSourcePrefixes; pre != NULL; pre = pre->next)
    if (startsWith(pre->name, group->source))
        return TRUE;
return FALSE;
}
        

static bool inclGroup(struct gffGroup *group)
/* check if a group should be included in the output */
{
if (clSourcePrefixes != NULL)
    {
    if (!sourceMatches(group))
        return FALSE;
    }
return TRUE;
}

static void gtfToGenePred(char *gtfFile, char *gpFile, char *infoFile)
/* gtfToGenePred - Look at a gff file and report some basic stats. */
{
struct gffFile *gtf = gffRead(gtfFile);
FILE *gpFh, *infoFh = NULL;
struct gffGroup *group;

if (!gtf->isGtf)
    errAbort("%s doesn't appear to be a GTF file (GFF not supported by this program)", gtfFile);
gffGroupLines(gtf);
gpFh = mustOpen(gpFile, "w");
if (infoFile != NULL)
    {
    infoFh = mustOpen(infoFile, "w");
    fputs(infoHeader, infoFh);
    }

for (group = gtf->groupList; group != NULL; group = group->next)
    if (inclGroup(group))
        gtfGroupToGenePred(gtf, group, gpFh, infoFh);

carefulClose(&gpFh);
gffFileFree(&gtf);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
clGenePredExt = optionExists("genePredExt");
clAllErrors = optionExists("allErrors");
clSourcePrefixes = optionMultiVal("sourcePrefix", NULL);

gtfToGenePred(argv[1], argv[2], optionVal("infoOut", NULL));
if (badGroupCount > 0)
    errAbort("%d errors", badGroupCount);
return 0;
}
