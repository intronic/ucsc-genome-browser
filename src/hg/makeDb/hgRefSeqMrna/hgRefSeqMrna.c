/* hgRefSeqMrna - Load refSeq mRNA alignments and other info into refGene table. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "fa.h"
#include "psl.h"
#include "jksql.h"
#include "hdb.h"
#include "hgRelate.h"
#include "obscure.h"


/* Variables that can be set from command line. */
boolean clTest = FALSE;
boolean clDots = 100;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgRefSeqMrna - Load refSeq mRNA alignments and other info into refGene table\n"
  "usage:\n"
  "   hgRefSeqMrna database refSeq.fa refSeq.ra refSeq.psl loc2ref refSeqPep.fa\n"
  "options:\n"
  "   -dots=N Output a dot every N records parsed\n"
  "   -test Generates tab files but doesn't actually load database.\n");
}

char *refLinkTableDef = 
"CREATE TABLE refLink (\n"
"    name varchar(255) not null,        # Name displayed in UI\n"
"    product varchar(255) not null, 	# Name of protein product\n"
"    mrnaAcc varchar(255) not null,	# mRNA accession\n"
"    protAcc varchar(255) not null,	# protein accession\n"
"    geneName int unsigned not null,	# pointer to geneName table\n"
"    prodName int unsigned not null,	# pointer to product name table\n"
"    locusLinkId int unsigned not null,	# Locus Link ID\n"
"              #Indices\n"
"    PRIMARY KEY(mrnaAcc(12)),\n"
"    index(name(10)),\n"
"    index(protAcc(10)),\n"
"    index(locusLinkId),\n"
"    index(prodName),\n"
"    index(geneName)\n"
")";

char *refGeneTableDef = 
"CREATE TABLE refGene ( \n"
"   name varchar(255) not null,	# mrna accession of gene \n"
"   chrom varchar(255) not null,	# Chromosome name \n"
"   strand char(1) not null,	# + or - for strand \n"
"   txStart int unsigned not null,	# Transcription start position \n"
"   txEnd int unsigned not null,	# Transcription end position \n"
"   cdsStart int unsigned not null,	# Coding region start \n"
"   cdsEnd int unsigned not null,	# Coding region end \n"
"   exonCount int unsigned not null,	# Number of exons \n"
"   exonStarts longblob not null,	# Exon start positions \n"
"   exonEnds longblob not null,	# Exon end positions \n"
          "   #Indices \n"
"   INDEX(name(10)), \n"
"   INDEX(chrom(12),txStart), \n"
"   INDEX(chrom(12),txEnd) \n"
")";

char *refPepTableDef =
"CREATE TABLE refPep (\n"
"    name varchar(255) not null,        # Accession of sequence\n"
"    seq longblob not null,     # Peptide sequence\n"
"              #Indices\n"
"    PRIMARY KEY(name(32))\n"
")\n";

char *refMrnaTableDef =
"CREATE TABLE refMrna (\n"
"    name varchar(255) not null,        # Accession of sequence\n"
"    seq longblob not null,     	# Nucleotide sequence\n"
"              #Indices\n"
"    PRIMARY KEY(name(32))\n"
")\n";



struct hash *loadNameTable(struct sqlConnection *conn, 
    char *tableName, int hashSize)
/* Create a hash and load it up from table. */
{
char query[128];
struct sqlResult *sr;
char **row;
struct hash *hash = newHash(hashSize);

sprintf(query, "select id,name from %s", tableName);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    hashAdd(hash, row[1], intToPt(sqlUnsigned(row[0])));
    }
sqlFreeResult(&sr);
return hash;
}

int putInNameTable(struct hash *hash, FILE *f, char *name)
/* Add to name table if it isn't there already. 
 * Return ID of name in table. */
{
struct hashEl *hel;
if (name == NULL)
    return 0;
if ((hel = hashLookup(hash, name)) != NULL)
    return ptToInt(hel->val);
else
    {
    int id = hgNextId();
    fprintf(f, "%u\t%s\n", id, name);
    hashAdd(hash, name, intToPt(id));
    return id;
    }
}

