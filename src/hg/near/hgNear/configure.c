/* configure - Do configuration page. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "obscure.h"
#include "jksql.h"
#include "htmshell.h"
#include "hdb.h"
#include "cart.h"
#include "web.h"
#include "hgNear.h"

static char const rcsid[] = "$Id: configure.c,v 1.34 2003/09/26 05:54:17 kent Exp $";

static char *onOffString(boolean on)
/* Return "on" or "off". */
{
return on ? "on" : "off";
}

static void configTable(struct column *colList, struct sqlConnection *conn)
/* Write out configuration table */
{
struct column *col;
char varName[128];
boolean isVis;

hPrintf("<TABLE BORDER=1>\n");

/* Write out first row - labels. */
hPrintf("<TR BGCOLOR=\"#EFEFFF\">");
hPrintf("<TH ALIGN=left>Name</TH>");
hPrintf("<TH ALIGN=left>On</TH>");
hPrintf("<TH ALIGN=left>Position</TH>");
hPrintf("<TH ALIGN=left>Description</TH>");
hPrintf("<TH ALIGN=left>Configuration</TH>");

/* Print out configuration controls. */
hPrintf("</TR>");

/* Write out other rows. */
for (col = colList; col != NULL; col = col->next)
    {
    hPrintf("<TR>");

    /* Do small label. */
    hPrintf("<TD>%s</TD>", col->shortLabel);

    /* Do on/off dropdown. */
    hPrintf("<TD>");
    safef(varName, sizeof(varName), "%s%s.vis", colConfigPrefix, col->name);
    isVis = cartUsualBoolean(cart, varName, col->on);
    cgiMakeCheckBox(varName, isVis);
    hPrintf("</TD>");

    /* Do left/right button */
    hPrintf("<TD ALIGN=CENTER>");
    safef(varName, sizeof(varName), "near.up.%s", col->name);
    if (col != colList)
	{
	// cgiMakeButton(varName, " up ");
	hPrintf("<INPUT NAME=\"%s\" TYPE=\"IMAGE\" VALUE=\"up\" ", varName);
	hPrintf("SRC=\"../images/up.gif\">");
	}
    safef(varName, sizeof(varName), "near.down.%s", col->name);
    if (col->next != NULL)
	{
	// cgiMakeButton(varName, "down");
	hPrintf("<INPUT NAME=\"%s\" TYPE=\"IMAGE\" VALUE=\"down\" ", varName);
	hPrintf("SRC=\"../images/down.gif\">");
	}
    hPrintf("</TD>");

    /* Do long label. */
    hPrintf("<TD>%s</TD>", col->longLabel);

    /* Do configuration controls. */
    if (col->configControls != NULL)
         col->configControls(col);
    else
         hPrintf("<TD>n/a</TD>");

    hPrintf("</TR>\n");
    }
hPrintf("</TABLE>\n");
}

static void savePriorities(struct column *colList)
/* Save priority list. */
{
struct dyString *dy = dyStringNew(4*1024);
struct column *col;

for (col = colList; col != NULL; col = col->next)
    dyStringPrintf(dy, "%s %f ", col->name, col->priority);
cartSetString(cart, colOrderVar, dy->string);
dyStringFree(&dy);
}

static void bumpColList(char *bumpHow, struct column **pColList)
/* Bump a column one way or another */
{
char *dupe = cloneString(bumpHow);
char *words[4], *upDown, *colName;

if (chopString(dupe, ".", words, ArraySize(words)) < 3)
    {
    warn("Strange bumpHow value %s in bumpColList", bumpHow);
    cartRemove(cart, bumpHow);
    noWarnAbort();
    }
upDown = words[1];
colName = words[2];

if (sameString(upDown, "up"))
    {
    struct column *col, *prev = NULL;
    for (col = *pColList; col != NULL; col = col->next)
        {
	if (sameString(col->name, colName))
	    {
	    if (prev != NULL)
	        {
		float swap = prev->priority;
		prev->priority = col->priority;
		col->priority = swap;
		}
	    break;
	    }
	prev = col;
	}
    }
else
    {
    struct column *col, *next = NULL;
    for (col = *pColList; col != NULL; col = col->next)
        {
	if (sameString(col->name, colName))
	    {
	    if ((next = col->next) != NULL)
	        {
		float swap = next->priority;
		next->priority = col->priority;
		col->priority = swap;
		}
	    break;
	    }
	}
    }
freez(&dupe);
slSort(pColList, columnCmpPriority);
savePriorities(*pColList);
}

static char *colorSchemeVals[] = {
/* Menu option for color scheme. */
   "red high/green low",
   "blue high/green low",
};

static void colorSchemeDropDown()
/* Make color drop-down. */
{
char *checked = cartUsualString(cart, expRatioColorVarName, colorSchemeVals[0]);
cgiMakeDropList(expRatioColorVarName, 
	colorSchemeVals, ArraySize(colorSchemeVals), checked);
}

boolean expRatioUseBlue()
/* Return TRUE if should use blue instead of red
 * in the expression ratios. */
{
char *val = cartUsualString(cart, expRatioColorVarName, colorSchemeVals[0]);
return !sameString(val, colorSchemeVals[0]);
}

