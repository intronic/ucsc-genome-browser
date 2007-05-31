/* wikiTrack.h was originally generated by the autoSql program, which also 
 * generated wikiTrack.c and wikiTrack.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef WIKITRACK_H
#define WIKITRACK_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define WIKITRACK_NUM_COLS 15

struct wikiTrack
/* wikiTrack bed 6+ structure */
    {
    struct wikiTrack *next;  /* Next in singly linked list. */
    unsigned short bin;	/* used for efficient position indexing */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 */
    char strand[2];	/* + or - */
    char *db;	/* database for item */
    char *owner;	/* creator of item */
    char *color;	/* rgb color of item (currently unused) */
    char *class;	/* classification of item (browser group) */
    char *creationDate;	/* date item created */
    char *lastModifiedDate;	/* date item last updated */
    char *descriptionKey;	/* name of wiki description page */
    unsigned id;	/* auto-increment item ID */
    };

void wikiTrackStaticLoad(char **row, struct wikiTrack *ret);
/* Load a row from wikiTrack table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wikiTrack *wikiTrackLoad(char **row);
/* Load a wikiTrack from row fetched with select * from wikiTrack
 * from database.  Dispose of this with wikiTrackFree(). */

struct wikiTrack *wikiTrackLoadAll(char *fileName);
/* Load all wikiTrack from whitespace-separated file.
 * Dispose of this with wikiTrackFreeList(). */

struct wikiTrack *wikiTrackLoadAllByChar(char *fileName, char chopper);
/* Load all wikiTrack from chopper separated file.
 * Dispose of this with wikiTrackFreeList(). */

#define wikiTrackLoadAllByTab(a) wikiTrackLoadAllByChar(a, '\t');
/* Load all wikiTrack from tab separated file.
 * Dispose of this with wikiTrackFreeList(). */

struct wikiTrack *wikiTrackLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all wikiTrack from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with wikiTrackFreeList(). */

void wikiTrackSaveToDb(struct sqlConnection *conn, struct wikiTrack *el, char *tableName, int updateSize);
/* Save wikiTrack as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use wikiTrackSaveToDbEscaped() */

void wikiTrackSaveToDbEscaped(struct sqlConnection *conn, struct wikiTrack *el, char *tableName, int updateSize);
/* Save wikiTrack as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than wikiTrackSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct wikiTrack *wikiTrackCommaIn(char **pS, struct wikiTrack *ret);
/* Create a wikiTrack out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wikiTrack */

void wikiTrackFree(struct wikiTrack **pEl);
/* Free a single dynamically allocated wikiTrack such as created
 * with wikiTrackLoad(). */

void wikiTrackFreeList(struct wikiTrack **pList);
/* Free a list of dynamically allocated wikiTrack's */

void wikiTrackOutput(struct wikiTrack *el, FILE *f, char sep, char lastSep);
/* Print out wikiTrack.  Separate fields with sep. Follow last field with lastSep. */

#define wikiTrackTabOut(el,f) wikiTrackOutput(el,f,'\t','\n');
/* Print out wikiTrack as a line in a tab-separated file. */

#define wikiTrackCommaOut(el,f) wikiTrackOutput(el,f,',',',');
/* Print out wikiTrack as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

/* configuration variables */
#define CFG_WIKI_URL "wikiTrack.URL"
#define WIKI_TRACK_TABLE "wikiTrack"

/* hgc variables */
#define G_CREATE_WIKI_ITEM "htcCreateWikiItem"
#define G_ADD_WIKI_COMMENTS "htcAddWikiComments"

boolean wikiTrackEnabled(char **wikiUserName);
/*determine if wikiTrack can be used, and is this user logged into the wiki ?*/

char *wikiTrackGetCreateSql(char *tableName);
/* return sql create statement for wiki track with tableName */


#endif /* WIKITRACK_H */
