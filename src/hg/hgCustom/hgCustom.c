/* hgCustom - Custom track management CGI. */
#include "common.h"
#include "obscure.h"
#include "linefile.h"
#include "hash.h"
#include "cart.h"
#include "cheapcgi.h"
#include "web.h"
#include "htmshell.h"
#include "hdb.h"
#include "hui.h"
#include "hCommon.h"
#include "customTrack.h"
#include "customFactory.h"
#include "portable.h"
#include "errCatch.h"

static char const rcsid[] = "$Id: hgCustom.c,v 1.25 2006/08/05 07:33:41 kate Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgCustom - Custom track management CGI\n"
  "usage:\n"
  "   hgCustom <CGI settings>\n"
  );
}

#define TEXT_ENTRY_ROWS 7
#define TEXT_ENTRY_COLS 73

#define SECTION_MANAGE_MSG      "Manage Custom Tracks"
#define SECTION_ADD_MSG         "Add Custom Tracks"

/* CGI variables */
#define hgCtDataText     "hgCt_dataText"
#define hgCtDataFile     "hgCt_dataFile"
#define hgCtDataFileName  hgCtDataFile "__filename"
#define hgCtDocText      "hgCt_docText"
#define hgCtDocFile      "hgCt_docFile"
#define hgCtDocTrackName "hgCt_docTrackName"
#define hgCtDoDelete     "hgCt_do_delete"
#define hgCtDeletePrefix "hgCt_del"

/* Global variables */
struct cart *cart;
struct hash *oldCart = NULL;
char *excludeVars[] = {"Submit", "submit", "SubmitFile", 
                        hgCtDoDelete, NULL};
char *database;
char *organism;
struct customTrack *ctList = NULL;
struct hash *ctHash;
struct slName *browserLines = NULL;
char *ctFileName;


void makeInitButton(char *label, char *text, char *field)
/* UI button that initializes a text field */
{
char javascript[1024];
char *form = "mainForm";

safef(javascript, sizeof(javascript), 
    //"document.%s.%s.value = '%s'; document.%s.submit();\"", 
    "document.%s.%s.value = '%s';\"", form, field, text);
cgiMakeOnClickButton(javascript, label);
}

void addCustom(char *err, char *warn)
/* display UI for adding custom tracks by URL or pasting data */
{
#ifdef NEW
puts("Display your own custom annotation tracks in the browser"
     " using the procedure described in the custom tracks"
"<A TARGET=_BLANK HREF=\"/goldenPath/help/customTrack.html\"> user's guide </A>.");
puts("For information on upload procedures and supported formats, see "
     "the \"Loading Custom Annotation Tracks\" section, below.");
#endif

puts("Display custom annotation tracks in the browser"
     " using the procedure described"
"<A TARGET=_BLANK HREF=\"/goldenPath/help/customTrack.html\"> here </A>.");

cgiParagraph("&nbsp;");
cgiSimpleTableStart();

/* first row - label entry for file upload */
cgiSimpleTableRowStart();
cgiTableField("Paste URLs or data:");
#ifndef NEW
puts("<TD ALIGN='RIGHT'>");
#else
puts("<TD COLSPAN=2 ALIGN='RIGHT'>");
#endif
puts("Or upload: ");
cgiMakeFileEntry(hgCtDataFile);
cgiTableFieldEnd();
cgiTableRowEnd();

/* second row - text entry box for  data, and clear button */
cgiSimpleTableRowStart();
puts("<TD COLSPAN=2>");
cgiMakeTextArea(hgCtDataText, cartCgiUsualString(cart, hgCtDataText, ""), 
        TEXT_ENTRY_ROWS, TEXT_ENTRY_COLS);
cgiTableFieldEnd();

cgiSimpleTableFieldStart();
cgiSimpleTableStart();

cgiSimpleTableRowStart();
makeInitButton("&nbsp; Clear &nbsp;", "", hgCtDataText);
cgiTableRowEnd();

cgiSimpleTableRowStart();
makeInitButton("Header", 
        "track name=\\'My Track\\' description=\\'My Data Track\\'", 
                        hgCtDataText);
cgiTableRowEnd();

cgiTableEnd();
cgiTableFieldEnd();
cgiTableRowEnd();

/* third row - label for description text entry */
cgiSimpleTableRowStart();
cgiTableField("Optional HTML doc: ");
puts("<TD ALIGN='RIGHT'>");
puts("Or upload: ");
cgiMakeFileEntry(hgCtDocFile);
cgiTableFieldEnd();
cgiTableRowEnd();

/* fourth row - text entry for description, and clear button(s) */
cgiSimpleTableRowStart();
puts("<TD COLSPAN=2>");
cgiMakeTextArea(hgCtDocText, cartCgiUsualString(cart, hgCtDocText, ""), 
        TEXT_ENTRY_ROWS, TEXT_ENTRY_COLS);
cgiTableFieldEnd();

cgiSimpleTableFieldStart();
cgiSimpleTableStart();

cgiSimpleTableRowStart();
makeInitButton("&nbsp; Clear &nbsp;", "", hgCtDocText);
cgiTableRowEnd();

cgiSimpleTableRowStart();
makeInitButton("Header", 
        "<!-- UCSC_GB_TRACK NAME=\\'My Track\\' -->", hgCtDocText);
cgiTableRowEnd();

cgiTableEnd();
cgiTableFieldEnd();
cgiTableRowEnd();

/* fifth row - submit button */
cgiSimpleTableRowStart();
printf("<TD><A TARGET=_BLANK HREF='../goldenPath/help/ct_description.txt'>HTML doc template</A></TD>");
puts("<TD ALIGN='RIGHT'>");
cgiMakeSubmitButton();
cgiTableRowEnd();
cgiTableEnd();

if (err)
    printf("<BR><FONT COLOR='RED'><B>%s</B></FONT>", err);
if (warn)
    printf("<BR><FONT COLOR='GREEN'><B>%s</B></FONT>", warn);
}