struct refSeqInfo
/* Information on one refSeq. */
    {
    struct refSeqInfo *next;
    char *mrnaAcc;		/* Accession of mRNA. */
    char *proteinAcc;		/* Accession of protein. */
    char *geneName;		/* Gene name */
    char *productName;		/* Product name (long name for protein) or NULL. */
    int locusLinkId;		/* LocusLink id (of mRNA) */
    int size;                   /* mRNA size. */
    int cdsStart, cdsEnd;       /* start/end of coding region (mRNA coords) */
    int ngi;			/* Nucleotide GI number. */
    struct psl *pslList;	/* List of aligments. */
    int geneNameId;		/* Id of gene name in table. */
    int productNameId;		/* Id of product name in table. */
    };

struct hash *hashNextRa(struct lineFile *lf)
/* Read in a record of a .ra file into a hash */
{
struct hash *hash = newHash(5);
char *line, *firstWord;
int lineSize;
int lineCount = 0;

while (lineFileNext(lf, &line, &lineSize))
    {
    if (line[0] == 0)
        break;
    ++lineCount;
    firstWord = nextWord(&line);
    hashAdd(hash, firstWord, cloneString(line));
    }
if (lineCount == 0)
    freeHash(&hash);
return hash;
}

void dotOut()
/* Print dot to standard output and flush it so user can
 * see it right away. */
{
fputc('.', stdout);
fflush(stdout);
}

void parseCds(char *gbCds, int start, int end, int *retStart, int *retEnd)
/* Parse genBank style cds string into something we can use... */
{
char *s = gbCds;
boolean gotStart = FALSE, gotEnd = TRUE;

gotStart = isdigit(s[0]);
s = strchr(s, '.');
if (s == NULL || s[1] != '.')
    errAbort("Malformed GenBank cds %s", gbCds);
s[0] = 0;
s += 2;
gotEnd = isdigit(s[0]);
if (gotStart) start = atoi(gbCds) - 1;
if (gotEnd) end = atoi(s);
*retStart = start;
*retEnd = end;
}

void findCdsStartEndInGenome(struct refSeqInfo *rsi, struct psl *psl, 
	int *retCdsStart, int *retCdsEnd)
/* Convert cdsStart/End from mrna to genomic coordinates. */
{
int strandStart, strandEnd;
int startOffset, endOffset;
int cdsStart, cdsEnd;
int i;

if (psl->strand[0] == '-')
    {
    strandStart = psl->qSize - rsi->cdsEnd;
    strandEnd = psl->qSize - rsi->cdsStart;
    }
else
    {
    strandStart = rsi->cdsStart;
    strandEnd = rsi->cdsEnd;
    }
startOffset = strandStart - psl->qStart;
if (startOffset < 0) startOffset = 0;
endOffset = psl->qEnd - strandEnd;
if (endOffset < 0) endOffset = 0;
#ifdef DEBUG
uglyf("strand %c, size %d, cdsStart %d, strandStart %d, startOffset %d, qStart %d\n",
    psl->strand[0], psl->qSize, rsi->cdsStart, strandStart, startOffset, psl->qStart);
uglyf("    cdsEnd %d, strandEnd %d, endOffset %d, qEnd %d\n",
    rsi->cdsEnd, strandEnd, endOffset, psl->qEnd);
#endif /* DEBUG */

/* Adjust starting pos. */
cdsStart = -1;
for (i=0; i<psl->blockCount; ++i)
    {
    int blockSize = psl->blockSizes[i];
    if (startOffset < blockSize)
	{
        cdsStart = psl->tStarts[i] + startOffset;
	break;
	}
    startOffset -= blockSize;
    }

/* Adjust end pos. */
cdsEnd = -1;
for (i=psl->blockCount-1; i >= 0; --i)
    {
    int blockSize = psl->blockSizes[i];
    if (endOffset < blockSize)
        {
	cdsEnd = psl->tStarts[i] + blockSize - endOffset;
	break;
	}
    endOffset -= blockSize;
    }

#ifdef DEBUG
uglyf("   final cdsStart %d, cdsEnd %d, tStart %d, tEnd %d\n",
	cdsStart, cdsEnd, psl->tStart, psl->tEnd);
#endif /* DEBUG */
if (cdsStart == -1 || cdsEnd == -1)
    {
    cdsEnd = cdsStart = psl->tEnd;
    }
*retCdsStart = cdsStart;
*retCdsEnd = cdsEnd;
}

char *unburyAcc(struct lineFile *lf, char *longNcbiName)
/* Return accession given a long style NCBI name.  
 * Cannibalizes the longNcbiName. */
{
char *parts[5];
int partCount;

partCount = chopByChar(longNcbiName, '|', parts, ArraySize(parts));
if (partCount < 4) 
    errAbort("Badly formed longNcbiName line %d of %s\n", lf->lineIx, lf->fileName);
chopSuffix(parts[3]);
return parts[3];
}

