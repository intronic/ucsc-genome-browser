/* pfamXref read and parse the Pfam-A data file to generate 
   the .tab files to enable xref between Pfam and SWISS-PROT entries */

#include "common.h"
#include "hCommon.h"
#include "hdb.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "pfamXref - create pfam xref .tab file "
  "usage:\n"
  "   pfamXref pn pfamInput pfamOutput pfamXref\n"
  "            pn is protein database name\n"
  "            pfamInput  is input  file name of Pfam data.\n"
  "            pfamOutput is output file name of output file of Pfam AC and description.\n"
  "            pfamXref   is output file name of xref file of Pfam AC and SWISS-PROT ID and KG ID.\n"
  "Example: pfamXref rn3 proteins07030 /cluster/store5/proteins/pfam/071803/Pfam-A.full pfamA.tab pfamAXref.tab\n");
}

char line[2000];
int main(int argc, char *argv[])
{
struct sqlConnection *conn, *conn2, *conn5;
    
FILE *inf;
FILE *o1, *o2;

char cond_str[256];
char *proteinDB;
char *proteinFileName;
char *answer;
char *outputFileName, *outputFileName2;
char *desc;
char *id;
char *chp0, *chp1, *chp2, *chp;
char *pfamID;
char *swissAC, *swissDisplayID;
char emptyString[10] = {""};
int done, gsDone, gsFound, acFound;

if (argc != 5) usage();
   
proteinDB	 = cloneString(argv[1]);
proteinFileName  = cloneString(argv[2]);
outputFileName   = cloneString(argv[3]);
outputFileName2  = cloneString(argv[4]);

conn = hAllocConn();
	
o1 = fopen(outputFileName, "w");
o2 = fopen(outputFileName2, "w");
    
if ((inf = fopen(proteinFileName, "r")) == NULL)
    {		
    fprintf(stderr, "Can't open file %s.\n", proteinFileName);
    exit(8);
    }
	
done = 0;
while (!done)
    {
    // get to the beginning of a Pfam record
    acFound = 0;
    while (fgets(line, 1000, inf) != NULL)
    	{
    	chp = strstr(line, "GF AC");
    	if (chp != NULL)
		{
		acFound = 1;
		break;
		}
	}
    if (!acFound) break;
    
    chp = chp + 8;
    *(chp + strlen(chp) - 1) = '\0'; // remove LF
    pfamID = strdup(chp);
    
    // Get Pfam description
    // Please note, Pfam-B does not have this field.

    fgets(line, 1000, inf);
    chp = strstr(line, "GF DE   ");
    chp = chp + 8;
    *(chp + strlen(chp) - 1) = '\0'; // remove LF
    desc = chp;
    fprintf(o1, "%s\t%s\n", pfamID, desc);
    fflush(o1); 

    // work on "#=GS ... AC ... " lines to get SWISS-PROT accession number

    gsDone  = 0;
    gsFound = 0;
    while (!gsDone)
	{
	fgets(line, 1000, inf);
    	chp = strstr(line, "#=GS ");
    	if (chp != NULL)
	    {
	    chp = strstr(chp, " AC ");
	    if (chp != NULL)
		{
		gsFound = 1;
	    	chp = chp + 4;
    	    	*(chp + strlen(chp) - 1) = '\0'; // remove LF
    		swissAC = chp;

		// get display ID from AC		
		sprintf(cond_str, "accession = '%s'", swissAC);
    		answer = sqlGetField(conn, proteinDB, "spXref3", "displayID", cond_str);
		if (answer != NULL)
		    {
		    swissDisplayID = answer;
		    }
		else
		    {
		    // ACs missing from spXref3 might be found from spSecondardy table
		    sprintf(cond_str, "accession2 = '%s'", swissAC);
    		    answer = sqlGetField(conn, proteinDB, "spSecondaryID", "displayID", cond_str);
		    if (answer != NULL)
		    	{
		    	swissDisplayID = answer;
		    	}
		    else
			{
		    	printf("%s not found in %s.spXref3\n", swissAC, proteinDB);fflush(stdout);
		    	swissDisplayID = emptyString;
			}
		    }
	    	fprintf(o2, "%s\t%s\t%s\n", pfamID, swissAC, swissDisplayID);
		}
	    }
    	else
	    {
	    if (gsFound)
		{
		gsDone = 1;
		}
	    }
	} 
    }
fclose(o1);
fclose(o2);
return(0);
}
