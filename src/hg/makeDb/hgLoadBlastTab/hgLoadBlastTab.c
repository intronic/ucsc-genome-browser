/* hgLoadBlastTab - Load blast table into database. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "jksql.h"
#include "options.h"
#include "hgRelate.h"
#include "blastTab.h"

static char const rcsid[] = "$Id: hgLoadBlastTab.c,v 1.2 2003/06/11 03:22:22 kent Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgLoadBlastTab - Load blast table into database\n"
  "usage:\n"
  "   hgLoadBlastTab database table file(s).tab\n"
  "File.tab is something generated via ncbi blast\n"
  "using the -m 8 flag\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void blastTabTableCreate(struct sqlConnection *conn, char *tableName)
/* Create a scored-ref table with the given name. */
{
static char *createString = "CREATE TABLE %s (\n"
"    query varchar(255) not null,	# Name of query sequence\n"
"    target varchar(255) not null,	# Name of target sequence\n"
"    identity float not null,	# Percent identity\n"
"    aliLength int unsigned not null,	# Length of alignment\n"
"    mismatch int unsigned not null,	# Number of mismatches\n"
"    gapOpen int unsigned not null,	# Number of gap openings\n"
"    qStart int unsigned not null,	# Start in query (0 based)\n"
"    qEnd int unsigned not null,	# End in query (non-inclusive)\n"
"    tStart int unsigned not null,	# Start in target (0 based)\n"
"    tEnd int unsigned not null,	# Start in query (non-inclusive)\n"
"    eValue double not null,	# Expectation value\n"
"    bitScore double not null,	# Bit score\n"
"              #Indices\n"
"    INDEX(query(12))\n"
")\n";
struct dyString *dy = newDyString(1024);
dyStringPrintf(dy, createString, tableName);
sqlRemakeTable(conn, tableName, dy->string);
dyStringFree(&dy);
}

void hgLoadBlastTab(char *database, char *table, int inCount, char *inNames[])
/* hgLoadBlastTab - Load blast table into database. */
{
int i;
FILE *f = hgCreateTabFile(".", table);
int count = 0;
struct sqlConnection *conn = NULL;
printf("Scanning through %d files\n", inCount);
for (i=0; i<inCount; ++i)
    {
    struct lineFile *lf = lineFileOpen(inNames[i], TRUE);
    struct blastTab bt;
    char *row[BLASTTAB_NUM_COLS];
    while (lineFileRow(lf, row))
        {
	blastTabStaticLoad(row, &bt);
	bt.qStart -= 1;
	bt.tStart -= 1;
	blastTabTabOut(&bt, f);
	++count;
	}
    lineFileClose(&lf);
    }
printf("Loading database with %d rows\n", count);
conn = sqlConnect(database);
blastTabTableCreate(conn, table);
hgLoadTabFile(conn, ".", table, &f);
hgRemoveTabFile(".", table);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc < 4)
    usage();
hgLoadBlastTab(argv[1], argv[2], argc-3, argv+3);
return 0;
}