void writeSeqTable(char *faName, FILE *out, boolean unburyAccession, boolean isPep)
/* Write out contents of fa file to name/sequence pairs in tabbed out file. */
{
struct lineFile *lf = lineFileOpen(faName, TRUE);
bioSeq seq;
int dotMod = 0;

printf("Reading %s\n", faName);
while (faSomeSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name, !isPep))
    {
    if (clDots > 0 && ++dotMod == clDots )
        {
	dotMod = 0;
	dotOut();
	}
    if (unburyAccession)
	seq.name = unburyAcc(lf, seq.name);
    fprintf(out, "%s\t%s\n", seq.name, seq.dna);
    }
if (clDots) printf("\n");
lineFileClose(&lf);
}

char *emptyForNull(char *s)
/* Return "" if s is empty, else return s. */
{
if (s == NULL) s = "";
return s;
}

void processRefSeq(char *faFile, char *raFile, char *pslFile, char *loc2refFile, 
	char *pepFile)
/* hgRefSeqMrna - Load refSeq mRNA alignments and other info into 
 * refSeqGene table. */
{
struct lineFile *lf;
struct hash *raHash, *rsiHash = newHash(0);
struct refSeqInfo *rsiList = NULL, *rsi;
char *s, *row[5];
int i, dotMod = 0;
int noLocCount = 0;
int rsiCount = 0;
int noProtCount = 0;
struct psl *psl;
int cdsStart, cdsEnd;
struct sqlConnection *conn = hgStartUpdate();
struct hash *productHash = loadNameTable(conn, "productName", 16);
struct hash *geneHash = loadNameTable(conn, "geneName", 16);
struct hash *kgHash = newHash(0);
char *kgName = "refGene";
FILE *kgTab = hgCreateTabFile(kgName);
FILE *productTab = hgCreateTabFile("productName");
FILE *geneTab = hgCreateTabFile("geneName");
FILE *refLinkTab = hgCreateTabFile("refLink");
FILE *refPepTab = hgCreateTabFile("refPep");
FILE *refMrnaTab = hgCreateTabFile("refMrna");
int productNameId, geneNameId;

/* Make refLink and other tables table if they don't exist already. */
sqlMaybeMakeTable(conn, "refLink", refLinkTableDef);
sqlUpdate(conn, "delete from refLink");
sqlMaybeMakeTable(conn, "refGene", refGeneTableDef);
sqlUpdate(conn, "delete from refGene");
sqlMaybeMakeTable(conn, "refPep", refPepTableDef);
sqlUpdate(conn, "delete from refPep");
sqlMaybeMakeTable(conn, "refMrna", refMrnaTableDef);
sqlUpdate(conn, "delete from refMrna");


/* Scan through .ra file and make up start of refSeqInfo
 * objects in hash and list. */
printf("Scanning %s\n", raFile);
lf = lineFileOpen(raFile, TRUE);
while ((raHash = hashNextRa(lf)) != NULL)
    {
    if (clDots > 0 && ++dotMod == clDots )
        {
	dotMod = 0;
	dotOut();
	}
    AllocVar(rsi);
    slAddHead(&rsiList, rsi);
    if ((s = hashFindVal(raHash, "acc")) == NULL)
        errAbort("No acc near line %d of %s", lf->lineIx, lf->fileName);
    rsi->mrnaAcc = cloneString(s);
    if ((s = hashFindVal(raHash, "siz")) == NULL)
        errAbort("No siz near line %d of %s", lf->lineIx, lf->fileName);
    rsi->size = atoi(s);
    if ((s = hashFindVal(raHash, "gen")) != NULL)
	rsi->geneName = cloneString(s);
    else
        warn("No gene name for %s", rsi->mrnaAcc);
    if ((s = hashFindVal(raHash, "cds")) != NULL)
        parseCds(s, 0, rsi->size, &rsi->cdsStart, &rsi->cdsEnd);
    else
        rsi->cdsEnd = rsi->size;
    if ((s = hashFindVal(raHash, "ngi")) != NULL)
        rsi->ngi = atoi(s);
    rsi->geneNameId = putInNameTable(geneHash, geneTab, rsi->geneName);
    s = hashFindVal(raHash, "pro");
    if (s != NULL)
        rsi->productName = cloneString(s);
    rsi->productNameId = putInNameTable(productHash, productTab, s);
    hashAdd(rsiHash, rsi->mrnaAcc, rsi);

    freeHashAndVals(&raHash);
    }
lineFileClose(&lf);
if (clDots) printf("\n");

/* Scan through loc2ref filling in some gaps in rsi. */
printf("Scanning %s\n", loc2refFile);
lf = lineFileOpen(loc2refFile, TRUE);
while (lineFileRow(lf, row))
    {
    if ((rsi = hashFindVal(rsiHash, row[1])) != NULL)
        {
	rsi->locusLinkId = lineFileNeedNum(lf, row, 0);
	rsi->proteinAcc = cloneString(row[4]);
	}
    }
lineFileClose(&lf);

/* Report how many seem to be missing from loc2ref file. 
 * Write out knownInfo file. */
printf("Writing %s\n", "refLink.tab");
for (rsi = rsiList; rsi != NULL; rsi = rsi->next)
    {
    ++rsiCount;
    if (rsi->locusLinkId == 0)
        ++noLocCount;
    if (rsi->proteinAcc == NULL)
        ++noProtCount;
    fprintf(refLinkTab, "%s\t%s\t%s\t%s\t%u\t%u\t%u\n",
	emptyForNull(rsi->geneName), 
	emptyForNull(rsi->productName),
    	emptyForNull(rsi->mrnaAcc), 
	emptyForNull(rsi->proteinAcc),
	rsi->geneNameId, rsi->productNameId, 
	rsi->locusLinkId);
    }
if (noLocCount) 
    printf("Missing locusLinkIds for %d of %d\n", noLocCount, rsiCount);
if (noProtCount)
    printf("Missing protein accessions for %d of %d\n", noProtCount, rsiCount);

/* Process alignments and write them out as genes. */
lf = pslFileOpen(pslFile);
dotMod = 0;
while ((psl = pslNext(lf)) != NULL)
    {
    if (clDots > 0 && ++dotMod == clDots )
        {
	dotMod = 0;
	dotOut();
	}
    fprintf(kgTab, "%s\t%s\t%c\t%d\t%d\t",
	psl->qName, psl->tName, psl->strand[0], psl->tStart, psl->tEnd);
    rsi = hashMustFindVal(rsiHash, psl->qName);
    findCdsStartEndInGenome(rsi, psl, &cdsStart, &cdsEnd);
    fprintf(kgTab, "%d\t%d\t", cdsStart, cdsEnd);
    fprintf(kgTab, "%d\t", psl->blockCount);
    for (i=0; i<psl->blockCount; ++i)
	fprintf(kgTab, "%d,", psl->tStarts[i]);
    fprintf(kgTab, "\t");
    for (i=0; i<psl->blockCount; ++i)
	fprintf(kgTab, "%d,", psl->tStarts[i] + psl->blockSizes[i]);
    fprintf(kgTab, "\n");
    }
if (clDots) printf("\n");

writeSeqTable(pepFile, refPepTab, TRUE, TRUE);
writeSeqTable(faFile, refMrnaTab, FALSE, FALSE);

carefulClose(&kgTab);
carefulClose(&productTab);
carefulClose(&geneTab);
carefulClose(&refLinkTab);
carefulClose(&refPepTab);
carefulClose(&refMrnaTab);
printf("Loading database with %s\n", kgName);
hgLoadTabFile(conn, kgName);
printf("Loading database with %s\n", "productName");
hgLoadTabFile(conn, "productName");
printf("Loading database with %s\n", "geneName");
hgLoadTabFile(conn, "geneName");
printf("Loading database with %s\n", "refLink");
hgLoadTabFile(conn, "refLink");
printf("Loading database with %s\n", "refPep");
hgLoadTabFile(conn, "refPep");
printf("Loading database with %s\n", "refMrna");
hgLoadTabFile(conn, "refMrna");
hgEndUpdate(&conn, "Added refSeq genes");
}

void hgRefSeqMrna(char *database, char *faFile, char *raFile, char *pslFile, 
	char *loc2refFile, char *pepFile)
/* hgRefSeqMrna - Load refSeq mRNA alignments and other info into 
 * refSeqGene table. */
{
hgSetDb(database);
processRefSeq(faFile, raFile, pslFile, loc2refFile, pepFile);
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
clTest = cgiBoolean("test");
clDots = cgiOptionalInt("dots", clDots);
if (argc != 7)
    usage();
hgRefSeqMrna(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
return 0;
}
