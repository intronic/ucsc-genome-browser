/* ldHgGene - load a set of gene predictions from GFF or GTF file into
 * mySQL database. */

#include "common.h"
#include "options.h"
#include "hash.h"
#include "dystring.h"
#include "linefile.h"
#include "gff.h"
#include "jksql.h"
#include "genePred.h"
#include "hgRelate.h"

static char const rcsid[] = "$Id: ldHgGene.c,v 1.27 2004/03/03 17:59:24 sugnet Exp $";

char *exonType = "exon";	/* Type field that signifies exons. */
<<<<<<< ldHgGene.c
boolean noDbLoad = FALSE;      /* Do we skip loading into the database? */
=======
boolean requireCDS = FALSE;     /* should genes with CDS be dropped */
char *outFile = NULL;	        /* Output file as alternative to database. */
>>>>>>> 1.15

/* command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"exon", OPTION_STRING},
    {"oldTable", OPTION_BOOLEAN},
    {"noncoding", OPTION_BOOLEAN},
    {"nonCoding", OPTION_BOOLEAN},
    {"gtf", OPTION_BOOLEAN},
<<<<<<< ldHgGene.c
    {"gpFile", OPTION_STRING},
    {"noDbLoad", OPTION_BOOLEAN},
=======
    {"predTab", OPTION_BOOLEAN},
<<<<<<< ldHgGene.c
>>>>>>> 1.13
=======
    {"requireCDS", OPTION_BOOLEAN},
    {"frame", OPTION_BOOLEAN},
    {"geneName", OPTION_BOOLEAN},
    {"id", OPTION_BOOLEAN},
    {"out", OPTION_STRING},
>>>>>>> 1.15
    {NULL, 0}
};

void usage()
{
errAbort(
    "ldHgGene - load database with gene predictions from a gff file.\n"
    "usage:\n"
    "     ldHgGene database table file(s).gff\n"
    "options:\n"
<<<<<<< ldHgGene.c
    "     -exon=type     Sets type field for exons to specific value.\n"
    "     -oldTable      Don't overwrite what's already in table.\n"
    "     -noncoding     Forces whole prediction to be UTR.\n"
    "     -gtf           input is GTF, stop codon is not in CDS.\n"
    "     -gpFile=<file> Name of file to save genePred records in.\n"
    "     -noDbLoad      Don't actually load the database, just generate file.\n");
=======
    "     -exon=type   Sets type field for exons to specific value\n"
    "     -oldTable    Don't overwrite what's already in table\n"
    "     -noncoding   Forces whole prediction to be UTR\n"
    "     -gtf         input is GTF, stop codon is not in CDS\n"
<<<<<<< ldHgGene.c
<<<<<<< ldHgGene.c
    "     -predTab     input is already in genePredTab format (one file only)\n");
>>>>>>> 1.13
=======
    "     -predTab     input is already in genePredTab format (one file only)\n"
=======
    "     -predTab     input is already in genePredTab format\n"
>>>>>>> 1.22
    "     -requireCDS  discard genes that don't have CDS annotation\n"
    "     -out=gpfile  write output, in genePred format, instead of loading\n"
<<<<<<< ldHgGene.c
    "                  table. Database is ignored.\n");
>>>>>>> 1.15
=======
    "                  table. Database is ignored.\n"
<<<<<<< ldHgGene.c
    "     -frame       load frame information\n");
>>>>>>> 1.22
=======
    "     -frame       load frame information\n"
    "     -geneName    load gene name from gene_id in GTF\n"
    "     -id          generate unique id and store in optional field\n");
>>>>>>> 1.25
}

boolean gOptFields = 0;  /* optional fields from cmdline */

void loadIntoDatabase(char *database, char *table, char *tabName,
                      bool appendTbl)
/* Load tabbed file into database table. Drop and create table. */
{
struct sqlConnection *conn = sqlConnect(database);

if (!appendTbl)
    {
    char *createSql = genePredGetCreateSql(table,  gOptFields, 0);
    sqlRemakeTable(conn, table, createSql);
    freeMem(createSql);
    }
sqlLoadTabFile(conn, tabName, table, SQL_TAB_FILE_WARN_ON_WARN);

/* add a comment to the history table and finish up connection */
hgHistoryComment(conn, "Add gene predictions to %s table %s frame info.", table,
                 ((gOptFields & genePredExonFramesFld) ? "with" : "w/o"));
sqlDisconnect(&conn);
}

char *convertSoftberryName(char *name)
/* Convert softberry name to simple form that is same as in
 * softberryPep table. */
{
static char *head = "gene_id S.";
char *s = strrchr(name, '.');

if (strstr(name, head) == NULL)
    errAbort("Unrecognized Softberry name %s, no %s", name, head);
return s+1;
}

