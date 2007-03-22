/* txGeneXref - Make kgXref type table for genes.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"
#include "spDb.h"
#include "cdsPick.h"
#include "txInfo.h"
#include "txRnaAccs.h"

static char const rcsid[] = "$Id: txGeneXref.c,v 1.5 2007/03/22 06:59:14 kent Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "txGeneXref - Make kgXref type table for genes.\n"
  "usage:\n"
  "   txGeneXref genomeDb uniProtDb gene.info genes.picks genes.ev output.xref\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void removePickVersions(struct cdsPick *pick)
/* Remove version suffixes from various pick fields. */
{
chopSuffix(pick->refProt);
chopSuffix(pick->refSeq);
}

void txGeneXref(char *genomeDb, char *uniProtDb, char *infoFile, char *pickFile, 
	char *evFile, char *outFile)
/* txGeneXref - Make kgXref type table for genes.. */
{
/* Load picks into hash.  We don't use cdsPicksLoadAll because empty fields
 * cause that autoSql-generated routine problems. */
struct hash *pickHash = newHash(18);
struct cdsPick *pick;
struct lineFile *lf = lineFileOpen(pickFile, TRUE);
char *row[CDSPICK_NUM_COLS];
while (lineFileRowTab(lf, row))
    {
    pick = cdsPickLoad(row);
    removePickVersions(pick);
    hashAdd(pickHash, pick->name, pick);
    }

/* Load evidence into hash */
struct hash *evHash = newHash(18);
struct txRnaAccs *ev, *evList = txRnaAccsLoadAll(evFile);
for (ev = evList; ev != NULL; ev = ev->next)
    hashAdd(evHash, ev->name, ev);

/* Open connections to our databases */
struct sqlConnection *gConn = sqlConnect(genomeDb);
struct sqlConnection *uConn = sqlConnect(uniProtDb);

/* Read in info file, and loop through it to make out file. */
struct txInfo *info, *infoList = txInfoLoadAll(infoFile);
FILE *f = mustOpen(outFile, "w");
int missingEvCount = 0;
char *missingEvAcc = NULL;
for (info = infoList; info != NULL; info = info->next)
    {
    char *kgID = info->name;
    char *mRNA = "";
    char *spID = "";
    char *spDisplayID = "";
    char *geneSymbol = NULL;
    char *refseq = "";
    char *protAcc = "";
    char *description = NULL;
    char query[256];
    pick = hashFindVal(pickHash, info->name);
    ev = hashFindVal(evHash, info->name);
    if (ev == NULL)
	{
        missingEvCount += 1;
	missingEvAcc = info->name;
	mRNA = cloneString("");
	}
    else
	{
	mRNA = cloneString(ev->primary);
	chopSuffix(mRNA);
	}
    if (pick != NULL)
       {
       /* Fill in the relatively straightforward fields. */
       refseq = pick->refSeq;
       if (info->orfSize > 0)
	    {
	    spID = pick->uniProt;
	    protAcc = pick->refProt;
	    if (pick->uniProt[0] != 0)
	       {
	       spDisplayID = spAnyAccToId(uConn, spID);
	       }
	    }

       /* Fill in gene symbol and description from refseq if possible. */
       if (refseq[0] != 0)
           {
	   struct sqlResult *sr;
	   safef(query, sizeof(query), "select name,product from refLink where mrnaAcc='%s'",
	   	refseq);
	   sr = sqlGetResult(gConn, query);
	   char **row = sqlNextRow(sr);
	   if (row != NULL)
	       {
	       geneSymbol = cloneString(row[0]);
	       if (!sameWord("unknown protein", row[1]))
		   description = cloneString(row[1]);
	       }
	    sqlFreeResult(&sr);
	   }

       /* If need be try uniProt for gene symbol and description. */
       if (spID[0] != 0 && (geneSymbol == NULL || description == NULL))
           {
	   char *acc = spLookupPrimaryAcc(uConn, spID);
	   if (description == NULL)
	       description = spDescription(uConn, acc);
	   if (geneSymbol == NULL)
	       {
	       struct slName *nameList = spGenes(uConn, acc);
	       if (nameList != NULL)
		   geneSymbol = cloneString(nameList->name);
	       slFreeList(&nameList);
	       }
	   }

       }
    /* Still no joy? Try genbank RNA records. */
    if (geneSymbol == NULL || description == NULL)
	{
	if (ev != NULL)
	    {
	    int i;
	    for (i=0; i<ev->accCount; ++i)
		{
		char *acc = ev->accs[i];
		chopSuffix(acc);
		if (geneSymbol == NULL)
		    {
		    safef(query, sizeof(query), 
			"select geneName.name from gbCdnaInfo,geneName "
			"where geneName.id=gbCdnaInfo.geneName and gbCdnaInfo.acc = '%s'", acc);
		    geneSymbol = sqlQuickString(gConn, query);
		    if (geneSymbol != NULL)
			{
			if (sameString(geneSymbol, "n/a"))
			   geneSymbol = NULL;
			}
		    }
		if (description == NULL)
		    {
		    safef(query, sizeof(query), 
			"select description.name from gbCdnaInfo,description "
			"where description.id=gbCdnaInfo.description "
			"and gbCdnaInfo.acc = '%s'", acc);
		    description = sqlQuickString(gConn, query);
		    if (description != NULL)
			{
			if (sameString(description, "n/a"))
			   description = NULL;
			}
		    }
		}
	    }
	}
    if (geneSymbol == NULL)
        geneSymbol = mRNA;
    if (description == NULL)
        description = mRNA;
    /* Get rid of some characters that will cause havoc downstream. */
    stripChar(geneSymbol, '\'');
    subChar(geneSymbol, '<', '[');
    subChar(geneSymbol, '>', ']');
    if (strlen(geneSymbol) > 40)
        strcpy(geneSymbol+37, "...");

    fprintf(f, "%s\t", kgID);
    fprintf(f, "%s\t", mRNA);
    fprintf(f, "%s\t", spID);
    fprintf(f, "%s\t", spDisplayID);
    fprintf(f, "%s\t", geneSymbol);
    fprintf(f, "%s\t", refseq);
    fprintf(f, "%s\t", protAcc);
    fprintf(f, "%s\n", description);
    }
if (missingEvCount > 4)
    errAbort("%d accessions in %s but not %s. Example: %s\n", 
    	missingEvCount, infoFile, evFile, missingEvAcc);
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 7)
    usage();
txGeneXref(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
return 0;
}
