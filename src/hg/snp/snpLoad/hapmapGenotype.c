/* hapmapGenotype - for each chrom, read a file with data from 5 populations and merge. */
/* Note: As of hg17, we don't have hapmap data for all chroms, just chr1-22 and chrX. */

/* Include an overall hash to check for multiple alignments?  (on the same or different chrom) */
/* Not checking if the same SNP is reported more than once for the same population. */
/* In this case, just use the final values */


#include "common.h"

#include "hash.h"
#include "hdb.h"
#include "linefile.h"
#include "sqlNum.h"

static char const rcsid[] = "$Id: hapmapGenotype.c,v 1.4 2007/01/19 23:34:52 heather Exp $";

FILE *errorFileHandle = NULL;
// database used to get chrom list
static char *db = NULL;
static struct hash *snpHash = NULL;
static struct hash *errorHash = NULL;

struct snpInfo
    {
    char *chrom;
    int  position;
    char *strand;
    char *observed;

    int  aCountCEU;       
    int  cCountCEU;       
    int  gCountCEU;       
    int  tCountCEU;       

    int  aCountCHB;       
    int  cCountCHB;       
    int  gCountCHB;
    int  tCountCHB;

    int  aCountJPT;       
    int  cCountJPT;       
    int  gCountJPT;
    int  tCountJPT;

    int  aCountYRI;       
    int  cCountYRI;       
    int  gCountYRI;       
    int  tCountYRI;       
    };

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hapmapGenotype - Read chrom files and merge over populations.\n"
  "usage:\n"
  "  hapmapGenotype database \n");
}

boolean validAllele(char *allele)
{
if (sameString(allele, "A")) return TRUE;
if (sameString(allele, "C")) return TRUE;
if (sameString(allele, "G")) return TRUE;
if (sameString(allele, "T")) return TRUE;
return FALSE;
}

boolean validPop(char *pop)
{
if (sameString(pop, "CEU")) return TRUE;
if (sameString(pop, "CHB")) return TRUE;
if (sameString(pop, "JPT")) return TRUE;
if (sameString(pop, "YRI")) return TRUE;
return FALSE;
}

void initCounts(struct snpInfo *si)
/* get this for free from AllocVar? */
{
si->aCountCEU = 0;       
si->cCountCEU = 0;       
si->gCountCEU = 0;       
si->tCountCEU = 0;       
si->aCountCHB = 0;       
si->cCountCHB = 0;       
si->gCountCHB = 0;
si->tCountCHB = 0;
si->aCountJPT = 0;       
si->cCountJPT = 0;       
si->gCountJPT = 0;
si->tCountJPT = 0;
si->aCountYRI = 0;       
si->cCountYRI = 0;       
si->gCountYRI = 0;       
si->tCountYRI = 0;       
}

void setCount(struct snpInfo *si, char *pop, char *allele, int count)
/* set the count for a specific population and allele. */
{
if (sameString(pop, "CEU"))
    {
    if (sameString(allele, "A")) si->aCountCEU = count;
    else if (sameString(allele, "C")) si->cCountCEU = count;
    else if (sameString(allele, "G")) si->gCountCEU = count;
    else if (sameString(allele, "T")) si->tCountCEU = count;
    return;
    }

if (sameString(pop, "CHB"))
    {
    if (sameString(allele, "A")) si->aCountCHB = count;
    else if (sameString(allele, "C")) si->cCountCHB = count;
    else if (sameString(allele, "G")) si->gCountCHB = count;
    else if (sameString(allele, "T")) si->tCountCHB = count;
    return;
    }

if (sameString(pop, "JPT"))
    {
    if (sameString(allele, "A")) si->aCountJPT = count;
    else if (sameString(allele, "C")) si->cCountJPT = count;
    else if (sameString(allele, "G")) si->gCountJPT = count;
    else if (sameString(allele, "T")) si->tCountJPT = count;
    return;
    }

if (sameString(pop, "YRI"))
    {
    if (sameString(allele, "A")) si->aCountYRI = count;
    else if (sameString(allele, "C")) si->cCountYRI = count;
    else if (sameString(allele, "G")) si->gCountYRI = count;
    else if (sameString(allele, "T")) si->tCountYRI = count;
    return;
    }

}

boolean validInput(char **row, int count, char *chrom)
/* This checks any input row. */
/* Errors are written to errorFileHandle. */
{
int count1, count2, total;

/* allele should be 'A', 'C', 'G', or 'T'  */
if (!validAllele(row[5]))
    {
    fprintf(errorFileHandle, "invalid allele %s for %s in chrom %s\n", row[5], row[1], chrom);
    return FALSE;
    }

if (count == 10)
    if (!validAllele(row[7]))
        {
        fprintf(errorFileHandle, "invalid allele %s for %s in chrom %s\n", row[7], row[1], chrom);
        return FALSE;
        }

/* check counts */
count1 = sqlSigned(row[6]);
if (count == 10)
    {
    count2 = sqlSigned(row[8]);
    total = sqlSigned(row[9]);
    }
else
    {
    count2 = 0;
    total = sqlSigned(row[7]);
    }
    
if (total != count1 + count2)
    {
    fprintf(errorFileHandle, "wrong counts for %s in pop %s, chrom %s\n", row[1], row[0], chrom);
    fprintf(errorFileHandle, "count1 = %d, count2 = %d, total = %d\n", count1, count2, total);
    return FALSE;
    }

/* check population (should be fine, I control this upstream) */
if (!validPop(row[0]))
    {
    fprintf(errorFileHandle, "unknown pop %s for %s in chrom %s\n", row[0], row[1], chrom);
    return FALSE;
    }
return TRUE;
}

