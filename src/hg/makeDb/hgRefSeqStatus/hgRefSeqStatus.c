/* hgRefSeqStatus - Create the refSeqStatus table from loc2ref file. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "jksql.h"
/* auto-generated by autoSql: */
#include "refSeqStatus.h"

/* Variables that can be set from command line. */
int taxId = 0;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgRefSeqStatus - Create the refSeqStatus table from a loc2ref file.\n"
  "usage:\n"
  "   hgRefSeqStatus database loc2ref\n"
  "where:\n"
  "   database: destination mysql database, often hgN for some N\n"
  "   loc2ref:  wget ftp://ftp.ncbi.nih.gov/refseq/LocusLink/loc2ref\n"
  "options:\n"
  "   -human:   keep only records with taxonomy id 9606\n"
  "   -mouse:   keep only records with taxonomy id 10090\n"
  "   -rat:     keep only records with taxonomy id 10116\n"
  "   -taxId N: keep only records with taxonomy id N\n"
  );
}

void readLoc2ref(char *fileName, struct hash **retStatusHash)
/* Read loc2ref file. Create hash of accession to status. */ 
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line;
char *words[6];
int wordCount;
struct hash *statHash = newHash(0);
char *acc, *dot, *stat;
int lineCount = 0, count = 0;
int thisTaxId = 0;

while (lineFileNext(lf, &line, NULL))
    {
    ++lineCount;
    wordCount = chopTabs(line, words);
    if (wordCount == 6)
        {
	/* [5] is the taxonomy id:
                      human:  9606
                      mouse: 10090 
                        rat: 10116
	   Skip this record if an organism/id has been specified and this id 
	   doesn't match.
	*/
	thisTaxId = atoi(words[5]);
	if ((taxId != 0) && (thisTaxId != taxId))
	    continue;
	acc = cloneString(words[1]);
	/* trim the version number, if present */
	dot = strchr(acc, '.');
	if (dot != NULL)
	  *dot = 0;
	stat = cloneString(words[3]);
	stat[0] = toupper(stat[0]);
	hashAdd(statHash, acc, stat);
	count++;
	}
    }
lineFileClose(&lf);
*retStatusHash = statHash;
printf("Added %d acc statuses from %s in %d lines\n",
       count, fileName, lineCount);
}

void hgRefSeqStatus(char *database, char *loc2ref)
/* hgRefSeqStatus - Create the refSeqStatus table from a loc2ref file. */
{
struct hash *statusHash = NULL;	/* Hash of accession -> status. */
struct hashEl *el = NULL;
struct refSeqStatus rss;
struct sqlConnection *conn;
char *tabName = "refSeqStatus.tab";
FILE *f = NULL;
char query[256];

readLoc2ref(loc2ref, &statusHash);

conn = sqlConnect(database);

printf("Writing %s\n", tabName);
f = mustOpen(tabName, "w");
for (el = hashElListHash(statusHash);  el != NULL;  el = el->next)
    {
    /* Fill out a refSeqStatus data structure. */
    zeroBytes(&rss, sizeof(rss));
    rss.mrnaAcc = el->name;
    rss.status  = (char *)el->val;

    refSeqStatusTabOut(&rss, f);
    }
carefulClose(&f);

/* First make table definition. */
sqlRemakeTable(conn, "refSeqStatus",
	       /* is there a cleaner way to include the .sql?? */
	       "CREATE TABLE refSeqStatus (\n    mrnaAcc varchar(255) not null,      # RefSeq gene accession name\n    status varchar(255) not null,       # Status (Reviewed, Provisional, Predicted)\n              #Indices\n    PRIMARY KEY(mrnaAcc)\n)");

printf("Loading database %s\n", database);
sqlUpdate(conn, "delete from refSeqStatus");
sprintf(query, "load data local infile '%s' into table refSeqStatus", tabName);
sqlUpdate(conn, query);

sqlDisconnect(&conn);
}


int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
taxId = cgiUsualInt("taxId", 0);
if (cgiBoolean("human"))
    taxId = 9606;
if (cgiBoolean("mouse"))
    taxId = 10090;
if (cgiBoolean("rat"))
    taxId = 10116;
if (argc != 3)
    usage();
hgRefSeqStatus(argv[1], argv[2]);
return 0;
}
