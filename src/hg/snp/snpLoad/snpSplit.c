/* snpSplit - split the ContigLoc table by chrom. */
#include "common.h"

#include "chromInfo.h"
#include "hash.h"
#include "hdb.h"
#include "snpTmp.h"

static char const rcsid[] = "$Id: snpSplit.c,v 1.1 2005/12/09 06:53:15 heather Exp $";

char *snpDb = NULL;
char *targetDb = NULL;
char *contigGroup = NULL;
static struct hash *chromHash = NULL;
static struct slName *chromList = NULL;

void usage()
/* Explain usage and exit. */
{
errAbort(
    "snpSplit - split the ContigLoc table by chrom\n"
    "usage:\n"
    "    snpSplit snpDb targetDb contigGroup\n");
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


boolean setCoords(struct snpTmp *el, int snpClass, char *startString, char *endString)
/* set coords and class */
/* can switch to using #define */
{
char *rangeString1, *rangeString2;

el->chromStart = atoi(startString);

if (snpClass == 1) 
    {
    el->class = cloneString("range");
    rangeString1 = cloneString(endString);
    rangeString2 = strstr(rangeString1, "..");
    if (rangeString2 == NULL) 
        {
        verbose(1, "error processing range snp %s with phys_pos %s\n", rangeString1);
        return FALSE;
        }
    rangeString2 = rangeString2 + 2;
    el->chromEnd = atoi(rangeString2);
    return TRUE;
    }

if (snpClass == 2)
    {
    el->class = cloneString("simple");
    el->chromEnd = atoi(endString);
    return TRUE;
    }

if (snpClass == 3)
    {
    el->class = cloneString("deletion");
    rangeString1 = cloneString(endString);
    rangeString2 = strstr(rangeString1, "^");
    if (rangeString2 == NULL) 
        {
        verbose(1, "error processing deletion snp %s with phys_pos %s\n", rangeString1);
        return FALSE;
	}
    rangeString2 = rangeString2++;
    el->chromEnd = atoi(rangeString2);
    return TRUE;
    }

/* set chromEnd = chromStart for insertions */
if (snpClass == 4)
    {
    el->class = cloneString("insertion");
    el->chromEnd = el->chromStart;
    return TRUE;
    }
verbose(5, "skipping snp %s with loc type %d\n", el->name, snpClass);
return FALSE;
}
struct snpTmp *readSnps(char *chromName, struct slName *contigList)
/* query ContigLoc for all snps in contig */
{
struct snpTmp *list=NULL, *el = NULL;
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
int snpClass;
int pos;
int count = 0;
struct slName *contigPtr;

verbose(1, "reading snps...\n");
for (contigPtr = contigList; contigPtr != NULL; contigPtr = contigPtr->next)
    {
    safef(query, sizeof(query), "select snp_id, ctg_id, loc_type, phys_pos_from, "
      "phys_pos, orientation, allele from ContigLoc where snp_type = 'rs' and ctg_id = '%s'", contigPtr->name);
    verbose(3, "ctg_id = %d\n", contigPtr->name);

    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
        {
        pos = atoi(row[3]);
        if (pos == 0) 
            {
	    verbose(5, "snp %s is unaligned\n", row[0]);
	    continue;
	    }
        snpClass = atoi(row[2]);
        if (snpClass < 1 || snpClass > 4) 
            {
	    verbose(1, "skipping snp %s with loc_type %d\n", el->name, snpClass);
	    continue;
	    }
        AllocVar(el);
        el->name = cloneString(row[0]);
        el->chrom = chromName;
        if(!setCoords(el, snpClass, row[3], row[4]))
            {
            free(el);
	    continue;
	    }
        if (atoi(row[5]) == 0) 
            strcpy(el->strand, "+");
        else if (atoi(row[5]) == 1)
            strcpy(el->strand, "-");
        else
            strcpy(el->strand, "?");
        el->refNCBI = cloneString(row[6]);
        slAddHead(&list,el);
	count++;
        }
    sqlFreeResult(&sr);
    }
hFreeConn(&conn);
verbose(1, "%d snps found\n", count);
return list;
}

struct slName *getContigs(char *chromName)
/* get list of contigs that are from chrom */
{
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
struct slName *contigList = NULL;
struct slName *el = NULL;
int count = 0;

verbose(1, "getting contigs...\n");
safef(query, sizeof(query), "select ctg_id from ContigInfo where contig_chr = '%s'", chromName);
verbose(5, "query = %s\n", query);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    verbose(5, "contig = %s\n", row[0]);
    el = newSlName(row[0]);
    slAddHead(&contigList, el);
    count++;
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
verbose(1, "contigs found = %d\n", count);
return(contigList);
}


void writeToTabFile(FILE *f, struct snpTmp *list)
/* write tab separated file */
{
struct snpTmp *el;
int count = 0;

verbose(1, "writing to tab file...\n");
for (el = list; el != NULL; el = el->next)
    {
    if (sameString(el->chrom, "ERROR")) continue;
    fprintf(f, "chr%s\t", el->chrom);
    fprintf(f, "%d\t", el->chromStart);
    fprintf(f, "%d\t", el->chromEnd);
    fprintf(f, "rs%s\t", el->name);
    fprintf(f, "%s\t", el->strand);
    fprintf(f, "%s\t", el->refNCBI);
    fprintf(f, "%s\t", el->class);
    fprintf(f, "\n");
    count++;
    }
verbose(1, "%d lines written\n", count);
}




void loadDatabase(char *chromName, struct snpTmp *list)
{
FILE *f;
struct sqlConnection *conn2 = hAllocConn2();
char fileName[64];

strcpy(fileName, "chr");
strcat(fileName, chromName);
strcat(fileName, "_snpTmp");
f = hgCreateTabFile(".", fileName);
writeToTabFile(f, list);
snpTmpTableCreate(conn2, fileName);
verbose(1, "loading...\n");
hgLoadTabFile(conn2, ".", fileName, &f);
hFreeConn2(&conn2);
}

int main(int argc, char *argv[])
/* Check args; read and load . */
{
struct snpTmp *list=NULL, *el;
struct slName *chromPtr, *contigList;

if (argc != 4)
    usage();

/* TODO: look in jksql for existence check for non-hgcentral database */
snpDb = argv[1];
targetDb = argv[2];
contigGroup = argv[3];
// if(!hDbExists(targetDb))
    // errAbort("%s does not exist\n", targetDb);

hSetDb(snpDb);
hSetDb2(targetDb);
chromHash = loadAllChromInfo();
chromList = hAllChromNamesDb(targetDb);

/* check for needed tables */
if(!hTableExistsDb(snpDb, "ContigLoc"))
    errAbort("no ContigLoc table in %s\n", snpDb);
if(!hTableExistsDb(snpDb, "ContigInfo"))
    errAbort("no ContigInfo table in %s\n", snpDb);

for (chromPtr = chromList; chromPtr != NULL; chromPtr = chromPtr->next)
    {
    stripString(chromPtr->name, "chr");
    }

for (chromPtr = chromList; chromPtr != NULL; chromPtr = chromPtr->next)
    {
    verbose(1, "chrom = %s\n", chromPtr->name);
    contigList = getContigs(chromPtr->name);
    if (contigList == NULL) continue;
    list = readSnps(chromPtr->name, contigList);
    slFreeList(&contigList);
    if (list == NULL)
        {
	verbose(1, "no SNPs found\n");
	continue;
	}
    slSort(&list, snpTmpCmp);
    loadDatabase(chromPtr->name, list);
    slFreeList(&list);
    }

return 0;
}