void tableHeaderFieldStart()
{
/* print table column header with white text on black background */
printf("<TD ALIGN='CENTER' BGCOLOR=#536ED3>");
}

void tableHeaderField(char *label, char *description)
{
/* print table column header with white text on black background */
puts("<TD ALIGN='CENTER' BGCOLOR=#536ED3 ");
if (description)
    printf("TITLE='%s'", description);
printf("><B>%s</B></TD> ", wrapWhiteFont(label));
}

void manageCustom()
/* list custom tracks and display checkboxes so user can select for delete */
{
struct customTrack *ct;
char buf[64];
char *pos = NULL;

hTableStart();
cgiSimpleTableRowStart();
tableHeaderField("Name", NULL);
tableHeaderField("Description", NULL);
tableHeaderField("Type", NULL);
tableHeaderField("Doc", "HTML track description");
tableHeaderField("Items", NULL);
tableHeaderField("Pos"," Default track position or first item");
tableHeaderFieldStart();
cgiMakeButton(hgCtDoDelete, "Del");
cgiTableFieldEnd();
cgiTableRowEnd();
for (ct = ctList; ct != NULL; ct = ct->next)
    {
    printf("<TR><TD>%s</TD><TD>%s</TD><TD>",  
                ct->tdb->shortLabel, ct->tdb->longLabel);
    printf("%s", ct->tdb->type ? ct->tdb->type : "&nbsp;");
    printf("</TD><TD ALIGN='CENTER'>%s", ct->tdb->html ? "X" : "&nbsp");
    if (ct->bedList)
        {
        printf("</TD><TD ALIGN='CENTER'>%d", slCount(ct->bedList));
        }
    else
        puts("</TD><TD>&nbsp;</TD><TD>&nbsp;");
    pos = hashFindVal(ct->tdb->settingsHash, "initialPos");
    if (!pos)
        {
        if (ct->bedList)
            {
            safef(buf, sizeof(buf), "%s:%d-%d", ct->bedList->chrom,
                    ct->bedList->chromStart, ct->bedList->chromEnd);
            pos = buf;
            }
        }
    if (pos)
        {
        char *chrom = cloneString(pos);
        chopSuffixAt(chrom, ':');
        printf("</TD><TD><A HREF='%s?%s&position=%s'>%s:</A>", 
                hgTracksName(), cartSidUrlString(cart), pos, chrom);
        }
    else
        puts("</TD><TD>&nbsp;");
    puts("</TD><TD ALIGN=CENTER>");
    safef(buf, sizeof(buf), "%s_%s", hgCtDeletePrefix, 
            ct->tdb->tableName);
    cgiMakeCheckBox(buf, FALSE);
    puts("</TD></TR>");
    }
hTableEnd();

/* close down the section */
/* webEndSection() with less vertical whitespace */
#ifdef NEW
puts(
    "" "\n"
    "	</TD><TD WIDTH=15></TD></TR></TABLE>" "\n"
    "	</TD></TR></TABLE>" "\n"
    "	</TD></TR></TABLE>" "\n"
    "	" );
#endif
}

