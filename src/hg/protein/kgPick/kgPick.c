/* kgPick - select the best repersentative mRNA/protein pair from KG candidates 
   having identical CDS Structure */
#include "common.h"
#include "hCommon.h"
#include "hdb.h"

char printedMrna[40], printedProt[40], printedCds[40];

void usage()
/* Explain usage and exit. */
{
errAbort(
  "kgPick - select the best repersentative mRNA/protein pair\n"
  "usage:\n"
  "   kgPick tempDb outfile\n"
  "      tempDb is the temp KG DB name\n"
  "      proteinsDb is the proteinsYYMMDDt DB name\n"
  "      outfile is the output file name\n"
  "      dupOutfile is the duplicate mrna/prot output file name\n"
  "example: kgPick kgHg17ETemp proteins050415 kg2.tmp dupSpMrna.tmp\n");
}

void printKg(struct sqlConnection *conn, struct sqlConnection *conn2, 
             char *kgTempDb, char *proteinsDb, char *cdsId, char *mrnaID, char *protAcc, 
	     char *alignID, FILE *outf)
{
char query[256];
struct sqlResult *sr;
char **row;
int i;
char condStr[256];
char *displayId;

safef(query, sizeof(query), "select * from %s.kgCandidate where alignID='%s'", kgTempDb, alignID);
sr = sqlMustGetResult(conn, query);
row = sqlNextRow(sr);
while (row != NULL)
    {
    for (i=0; i<10; i++) 
    	{
	fprintf(outf, "%s\t", row[i]);
	}
    
    /* get protein display ID */
    sprintf(condStr, "accession='%s'", protAcc);
    displayId = sqlGetField(conn2, proteinsDb, "spXref3", "displayID", condStr);
    
    fprintf(outf, "%s\t%s\t%s\n", displayId, alignID, cdsId);
    safef(printedCds, sizeof(printedCds), cdsId);
    safef(printedMrna, sizeof(printedCds), row[0]);
    safef(printedProt, sizeof(printedProt), displayId);
    row = sqlNextRow(sr);
    }
sqlFreeResult(&sr);
}

int main(int argc, char *argv[])
{
struct sqlConnection *conn2, *conn3, *conn4, *conn5;
char query2[256], query3[256];
struct sqlResult *sr2, *sr3;
char **row2, **row3;
char *kgTempDb;
char *outfileName, *dupOutfileName;
FILE *outf, *dupOutf;
char *score;
int  i;
char *protAcc;
char *ranking;
char *cdsId, *mrnaID, *alignID;
char *protDbId;
char *proteinsDb;
boolean gotRefseq, gotNonRef;
boolean isRefseq;

if (argc != 5) usage();
kgTempDb       = argv[1];
proteinsDb     = argv[2];
outfileName    = argv[3];
dupOutfileName = argv[4];

outf    = mustOpen(outfileName, "w");
dupOutf = mustOpen(dupOutfileName, "w");

conn2= hAllocConn();
conn3= hAllocConn();
conn4= hAllocConn();
conn5= hAllocConn();

strcpy(printedCds, "");
strcpy(printedMrna, "");
strcpy(printedProt, "");

/* go through each unique cds structure */
safef(query2, sizeof(query2), "select distinct cdsId from %s.kgCandidateZ", kgTempDb);
sr2 = sqlMustGetResult(conn2, query2);
row2 = sqlNextRow(sr2);
while (row2 != NULL)
    {
    /* get all mrna/prot pairs with this CDS structure */
    cdsId = row2[0];

    /* ranking reflects CDS quaility and preference for RefSeq and MGC */
    /* Swiss-Prot is prefered than TrEMBL */
    /* finally, higher prot-Mrna alignment score is prefered */
    safef(query3, sizeof(query3), 
    "select * from %s.kgCandidateZ where cdsId='%s' order by ranking, protDbId, score desc", 
    	    kgTempDb, cdsId);
    sr3  = sqlMustGetResult(conn3, query3);
    row3 = sqlNextRow(sr3);
	      
    gotRefseq    = FALSE;
    gotNonRef    = FALSE;
    
    while (row3 != NULL)
        {
        cdsId    = row3[0];
	ranking  = row3[1];
	protDbId = row3[2];
	score    = row3[3];
	mrnaID   = row3[4];
	protAcc  = row3[5];
	alignID  = row3[6];

    	isRefseq    = FALSE;
	
	if ((mrnaID[0] == 'N') && (mrnaID[0] == 'M') && (mrnaID[0] == '_'))
	    {
	    isRefseq  = TRUE;
	    }
	
	if (isRefseq)
	    {
	    /* print one qualified RefSeq */
	    if (!gotRefseq)
	    	{
		printKg(conn4, conn5, kgTempDb, proteinsDb, cdsId, mrnaID, protAcc, alignID, outf);
		gotRefseq = TRUE;
		}
	    }
	else
	    {
	    /* print out only one non-RefSeq entry for this CDS structure */
	    if (!gotNonRef)
	    	{
	        printKg(conn4, conn5, kgTempDb, proteinsDb, cdsId, mrnaID, protAcc, alignID, outf);
		gotNonRef = TRUE;
		}
	    }
	
	/* save duplicates of mrna/prot that has same CDS but does not make it to final KG set */
	if (sameWord(printedCds, cdsId))
	    {
	    if ( (!sameWord(printedMrna, mrnaID)) || (!sameWord(printedProt, protAcc)) )
	    	{
	    	fprintf(dupOutf, "%s\t%s\t%s\t%s\n", printedMrna, printedProt, mrnaID, protAcc);
	   	}
	    }
        row3 = sqlNextRow(sr3);
	}
    
    sqlFreeResult(&sr3);
    row2 = sqlNextRow(sr2);
    }
sqlFreeResult(&sr2);

hFreeConn(&conn2);
hFreeConn(&conn3);
fclose(outf);
fclose(dupOutf);
return(0);
}

