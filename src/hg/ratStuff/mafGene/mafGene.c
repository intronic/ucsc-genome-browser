/* mafGene - output protein alignments using maf and frames. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "hdb.h"
#include "maf.h"
#include "obscure.h"
#include "genePred.h"
#include "mafGene.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "mafGene - output protein alignments using maf and genePred\n"
  "usage:\n"
  "   mafGene dbName mafTable genePredTable species.lst output\n"
  "arguments:\n"
  "   dbName         name of SQL database\n"
  "   mafTable       name of maf file table\n"
  "   genePredTable  name of the genePred table\n"
  "   species.lst    list of species names\n"
  "   output         put output here\n"
  "options:\n"
  "   -geneName=foobar   name of gene as it appears in genePred\n"
  "   -geneList=foolst   name of file with list of genes\n"
  "   -chrom=chr1        name of chromosome from which to grab genes\n"
  "   -exons             output exons\n"
  "   -noTrans           don't translate output into amino acids\n"
  "   -delay=N           delay N seconds between genes (default 0)\n" 
  "   -transUC           use kgXref table to translate uc* names\n"
  );
}

static struct optionSpec options[] = {
   {"geneName", OPTION_STRING},
   {"geneList", OPTION_STRING},
   {"chrom", OPTION_STRING},
   {"exons", OPTION_BOOLEAN},
   {"noTrans", OPTION_BOOLEAN},
   {"transUC", OPTION_BOOLEAN},
   {"delay", OPTION_INT},
   {NULL, 0},
};

char *geneName = NULL;
char *geneList = NULL;
char *onlyChrom = NULL;
boolean inExons = FALSE;
boolean noTrans = TRUE;
boolean transUC = FALSE;
int delay = 0;
boolean newTableType;

/* load one or more genePreds from the database */
struct genePred *getPreds(char *geneName, char *geneTable, char *db)
{
struct sqlConnection *conn = hAllocConn();
struct genePred *list = NULL;
char query[1024];
struct sqlResult *sr = NULL;
char **row;
char splitTable[HDB_MAX_TABLE_STRING];
boolean hasBin;

boolean found =  hFindSplitTableDb(db, NULL, geneTable,
	splitTable, &hasBin);

if (!found)
    errAbort("can't find table %s\n", geneTable);

if (onlyChrom != NULL)
    safef(query, sizeof query, 
	    "select * from %s where name='%s' and chrom='%s' \n",
	    splitTable,  geneName, onlyChrom);
else
    safef(query, sizeof query, 
	    "select * from %s where name='%s' \n",
	    splitTable,  geneName);

sr = sqlGetResult(conn, query);

while ((row = sqlNextRow(sr)) != NULL)
    {
    struct genePred *gene = genePredExtLoad(hasBin ? &row[1] : row,
	GENEPREDX_NUM_COLS );

    if (gene->exonFrames == NULL)
	genePredAddExonFrames(gene);

    verbose(2, "got gene %s\n",gene->name);
    slAddHead(&list, gene);
    }

if (list == NULL)
    errAbort("no genePred for gene %s in %s\n",geneName, geneTable);

slReverse(&list);

sqlFreeResult(&sr);
hFreeConn(&conn);

return list;
}

#ifdef NOTNOW
char *getGeneName(char *ucName)
{
struct sqlConnection *conn = hAllocConn();
char query[1024];
struct sqlResult *sr = NULL;
char **row;

safef(query, sizeof query, 
    "select geneSymbol from kgXref where kgID='%s'\n", ucName);
sr = sqlGetResult(conn, query);

if ((row = sqlNextRow(sr)) == NULL)
    {
    hFreeConn(&conn);
    return NULL;
    }

safef(geneNameBuffer, sizeof geneNameBuffer, "%s", row[0]);
sqlFreeResult(&sr);
hFreeConn(&conn);
spaceToUnderbar(geneNameBuffer);

return geneNameBuffer;
}

void maybePrintGeneName(char *name, FILE *f)
{
if (transUC)
    {
    char *geneName = getGeneName(name);
    fprintf(f, " %s", geneName);
    }
}
#endif

/* output the sequence for one gene for every species to 
 * the file stream 
 */
void outGene(FILE *f, char *geneName, char *dbName, char *mafTable, 
    char *geneTable, struct slName *speciesNameList)
{
struct genePred *pred = getPreds(geneName, geneTable, dbName);

for(; pred; pred = pred->next)
    mafGeneOutPred(f, pred, dbName, mafTable, speciesNameList,
	inExons, noTrans);
}

/* read a list of single words from a file */
static struct slName *readList(char *fileName)
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct slName *list = NULL;
char *row[1];

while (lineFileRow(lf, row))
    {
    int len = strlen(row[0]);
    struct slName *sn = needMem(sizeof(*sn)+len);
    strcpy(sn->name, row[0]);
    slAddHead(&list, sn);
    }

slReverse(&list);

lineFileClose(&lf);
return list;
}

/* query the list of gene names from the frames table */
static struct slName *queryNames(char *dbName, char *geneTable)
{
struct sqlConnection *conn = hAllocConn();
struct slName *list = NULL;
char query[1024];
struct sqlResult *sr = NULL;
char **row;

if (onlyChrom != NULL)
    safef(query, sizeof query, 
	"select distinct(name) from %s where chrom='%s' and cdsStart != cdsEnd\n",
	geneTable, onlyChrom);
else
    safef(query, sizeof query, 
	"select distinct(name) from %s and cdsStart != cdsEnd\n", geneTable);

sr = sqlGetResult(conn, query);

while ((row = sqlNextRow(sr)) != NULL)
    {
    int len = strlen(row[0]);
    struct slName *sn = needMem(sizeof(*sn)+len);
    strcpy(sn->name, row[0]);
    slAddHead(&list, sn);
    }
sr = sqlGetResult(conn, query);

sqlFreeResult(&sr);
hFreeConn(&conn);

return list;
}

void mafGene(char *dbName, char *mafTable, char *geneTable, 
   char *speciesList, char *outName)
/* mafGene - output protein alignments using maf and genePred. */
{
struct slName *geneNames = NULL;
struct slName *speciesNames = readList(speciesList);
FILE *f = mustOpen(outName, "w");

hSetDb(dbName);

if (geneList != NULL)
    geneNames = readList(geneList);
else if (geneName != NULL)
    {
    int len = strlen(geneName);
    geneNames = needMem(sizeof(*geneNames)+len);
    strcpy(geneNames->name, geneName);
    }
else
    geneNames = queryNames(dbName, geneTable);

for(; geneNames; geneNames = geneNames->next)
    {
    verbose(2, "outting  gene %s \n",geneNames->name);
    outGene(f, geneNames->name, dbName, mafTable, 
	geneTable,  speciesNames);
    if (delay)
	{
	verbose(2, "delaying %d seconds\n",delay);
	sleep(delay);
	}
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);

if (argc != 6)
    usage();

geneName = optionVal("geneName", geneName);
geneList = optionVal("geneList", geneList);
onlyChrom = optionVal("chrom", onlyChrom);
inExons = optionExists("exons");
noTrans = optionExists("noTrans");
transUC = optionExists("transUC");
delay = optionInt("delay", delay);

if ((geneName != NULL) && (geneList != NULL))
    errAbort("cannot specify both geneList and geneName");

mafGene(argv[1],argv[2],argv[3],argv[4],argv[5]);
return 0;
}