void helpCustom()
/* display documentation */
{
webNewSection("Loading Custom Tracks");
webIncludeFile("/goldenPath/help/loadingCustomTracks.html");
webEndSection();
}

void doBrowserLines(struct slName *browserLines, char **retInitialPos)
/*  parse variables from browser lines into the cart.
    Return browser initial position, if specified */
{
struct slName *bl;
for (bl = browserLines; bl != NULL; bl = bl->next)
    {
    char *words[96];
    int wordCount;

    wordCount = chopLine(bl->name, words);
    if (wordCount > 1)
        {
	char *command = words[1];
	if (sameString(command, "hide") 
            || sameString(command, "dense") 
            || sameString(command, "pack") 
            || sameString(command, "squish") 
            || sameString(command, "full"))
	    {
	    if (wordCount > 2)
	        {
		int i;
		for (i=2; i<wordCount; ++i)
		    {
		    char *s = words[i];
		    if (sameWord(s, "all"))
                        {
                        if (sameString(command, "hide"))
                            cartSetBoolean(cart, "hgt.hideAllNotCt", TRUE);
                        else
                            cartSetString(cart, "hgt.visAll", command);
                        }
                    else
                        cartSetString(cart, s, command);
		    }
		}
	    }
	else if (sameString(command, "position"))
	    {
	    if (wordCount < 3)
	        errAbort("Expecting 3 words in browser position line");
	    if (!hgIsChromRange(words[2])) 
	        errAbort("browser position needs to be in chrN:123-456 format");
            cartSetString(cart, "position", words[2]);
            if (retInitialPos)
                *retInitialPos = cloneString(words[2]);
	    }
	}
    }
}

struct customTrack *parseTracks(char *var, char **err, char **warn)
/* get tracks from CGI/cart variable and add to custom track list */
{
struct customTrack *addCts = NULL;
struct customTrack *ct, *oldCt, *next;
struct errCatch *errCatch = errCatchNew();
struct dyString *ds = dyStringNew(80);
char *initialPos = NULL;

*err = NULL;
*warn = NULL;
if (errCatchStart(errCatch))
    {
    addCts = customFactoryParse(cartString(cart, var), FALSE, &browserLines);
    doBrowserLines(browserLines, &initialPos);
    for (ct = addCts; ct != NULL; ct = next)
        {
        if ((oldCt = hashFindVal(ctHash, ct->tdb->tableName)) != NULL)
            {
            dyStringPrintf(ds, "Replacing track: %s", ct->tdb->tableName);
            *warn = dyStringCannibalize(&ds);
            slRemoveEl(&ctList, oldCt);
            }
        next = ct->next;
        if (initialPos)
            hashAdd(ct->tdb->settingsHash, "initialPos", initialPos);
        slAddTail(&ctList, ct); 
        }
    cartRemovePrefix(cart, var);
    }
else {}
errCatchEnd(errCatch);
if (errCatch->gotError)
    *err = cloneString(errCatch->message->string);
errCatchFree(&errCatch);
return addCts;
}

struct hash *getCustomTrackDocs(char *text, char *defaultTrackName)
/* get HTML descriptions from text with special comments to name 
 * and delimit tracks */
{
char *line;
char buf[64];
struct hash *docHash = hashNew(6);
char *trackName = defaultTrackName;
struct lineFile *lf = lineFileOnString("custom HTML", TRUE, text);
struct dyString *ds = dyStringNew(1000);

if (!text)
    return NULL;
while (lineFileNextReal(lf, &line))
    {
    if (sscanf(line, "<!-- UCSC_GB_TRACK NAME=%[^-] -->", buf) == 1)
        {
        if (strlen(ds->string))
            {
            hashAdd(docHash, trackName, dyStringCannibalize(&ds));
            }
        /* remove quotes and surrounding whitespace from track identifier*/
        trackName = skipLeadingSpaces(cloneString(buf));
        eraseTrailingSpaces(trackName);
        stripChar(trackName, '\'');
        stripChar(trackName, '\"');
        ds = dyStringNew(1000);
        continue;
        }
    dyStringAppend(ds, line);
    }
if (strlen(ds->string))
    {
    hashAdd(docHash, trackName, dyStringCannibalize(&ds));
    }
return docHash;
}