boolean validExtension(char **row, struct snpInfo *si, char *chrom)
/* This checks a row that is being added to an existing snpInfo */
{
if (differentString(si->observed, row[2]))
    {
    fprintf(errorFileHandle, "different observed strings for %s in chrom %s\n", row[1], chrom);
    return FALSE;
    }
if (si->position != sqlSigned(row[3]))
    {
    fprintf(errorFileHandle, "multiple positions for %s in chrom %s\n", row[1], chrom);
    return FALSE;
    }
if (differentString(si->strand, row[4]))
    {
    fprintf(errorFileHandle, "different strands for %s in chrom %s\n", row[1], chrom);
    return FALSE;
    }
return TRUE;
}

void readFile(char *chrom)
/* Read input.  Use hashes to manage rsIds. */
/* Move snpInfo to errorHash if necessary. */
{
char inputFileName[64];
struct lineFile *lf = NULL;
char *line;
char *row[10];
struct hashEl *hel = NULL;
char *rsId;
struct snpInfo *si = NULL;
int elementCount;
boolean isValid;

safef(inputFileName, sizeof(inputFileName), "%s.hapmap", chrom);
lf = lineFileOpen(inputFileName, TRUE);
while (lineFileNext(lf, &line, NULL)) 
    {
    elementCount = chopString(line, " ", row, ArraySize(row));
    if (elementCount > 10)
        {
	fprintf(errorFileHandle, "unexpected format: %s\n", line);
	continue;
	}

    isValid = validInput(row, elementCount, chrom);
    if (!isValid) continue;

    rsId = cloneString(row[1]);

    /* If we've already encountered a problem with this rsId in another population, just skip */
    hel = hashLookup(errorHash, rsId);
    if (hel)
        continue;

    hel = hashLookup(snpHash, rsId);
    if (hel == NULL)
        {
	AllocVar(si);
	initCounts(si);
        si->chrom = cloneString(chrom);
	si->observed = cloneString(row[2]);
        si->position = sqlSigned(row[3]);
        si->strand = cloneString(row[4]);
	setCount(si, row[0], row[5], sqlSigned(row[6])); 
	if (elementCount == 10)
	    setCount(si, row[0], row[7], sqlSigned(row[8])); 
	hashAdd(snpHash, rsId, si);
	}
    else
        {
	si = (struct snpInfo *)hel->val;
	isValid = validExtension(row, si, chrom);
	if (!isValid)
	    {
	    hashRemove(snpHash, rsId);
	    hashAdd(errorHash, rsId, NULL);
            continue;
	    }
	setCount(si, row[0], row[5], sqlSigned(row[6])); 
	if (elementCount == 10)
	    setCount(si, row[0], row[7], sqlSigned(row[8])); 
	}
    }
lineFileClose(&lf);
}

void printOutput(char *chrom)
/* generate bed+ output into files chrN.hapmap.new */
{
char outputFileName[64];
FILE *outputFileHandle = NULL;
struct hashCookie cookie;
struct hashEl *hel= NULL;
struct snpInfo *si;
char *rsId;

safef(outputFileName, sizeof(outputFileName), "%s.hapmap.new", chrom);
outputFileHandle = mustOpen(outputFileName, "w");

cookie = hashFirst(snpHash);

while ((hel = hashNext(&cookie)) != NULL)
    {
    si = (struct snpInfo *)hel->val;
    rsId = (char *)hel->name;
    fprintf(outputFileHandle, "%s ", si->chrom);
    fprintf(outputFileHandle, "%d ", si->position - 1);
    fprintf(outputFileHandle, "%d ", si->position);
    fprintf(outputFileHandle, "%s ", rsId);
    fprintf(outputFileHandle, "0 ");
    fprintf(outputFileHandle, "%s ", si->strand);
    fprintf(outputFileHandle, "%s ", si->observed);

    fprintf(outputFileHandle, "%d %d %d %d ", 
                              si->aCountCEU, si->aCountCHB, si->aCountJPT, si->aCountYRI);
    fprintf(outputFileHandle, "%d %d %d %d ", 
                              si->cCountCEU, si->cCountCHB, si->cCountJPT, si->cCountYRI);
    fprintf(outputFileHandle, "%d %d %d %d ", 
                              si->gCountCEU, si->gCountCHB, si->gCountJPT, si->gCountYRI);
    fprintf(outputFileHandle, "%d %d %d %d\n", 
                              si->tCountCEU, si->tCountCHB, si->tCountJPT, si->tCountYRI);

    }
carefulClose(&outputFileHandle);

}

int main(int argc, char *argv[])
/* read hapmap allele frequency files which have one line for each of 4 populations */
/* merge into a single line for each rsId */
{
struct slName *chromList, *chromPtr;
char inputFileName[64];
if (argc != 2)
    usage();

db = argv[1];
hSetDb(db);
chromList = hAllChromNamesDb(db);

errorFileHandle = mustOpen("hapmap.errors", "w");

for (chromPtr = chromList; chromPtr != NULL; chromPtr = chromPtr->next)
    {
    safef(inputFileName, sizeof(inputFileName), "%s.hapmap", chromPtr->name);
    if (!fileExists(inputFileName)) continue;
    printf("%s\n", chromPtr->name);
    snpHash = newHash(16);  // max 300K SNPs in one chrom
    errorHash = newHash(0);  
    readFile(chromPtr->name);
    printOutput(chromPtr->name);
    // clean out snp and error hashes
    freeHash(&snpHash);
    freeHash(&errorHash);
    }
carefulClose(&errorFileHandle);

return 0;
}