void ldHgGenePred(char *database, char *table, int gCount, char *gNames[])
/* Load up database from a bunch of genePred files. */
{
char *tabName = "genePred.tab";
FILE *f;
struct genePred *gpList = NULL, *gp;
int i;

for (i=0; i<gCount; ++i)
    {
    printf("Reading %s\n", gNames[i]);
    gpList = slCat(genePredExtLoadAll(gNames[i]), gpList);
    }
printf("%d gene predictions\n", slCount(gpList));
slSort(&gpList, genePredCmp);

/* Create tab-delimited file. */
if (outFile != NULL)
    f = mustOpen(outFile, "w");
else
    f = mustOpen(tabName, "w");
for (gp = gpList; gp != NULL; gp = gp->next)
    {
    genePredTabOut(gp, f);
    }
carefulClose(&f);

if (outFile == NULL)
    loadIntoDatabase(database, table, tabName, optionExists("oldTable"));
}

void ldHgGene(char *database, char *table, int gtfCount, char *gtfNames[])
/* Load up database from a bunch of GTF files. */
{
struct gffFile *gff = gffFileNew("");
struct gffGroup *group;
int i;
int lineCount;
struct genePred *gpList = NULL, *gp;
char *tabName = optionVal("gpFile", "genePred.tab");
FILE *f;
boolean nonCoding = optionExists("noncoding") || optionExists("nonCoding");
boolean isGtf = optionExists("gtf");

boolean isSoftberry = sameWord("softberryGene", table);
boolean isEnsembl = sameWord("ensGene", table);
boolean isSanger22 = sameWord("sanger22", table);

for (i=0; i<gtfCount; ++i)
    {
    printf("Reading %s\n", gtfNames[i]);
    gffFileAdd(gff, gtfNames[i], 0);
    }
lineCount = slCount(gff->lineList);
printf("Read %d transcripts in %d lines in %d files\n", 
	slCount(gff->groupList), lineCount, gtfCount);
gffGroupLines(gff);
printf("  %d groups %d seqs %d sources %d feature types\n",
    slCount(gff->groupList), slCount(gff->seqList), slCount(gff->sourceList),
    slCount(gff->featureList));

/* Convert from gffGroup to genePred representation. */
for (group = gff->groupList; group != NULL; group = group->next)
    {
    char *name = group->name;
    if (isSoftberry)
        {
	name = convertSoftberryName(name);
	}
    if (isGtf)
        gp = genePredFromGroupedGtf(gff, group, name, gOptFields);
    else
        gp = genePredFromGroupedGff(gff, group, name, exonType, gOptFields);
    if (gp != NULL)
	{
	if (nonCoding)
	    gp->cdsStart = gp->cdsEnd = 0;
        if (requireCDS && (gp->cdsStart == gp->cdsEnd))
            genePredFree(&gp);
        else
            slAddHead(&gpList, gp);
	}
    }
printf("%d gene predictions\n", slCount(gpList));
slSort(&gpList, genePredCmp);

/* Create tab-delimited file. */
if (outFile != NULL)
    f = mustOpen(outFile, "w");
else
    f = mustOpen(tabName, "w");
for (gp = gpList; gp != NULL; gp = gp->next)
    {
    genePredTabOut(gp, f);
    }
<<<<<<< ldHgGene.c
fclose(f);
if(!noDbLoad) 
    loadIntoDatabase(database, table, tabName);
=======
carefulClose(&f);

if (outFile == NULL)
<<<<<<< ldHgGene.c
    loadIntoDatabase(database, table, tabName);
>>>>>>> 1.15
=======
    loadIntoDatabase(database, table, tabName, optionExists("oldTable"));
>>>>>>> 1.22
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc < 3)
    usage();
if (optionExists("exon") && optionExists("gtf"))
    errAbort("can't specify -exon= with -gtf");
exonType = optionVal("exon", exonType);
<<<<<<< ldHgGene.c
<<<<<<< ldHgGene.c
noDbLoad = optionExists("noDbLoad");
ldHgGene(argv[1], argv[2], argc-3, argv+3);
=======
=======
outFile = optionVal("out", NULL);
requireCDS = optionExists("requireCDS");
<<<<<<< ldHgGene.c
<<<<<<< ldHgGene.c
>>>>>>> 1.15
=======
gFrame = optionExists("frame");
=======
 if (optionExists("frame"))
     gOptFields |= (genePredCdsStatFld|genePredExonFramesFld);
 if (optionExists("geneName"))
     gOptFields |= genePredName2Fld;
>>>>>>> 1.25

>>>>>>> 1.22
if (optionExists("predTab"))
    ldHgGenePred(argv[1], argv[2], argc-3, argv+3);
else
    ldHgGene(argv[1], argv[2], argc-3, argv+3);
>>>>>>> 1.13
return 0;
}
