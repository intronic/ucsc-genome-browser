/* hgLoadNet - Load a chain/net file into database. */
#include "common.h"
#include "linefile.h"
#include "obscure.h"
#include "hash.h"
#include "cheapcgi.h"
#include "jksql.h"
#include "dystring.h"
#include "bed.h"
#include "hdb.h"
#include "chainNet.h"

/* Command line switches. */
boolean noBin = FALSE;		/* Suppress bin field. */
boolean oldTable = FALSE;	/* Don't redo table. */
char *sqlTable = NULL;		/* Read table from this .sql if non-NULL. */


void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgLoadNet - Load a generic net file into database\n"
  "usage:\n"
  "   hgLoadNet database track files(s).net\n"
  "options:\n"
  "   -noBin   suppress bin field\n"
  "   -oldTable add to existing table\n"
  "   -sqlTable=table.sql Create table from .sql file\n"
  );
}


void loadDatabase(char *database, char *tab, char *track)
/* Load database from tab file. */
{
struct sqlConnection *conn = sqlConnect(database);
struct dyString *dy = newDyString(1024);
/* First make table definition. */
if (sqlTable != NULL)
    {
    /* Read from file. */
    char *sql, *s;
    readInGulp(sqlTable, &sql, NULL);

    /* Chop of end-of-statement semicolon if need be. */
    s = strchr(sql, ';');
    if (s != NULL) *s = 0;
    
    sqlRemakeTable(conn, track, sql);
    freez(&sql);
    }
else if (!oldTable)
    {
    /* Create definition statement. */
    printf("Creating table definition for %s\n", track);
    dyStringPrintf(dy, "CREATE TABLE %s (\n", track);
    if (!noBin)
	dyStringAppend(dy, "  bin smallint unsigned not null,\n");
    dyStringAppend(dy, "  level int unsigned not null,\n");
    dyStringAppend(dy, "  tName varchar(255) not null,\n");
    dyStringAppend(dy, "  tStart int unsigned not null,\n");
    dyStringAppend(dy, "  tEnd int unsigned not null,\n");
    dyStringAppend(dy, "  strand char(1) not null,\n");
    dyStringAppend(dy, "  qName varchar(255) not null,\n");
    dyStringAppend(dy, "  qStart int unsigned not null,\n");
    dyStringAppend(dy, "  qEnd int unsigned not null,\n");
    dyStringAppend(dy, "  chainId int unsigned not null,\n");
    dyStringAppend(dy, "  ali int unsigned not null,\n");
    dyStringAppend(dy, "  score double not null,\n");
    dyStringAppend(dy, "  qOver int not null, \n");
    dyStringAppend(dy, "  qFar int not null, \n");
    dyStringAppend(dy, "  qDup int not null, \n");
    dyStringAppend(dy, "  type varchar(255) not null,\n");
    dyStringAppend(dy, "  tN int not null, \n");
    dyStringAppend(dy, "  qN int not null, \n");
    dyStringAppend(dy, "  tR int not null, \n");
    dyStringAppend(dy, "  qR int not null, \n");
    dyStringAppend(dy, "  tNewR int not null, \n");
    dyStringAppend(dy, "  qNewR int not null, \n");
    dyStringAppend(dy, "  tOldR int not null, \n");
    dyStringAppend(dy, "  qOldR int not null, \n");
    dyStringAppend(dy, "  tTrf int not null, \n");
    dyStringAppend(dy, "  qTrf int not null, \n");
    dyStringAppend(dy, "#Indices\n");
    if (!noBin)
	dyStringAppend(dy, "  INDEX(tName(8),bin),\n");
    dyStringAppend(dy, "  INDEX(tName(8),tStart)\n");
    dyStringAppend(dy, ")\n");
    sqlRemakeTable(conn, track, dy->string);
    }

dyStringClear(dy);
dyStringPrintf(dy, "load data local infile '%s' into table %s", tab, track);
printf("Loading %s into %s\n", database, track );
sqlUpdate(conn, dy->string);
sqlDisconnect(&conn);
}

void cnWriteTables(char *chrom, struct cnFill *fillList, FILE *f, int depth)
/* Recursively write out fill and gap lists. */
{
struct cnFill *fill;
for (fill = fillList; fill != NULL; fill = fill->next)
    {
    if (fill->chainId != 0)
	{
	if (fill->type == NULL)
	    errAbort("No type field, please run netSyntenic on input");
	if (fill->tN < 0)
	    errAbort("Missing fields.  Pleaser run netClassify on input");
	}
    if (fill->score < 0)
        fill->score = 0;
    if (!noBin)
	fprintf(f, "%d\t", hFindBin(fill->tStart, fill->tStart + fill->tSize));
    fprintf(f, "%d\t", depth);
    fprintf(f, "%s\t", chrom);
    fprintf(f, "%d\t", fill->tStart);
    fprintf(f, "%d\t", fill->tStart + fill->tSize);
    fprintf(f, "%c\t", fill->qStrand);
    fprintf(f, "%s\t", fill->qName);
    fprintf(f, "%d\t", fill->qStart);
    fprintf(f, "%d\t", fill->qStart + fill->qSize);
    fprintf(f, "%d\t", fill->chainId);
    fprintf(f, "%d\t", fill->ali);
    fprintf(f, "%1.1f\t", fill->score);
    fprintf(f, "%d\t", fill->qOver);
    fprintf(f, "%d\t", fill->qFar);
    fprintf(f, "%d\t", fill->qDup);
    fprintf(f, "%s\t", (fill->type == NULL ? "gap" : fill->type));
    fprintf(f, "%d\t", fill->tN);
    fprintf(f, "%d\t", fill->qN);
    fprintf(f, "%d\t", fill->tR);
    fprintf(f, "%d\t", fill->qR);
    fprintf(f, "%d\t", fill->tNewR);
    fprintf(f, "%d\t", fill->qNewR);
    fprintf(f, "%d\t", fill->tOldR);
    fprintf(f, "%d\t", fill->qOldR);
    fprintf(f, "%d\t", fill->tTrf);
    fprintf(f, "%d\n", fill->qTrf);
    if (fill->children)
        cnWriteTables(chrom, fill->children, f, depth+1);
    }
}

void hgLoadNet(char *database, char *track, int netCount, char *netFiles[])
/* hgLoadNet - Load a net file into database. */
{
int i;
struct hash *qHash, *tHash;
struct chrom *chromList = NULL, *chrom;
struct chainNet *cn;
struct lineFile *lf ;
struct chainNet *net;
char alignFileName[] ="align.tab";
FILE *alignFile = mustOpen(alignFileName,"w");

for (i=0; i<netCount; ++i)
    {
    lf = lineFileOpen(netFiles[i], TRUE);
    while ((net = chainNetRead(lf)) != NULL)
        {
        printf("read %s\n",net->name);
        cnWriteTables(net->name,net->fillList, alignFile, 1);
	chainNetFree(&net);
        }

    }
fclose(alignFile);
loadDatabase(database, alignFileName, track);
remove(alignFileName);
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
if (argc < 4)
    usage();
noBin = cgiBoolean("noBin");
oldTable = cgiBoolean("oldTable");
sqlTable = cgiOptionalString("sqlTable");
hgLoadNet(argv[1], argv[2], argc-3, argv+3);
return 0;
}
