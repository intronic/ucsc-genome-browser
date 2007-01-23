/* hapmapLookup - standalone utility program to check hapmap data against dbSNP. */

#include "common.h"

#include "hash.h"
#include "hdb.h"

static char const rcsid[] = "$Id: hapmapLookup.c,v 1.1 2007/01/23 01:12:05 heather Exp $";

FILE *errorFileHandle = NULL;

struct snpSubset 
    {
    char *chrom;
    int start;
    int end;
    char strand;
    char *observed;
    char *class;
    char *locType;
    };

void usage()
/* Explain usage and exit. */
{
errAbort(
    "hapmapLookup - get coords using rsId\n"
    "usage:\n"
    "    hapmapLookup db hapmapTable snpTable exceptionTable \n");
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
      "select name, chrom, chromStart, chromEnd, strand, observed, class, locType from %s", tableName);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    hel = hashLookup(exceptionHash, row[0]);
    if (hel != NULL) continue;
    AllocVar(subsetElement);
    subsetElement->chrom = cloneString(row[1]);
    subsetElement->start = sqlUnsigned(row[2]);
    subsetElement->end = sqlUnsigned(row[3]);
    subsetElement->strand = row[4][0];
    subsetElement->observed = cloneString(row[5]);
    subsetElement->class = cloneString(row[6]);
    subsetElement->locType = cloneString(row[7]);
    hashAdd(ret, cloneString(row[0]), subsetElement);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return ret;
}


boolean samePosition(char **row, struct snpSubset *ss)
/* simple comparison function */
{
if (sameString(row[3], "rs3963317"))
    verbose(1, "samePosition for rs3963317\n");
/* don't check if not a fair comparison */
if (differentString(ss->class, "single")) return FALSE;
if (differentString(ss->locType, "exact")) return FALSE;
if (ss->end != ss->start + 1) return FALSE;

/* now compare */
if (differentString(row[0], ss->chrom)) return FALSE;
if (sqlUnsigned(row[1]) != ss->start) return FALSE;
if (sqlUnsigned(row[2]) != ss->end) return FALSE;
if (sameString(row[3], "rs3963317"))
    verbose(1, "samePosition for rs3963317 TRUE\n");
return TRUE;
}

boolean reverseComplemented(char **row, struct snpSubset *ss)
{
char *rc;
int i, len;

len = strlen(ss->observed);
rc = needMem(len + 1);
rc = cloneString(ss->observed);
for (i = 0; i < len; i = i+2)
    {
    char c = ss->observed[i];
    rc[len-i-1] = ntCompTable[(int)c];
    }

if (sameString(rc, row[5])) return TRUE;

return FALSE;
}

void checkObservedAndStrand(char **row, struct snpSubset *ss)
{

/* if we don't know the observed string from dbSNP, all we can check is strand */
if (sameString(ss->observed, "unknown") || sameString(ss->observed, "n/a"))
    {
    if (row[4][0] != ss->strand)
        fprintf(errorFileHandle, "snp %s strand mismatch\n", row[3]);
    return;
    }

/* perhaps this is a reverse complement */
if (differentString(row[5], ss->observed) && row[4][0] != ss->strand)
    {
    if (reverseComplemented(row, ss))
        fprintf(errorFileHandle, "snp %s reverse complemented\n", row[3]);
    else
        fprintf(errorFileHandle, "snp %s: hapmap observed and strand issue\n", row[3]);
    return;
    }
        
if (sameString(row[5], ss->observed) && row[4][0] != ss->strand)
    {
    fprintf(errorFileHandle, "snp %s strand mismatch\n", row[3]);
    return;
    }

if (differentString(row[5], ss->observed) && row[4][0] == ss->strand)
    fprintf(errorFileHandle, "snp %s observed mismatch\n", row[3]);
        
}

void processSnps(struct hash *snpHash, struct hash *exceptionHash, char *hapmapTable)
/* read hapmapTable, lookup in snpHash */
/* report if SNP missing (could be due to multiple alignment, handle this if necessary) */
/* report if location is different */
/* report if observed is different */
/* report if strand is different */
{
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
struct hashEl *hel = NULL;
struct hashEl *hel2 = NULL;
struct snpSubset *subsetElement = NULL;

verbose(1, "process SNPs...\n");
safef(query, sizeof(query), "select chrom, chromStart, chromEnd, name, strand, observed from %s", hapmapTable);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    hel = hashLookup(snpHash, row[3]);
    if (hel == NULL) 
        {
	hel2 = hashLookup(exceptionHash, row[3]);
	if (hel2 == NULL)
	    fprintf(errorFileHandle, "%s not found\n", row[3]);
	else
	    fprintf(errorFileHandle, "%s aligns multiple places\n", row[3]);
	continue;
	}
    subsetElement = (struct snpSubset *)hel->val;

    if (!samePosition(row, subsetElement))
        {
	fprintf(errorFileHandle, "position mismatch for snp %s\n", row[3]);
	continue;
	}

    checkObservedAndStrand(row, subsetElement);
    }
}


int main(int argc, char *argv[])
/* load SNPs into hash */
{
char *snpDb = NULL;
char *hapmapTableName = NULL;
char *snpTableName = NULL;
char *exceptionTableName = NULL;

struct hash *snpHash = NULL;
struct hash *exceptionHash = NULL;

if (argc != 5)
    usage();

snpDb = argv[1];
hapmapTableName = argv[2];
snpTableName = argv[3];
exceptionTableName = argv[4];

/* initialize ntCompTable */
dnaUtilOpen();

/* process args */
hSetDb(snpDb);
if (!hTableExists(hapmapTableName))
    errAbort("no %s table in %s\n", hapmapTableName, snpDb);
if (!hTableExists(snpTableName))
    errAbort("no %s table in %s\n", snpTableName, snpDb);
if (!hTableExists(exceptionTableName))
    errAbort("no %s table in %s\n", exceptionTableName, snpDb);

errorFileHandle = mustOpen("hapmapLookup.error", "w");

exceptionHash = storeExceptions(exceptionTableName);
snpHash = storeSnps(snpTableName, exceptionHash);
verbose(1, "finished creating dbSNP hash\n");
processSnps(snpHash, exceptionHash, hapmapTableName);

carefulClose(&errorFileHandle);

return 0;
}