boolean showOnlyCanonical()
/* Return TRUE if we only show canonical splicing variants. */
{
return !cartUsualBoolean(cart, showAllSpliceVarName, FALSE);
}

struct userSettings *colUserSettings()
/* Return userSettings object for columns. */
{
struct userSettings *us = userSettingsNew(cart, 
	"Current Column Configuration",
	savedCurrentConfName, colSaveSettingsPrefix);
userSettingsCapturePrefix(us, colConfigPrefix);
userSettingsCaptureVar(us, colOrderVar);
return us;
}

void doConfigure(struct sqlConnection *conn, struct column *colList, char *bumpVar)
/* Generate configuration page. */
{
struct userSettings *us = colUserSettings();
if (bumpVar)
    bumpColList(bumpVar, &colList);
makeTitle("Configure Gene Family Browser", "hgNearConfigure.html");
hPrintf("<FORM ACTION=\"../cgi-bin/hgNear\" METHOD=POST>\n");
cartSaveSession(cart);

hPrintf("<TABLE BORDER=0 CELLSPACING=1 CELLPADDING=1>\n");
hPrintf("<TR><TD ALIGN=LEFT>");
cgiMakeButton("submit", "Submit");
hPrintf("</TD><TD> ");
hPrintf("</TD><TD> ");
hPrintf("</TD><TD>");
hPrintf("Columns:");
hPrintf("</TD><TD> ");
cgiMakeButton(hideAllConfName, "Hide All");
hPrintf("</TD><TD>");
cgiMakeButton(showAllConfName, "Show All");
hPrintf("</TD><TD>");
cgiMakeButton(defaultConfName, "Default");
hPrintf("</TD><TD>");
hPrintf("Settings:");
hPrintf("</TD><TD>");
cgiMakeButton(saveCurrentConfName, "Save");
hPrintf("</TD><TD>");
cgiMakeOptionalButton(useSavedConfName, "Load", !userSettingsAnySaved(us));
hPrintf("</TD></TR></TABLE>");

// hPrintf("<HR>");
// hPrintf("<H2>Column Configuration</H2>\n");
hPrintf("<TABLE BORDER=0 CELLSPACING=1 CELLPADDING=1>\n");
hPrintf("<TR><TD ALIGN=LEFT>");
hPrintf("Expression ratio colors: ");
colorSchemeDropDown();
hPrintf("</TD><TD>");
hPrintf("Show all splicing variants: ");
cgiMakeCheckBox(showAllSpliceVarName, 
	cartUsualBoolean(cart, showAllSpliceVarName, FALSE));
hPrintf(" ");
hPrintf("</TD></TR></TABLE>");
configTable(colList, conn);
hPrintf("</FORM>");
}

static void restoreDefaultOrder(struct column **pColList)
/* Restore order of columns to default using priority settings. */
{
struct column *col;
for (col = *pColList; col != NULL; col = col->next)
    {
    char *priority = columnSetting(col, "priority", NULL);
    if (priority != NULL)
        col->priority = atof(priority);
    }
slSort(pColList, columnCmpPriority);
}

void doDefaultConfigure(struct sqlConnection *conn, struct column *colList)
/* Do configuration starting with defaults. */
{
struct column *col;
char varPattern[64];
for (col=colList; col != NULL; col = col->next)
    col->on = col->defaultOn;
cartRemovePrefix(cart, colConfigPrefix);
cartRemove(cart, colOrderVar);
restoreDefaultOrder(&colList);
doConfigure(conn, colList, NULL);
}

static void  configAllVis(struct sqlConnection *conn, struct column *colList,
	boolean how)
/* Respond to hide all button in configuration page. */
{
char varName[64];
struct column *col;
for (col = colList; col != NULL; col = col->next)
    {
    safef(varName, sizeof(varName), "%s%s.vis", colConfigPrefix, col->name);
    cartSetBoolean(cart, varName, how);
    }
doConfigure(conn, colList, NULL);
}

void doConfigHideAll(struct sqlConnection *conn, struct column *colList)
/* Respond to hide all button in configuration page. */
{
configAllVis(conn, colList, 0);
}

void doConfigShowAll(struct sqlConnection *conn, struct column *colList)
/* Respond to hide all button in configuration page. */
{
configAllVis(conn, colList, 1);
}

void doConfigUseSaved(struct sqlConnection *conn, struct column *colList)
/* Respond to Use Saved Settings button in configuration page. */
{
struct userSettings *us = colUserSettings();
userSettingsLoadForm(us);
}


void doNameCurrentColumns()
/* Put up page to save current column configuration. */
{
struct userSettings *us = colUserSettings();
userSettingsSaveForm(us);
}

void doSaveCurrentColumns(struct sqlConnection *conn, struct column *colList)
/* Save the current columns, and then go on. */
{
struct userSettings *us = colUserSettings();
if (userSettingsProcessForm(us))
    {
    refinePriorities(colList);
    slSort(&colList, columnCmpPriority);
    doConfigure(conn, colList, NULL);
    }
}
