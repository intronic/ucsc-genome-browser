/* hgSeqSearch - CGI-script to manage fast human genome sequence searching. */
#include "common.h"
#include "hCommon.h"
#include "portable.h"
#include "linefile.h"
#include "dnautil.h"
#include "fa.h"
#include "psl.h"
#include "genoFind.h"
#include "cheapcgi.h"
#include "htmshell.h"

/* Some variables that say where sequence is.  These are default
 * values that can be overriden by CGI command. */
char *hostName = "kks00.cse.ucsc.edu";
char *hostPort = "17777";
char *nibDir = "/projects/cc/hg/oo.23/nib";
char *database = "hg5";
boolean tx = FALSE;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgSeqSearch - CGI-script to manage fast human genome sequence searching\n"
  "usage:\n"
  "   hgBlat XXX\n");
}

int pslCmpMatches(const void *va, const void *vb)
/* Compare to sort based on query. */
{
const struct psl *a = *((struct psl **)va);
const struct psl *b = *((struct psl **)vb);
int aScore = a->match + (a->repMatch>>1) - a->misMatch - a->qNumInsert;
int bScore = b->match - (b->repMatch>>1) - b->misMatch - b->qNumInsert;
return bScore - aScore;
}

void showAliPlaces(char *pslName, char *faName)
/* Show all the places that align. */
{
struct lineFile *lf = pslFileOpen(pslName);
struct psl *pslList = NULL, *psl;
char *browserUrl = hgTracksName();
char *extraCgi = "";

while ((psl = pslNext(lf)) != NULL)
    {
    slAddHead(&pslList, psl);
    }
lineFileClose(&lf);
if (pslList == NULL)
    errAbort("Sorry, no matches found");

slSort(&pslList, pslCmpMatches);
printf("<TT><PRE>");
printf(" SIZE IDENTITY CHROMOSOME STRAND  START     END       cDNA   START  END  TOTAL\n");
printf("------------------------------------------------------------------------------\n");
for (psl = pslList; psl != NULL; psl = psl->next)
    {
    printf("<A HREF=\"%s?position=%s:%d-%d&db=%s&ss=%s+%s%s\">",
	browserUrl, psl->tName, psl->tStart, psl->tEnd, database, 
	pslName, faName, extraCgi);
    printf("%5d  %5.1f%%  %9s     %s %9d %9d  %8s %5d %5d %5d</A>\n",
	psl->match + psl->misMatch + psl->repMatch + psl->nCount,
	100.0 - pslCalcMilliBad(psl, TRUE) * 0.1,
	skipChr(psl->tName), psl->strand, psl->tStart + 1, psl->tEnd,
	psl->qName, psl->qStart+1, psl->qEnd, psl->qSize);
    }
pslFreeList(&pslList);
printf("</TT></PRE>");
}

void blatSeq(char *userSeq)
/* Blat sequence user pasted in. */
{
FILE *f;
static struct dnaSeq *seq;
struct tempName pslTn, faTn;
int maxSize = 20000;
char *port = cgiOptionalString("port");
char *host = cgiOptionalString("host");
char *nib = cgiOptionalString("nib");
char *db = cgiOptionalString("db");
int conn;

tx = cgiBoolean("tx");
if (tx)
    {
    hostPort = "17778";
    hostName = "cc.cse.ucsc.edu";
    nibDir = "/projects/hg2/gs.6/oo.27/nib";
    database = "hg6";
    }
if (port != NULL)
    hostPort = port;
if (host != NULL)
    hostName = host;
if (nib != NULL)
    nibDir = nib;
if (db != NULL)
    database = db;

/* Load up sequence from CGI. */
seq = faSeqFromMemText(cloneString(userSeq), !tx);
if (seq->name[0] == 0)
    seq->name = "YourSeq";

if (seq->size > maxSize)
   {
   printf("Warning: only the first %d of %d bases used.<BR>\n",
   	maxSize, seq->size);
   seq->size = maxSize;
   seq->dna[maxSize] = 0;
   }

makeTempName(&faTn, "hgSs", ".fa");
faWrite(faTn.forCgi, seq->name, seq->dna, seq->size);

makeTempName(&pslTn, "hgSs", ".pslx");
f = mustOpen(pslTn.forCgi, "w");


/* Create a temporary .psl file with the alignments against genome. */
conn = gfConnect(hostName, hostPort);
if (tx)
    {
    static struct gfSavePslxData data;
    data.f = f;
    data.reportTargetStrand = TRUE;
    pslxWriteHead(f, gftProt, gftDnaX);
    gfAlignTrans(conn, nibDir, seq, 12, gfSavePslx, &data);
    }
else
    {
    pslxWriteHead(f, gftDna, gftDna);
    gfAlignStrand(conn, nibDir, seq, FALSE, ffCdna, 36, gfSavePsl, f);
    close(conn);
    reverseComplement(seq->dna, seq->size);
    conn = gfConnect(hostName, hostPort);
    gfAlignStrand(conn, nibDir, seq, TRUE,  ffCdna, 36, gfSavePsl, f);
    }
close(conn);
carefulClose(&f);

showAliPlaces(pslTn.forCgi, faTn.forCgi);
}

