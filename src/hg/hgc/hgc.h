/* hgc - Human Genome Click processor - gets called when user clicks
 * on something in human tracks display. This file contains stuff
 * shared with other modules in hgc,  but not in other programs. */

#ifndef CART_H
#include "cart.h"
#endif

#ifndef TRACKDB_H
#include "trackDb.h"
#endif 

#ifndef BED_H
#include "bed.h"
#endif

#ifndef HDB_H
#include "hdb.h"
#endif 

extern struct cart *cart;	/* User's settings. */
extern char *seqName;		/* Name of sequence we're working on. */
extern int winStart, winEnd;    /* Bounds of sequence. */
extern char *database;		/* Name of mySQL database. */
extern struct hash *trackHash;	/* A hash of all tracks - trackDb valued */


void hgcStart(char *title);
/* Print out header of web page with title.  Set
 * error handler to normal html error handler. */

char *hgcPath();
/* Return path of this CGI script. */

char *hgcPathAndSettings();
/* Return path with this CGI script and session state variable. */

void hgcAnchorSomewhere(char *group, char *item, char *other, char *chrom);
/* Generate an anchor that calls click processing program with item 
 * and other parameters. */

void hgcAnchorWindow(char *group, char *item, int thisWinStart, 
        int thisWinEnd, char *other, char *chrom);
/* Generate an anchor that calls click processing program with item
 * and other parameters, INCLUDING the ability to specify left and
 * right window positions different from the current window*/

void hgcAnchor(char *group, char *item, char *other);
/* Generate an anchor that calls click processing program with item 
 * and other parameters. */

void linkToOtherBrowser(char *otherDb, char *chrom, int start, int end);
/* Make anchor tag to open another browser window. */

boolean clipToChrom(int *pStart, int *pEnd);
/* Clip start/end coordinates to fit in chromosome. */

void printCappedSequence(int start, int end, int extra);
/* Print DNA from start to end including extra at either end.
 * Capitalize bits from start to end. */

void printPos(char *chrom, int start, int end, char *strand, boolean featDna,
	      char *item);
/* Print position lines.  'strand' argument may be null. */

void bedPrintPos(struct bed *bed, int bedSize);
/* Print first three fields of a bed type structure in
 * standard format. */

void genericHeader(struct trackDb *tdb, char *item);
/* Put up generic track info. */

void printTrackHtml(struct trackDb *tdb);
/* If there's some html associated with track print it out. */

void abbr(char *s, char *fluff);
/* Cut out fluff from s. */

void printTableHeaderName(char *name, char *clickName, char *url);
/* creates a table to display a name vertically,
 * basically creates a column of letters */

/* ----Routines in other modules in the same directory---- */
void genericMafClick(struct sqlConnection *conn, struct trackDb *tdb, 
	char *item, int start);
/* Display details for MAF tracks. */

void genericAxtClick(struct sqlConnection *conn, struct trackDb *tdb, 
	char *item, int start, char *otherDb);
/* Display details for AXT tracks. */

void genericExpRatio(struct sqlConnection *conn, struct trackDb *tdb, 
	char *item, int start);
/* Display details for expRatio tracks. */

void rosettaDetails(struct trackDb *tdb, char *item);
/* Set up details for rosetta. */

void affyDetails(struct trackDb *tdb, char *item);
/* Set up details for affy. */

void gnfExpRatioDetails(struct trackDb *tdb, char *item);
/* Put up details on some gnf track. */

void affyUclaDetails(struct trackDb *tdb, char *item);
/* Set up details for affyUcla. */

void cghNci60Details(struct trackDb *tdb, char *item);
/* Set up details for cghNci60. */

void nci60Details(struct trackDb *tdb, char *item);
/* Set up details for nci60. */

void getMsBedExpDetails(struct trackDb *tdb, char *expName, boolean all);
/* Create tab-delimited output to download */
 
void printPslFormat(struct sqlConnection *conn, struct trackDb *tdb, char *item, int start, char *subType);
/*Handles click in Affy tracks and prints out alignment details with link*/
/* to sequences if available in the database */
 
void doAffy(struct trackDb *tdb, char *item, char *itemForUrl);
/* Display alignment information for Affy tracks */

void doScaffoldEcores(struct trackDb *tdb, char *item);
/* Creates details page and gets the scaffold co-ordinates for unmapped */
/* genomes for display and to use to create the correct outside link URL */
