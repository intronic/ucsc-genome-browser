/* hgLoadPsl - Load up a mySQL database with psl alignment tables. */
#include "common.h"
#include "options.h"
#include "jksql.h"
#include "dystring.h"
#include "psl.h"
#include "xAli.h"
#include "hdb.h"

boolean tNameIx = FALSE;
boolean noBin = FALSE;
boolean xaFormat = FALSE;
char *clTableName = NULL;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgLoadPsl - Load up a mySQL database with psl alignment tables\n"
  "usage:\n"
  "   hgLoadPsl database file1.psl ... fileN.psl\n"
  "This must be run in the same directory as the .psl files\n"
  "It will create a table for each psl file.\n"
  "options:\n"
  "   -table=tableName  Explicitly set tableName.  (Defaults to file name)\n"
  "   -tNameIx  add target name index\n"
  "   -xa    Include sequence info\n"
  "   -nobin Repress binning");
}

char *createString = 
"CREATE TABLE %s (\n"
    "%s"				/* Optional bin */
    "matches int unsigned not null,	# Number of bases that match that aren't repeats\n"
    "misMatches int unsigned not null,	# Number of bases that don't match\n"
    "repMatches int unsigned not null,	# Number of bases that match but are part of repeats\n"
    "nCount int unsigned not null,	# Number of 'N' bases\n"
    "qNumInsert int unsigned not null,	# Number of inserts in query\n"
    "qBaseInsert int unsigned not null,	# Number of bases inserted in query\n"
    "tNumInsert int unsigned not null,	# Number of inserts in target\n"
    "tBaseInsert int unsigned not null,	# Number of bases inserted in target\n"
    "strand char(2) not null,	# + or - for strand.  First character is query, second is target.\n"
    "qName varchar(255) not null,	# Query sequence name\n"
    "qSize int unsigned not null,	# Query sequence size\n"
    "qStart int unsigned not null,	# Alignment start position in query\n"
    "qEnd int unsigned not null,	# Alignment end position in query\n"
    "tName varchar(255) not null,	# Target sequence name\n"
    "tSize int unsigned not null,	# Target sequence size\n"
    "tStart int unsigned not null,	# Alignment start position in target\n"
    "tEnd int unsigned not null,	# Alignment end position in target\n"
    "blockCount int unsigned not null,	# Number of blocks in alignment\n"
    "blockSizes longblob not null,	# Size of each block\n"
    "qStarts longblob not null,	# Start of each block in query.\n"
    "tStarts longblob not null,	# Start of each block in target.\n";

char *indexString = 
	  "#Indices\n"
    "%s"                            /* Optional bin. */
    "INDEX(%stStart),\n"
    "INDEX(qName(12)),\n"
    "INDEX(%stEnd)\n"
")\n";

void hgLoadPsl(char *database, int pslCount, char *pslNames[])
/* hgLoadPsl - Load up a mySQL database with psl alignment tables. */
{
int i;
char table[128];
char *pslName;
struct sqlConnection *conn = sqlConnect(database);
struct dyString *ds = newDyString(2048);
char *extraIx = (tNameIx ? "tName(8)," : "" );
char *binIxString = "";

if (!noBin)
    {
    if (tNameIx)
	binIxString = "INDEX(tName(8),bin),\n";
    else
        binIxString = "INDEX(bin),\n";
    }
for (i = 0; i<pslCount; ++i)
    {
    pslName = pslNames[i];
    printf("Processing %s\n", pslName);
    if (clTableName != NULL)
        strcpy(table, clTableName);
    else
	splitPath(pslName, NULL, table, NULL);
    if (sqlTableExists(conn, table))
	{
	dyStringClear(ds);
	dyStringPrintf(ds, "drop table %s", table);
	sqlUpdate(conn, ds->string);
	}
    dyStringClear(ds);
    dyStringPrintf(ds, createString, table, 
        (noBin ? "" : "bin smallint unsigned not null,\n"));
    if (xaFormat)
	{
        dyStringPrintf(ds, "qSeq longblob not null,\n");
        dyStringPrintf(ds, "tSeq longblob not null,\n");
	}
    dyStringPrintf(ds, indexString, binIxString, extraIx, extraIx);
    sqlUpdate(conn, ds->string);
    dyStringClear(ds);
    if (noBin)
	{
	dyStringPrintf(ds, 
	   "LOAD data local infile '%s' into table %s", pslName, table);
	sqlUpdate(conn, ds->string);
	}
    else
        {
	char *tab = "psl.tab";
	FILE *f = mustOpen(tab, "w");
	struct lineFile *lf = NULL;
	if (xaFormat)
	    {
	    struct xAli *xa;
	    char *row[23];
	    lf = lineFileOpen(pslName, TRUE);
	    while (lineFileRow(lf, row))
	        {
		xa = xAliLoad(row);
		fprintf(f, "%u\t", hFindBin(xa->tStart, xa->tEnd));
		xAliTabOut(xa, f);
		xAliFree(&xa);
		}
	    }
	else
	    {
	    struct psl *psl;
	    lf = pslFileOpen(pslName);
	    while ((psl = pslNext(lf)) != NULL)
		{
		fprintf(f, "%u\t", hFindBin(psl->tStart, psl->tEnd));
		pslTabOut(psl, f);
		pslFree(&psl);
		}
	    lineFileClose(&lf);
	    }
	carefulClose(&f);
	dyStringPrintf(ds, 
	   "LOAD data local infile '%s' into table %s", tab, table);
	sqlUpdate(conn, ds->string);
	}
    }
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
tNameIx = optionExists("tNameIx");
clTableName = optionVal("table", NULL);
xaFormat = optionExists("xa");
if (argc < 3)
    usage();
hgLoadPsl(argv[1], argc-2, argv+2);
return 0;
}
