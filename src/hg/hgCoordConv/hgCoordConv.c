/**
   cgi program to convert coordinates from one draft of the 
   genome to another 
*/
#include "common.h"
#include "errabort.h"
#include "htmshell.h"
#include "jksql.h"
#include "coordConv.h"
#include "hdb.h"
#include "dnautil.h"
#include "dnaseq.h"
#include "fa.h"
#include "web.h"
#include "cheapcgi.h"
#include "psl.h"
#include "dystring.h"
char *defaultOldDb = "Aug. 6, 2001";
char *defaultNewDb = "Dec. 22, 2001";
char *newDb = NULL;
char *oldDb = NULL;
char *chrom = NULL;
int start = -1;
int end = -1;
bool calledSelf = FALSE;
char *browserUrl = "../cgi-bin/hgTracks?";
char *blatUrl = "../cgi-bin/hgBlat?";
bool onWeb = FALSE;
bool fakeWeb = FALSE;
char *position = NULL;
char *newGenome = NULL;
char *origGenome = NULL;
char *defaultPos = "chr22:17045228-17054909";
char *origDb = NULL;
struct dyString *webWarning = NULL;

/* keeps track of the database version names and hg's */
struct namePair 
{
    char *database;
    char *version;
};

/* Information on a blat server. */
struct serverTab
   {
   char *db;		/* Database name. */
   char *genome;	/* Genome name. */
   boolean isTrans;	/* Is tranlated to protein? */
   char *host;		/* Name of machine hosting server. */
   char *port;		/* Port that hosts server. */
   char *nibDir;	/* Directory of sequence files. */
   };

/* old draft versions to choose from */
struct namePair oldVersions[] = { 
				  { "April 1, 2001","hg7" },
				  { "Aug. 6, 2001", "hg8" },
				  { "Dec. 22, 2001", "hg10"},
};

struct serverTab serverTab[] =  {
{"hg7", "April 1, 2001", FALSE, "blat1", "17779", "/projects/hg3/gs.7/oo.29/nib"},
{"hg8", "Aug. 6, 2001", FALSE, "blat3", "17779", "/projects/hg3/gs.8/oo.33/nib"},
{"hg10", "Dec. 22, 2001", FALSE, "blat4", "17779", "/cluster/store1/gs.11/build28/nib"}
};

/** print usage and quit */
void usage()
{
errAbort("hgCoordConv - tries to convert coordinates from one draft to another.\n"
	 "usage:\n"
	 "\thgCoordConv oldDb=<hg5> chrom=<chromosome> start=<chromStart> end=<chromEnd> newDb=<hg6> -fakeWeb\n");
}

/* keep track of error messages for the user */
void appendWarningMsg(char *warning)
{
struct dyString *warn = newDyString(1024);
dyStringPrintf(warn, "%s", warning);
slAddHead(&webWarning, warn);
}

/** print error message about format of position input */
void posAbort() 
{
webAbort("Error", "Expecting position in the form chrN:10000-20000");
}

/** Parse the coordinate information from the user text */
void parsePosition(char *pos, char **chr, int *s, int *e) 
{
/* trying to parse something that looks like chrN:10000-20000 */
char *tmp = NULL;
char *tmp2 = NULL;
tmp = strstr(pos, ":");
if(tmp == NULL) 
    posAbort();
*tmp='\0';
tmp++;
*chr = cloneString(pos);
tmp2 = strstr(tmp, "-");
if(tmp2 == NULL)
    posAbort();
*tmp2 = '\0';
tmp2++;
*s = atoi(tmp);
*e = atoi(tmp2);
}

/** convert something like Dec. 7, 2000 to something like hg6 */
char *findOldDbForGenome(char *version)
{ 
int i;
for(i=0;i<ArraySize(oldVersions); i++)
    {
    if(sameString(version, oldVersions[i].database))
	return cloneString(oldVersions[i].version);
    }
return NULL;
}

/** convert something like Dec. 7, 2000 to something like hg6 */
char *findNewDbForGenome(char *version)
{
int i;
for(i=0;i<ArraySize(serverTab); i++)
    {
    if(sameString(version, serverTab[i].genome))
	return cloneString(serverTab[i].db);
    }
return NULL;
}

/** setup our parameters depending on whether we've been called as a
    cgi script or from the command line */ 
