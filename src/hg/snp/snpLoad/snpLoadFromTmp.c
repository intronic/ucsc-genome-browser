/* snpLoadFromTmp - create snp table from build125 database, assuming split tables. */
#include "common.h"

#include "chromInfo.h"
#include "hash.h"
#include "hdb.h"
#include "jksql.h"
#include "snp125.h"

static char const rcsid[] = "$Id: snpLoadFromTmp.c,v 1.1 2005/12/09 07:57:05 heather Exp $";

char *snpDb = NULL;
char *targetDb = NULL;
static struct hash *chromHash = NULL;
static struct slName *chromList = NULL;

void usage()
/* Explain usage and exit. */
{
errAbort(
    "snpLoadFromTmp - create snp table from build125 database, assuming split tables\n"
    "usage:\n"
    "    snpLoad snpDb targetDb\n");
}

/* Copied from hgLoadWiggle. */
static struct hash *loadAllChromInfo()
/* Load up all chromosome infos. */
{
struct chromInfo *el;
struct sqlConnection *conn = sqlConnect(targetDb);
struct sqlResult *sr = NULL;
struct hash *ret;
char **row;

ret = newHash(0);

sr = sqlGetResult(conn, "select * from chromInfo");
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = chromInfoLoad(row);
    verbose(4, "Add hash %s value %u (%#lx)\n", el->chrom, el->size, (unsigned long)&el->size);
    hashAdd(ret, el->chrom, (void *)(& el->size));
    }
sqlFreeResult(&sr);
sqlDisconnect(&conn);
return ret;
}

/* also copied from hgLoadWiggle. */
static unsigned getChromSize(char *chrom)
/* Return size of chrom.  */
{
struct hashEl *el = hashLookup(chromHash,chrom);

if (el == NULL)
    errAbort("Couldn't find size of chrom %s", chrom);
    return *(unsigned *)el->val;
}



struct snp125 *readSnps(char *chromName)
/* slurp in chrN_snpTmp */
{
struct snp125 *list=NULL, *el = NULL;
char tableName[64];
char query[512];
// struct sqlConnection *conn = hAllocConn2();
struct sqlConnection *conn = sqlConnect(targetDb);
struct sqlResult *sr;
char **row;
int count = 0;

verbose(1, "reading snps...\n");
strcpy(tableName, "chr");
strcat(tableName, chromName);
strcat(tableName, "_snpTmp");

if (!sqlTableExists(conn, tableName))
    return list;

safef(query, sizeof(query), "select chrom, chromStart, chromEnd, name, strand, refNCBI, class from %s", tableName);

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    AllocVar(el);
    el->name = cloneString(row[3]);
    el->chrom = chromName;
    el->score = 0;
    el->chromStart = atoi(row[1]);
    el->chromEnd = atoi(row[2]);
    
    if (atoi(row[4]) == 0) 
        strcpy(el->strand, "+");
    else if (atoi(row[4]) == 1)
        strcpy(el->strand, "-");
    else
        strcpy(el->strand, "?");
    el->refNCBI = cloneString(row[5]);
    el->class = cloneString(row[6]);
    slAddHead(&list,el);
    count++;
    }
sqlFreeResult(&sr);
// hFreeConn2(&conn);
sqlDisconnect(&conn);
verbose(1, "%d snps found\n", count);
return list;
}



void lookupFunction(struct snp125 *list)
/* get function from ContigLocusId table */
{
struct snp125 *el;
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
int functionValue = 0;

verbose(1, "looking up function...\n");
for (el = list; el != NULL; el = el->next)
    {
    safef(query, sizeof(query), "select fxn_class from ContigLocusId where snp_id = '%s'", el->name);
    sr = sqlGetResult(conn, query);
    /* need a joiner check rule for this */
    row = sqlNextRow(sr);
    if (row == NULL)
        {
        el->func = cloneString("unknown");
        sqlFreeResult(&sr);
	continue;
	}
    functionValue = atoi(row[0]);
    switch (functionValue)
        {
	    case 1:
                el->func = cloneString("unknown");
	        break;
	    case 2:
                el->func = cloneString("unknown");
	        break;
	    case 3:
                el->func = cloneString("coding-synon");
	        break;
	    case 4:
                el->func = cloneString("coding-nonsynon");
	        break;
	    case 5:
                el->func = cloneString("untranslated");
	        break;
	    case 6:
                el->func = cloneString("intron");
	        break;
	    case 7:
                el->func = cloneString("splice-site");
	        break;
	    case 8:
                el->func = cloneString("coding");
	        break;
	    default:
                el->func = cloneString("unknown");
	        break;
	}
    sqlFreeResult(&sr);
    }
hFreeConn(&conn);
}


void lookupHet(struct snp125 *list)
/* get heterozygosity from SNP table */
{
struct snp125 *el;
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;

verbose(1, "looking up heterozygosity...\n");
for (el = list; el != NULL; el = el->next)
    {
    safef(query, sizeof(query), "select avg_heterozygosity, het_se from SNP where snp_id = '%s'", el->name);
    sr = sqlGetResult(conn, query);
    /* need a joiner check rule for this */
    row = sqlNextRow(sr);
    if (row == NULL)
        {
        el->avHet = 0.0;
        el->avHetSE = 0.0;
        sqlFreeResult(&sr);
	continue;
	}
    el->avHet = atof(cloneString(row[0]));
    el->avHetSE = atof(cloneString(row[1]));
    sqlFreeResult(&sr);
    }
hFreeConn(&conn);
}