char *defaultOrDb(char *db)
/* Return default database if db is null, else db. */
{
if (db == NULL)
    db = database;
return db;
}

char *dateForDb(char *db)
/* Return date associated with database. */
{
if (db == NULL)
   db = database;
if (sameString(db, "hg6"))
    return "12 Dec. 2000";
else
    return "7 Oct. 2000";
}

void askForSeq()
/* Put up a little form that asks for sequence.
 * Call self.... */
{
char *db = cgiOptionalString("db");
char *port, *host, *nib;

printf("%s", 
"<FORM ACTION=\"../cgi-bin/hgBlat\" METHOD=POST>\n"
"<H1 ALIGN=CENTER>BLAT Search Human Genome</H1>\n"
"<P>\n"
"<TABLE BORDER=0 WIDTH=\"94%\">\n"
"<TR>\n"
"<TD WIDTH=\"85%\">Please paste in a DNA sequence to see where it is located in the ");
printf("%s ", dateForDb(db));
printf("%s", "UCSC assembly\n"
"of the human genome.</TD>\n"
"<TD WIDTH=\"15%\">\n"
"<CENTER>\n"
"<P><INPUT TYPE=SUBMIT NAME=Submit VALUE=Submit>\n"
"</CENTER>\n"
"</TD>\n"
"</TR>\n"
"</TABLE>\n"
"<TEXTAREA NAME=userSeq ROWS=14 COLS=72></TEXTAREA>\n");


cgiContinueHiddenVar("tx");
if (db != NULL)
    {
    if (sameString(db, "hg5"))
        {
	port = "17777";
	nib = "/projects/cc/hg/oo.23/nib";
	host = "kks00.cse.ucsc.edu";
	}
    else if (sameString(db, "hg6"))
        {
	if (cgiVarExists("tx"))
	    {
	    port = "17778";
	    nib = "/projects/hg2/gs.6/oo.27/nib";
	    host = "cc.cse.ucsc.edu";
	    }
	else
	    {
	    port = "17779";
	    nib = "/projects/hg2/gs.6/oo.27/nib";
	    host = "kks00.cse.ucsc.edu";
	    }
	}
   else 
	{
        errAbort("Unknown database %s", db);
	}
    cgiMakeHiddenVar("port", port);
    cgiMakeHiddenVar("host", host);
    cgiMakeHiddenVar("nib", nib);
    cgiMakeHiddenVar("db", db);
    }
else
    {
    db = database;
    cgiContinueHiddenVar("port");
    cgiContinueHiddenVar("host");
    cgiContinueHiddenVar("nib");
    cgiContinueHiddenVar("db");
    }
printf("%s", 
"<P>Only the first 20,000 bases of a sequence will be used.  BLAT is designed to\n"
"quickly find sequences of 95% and greater similarity of length 40 bases or\n"
"more.  It may miss more divergent or shorter sequence alignments.  It will find\n"
"perfect sequence matches of 36 bases, and sometimes find them down to 24 bases.</P>\n"
"<P>BLAT is not BLAST.  BLAT works by keeping an index of the entire genome\n"
"in memory.  The index consists of all non-overlapping 12-mers except for\n"
"those heavily involved in repeats.  The index takes up a bit less than\n"
"a gigabyte of RAM.  The genome itself is not kept in memory, allowing\n"
"BLAT to deliver high performance on a reasonably priced Linux box.\n"
"The index is used to find areas of probable homology, which are then\n"
"loaded into memory for a detailed alignment.</P>\n"
"<P>BLAT was written by <A HREF=\"mailto:jim_kent@pacbell.net\">Jim Kent</A>.\n"
"Like most of Jim's software use on this web server is free to all.\n"
"Sources and executables to run on your server are available free\n"
"for academic, personal, and non-profit purposes.  Non-exclusive commercial\n"
"licenses are also available.  Contact Jim for details.</P>\n"
"\n"
"</FORM>\n");
}

void doMiddle()
{
char *userSeq = cgiOptionalString("userSeq");

if (userSeq == NULL)
    askForSeq();
else
    blatSeq(userSeq);
}


int main(int argc, char *argv[])
/* Process command line. */
{
dnaUtilOpen();
htmlSetBackground("../images/floret.jpg");
htmShell("BLAT Search", doMiddle, NULL);
return 0;
}
