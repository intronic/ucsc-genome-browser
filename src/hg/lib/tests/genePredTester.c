/* genePredTester - test program for genePred and genePredReader */
#include "common.h"
#include "genePred.h"
#include "genePredReader.h"
#include "options.h"
#include "psl.h"
#include "genbank.h"
#include "localmem.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "binRange.h"

static char const rcsid[] = "$Id: genePredTester.c,v 1.1 2004/02/14 10:36:58 markd Exp $";

void usage(char *msg)
/* Explain usage and exit. */
{
errAbort(
    "%s\n\n"
    "genePredTester - test program for genePred and genePredReader\n"
    "\n"
    "Usage:\n"
    "   genePredTester [options] task [args...]\n"
    "\n"
    "Tasks and arguments:\n"
    "\n"
    "o readTable db table\n"
    "  Read rows from db.table into genePred objects, determining the\n"
    "  columns dynamically. If -output is specified, the objects are\n"
    "  written to that file. Field name columns are ignored.\n"
    "\n"
    "o loadTable db table gpFile\n"
    "  Create and load the specified table from gbFile.  Field command options\n"
    "  defined optional columns in file.\n"
    "\n"
    "o readFile gpFile\n"
    "  Read rows from the table-separated file, Field command options\n"
    "  defined optional columns in file.  If -output is specified, the\n"
    "  objects are written to that file.\n"
    "\n"
    "o fromPsl pslFile cdsFile \n"
    "  Create genePred objects from PSL.If -output is specified, the\n"
    "  objects are written to that file.\n"
    "\n"
    "Options:\n"
    "  -verbose=0 - set verbose level\n"
    "  -idFld - include id field\n"
    "  -name2Fld - include name2 field\n"
    "  -cdsStatFld - include cdsStat fields\n"
    "  -exonFramesFld - include exonFrames field\n"
    "  -maxRows=n - maximum number of records to process\n"
    "  -minRows=1 - error if less than this number of rows read\n"
    "  -needRows=n - error if this number of rows read doesn't match\n"
    "  -where=where - optional where clause for query\n"
    "  -chrom=chrom - restrict file reading to this chromosome\n"
    "  -output=fname - write output to this file \n"
    "  -info=fname - write info about genePred to this file.\n"
    "  -withBin - create bin column when loading a table\n",
    msg);
}

static struct optionSpec options[] = {
    {"verbose", OPTION_INT},
    {"idFld", OPTION_BOOLEAN},
    {"name2Fld", OPTION_BOOLEAN},
    {"cdsStatFld", OPTION_BOOLEAN},
    {"exonFramesFld", OPTION_BOOLEAN},
    {"maxRows", OPTION_INT},
    {"minRows", OPTION_INT},
    {"needRows", OPTION_INT},
    {"db", OPTION_STRING},
    {"where", OPTION_STRING},
    {"output", OPTION_STRING},
    {"info", OPTION_STRING},
    {"withBin", OPTION_BOOLEAN},
    {NULL, 0},
};
int gVerbose = 0;
unsigned gOptFields = 0;
unsigned gCreateOpts = 0;
int gMaxRows = BIGNUM;
int gMinRows = 1;
int gNeedRows = -1;
char *gWhere = NULL;
char *gChrom = NULL;
char *gOutput = NULL;
char *gInfo = NULL;

void checkNumRows(char *src, int numRows)
/* check the number of row constraints */
{
if (numRows < gMinRows)
    errAbort("expected at least %d rows from %s, got %d", src, gMinRows, numRows);
if ((gNeedRows >= 0) && (numRows != gNeedRows))
    errAbort("expected %d rows from %s, got %d", src, gNeedRows, numRows);
if (gVerbose > 0)
    printf("read %d rows from %s\n", numRows, src);
}

void writeOptField(FILE* infoFh, struct genePred *gp, char* desc,
                   enum genePredFields field)
/* write info about one field */
{
fprintf(infoFh, "%s: %s\n", desc, ((gp->optFields & field) ? "yes" : "no"));
}

void writeInfo(struct genePred *gp)
/* write info about the genePred to the info file */
{
if (gInfo != NULL)
    {
    FILE *infoFh = mustOpen(gInfo, "w");
    writeOptField(infoFh, gp, "genePredIdFld", genePredIdFld);
    writeOptField(infoFh, gp, "genePredName2Fld", genePredName2Fld);
    writeOptField(infoFh, gp, "genePredCdsStatFld", genePredCdsStatFld);
    writeOptField(infoFh, gp, "genePredExonFramesFld", genePredExonFramesFld);
    carefulClose(&infoFh);
    }
}

void readTableTask(char *db, char *table)
/* Implements the readTable task */
{
FILE *outFh = NULL;
struct sqlConnection *conn = sqlConnectReadOnly(db);
struct genePredReader* gpr = genePredReaderQuery(conn, table, gWhere);
struct genePred* gp;
int numRows = 0;

if (gOutput != NULL)
    outFh = mustOpen(gOutput, "w");

while ((numRows < gMaxRows) && ((gp = genePredReaderNext(gpr)) != NULL))
    {
    if (outFh != NULL)
        genePredTabOut(gp, outFh);
    if (numRows == 0)
        writeInfo(gp);
    genePredFree(&gp);
    numRows++;
    }

carefulClose(&outFh);
genePredReaderFree(&gpr);

sqlDisconnect(&conn);
checkNumRows(table, numRows);
}

