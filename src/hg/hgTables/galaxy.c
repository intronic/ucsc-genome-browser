/* galaxy - stuff related to galaxy simple queries and db info. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "cheapcgi.h"
#include "trackDb.h"
#include "customTrack.h"
#include "hdb.h"
#include "web.h"
#include "portable.h"
#include "hui.h"
#include "featureBits.h"
#include "hgTables.h"
#include "obscure.h"
#include "cart.h"
#include "grp.h"
#include "net.h"
#include "htmlPage.h"
#include "wiggle.h"
#include "trashDir.h"

static char const rcsid[] = "$Id: galaxy.c,v 1.8 2007/02/28 20:39:08 giardine Exp $";

char *getGalaxyUrl()
/* returns the url for the galaxy cgi, based on script name */
{
char *url = NULL;
/* use parameter if available */
if (cartVarExists(cart, "GALAXY_URL"))
    url = cartString(cart, "GALAXY_URL");
else
    url = cloneString("http://main.g2.bx.psu.edu/");
return url;
}

void galaxyHandler (char *format, va_list args)
/* error Handler that passes error on to Galaxy */
{
char msg[512];
sprintf(msg, format, args);
noWarnAbort();
}

void sendParamsToGalaxy(char *doParam, char *paramVal)
/* intermediate page for formats printed directly from top form */
{
char *shortLabel = curTable;
char hgsid[64];

if (curTrack != NULL)
    shortLabel = curTrack->shortLabel;
htmlOpen("Output %s as %s", "results to Galaxy", shortLabel);
startGalaxyForm();
/* send the hgta_do parameter that won't be in the cart */
cgiMakeHiddenVar(doParam, paramVal);
/* need to send sessionId */
safef(hgsid, sizeof(hgsid), "%u", cartSessionId(cart));
cgiMakeHiddenVar(cartSessionVarName(cart), hgsid);
printGalaxySubmitButtons();
htmlClose();
}

void startGalaxyForm ()
/* start form to send parameters to Galaxy, also send required params */
{
char selfUrl[256];
int hguid = cartUserId(cart);

hPrintf("<FORM ACTION=\"%s\" METHOD=POST>\n", getGalaxyUrl());
/* copy cart parameters into hidden fields to send to Galaxy */
hPrintf("\n"); /* make more readable */
/* Galaxy requires tool_id and URL */
/* set default if no tool_id for Galaxy */
if (!cartVarExists(cart, "tool_id"))
    cgiMakeHiddenVar("tool_id", "ucsc_table_direct1");
else
    cgiMakeHiddenVar("tool_id", cartString(cart, "tool_id"));
safef(selfUrl, sizeof(selfUrl), "http://%s%s", cgiServerName(), cgiScriptName());
cgiMakeHiddenVar("URL", selfUrl);
hPrintf("\n");
/* forward user parameters */
if (hguid > 0)
    {
    char id[25];
    safef(id, sizeof(id), "%u", hguid);
    cgiMakeHiddenVar("hguid", id);
    hPrintf("\n");
    }
/* send database and organism and table for Galaxy's info */
cgiMakeHiddenVar("db", database);
if (cartVarExists(cart, "org"))
    cgiMakeHiddenVar("org", cartString(cart, "org"));
cgiMakeHiddenVar("hgta_table", curTable);
cgiMakeHiddenVar("hgta_track", cartString(cart, "hgta_track"));
if (cartVarExists(cart, "hgta_regionType"))
    cgiMakeHiddenVar("hgta_regionType", cartString(cart, "hgta_regionType"));
if (cartVarExists(cart, "hgta_outputType"))
    cgiMakeHiddenVar("hgta_outputType", cartString(cart, "hgta_outputType"));
if (cartVarExists(cart, "position"))
    cgiMakeHiddenVar("position", cartString(cart, "position"));
}

void printGalaxySubmitButtons ()
/* print submit button to send query results to Galaxy */
{
cgiMakeButton(hgtaDoGalaxyQuery, "Send query to Galaxy");
hPrintf("</FORM>\n");
hPrintf(" ");
/* new form as action is different */
hPrintf("<FORM ACTION=\"..%s\" METHOD=GET>\n", cgiScriptName());
cgiMakeButton(hgtaDoMainPage, "Cancel");
hPrintf("</FORM>\n");
}

boolean doGalaxy ()
/* has the send to Galaxy checkbox been selected? */
{
return cartUsualBoolean(cart, "sendToGalaxy", FALSE);
}