void checkArguments() 
{
origDb = cgiOptionalString("origDb");
origGenome = cgiOptionalString("origGenome");
newGenome = cgiOptionalString("newGenome");
position = cgiOptionalString("position");
newDb = cgiOptionalString("newDb");
oldDb = cgiOptionalString("oldDb");
chrom = cgiOptionalString("chrom");
if( cgiBoolean("fakeWeb") || onWeb) 
    onWeb = TRUE;
start = cgiOptionalInt("start", -1);
end = cgiOptionalInt("end", -1);
calledSelf = cgiBoolean("calledSelf");

/* parse the position string and make sure that it makes sense */

if (position != NULL && position[0] != 0)
    {
    parsePosition(cloneString(position), &chrom, &start, &end);
    }
if (start > end && onWeb)
    { 
    webAbort("Error:", "Start of range is greater than end. %d > %d", start, end);
    }
else if( start > end) 
    {
    errAbort( "Start of range is greater than end. %d > %d", start, end);
    }

/* convert the genomes requested to hgN format */
if(origGenome != NULL && oldDb == NULL)
    oldDb = findOldDbForGenome(origGenome);
if(newGenome != NULL && newDb == NULL)
    newDb = findNewDbForGenome(newGenome);


/* make sure that we've got valid arguments */
if((newDb == NULL || oldDb == NULL || chrom == NULL || start == -1 || end == -1) && (onWeb && calledSelf)) 
    {
    if(onWeb) 
	{
	webAbort("Error:", "Missing some inputs.");
	}
    else 
	{
	usage();
	}
    }
if( oldDb != NULL && sameString(oldDb, newDb) && onWeb)
    {
    struct dyString *warning = newDyString(1024);
    dyStringPrintf(warning, "Did you really want to convert from %s to %s (the same genome)?", origGenome, newGenome);
    appendWarningMsg(warning->string);
    dyStringFree(&warning);
    }
}

/** create a url of the form hgTracks likes */
char *makeBrowserUrl(char *version, char*chrom, int start, int end)
{
char url[256];
sprintf(url, "%sposition=%s:%d-%d&db=%s", browserUrl, chrom, start, end, version);
return cloneString(url);
}

/** output a link to hgBlat a certain sequence */
void outputBlatLink(char *link, char *db, struct dnaSeq *seq) 
{
printf("<a href=\"%stype=DNA&genome=%s&sort=query,score&output=hyperlink&userSeq=%s\">%s</a>",blatUrl, db,seq->dna, link);
}

/** print out any warning messages that we may have
 * for the user */
void printWebWarnings() 
{
struct dyString *warn = NULL;
if(webWarning != NULL)
    {
    printf("<font color=red>\n");
    printf("<h3>Warning:</h3><ul>\n");
    for(warn = webWarning; warn != NULL; warn = warn->next)
	{
	printf("<li>%s</li>\n", warn->string);
	}
    printf("</ul></font>\n");
    }
}

/** output a blat link and the fasta in cut and past form */
void webOutFasta(struct dnaSeq *seq, char *db)
{
printf("<pre>\n");
faWriteNext(stdout, seq->name, seq->dna, seq->size);
printf("</pre>\n");
outputBlatLink("Blat Sequence on new Draft", db, seq);
printf("<br><br>");
}

/** Make sure the user knows to take this with a grain of salt */
void printSucessWarning() {
printf("<p>Please be aware that this is merely our best guess of converting from one draft to another. Make sure to check with local landmarks and use common sense.\n");
}

/** print out the information used to try and convert */
void printTroubleShooting(struct coordConvRep *ccr) 
{
webNewSection("Alignment Details:");
printf("<p>The following sequences from the original draft were aligned to determine the coordinates on the new draft:<br>\n");
webOutFasta(ccr->upSeq, ccr->to->version);
webOutFasta(ccr->midSeq, ccr->to->version);
webOutFasta(ccr->downSeq, ccr->to->version);
printf("<br><br>");
printf("<i><font size=-1>Comments, Questions, Bug Reports: <a href=\"mailto:sugnet@cse.ucsc.edu\">sugnet@cse.ucsc.edu</a></font></i>\n");
}


/** output the result of a successful conversion */
void doGoodReport(struct coordConvRep *ccr) 
{
webStart("Coordinate Conversion for %s %s:%d-%d", ccr->from->date, ccr->from->chrom, ccr->from->chromStart, ccr->from->chromEnd);
printWebWarnings();
printf("<p><b>Success:</b> %s\n", ccr->msg);
if(sameString(ccr->midPsl->strand, "-")) 
    {
    printf(" It appears that the orientation of your coordinate range has been inverted.\n");
    }
printSucessWarning(); 
printf("<ul><li><b>Old Coordinates:</b> %s %s:%d-%d  ", ccr->from->date ,ccr->from->chrom, ccr->from->chromStart, ccr->from->chromEnd);
printf("<a href=\"%s\">[browser]</a></li>\n", 
       makeBrowserUrl(ccr->from->version, ccr->from->chrom, ccr->from->chromStart, ccr->from->chromEnd));
printf("<li><b>New Coordinates:</b> %s %s:%d-%d  ", ccr->to->date ,ccr->to->chrom, ccr->to->chromStart, ccr->to->chromEnd);
printf("<a href=\"%s\">[browser]</a></li></ul>\n", 
       makeBrowserUrl(ccr->to->version, ccr->to->chrom, ccr->to->chromStart, ccr->to->chromEnd));
printTroubleShooting(ccr);
webEnd();
}