void loadTableTask(char *db, char *table, char* gpFile)
/* Implements the loadTable task */
{
struct genePredReader* gpr = genePredReaderFile(gpFile, gOptFields, gChrom);
char* sqlCmd = genePredGetCreateSql(table, gOptFields, gCreateOpts);
struct sqlConnection *conn;
struct genePred* gp;
int numRows = 0;
char tabFile[PATH_LEN];
FILE *tabFh;

safef(tabFile, sizeof(tabFile), "genePred.%d.tmp", getpid());
tabFh = mustOpen(tabFile, "w");


while ((numRows < gMaxRows) && ((gp = genePredReaderNext(gpr)) != NULL))
    {
    if (gCreateOpts & genePredWithBin)
        fprintf(tabFh, "%u\t", binFromRange(gp->txStart, gp->txEnd));
    genePredTabOut(gp, tabFh);
    if (numRows == 0)
        writeInfo(gp);
    genePredFree(&gp);
    numRows++;
    }

carefulClose(&tabFh);
conn = sqlConnect(db);
sqlRemakeTable(conn, table, sqlCmd);
sqlLoadTabFile(conn, tabFile, table, 0);
sqlDisconnect(&conn);

unlink(tabFile);
freeMem(sqlCmd);
genePredReaderFree(&gpr);
checkNumRows(gpFile, numRows);
}

void readFile(char *gpFile)
/* Implements the readFile task */
{
FILE *outFh = NULL;
struct genePredReader* gpr = genePredReaderFile(gpFile, gOptFields, gChrom);
struct genePred* gp;
int numRows = 0;

if (gOutput != NULL)
    outFh = mustOpen(gOutput, "w");

while ((numRows < gMaxRows) && ((gp = genePredReaderNext(gpr)) != NULL))
    {
    if (outFh != NULL)
        genePredTabOut(gp, outFh);
    if (numRows == 0)
        writeInfo(gp);
    genePredFree(&gp);
    numRows++;
    }

carefulClose(&outFh);
genePredReaderFree(&gpr);
checkNumRows(gpFile, numRows);
}

struct hash* loadCds(char* cdsFile)
/* load a CDS file into a hash */
{
struct lineFile* lf = lineFileOpen(cdsFile, TRUE);
struct hash* cdsTbl = hashNew(0);
char *row[2];

while (lineFileNextRowTab(lf, row, 2))
    {
    struct genbankCds* cds;
    lmAllocVar(cdsTbl->lm, cds);
    if (!genbankCdsParse(row[1], cds))
        errAbort("invalid CDS for %s: %s", row[0], row[1]);
    hashAdd(cdsTbl, row[0], cds);
    }

lineFileClose(&lf);
return cdsTbl;
}

void fromPsl(char *pslFile, char* cdsFile)
/* Implements the fromPsl task */
{
struct psl* pslList = pslLoadAll(pslFile);
struct hash *cdsTbl = loadCds(cdsFile);
struct psl *psl;
FILE *outFh = NULL;
int numRows = 0;

if (gOutput != NULL)
    outFh = mustOpen(gOutput, "w");

for (psl = pslList; (psl != NULL) && (numRows < gMaxRows);
     psl = psl->next, numRows++)
    {
    struct genbankCds* cds = hashFindVal(cdsTbl, psl->qName);
    struct genePred* gp = genePredFromPsl2(psl, gOptFields, cds, 5);
    if (outFh != NULL)
        genePredTabOut(gp, outFh);
    if (numRows == 0)
        writeInfo(gp);
    genePredFree(&gp);
    }
pslFreeList(&pslList);
carefulClose(&outFh);
hashFree(&cdsTbl);
checkNumRows(pslFile, numRows);
}

int main(int argc, char *argv[])
/* Process command line. */
{
char *task;
optionInit(&argc, argv, options);
if (argc < 2)
    usage("must supply a task");
task = argv[1];
gVerbose = optionInt("verbose", gVerbose);
if (optionExists("idFld"))
    gOptFields |= genePredIdFld;
if (optionExists("name2Fld"))
    gOptFields |= genePredName2Fld;    
if (optionExists("cdsStatFld"))
    gOptFields |= genePredCdsStatFld;    
if (optionExists("exonFramesFld"))
    gOptFields |= genePredExonFramesFld;
gMaxRows = optionInt("maxRows", gMaxRows);
gMinRows = optionInt("minRows", gMinRows);
gNeedRows = optionInt("minRows", gNeedRows);
gWhere = optionVal("where", gWhere);
gOutput = optionVal("output", gOutput);
gInfo = optionVal("info", gInfo);

if (optionExists("withBin"))
    gCreateOpts |= genePredWithBin;

if (sameString(task, "readTable"))
    {
    if (argc != 4)
        usage("readTable task requires two arguments");
    readTableTask(argv[2], argv[3]);
    }
else if (sameString(task, "loadTable"))
    {
    if (argc != 5)
        usage("loadTable task requires three arguments");
    loadTableTask(argv[2], argv[3], argv[4]);
    }
else if (sameString(task, "readFile"))
    {
    if (argc != 3)
        usage("readFile task requires one argument");
    readFile(argv[2]);
    }
else if (sameString(task, "fromPsl"))
    {
    if (argc != 4)
        usage("fromPsl task requires two argument");
    fromPsl(argv[2], argv[3]);
    }
else 
    {
    usage("invalid task");
    }


return 0;
}
