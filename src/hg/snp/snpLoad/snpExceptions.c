/* snpExceptions - read chrN_snp125 tables, write to snp125Exceptions table */
#include "common.h"

#include "hdb.h"
#include "snp125.h"
#include "snp125Exceptions.h"

static char const rcsid[] = "$Id: snpExceptions.c,v 1.1 2006/01/12 20:54:17 heather Exp $";

char *database = NULL;
static struct slName *chromList = NULL;

void usage()
/* Explain usage and exit. */
{
errAbort(
    "snpExceptions - read chrN_snp125 tables, write to snp125Exceptions table\n"
    "usage:\n"
    "    snpExceptions database\n");
}

void createTable()
/* create the table one time, load each time */
{
struct sqlConnection *conn = hAllocConn();
snp125ExceptionsTableCreate(conn);
hFreeConn(&conn);
}

struct snp125 *readSnps(char *chromName)
/* slurp in chrN_snp125 */
{
struct snp125 *list=NULL, *el = NULL;
char tableName[64];
char query[512];
struct sqlConnection *conn = sqlConnect(database);
struct sqlResult *sr;
char **row;
int count = 0;

verbose(1, "reading snps...\n");
strcpy(tableName, "chr");
strcat(tableName, chromName);
strcat(tableName, "_snp125");

if (!sqlTableExists(conn, tableName))
    return list;

/* not checking chrom */
safef(query, sizeof(query), "select chrom, chromStart, chromEnd, name, strand, refNCBI, refUCSC, "
                            "observed, class from %s", tableName);

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    AllocVar(el);
    // el->chrom = chromName;
    el->chrom = cloneString(row[0]);
    // el->score = 0;
    el->chromStart = atoi(row[1]);
    el->chromEnd = atoi(row[2]);
    el->name = cloneString(row[3]);
    strcpy(el->strand, row[4]);
    el->refNCBI = cloneString(row[5]);
    el->refUCSC = cloneString(row[6]);
    el->observed = cloneString(row[7]);
    el->class = cloneString(row[8]);
    slAddHead(&list,el);
    count++;
    }
sqlFreeResult(&sr);
sqlDisconnect(&conn);
verbose(1, "%d snps found\n", count);
slReverse(&list);
return list;
}

void writeOneException(FILE *f, struct snp125 *el, char exception[])
{
verbose(1, "%s:%d-%d (%s) %s\n", el->chrom, el->chromStart, el->chromEnd, el->name, exception);
fprintf(f, "chr%s\t", el->chrom);
fprintf(f, "%d\t", el->chromStart);
fprintf(f, "%d\t", el->chromEnd);
fprintf(f, "%s\t", el->name);
fprintf(f, "%s\t", exception);
fprintf(f, "\n");
}

void writeSizeExceptions(FILE *f, struct snp125 *list)
{
struct snp125 *el = NULL;
int size = 0;

for (el = list; el != NULL; el = el->next)
    {

    if (sameString(el->class, "unknown"))
        continue;

    size = el->chromEnd - el->chromStart;

    if (sameString(el->class, "simple"))
        {
        if (size != 1) 
	    writeOneException(f, el, "SimpleClassWrongSize");
	continue;
        }

    if (sameString(el->class, "insertion"))
        {
        if (size != 0) 
            writeOneException(f, el, "InsertionClassWrongSize");
	continue;
        }

    if (sameString(el->class, "deletion"))
        {
        if (size == 0) 
            writeOneException(f, el, "DeletionClassWrongSize");
	continue;
        }

    if (sameString(el->class, "range"))
        {
        if (size <= 1) 
            writeOneException(f, el, "RangeClassWrongSize");
        }

    }

}

void writeObservedExceptions(FILE *f, struct snp125 *list)
{
}


void loadDatabase(FILE *f, char *fileName)
{
struct sqlConnection *conn = hAllocConn();
hgLoadNamedTabFile(conn, ".", "snp125Exceptions", fileName, &f);
hFreeConn(&conn);
}


int main(int argc, char *argv[])
/* Write chrN_snp125Exceptions.tab file for each chrom. */
{
struct snp125 *list=NULL, *el;
struct slName *chromPtr;
FILE *f;
char fileName[64];

if (argc != 2)
    usage();

database = argv[1];
hSetDb(database);

/* create output table */
createTable();

chromList = hAllChromNamesDb(database);

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
	printf("no SNPs for chr%s\n", chromPtr->name);
        verbose(1, "---------------------------------------\n");
	continue;
	}

    strcpy(fileName, "chr");
    strcat(fileName, chromPtr->name);
    strcat(fileName, "_snp125Exceptions");
    f = hgCreateTabFile(".", fileName);

    writeSizeExceptions(f, list);
    writeObservedExceptions(f, list);

    loadDatabase(f, fileName);
    slFreeList(&list);
    verbose(1, "---------------------------------------\n");
    }

return 0;
}
