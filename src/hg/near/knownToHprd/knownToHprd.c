/* knownToHprd - Create knownToHprd table by reading HPRD FLAT_FILES and using kgXref.
 *   This utilility should make automating incremental kg build easier. 
 *   It is assumed that the HPRD flat files have been decompressed in
 *   a directory like /cluster/data/hg18/p2p/hprd/FLAT_FILES/ 
 *   The data has this structure:
[hgwdev:FLAT_FILES> head HPRD_ID_MAPPINGS.txt
00001   ALDH1A1 NM_000689.3     NP_000680.2     216     100640  P00352  Aldehyde dehydrogenase 1
 */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "options.h"
#include "jksql.h"
#include "hgRelate.h"
#include "genePred.h"
#include "psl.h"
#include "rangeTree.h"
#include "binRange.h"
#include "hdb.h"
#include "visiGene.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "knownToHprd - Create knownToHprd table using HPRD flat file and kgXref\n"
  "usage:\n"
  "   knownToVisiGene database hprdFile\n"
  );
  /*
  "options:\n"
  "   -xxx=XXX\n"
  */
}

char *outTable = "knownToHprd";

static struct optionSpec options[] = {
   {NULL, 0},
};

struct prioritizedImage 
    {
    int imageId;	/* ID of image */
    float priority;	/* Image priority - smaller is more urgent */
    };

void createTable(struct sqlConnection *conn, char *tableName)
/* Create our name/value table, dropping if it already exists. */
{
struct dyString *dy = dyStringNew(512);
dyStringPrintf(dy,
"CREATE TABLE  %s (\n"
"    name varchar(255) not null,\n"
"    value varchar(255) not null,\n"
"#Indices\n"
"    PRIMARY KEY(name),\n"
"    INDEX(value)\n"
")\n",  tableName);
sqlRemakeTable(conn, tableName, dy->string);
dyStringFree(&dy);
}


void knownToHprd(char *database, char *hprdFile)
/* knownToHprd - Create knownToHprd table by using hprd flat file and kgXref. */
{
char *tempDir = ".";
FILE *f = hgCreateTabFile(tempDir, outTable);
struct lineFile *lf = lineFileOpen(hprdFile, TRUE);
char *lrow[8];

struct sqlConnection *hConn = sqlConnect(database);
struct sqlResult *sr;
char **row;
struct hash *mRnaHash = newHash(18);
struct hash *spIdHash = newHash(18);
struct hash *kgIdHash = newHash(18);
int missing=0;  // how many recs had no match via kgXref

sr = sqlGetResult(hConn, "select kgId, mRNA, spId from kgXref");
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *kgId = row[0];
    char *mRNA = row[1];
    char *spId = row[2];
    hashAdd(mRnaHash, mRNA, cloneString(kgId));
    if (spId)
	hashAdd(spIdHash, spId, cloneString(kgId));

    //uglyf("kgId=%s mRNA=%s spId=%s\n",kgId,mRNA,spId);

    }
sqlFreeResult(&sr);

while (lineFileRow(lf, lrow))
    {
    char *hprdId = lrow[0];
    char *cDna = lrow[2];
    char *uniProt = lrow[6];
    struct hashEl *kgId = NULL;
    chopSuffixAt(cDna,'.');

    //uglyf("hprdId=%s cDna=%s uniProt=%s\n",hprdId,cDna,uniProt);
    boolean found = FALSE;
    kgId = hashLookup(mRnaHash, cDna);	
    while(kgId)
	{
	found = TRUE;
	if (!hashFindVal(kgIdHash, kgId->val))  // each kgId only once 
	    {
	    hashAdd(kgIdHash, kgId->val, NULL);
	    fprintf(f,"%s\t%s\n",(char *)kgId->val,hprdId);
	    }
     	kgId = hashLookupNext(kgId);
	}

    kgId = hashLookup(spIdHash, uniProt);
    while(kgId)
	{
	found = TRUE;
	if (!hashFindVal(kgIdHash, kgId->val))  // each kgId only once 
	    {
	    hashAdd(kgIdHash, kgId->val, NULL);
	    fprintf(f,"%s\t%s\n",(char *)kgId->val,hprdId);
	    }
     	kgId = hashLookupNext(kgId);
	}

    if (!found)
	{
	++missing;
	verbose(2, "missing %s %s %s\n", hprdId, cDna, uniProt);
	}
    }
lineFileClose(&lf);
carefulClose(&f);

verbose(2, "missing=%d\n",missing);

uglyf("see %s/%s.tab\n",tempDir, outTable);

createTable(hConn, outTable);
hgLoadTabFile(hConn, tempDir, outTable, &f);
//hgRemoveTabFile(tempDir, outTable);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
knownToHprd(argv[1],argv[2]);
return 0;
}