void startCustomForm()
{
/* create form for adding new custom tracks */
puts("<FORM ACTION=\"/cgi-bin/hgCustom\" METHOD=\"POST\" "
               " ENCTYPE=\"multipart/form-data\" NAME=\"mainForm\">\n");
}

void endCustomForm()
/* end form for adding new custom tracks */
{
puts("</FORM>\n");
}

void doMiddle(struct cart *theCart)
/* create web page */
{
struct tempName tn;
struct customTrack *ct;
struct customTrack *addCts = NULL;
char *firstSectionMsg;
char *err = NULL, *warn = NULL;

cart = theCart;
/* needed ? */
getDbAndGenome(cart, &database, &organism);
saveDbAndGenome(cart, database, organism);
hSetDb(database);
cartSaveSession(cart);

/* get existing custom tracks from cart */
ctList = customTracksParseCart(cart, &browserLines, &ctFileName);
doBrowserLines(browserLines, NULL);
ctHash = hashNew(5);
for (ct = ctList; ct != NULL; ct = ct->next)
    {
    hashAdd(ctHash, ct->tdb->tableName, ct);
    }

/* process submit buttons */
if (cartNonemptyString(cart, hgCtDataFileName))
    {
    if (cartNonemptyString(cart, hgCtDataFile))
        addCts = parseTracks(hgCtDataFile, &err, &warn);
    else
        {
        struct dyString *ds = dyStringNew(80);
        dyStringPrintf(ds, "Error reading file:  %s", 
                        cartString(cart, hgCtDataFileName));
        err = dyStringCannibalize(&ds);
        }
    }
else if (cartVarExists(cart, hgCtDoDelete))
    {
    /* delete tracks */
    for (ct = ctList; ct != NULL; ct = ct->next)
        {
        char var[64];
        safef(var, sizeof var, "%s_%s", hgCtDeletePrefix, ct->tdb->tableName);
        if (cartBoolean(cart, var))
            slRemoveEl(&ctList, ct);
        }
    }
else if (cartNonemptyString(cart, hgCtDataText))
    {
    addCts = parseTracks(hgCtDataText, &err, &warn);
    }

if (ctList == NULL)
    firstSectionMsg = SECTION_ADD_MSG;
else
    {
    firstSectionMsg = SECTION_MANAGE_MSG;

    /* get HTML docs */
    char *html = NULL;
    struct hash *docHash = NULL;
    char *defaultTrackName;
    if (cartNonemptyString(cart, hgCtDocText))
        {
        html = cartString(cart, hgCtDocText);
        }
    else if (cartNonemptyString(cart, hgCtDocFile))
        {
        html = cartString(cart, hgCtDocFile);
        }
    defaultTrackName = (addCts != NULL ?
                        addCts->tdb->shortLabel: ctList->tdb->shortLabel);
    docHash = getCustomTrackDocs(html, defaultTrackName);
    if (docHash)
        {
        for (ct = ctList; ct != NULL; ct = ct->next)
            {
            if ((html = hashFindVal(docHash, ct->tdb->shortLabel)) != NULL)
                {
                ct->tdb->html = cloneString(html);
                }
            }
        }
    }

/* display header and first section header */
cartWebStart(cart, firstSectionMsg);

/* create form for input */
startCustomForm();

if (ctList != NULL)
    {
    /* display list of custom tracks, with checkboxes to select for delete */
    manageCustom();

    /* create custom track file in trash dir, if needed */
    if (ctFileName == NULL)
        {
	customTrackTrashFile(&tn, ".bed");
        ctFileName = tn.forCgi;
        cartSetString(cart, "ct", ctFileName);
        }

    /* save custom tracks to file */
    customTrackSave(ctList, ctFileName);
    cartSetString(cart, "hgta_group", "user");

    cartRemovePrefix(cart, "hgCt_");
    webNewSection("Add Custom Tracks");
    }
else
    {
    /* clean up cart.  File cleanup handled by trash cleaner */
    cartRemove(cart, "ct");
    }

/* display form to add custom tracks -- either URL-based or file-based */
addCustom(err, warn);

endCustomForm();

helpCustom();
cartWebEnd(cart);
}

int main(int argc, char *argv[])
/* Process command line. */
{
htmlPushEarlyHandlers();
oldCart = hashNew(8);
cgiSpoof(&argc, argv);
cartEmptyShell(doMiddle, hUserCookie(), excludeVars, oldCart);
return 0;
}
