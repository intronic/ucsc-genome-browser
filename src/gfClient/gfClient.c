/* gfClient - A client for the genomic finding program that produces a .psl file. */
/* Copyright 2001-2002 Jim Kent.  All rights reserved. */
#include "common.h"
#include "linefile.h"
#include "aliType.h"
#include "fa.h"
#include "genoFind.h"
#include "psl.h"
#include "cheapcgi.h"
#include "fuzzyFind.h"

/* Variables that can be overridden by command line. */
int dots = 0;
int minScore = 30;
double minIdentity = 90;
char *outputFormat = "psl";
char *qType = "dna";
char *tType = "dna";

void usage()
/* Explain usage and exit. */
{
printf(
  "gfClient v. %d - A client for the genomic finding program that produces a .psl file\n"
  "usage:\n"
  "   gfClient host port nibDir in.fa out.psl\n"
  "where\n"
  "   host is the name of the machine running the gfServer\n"
  "   port is the same as you started the gfServer with\n"
  "   nibDir is the path of the nib files relative to the current dir\n"
  "       (note these are needed by the client as well as the server)\n"
  "   in.fa a fasta format file.  May contain multiple records\n"
  "   out.psl where to put the output\n"
  "options:\n"
  "   -t=type     Database type.  Type is one of:\n"
  "                 dna - DNA sequence\n"
  "                 prot - protein sequence\n"
  "                 dnax - DNA sequence translated in six frames to protein\n"
  "               The default is dna\n"
  "   -q=type     Query type.  Type is one of:\n"
  "                 dna - DNA sequence\n"
  "                 rna - RNA sequence\n"
  "                 prot - protein sequence\n"
  "                 dnax - DNA sequence translated in six frames to protein\n"
  "                 rnax - DNA sequence translated in three frames to protein\n"
  "   -prot       Synonymous with -d=prot -q=prot\n"
  "   -dots=N   Output a dot every N query sequences\n"
  "   -nohead   Suppresses psl five line header\n"
  "   -minScore=N sets minimum score.  This is twice the matches minus the \n"
  "               mismatches minus some sort of gap penalty.  Default is 30\n"
  "   -minIdentity=N Sets minimum sequence identity (in percent).  Default is\n"
  "               90 for nucleotide searches, 25 for protein or translated\n"
  "               protein searches.\n"
  "   -out=type   Controls output file format.  Type is one of:\n"
  "                   psl - Default.  Tab separated format without actual sequence\n"
  "                   pslx - Tab separated format with sequence\n"
  "                   axt - blastz-associated axt format\n"
  "                   maf - multiz-associated maf format\n"
  "                   sim4 - similar to sim4 format\n"
  "                   wublast - similar to wublast format\n"
  "                   blast - similar to NCBI blast format\n"
  "                   blast8- NCBI blast tabular format\n"
  "                   blast9 - NCBI blast tabular format with comments\n"
  "   -maxIntron=N  Sets maximum intron size. Default is %d\n",
                        gfVersion, ffIntronMaxDefault);
exit(-1);
}


struct gfOutput *gvo;

void gfClient(char *hostName, char *portName, char *nibDir, char *inName, 
	char *outName, char *tTypeName, char *qTypeName)
/* gfClient - A client for the genomic finding program that produces a .psl file. */
{
struct lineFile *lf = lineFileOpen(inName, TRUE);
static bioSeq seq;
struct ssBundle *bundleList;
FILE *out = mustOpen(outName, "w");
enum gfType qType = gfTypeFromName(qTypeName);
enum gfType tType = gfTypeFromName(tTypeName);
int dotMod = 0;
char databaseName[256];

snprintf(databaseName, sizeof(databaseName), "%s:%s", hostName, portName);

gvo = gfOutputAny(outputFormat,  round(minIdentity*10), qType == gftProt, tType == gftProt,
	cgiVarExists("nohead"), databaseName, 23, 3.0e9, out);
gfOutputHead(gvo, out);
while (faSomeSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name, qType != gftProt))
    {
    int conn = gfConnect(hostName, portName);
    if (dots != 0)
        {
	if (++dotMod >= dots)
	    {
	    dotMod = 0;
	    fputc('.', stdout);
	    fflush(stdout);
	    }
	}
    if (qType == gftProt && (tType == gftDnaX || tType == gftRnaX))
        {
	gvo->reportTargetStrand = TRUE;
	gfAlignTrans(&conn, nibDir, &seq, minScore, gvo);
	}
    else if ((qType == gftRnaX || qType == gftDnaX) && (tType == gftDnaX || tType == gftRnaX))
        {
	gvo->reportTargetStrand = TRUE;
	gfAlignTransTrans(&conn, nibDir, &seq, FALSE, minScore, gvo, qType == gftRnaX);
	if (qType == gftDnaX)
	    {
	    reverseComplement(seq.dna, seq.size);
	    close(conn);
	    conn = gfConnect(hostName, portName);
	    gfAlignTransTrans(&conn, nibDir, &seq, TRUE, minScore, gvo, FALSE);
	    }
	}
    else if ((tType == gftDna || tType == gftRna) && (qType == gftDna || qType == gftRna))
	{
	gfAlignStrand(&conn, nibDir, &seq, FALSE, minScore, gvo);
	conn = gfConnect(hostName, portName);
	reverseComplement(seq.dna, seq.size);
	gfAlignStrand(&conn, nibDir, &seq, TRUE,  minScore, gvo);
	}
    else
        {
	errAbort("Comparisons between %s queries and %s databases not yet supported",
		qTypeName, tTypeName);
	}
    gfOutputQuery(gvo, out);
    }
if (out != stdout)
    printf("Output is in %s\n", outName);
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiFromCommandLine(&argc, argv, FALSE);
if (argc != 6)
    usage();
if (cgiVarExists("prot"))
    qType = tType = "prot";
qType = cgiUsualString("q", qType);
tType = cgiUsualString("t", tType);
if (sameWord(tType, "prot") || sameWord(tType, "dnax") || sameWord(tType, "rnax"))
    minIdentity = 25;
minIdentity = cgiUsualDouble("minIdentity", minIdentity);
minScore = cgiOptionalInt("minScore", minScore);
dots = cgiOptionalInt("dots", 0);
outputFormat = cgiUsualString("out", outputFormat);
/* set global for fuzzy find functions */
setFfIntronMax(cgiOptionalInt("maxIntron", ffIntronMaxDefault));
gfClient(argv[1], argv[2], argv[3], argv[4], argv[5], tType, qType);
return 0;
}