void lookupObserved(struct snp125 *list)
/* get observed and molType from snpFasta table */
{
struct snp125 *el;
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
char rsName[64];

verbose(1, "looking up observed...\n");
for (el = list; el != NULL; el = el->next)
    {
    strcpy(rsName, "rs");
    strcat(rsName, el->name);
    safef(query, sizeof(query), "select molType, observed from snpFasta where rsId = '%s'", rsName);
    // sr = sqlGetResult(conn, query);
    // row = sqlNextRow(sr);
    // if (row == NULL)
        // {
        el->observed = "n/a";
	el->molType = "unknown";
	// continue;
	// }
    // el->molType = cloneString(row[0]);
    // el->observed = cloneString(row[1]);
    // sqlFreeResult(&sr);
    }
hFreeConn(&conn);
}

void lookupRefAllele(struct snp125 *list)
/* get reference allele from nib files */
{
struct snp125 *el = NULL;
struct dnaSeq *seq;
char fileName[HDB_MAX_PATH_STRING];
char chromName[64];
int chromSize = 0;

AllocVar(seq);
verbose(1, "looking up reference allele...\n");
for (el = list; el != NULL; el = el->next)
    {
    el->refUCSC = cloneString("n/a");
    if (sameString(el->class, "simple") || sameString(el->class, "range"))
        {
	strcpy(chromName, "chr");
	strcat(chromName, el->chrom);
        chromSize = getChromSize(chromName);
	if (el->chromStart > chromSize || el->chromEnd > chromSize)
	    {
	    verbose(1, "unexpected coords %s %s:%d-%d\n",
	            el->name, chromName, el->chromStart, el->chromEnd);
            continue;
	    }
        hNibForChrom2(chromName, fileName);
        seq = hFetchSeq(fileName, chromName, el->chromStart, el->chromEnd);
	touppers(seq->dna);
	el->refUCSC = cloneString(seq->dna);
	}
    }
}

void appendToTabFile(FILE *f, struct snp125 *list)
/* write tab separated file */
{
struct snp125 *el;
int score = 0;
int bin = 0;
int count = 0;

for (el = list; el != NULL; el = el->next)
    {
    bin = hFindBin(el->chromStart, el->chromEnd);
    fprintf(f, "%d\t", bin);
    fprintf(f, "chr%s\t", el->chrom);
    fprintf(f, "%d\t", el->chromStart);
    fprintf(f, "%d\t", el->chromEnd);
    fprintf(f, "%s\t", el->name);
    fprintf(f, "%d\t", score);
    fprintf(f, "%s\t", el->strand);
    fprintf(f, "%s\t", el->refNCBI);
    fprintf(f, "%s\t", el->refUCSC);
    fprintf(f, "%s\t", el->observed);
    fprintf(f, "%s\t", el->molType);
    fprintf(f, "%s\t", el->class);
    fprintf(f, "unknown\t");
    fprintf(f, "%f\t", el->avHet);
    fprintf(f, "%f\t", el->avHetSE);
    fprintf(f, "%s\t", el->func);
    fprintf(f, "unknown\t");
    fprintf(f, "dbSNP125\t");
    fprintf(f, "0\t");
    fprintf(f, "\n");
    count++;
    }
if (count > 0)
    verbose(1, "%d lines written\n", count);
}




void loadDatabase(FILE *f)
{
struct sqlConnection *conn2 = hAllocConn2();
// hGetMinIndexLength requires chromInfo
// could add hGetMinIndexLength2 to hdb.c
// snp125TableCreate(conn2, hGetMinIndexLength2());
snp125TableCreate(conn2, 32);
verbose(1, "loading...\n");
hgLoadTabFile(conn2, ".", "snp125", &f);
hFreeConn2(&conn2);
}

int main(int argc, char *argv[])
/* Check args; read and load . */
{
struct snp125 *list=NULL, *el;
struct slName *chromPtr, *contigList;
FILE *f = hgCreateTabFile(".", "snp125");

if (argc != 3)
    usage();

/* TODO: look in jksql for existence check for non-hgcentral database */
snpDb = argv[1];
targetDb = argv[2];
// if(!hDbExists(targetDb))
    // errAbort("%s does not exist\n", targetDb);

hSetDb(snpDb);
hSetDb2(targetDb);
chromHash = loadAllChromInfo();
chromList = hAllChromNamesDb(targetDb);

/* check for needed tables */
if(!hTableExistsDb(snpDb, "ContigLocusId"))
    errAbort("no ContigLocusId table in %s\n", snpDb);
if(!hTableExistsDb(snpDb, "SNP"))
    errAbort("no SNP table in %s\n", snpDb);

for (chromPtr = chromList; chromPtr != NULL; chromPtr = chromPtr->next)
    {
    stripString(chromPtr->name, "chr");
    }

for (chromPtr = chromList; chromPtr != NULL; chromPtr = chromPtr->next)
    {
    verbose(1, "chrom = %s\n", chromPtr->name);
    list = readSnps(chromPtr->name);
    if (list == NULL) 
        {
        verbose(1, "--------------------------------------\n");
        continue;
	}
    lookupFunction(list);
    lookupHet(list);
    lookupObserved(list);
    lookupRefAllele(list);
    appendToTabFile(f, list);
    slFreeList(&list);
    verbose(1, "---------------------------------------\n");
    }

loadDatabase(f);
return 0;
}