/** output the result of a flawed conversion */
void doBadReport(struct coordConvRep *ccr) 
{
webStart("Coordinate Conversion for %s %s:%d-%d", ccr->from->date, ccr->from->chrom, ccr->from->chromStart, ccr->from->chromEnd);
printWebWarnings();
printf("<p><b>Conversion Not Successful:</B> %s\n", ccr->msg);
printf("<p><a href=\"%s\">View old Coordinates in %s browser.</a>\n", 
       makeBrowserUrl(ccr->from->version, ccr->from->chrom, ccr->from->chromStart, ccr->from->chromEnd),
       ccr->from->date);
printTroubleShooting(ccr);
webEnd();
}

/* Return server for given database. */
struct serverTab *findServer(char *db)
{
int i;
struct serverTab *serve;

if (db == NULL)
    errAbort("findServer() - db can't be NULL.");

for (i=0; i<ArraySize(serverTab); ++i)
    {
    serve = &serverTab[i];
    if (sameWord(serve->db, db))
        return serve;
    if (sameWord(serve->genome, db))
        return serve;
    }
errAbort("Can't find a server for %s DNA database ", db);
return NULL;
}

/* tries to convert coordinates and prints report */
void convertCoordinates() 
{
struct serverTab *serve = NULL;
struct coordConvRep *ccr = NULL;
serve = findServer(newDb);
ccr = coordConvConvertPos(chrom, start, end, oldDb, newDb, 
	       	       serve->host, serve->port, serve->nibDir);
if(ccr->good)
    {
    doGoodReport(ccr);
    }
else 
    {
    doBadReport(ccr);
    }
coordConvRepFreeList(&ccr);
}


/* match up our possible databases with the date version i.e. Dec 17, 2000 */
char *chooseDb(char *db1, char *db2, struct namePair *versions, int numVersions)
{
int i;
if(db1 != NULL) 
    {
    for(i=0;i<numVersions; i++)
	{
	if(sameString(versions[i].database, db1) || sameString(versions[i].version, db1))
	    return versions[i].database;
	}
    }
else 
    {
    for(i=0;i<numVersions; i++)
	{
	if(sameString(versions[i].database, db2) || sameString(versions[i].version, db2))
	    return versions[i].database;
	}
    }
return NULL;
}

/** Print out the form for users */
void doForm() 
{
char *origChoices[ArraySize(oldVersions)];
char *newChoices[ArraySize(serverTab)];
char *dbChoice = NULL;
int i = 0;

for(i=0; i< ArraySize(origChoices); i++) 
    {
    origChoices[i] = oldVersions[i].database;
    }
for(i=0; i< ArraySize(newChoices); i++) 
    {
    newChoices[i] = serverTab[i].genome;
    }

webStart("Converting Coordinates Between Drafts");
puts( 
     "<p>This page attempts to convert coordinates from one draft of the human genome\n"
     "to another. The mechanism for doing this is to cut out and align pieces from the\n"
     "old draft and align them to the new draft making sure that\n"
     "they are in the same order and orientation. In general the smaller the sequence the better\n"
     "the chances of successful conversion.\n"
     );
printf("<form action=\"../cgi-bin/hgCoordConv\" method=get>\n");
printf("<br><br>\n");
printf("<table><tr>\n");
printf("<b><td><table><tr><td>Original Draft: </b>\n");

/* choose whether to use the db supplied by cgi or our default */
dbChoice = chooseDb(origDb, defaultOldDb, oldVersions, ArraySize(oldVersions)); 
cgiMakeDropList("origGenome", origChoices, ArraySize(origChoices), dbChoice);
printf("</td></tr></table></td>\n");
printf("  <b><td><table><tr><td>Original Position:  </b>\n");

/* if someone has passed in a position fill it in for them */
if(position == NULL) 
    cgiMakeTextVar("position",defaultPos, 30);
else
    cgiMakeTextVar("position",position, 30);
printf("</td></tr></table></td>\n");
printf("<b><td><table><tr><td>New Draft: </b>\n");
cgiMakeDropList("newGenome", newChoices, ArraySize(newChoices), defaultNewDb);
printf("</td></tr></table></td></tr>\n");
printf("<tr><td colspan=6 align=right><br>\n");
cgiMakeButton("Submit","submit");
printf("</center></td></tr></table>\n");
cgiMakeHiddenVar("calledSelf", "on");
printf("</form>\n");
webEnd();
}

int main(int argc, char *argv[]) 
{
onWeb = cgiIsOnWeb();
cgiSpoof(&argc, argv);
checkArguments();
/* do our thing  */
if(calledSelf)
    htmEmptyShell(convertCoordinates, NULL);
else
    htmEmptyShell(doForm, NULL);
return 0;
}
