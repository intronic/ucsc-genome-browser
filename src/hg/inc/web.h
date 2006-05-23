/* web.c - some functions to output HTML code */

#ifndef WEB_H
#define WEB_H

#include "cart.h"
#include "dbDb.h"

void webSetStyle(char *style);
/* set a style to add to the header */
  
void webStartText();
/* output the head for a text page */

void webStart(struct cart *theCart, char* format,...);
/* output a CGI and HTML header with the given title in printf format */

void webStartHeader(struct cart *theCart, char *header, char* format,...);
/* output a CGI and HTML header with the given title in printf format */

void webStartWrapper(struct cart *theCart, char *format, va_list args, boolean withHttpHeader,
	boolean withLogo);
/* output a CGI and HTML header with the given title in printf format */

void webStartWrapperDetailed(struct cart *theCart, char *headerText,
	char *format, va_list args, boolean withHttpHeader,
	boolean withLogo, boolean skipSectionHeader, boolean withHtmlHeader);
/* output a CGI and HTML header with the given title in printf format with
 * quite a few options. */

void webNewSection(char* format, ...);
/* create a new section on the web page */

void webEnd();
/* output the footer of the HTML page */

void webEndSection();
/* Close down a section */

void webEndSectionTables();
/* Finish with section tables (but don't do /BODY /HTML lik
 * webEnd does. */

void webVaWarn(char *format, va_list args);
/* Warning handler that closes off web page. */

void webAbort(char* title, char* format, ...);
/* an abort function that outputs a error page */

void printCladeListHtml(char *genome, char *onChangeText);
/* Make an HTML select input listing the clades. */

void printGenomeListHtml(char *db, char *onChangeText);
/* Prints to stdout the HTML to render a dropdown list containing 
 * a list of the possible genomes to choose from.  
 * param db - The database whose genome will be selected by default. 
 * If NULL, no default selection.  
 * param onChangeText - Optional (can be NULL) text to pass in any 
 * onChange javascript.
 */

void printSomeGenomeListHtmlNamed(char *customOrgCgiName, char *db, struct dbDb *dbList, char *OnChangeText);

void printSomeGenomeListHtml(char *db, struct dbDb *dbList, char *onChangeText);
/* Prints to stdout the HTML to render a dropdown list 
 * containing a list of the possible genomes to choose from.
 * param db - a database whose genome will be the default genome.
 *                       If NULL, no default selection.  
 * param onChangeText - Optional (can be NULL) text to pass in 
 *                              any onChange javascript. */

void printGenomeListForCladeHtml(char *db, char *onChangeText);
/* Prints to stdout the HTML to render a dropdown list containing 
 * a list of the possible genomes from db's clade to choose from.  
 * db's genome is the default for the select.
 */

void webPushErrHandlers();
/* Push warn and abort handler for errAbort(). */

void webPushErrHandlersCart(struct cart *cart);
/* Push warn and abort handler for errAbort(); save cart for use in handlers. */

void webPopErrHandlers();
/* Pop warn and abort handler for errAbort(). */

void printAssemblyListHtml(char *curDb, char *onChangeText);
/*
Prints to stdout the HTML to render a dropdown list containing a list of the possible
assemblies to choose from.

param curDb - The assembly (the database name) to choose as selected. 
If NULL, no default selection.
 */

void printAssemblyListHtmlExtra(char *curDb, char *javascript);
/*
Prints to stdout the HTML to render a dropdown list containing a list of the possible
assemblies to choose from.

param curDb - The assembly (the database name) to choose as selected. 
If NULL, no default selection.
param javascript - The javascript text for the select box
 */

void printSomeAssemblyListHtml(char *db, struct dbDb *dbList, char *javascript);
/* Find all assemblies from the list that are active, and print
 * HTML to render dropdown list 
 * param db - default assembly.  If NULL, no default selection */

void printSomeAssemblyListHtmlNamed(char *name, char *db, struct dbDb *dbList, char *onChangeText);
/* Find all assemblies from the list that are active, and print
 * HTML to render dropdown list 
 * param db - default assembly.  If NULL, no default selection */

void printAllAssemblyListHtmlParm(char *db, struct dbDb *dbList, 
                            char *dbCgi, bool allowInactive, char *javascript);
/* Prints to stdout the HTML to render a dropdown list containing the list 
of assemblies for the current genome to choose from.  By default,
 this includes only active assemblies with a database (with the
 exception of the default assembly, which will be included even
 if it isn't active).

param db - The default assembly (the database name) to choose as selected. 
                If NULL, no default selection.
param allowInactive - if set, print all assemblies for this genome,
                        even if they're inactive or have no database
*/

void printSomeAssemblyListHtmlParm(char *db, struct dbDb *dbList, 
                                        char *dbCgi, char *javascript);
/* Find all the assemblies from the list that are active.
Prints to stdout the HTML to render a dropdown list containing the list 
of the possible assemblies to choose from.

param db - The default assembly (the database name) to choose as selected. 
                If NULL, no default selection.
 */

void printOrgAssemblyListAxtInfo(char *dbCgi, char *javascript);
/* Find all the organisms/assemblies that are referenced in axtInfo, 
 * and print the dropdown list. */

void printAlignmentListHtml(char *db, char *alCgiName, char *selected);
/* Find all the alignments (from axtInfo) that pertain to the selected
 * genome.  Prints to stdout the HTML to render a dropdown list
 * containing a list of the possible alignments to choose from.
 */

void printBlatAssemblyListHtml(char *curDb);
/*
Prints to stdout the HTML to render a dropdown list containing a list of the possible
assemblies to choose from that have blat servers available.

param curDb - The assembly (the database name) to choose as selected. 
If NULL, no default selection.
 */

void getDbGenomeClade(struct cart *cart, char **retDb, char **retGenome,
	char **retClade);
/*
  The order of preference here is as follows:
If we got a request that explicitly names the db, that takes
highest priority, and we synch the organism to that db.
If we get a cgi request for a specific organism then we use that
organism to choose the DB.  If just clade, go from there.

In the cart only, we use the same order of preference.
If someone requests an organism we try to give them the same db as
was in their cart, unless the organism doesn't match.
*/

void getDbAndGenome(struct cart *cart, char **retDb, char **retGenome);
/* Like getDbGenomeClade above, but just get db and genome. */

void saveDbAndGenome(struct cart *cart, char *db, char *genome);
/* Save db and genome (as org) in cart. */

#endif /* WEB_H */
