/* rtdbWebUpdate - CGI program for remote triggering of MGC RTDB update. */

#include "common.h"
#include "errabort.h"
#include "hCommon.h"
#include "jksql.h"
#include "portable.h"
#include "linefile.h"
#include "dnautil.h"
#include "fa.h"
#include "cheapcgi.h"
#include "htmshell.h"
#include "hdb.h"
#include "hui.h"
#include "cart.h"
#include "web.h"
#include "hash.h"
#include "hgConfig.h"

/* Global Variables */
struct cart *cart;	        /* CGI and other variables */
struct hash *oldCart = NULL;

void makeForm()
/* If the button wasn't pressed already, show it. */
{
cgiParagraph("Pressing the button below will trigger an update to the MGC RTDB database:");
/* HTML form */
puts("<FORM ACTION=\"../cgi-bin/rtdbWebUpdate\" METHOD=\"POST\" "
       " ENCTYPE=\"multipart/form-data\" NAME=\"mainForm\">\n");
cartSaveSession(cart);
cgiMakeButton("RTDBSubmit","Update RTDB");
cartSaveSession(cart);
puts("</FORM>");
}

void runUpdate(char *rtdbCmd)
/* Button pressed, so time to run things. */
{
cgiParagraph("Running RTDB Update script:<BR>");
puts("<PRE>");
fflush(stdout);
fflush(stderr);
system(rtdbCmd);
fflush(stdout);
fflush(stderr);
puts("</PRE>");
}

void doMiddle(struct cart *theCart)
/* Set up globals and make web page */
{
char *rtdbCmd = cfgOption("rtdb.update");
cart = theCart;

cartWebStart(cart, "MGC RTDB Update");
if (!rtdbCmd)
    errAbort("rtdb.update not defined in the hg.conf file. "
	     "Chances are this CGI isn't meant for this machine.");
/* create HMTL form if button wasn't pressed.  Otherwise, run the update */
if (!cgiVarExists("RTDBSubmit"))
    makeForm();
else
    runUpdate(rtdbCmd);
cartWebEnd();
}

/* Null terminated list of CGI Variables we don't want to save
 * permanently. */
char *excludeVars[] = {"Submit", "submit", NULL};

int main(int argc, char *argv[])
/* Process command line. */
{
oldCart = hashNew(8);
cgiSpoof(&argc, argv);
cartEmptyShell(doMiddle, hUserCookie(), excludeVars, oldCart);
return 0;
}
