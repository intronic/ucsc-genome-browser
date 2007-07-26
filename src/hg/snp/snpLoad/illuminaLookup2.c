/* illuminaLookup2 - validate illumina SNP arrays. */
/* report if SNP missing */
/* report unexpected position */
/* check strand */
/* check observed */
/* report if class != single */
/* report if locType != exact */

#include "common.h"

#include "hash.h"
#include "hdb.h"

static char const rcsid[] = "$Id: illuminaLookup2.c,v 1.1 2007/07/26 23:55:25 heather Exp $";

struct snpSubset 
    {
    char *chrom;
    int start;
    int end;
    char *strand;
    char *observed;
    char *class;
    char *locType;
    char *func;
    };

void usage()
/* Explain usage and exit. */
{
errAbort(
    "illuminaLookup2 - just use rsId/chrom/pos from illumina\n"
    "usage:\n"
    "    illuminaLookup db illuminaPrelimTable snpTable exceptionTable \n");
}


struct hash *storeExceptions(char *tableName)
/* store multiply-aligning SNPs */
{
struct hash *ret = NULL;
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;

verbose(1, "reading exceptions...\n");
ret = newHash(0);
safef(query, sizeof(query), "select name, exception from %s", tableName);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (!sameString(row[1], "MultipleAlignments")) continue;
    hashAdd(ret, cloneString(row[0]), NULL);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return ret;
}

struct hash *storeSnps(char *tableName, struct hash *exceptionHash)
/* store subset data for SNPs in a hash */
/* exclude SNPs that align multiple places */
{
struct hash *ret = NULL;
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
struct snpSubset *subsetElement = NULL;
struct hashEl *hel = NULL;

verbose(1, "creating SNP hash...\n");
ret = newHash(16);
safef(query, sizeof(query), 
      "select name, chrom, chromStart, chromEnd, strand, observed, class, locType, func from %s", 
      tableName);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    hel = hashLookup(exceptionHash, row[0]);
    if (hel != NULL)
	continue;
    AllocVar(subsetElement);
    subsetElement->chrom = cloneString(row[1]);
    subsetElement->start = sqlUnsigned(row[2]);
    subsetElement->end = sqlUnsigned(row[3]);
    subsetElement->strand = cloneString(row[4]);
    subsetElement->observed = cloneString(row[5]);
    subsetElement->class = cloneString(row[6]);
    subsetElement->locType = cloneString(row[7]);
    subsetElement->func = cloneString(row[8]);
    hashAdd(ret, cloneString(row[0]), subsetElement);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return ret;
}

boolean isTriallelic(char *obs)
{
if (sameString(obs, "A/C/G")) return TRUE;
if (sameString(obs, "A/C/T")) return TRUE;
if (sameString(obs, "A/G/T")) return TRUE;
if (sameString(obs, "C/G/T")) return TRUE;
return FALSE;
}

boolean isReverseComplement(char *obs1, char *obs2)
{
if (sameString(obs1, "A/C"))
    {
    if (sameString(obs2, "G/T")) return TRUE;
    return FALSE;
    }
if (sameString(obs1, "A/G"))
    {
    if (sameString(obs2, "C/T")) return TRUE;
    return FALSE;
    }
if (sameString(obs1, "C/T"))
    {
    if (sameString(obs2, "A/G")) return TRUE;
    return FALSE;
    }
if (sameString(obs1, "G/T"))
    {
    if (sameString(obs2, "A/C")) return TRUE;
    return FALSE;
    }
/* shouldn't get here */
return FALSE;
}

char *reverseObserved(char *obs)
{
if (sameString(obs, "A/C"))
    return "G/T";
if (sameString(obs, "A/G"))
    return "C/T";
if (sameString(obs, "C/T"))
    return "A/G";
if (sameString(obs, "G/T"))
    return "A/C";
return obs;
}




void processSnps(struct hash *snpHash, char *illuminaTable)
/* read illuminaTable */
/* lookup details in snpHash */
/* report if SNP missing */
/* report if class != single */
/* report if locType != exact */
{
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
struct hashEl *hel = NULL;
struct snpSubset *subsetElement = NULL;
FILE *output = mustOpen("illuminaLookup.out", "w");
FILE *errors = mustOpen("illuminaLookup.err", "w");
int pos = 0;
char *rsID = NULL;

verbose(1, "process SNPs...\n");
safef(query, sizeof(query), "select chrom, chromStart, chromEnd, name from %s", illuminaTable);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    rsID = cloneString(row[3]);
    hel = hashLookup(snpHash, rsID);
    if (hel == NULL) 
        {
	fprintf(errors, "%s not found\n", rsID);
	continue;
	}
    subsetElement = (struct snpSubset *)hel->val;

    if (!sameString(subsetElement->chrom, row[0]))
        {
	fprintf(errors, "unexpected chrom %s for snp %s\n", row[0], rsID);
	continue;
	}

    pos = sqlUnsigned(row[1]);
    if (pos != subsetElement->start)
        {
        fprintf(errors, "unexpected position %d for snp %s\n", pos, rsID);
	continue;
	}

    if (!sameString(subsetElement->class, "single"))
        {
	fprintf(errors, "unexpected class %s for snp %s\n", subsetElement->class, row[0]);
	continue;
	}

    if (!sameString(subsetElement->locType, "exact"))
        {
	fprintf(errors, "unexpected locType %s for snp %s\n", subsetElement->locType, row[0]);
	continue;
	}

    fprintf(output, "%s\t%s\t%s\t%s0\t%s\t%s\n", row[0], row[1], row[2], row[3], subsetElement->strand, subsetElement->observed);

    }

sqlFreeResult(&sr);
hFreeConn(&conn);
carefulClose(&errors);
carefulClose(&output);
}


int main(int argc, char *argv[])
/* load SNPs into hash */
{
char *snpDb = NULL;
char *illuminaTableName = NULL;
char *snpTableName = NULL;
char *exceptionTableName = NULL;

struct hash *snpHash = NULL;
struct hash *exceptionHash = NULL;

if (argc != 5)
    usage();

snpDb = argv[1];
illuminaTableName = argv[2];
snpTableName = argv[3];
exceptionTableName = argv[4];

/* process args */
hSetDb(snpDb);
if (!hTableExists(illuminaTableName))
    errAbort("no %s table in %s\n", illuminaTableName, snpDb);
if (!hTableExists(snpTableName))
    errAbort("no %s table in %s\n", snpTableName, snpDb);
if (!hTableExists(exceptionTableName))
    errAbort("no %s table in %s\n", exceptionTableName, snpDb);

exceptionHash = storeExceptions(exceptionTableName);
snpHash = storeSnps(snpTableName, exceptionHash);
processSnps(snpHash, illuminaTableName);

return 0;
}
