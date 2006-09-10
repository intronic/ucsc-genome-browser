/* oreganno.h was originally generated by the autoSql program, which also 
 * generated oreganno.c and oreganno.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef OREGANNO_H
#define OREGANNO_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define OREGANNO_NUM_COLS 6

struct oreganno
/* track for regulatory regions from ORegAnno */
    {
    struct oreganno *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *id;	/* unique ID to identify this regulatory region */
    char *name;	/* name of regulatory region */
    unsigned short bin; /* A field to speed indexing */
    };

void oregannoStaticLoad(char **row, struct oreganno *ret);
/* Load a row from oreganno table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct oreganno *oregannoLoad(char **row);
/* Load a oreganno from row fetched with select * from oreganno
 * from database.  Dispose of this with oregannoFree(). */

struct oreganno *oregannoLoadAll(char *fileName);
/* Load all oreganno from whitespace-separated file.
 * Dispose of this with oregannoFreeList(). */

struct oreganno *oregannoLoadAllByChar(char *fileName, char chopper);
/* Load all oreganno from chopper separated file.
 * Dispose of this with oregannoFreeList(). */

#define oregannoLoadAllByTab(a) oregannoLoadAllByChar(a, '\t');
/* Load all oreganno from tab separated file.
 * Dispose of this with oregannoFreeList(). */

struct oreganno *oregannoLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all oreganno from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with oregannoFreeList(). */

void oregannoSaveToDb(struct sqlConnection *conn, struct oreganno *el, char *tableName, int updateSize);
/* Save oreganno as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use oregannoSaveToDbEscaped() */

void oregannoSaveToDbEscaped(struct sqlConnection *conn, struct oreganno *el, char *tableName, int updateSize);
/* Save oreganno as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than oregannoSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct oreganno *oregannoCommaIn(char **pS, struct oreganno *ret);
/* Create a oreganno out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new oreganno */

void oregannoFree(struct oreganno **pEl);
/* Free a single dynamically allocated oreganno such as created
 * with oregannoLoad(). */

void oregannoFreeList(struct oreganno **pList);
/* Free a list of dynamically allocated oreganno's */

void oregannoOutput(struct oreganno *el, FILE *f, char sep, char lastSep);
/* Print out oreganno.  Separate fields with sep. Follow last field with lastSep. */

#define oregannoTabOut(el,f) oregannoOutput(el,f,'\t','\n');
/* Print out oreganno as a line in a tab-separated file. */

#define oregannoCommaOut(el,f) oregannoOutput(el,f,',',',');
/* Print out oreganno as a comma separated list including final comma. */

#define OREGANNOATTR_NUM_COLS 3

struct oregannoAttr
/* attributes associated with an ORegAnno regulatory region */
    {
    struct oregannoAttr *next;  /* Next in singly linked list. */
    char *id;	/* key into the oreganno table */
    char *attribute;	/* name of attribute being listed */
    char *attrVal;	/* value of this oreganno attribute */
    };

void oregannoAttrStaticLoad(char **row, struct oregannoAttr *ret);
/* Load a row from oregannoAttr table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct oregannoAttr *oregannoAttrLoad(char **row);
/* Load a oregannoAttr from row fetched with select * from oregannoAttr
 * from database.  Dispose of this with oregannoAttrFree(). */

struct oregannoAttr *oregannoAttrLoadAll(char *fileName);
/* Load all oregannoAttr from whitespace-separated file.
 * Dispose of this with oregannoAttrFreeList(). */

struct oregannoAttr *oregannoAttrLoadAllByChar(char *fileName, char chopper);
/* Load all oregannoAttr from chopper separated file.
 * Dispose of this with oregannoAttrFreeList(). */

#define oregannoAttrLoadAllByTab(a) oregannoAttrLoadAllByChar(a, '\t');
/* Load all oregannoAttr from tab separated file.
 * Dispose of this with oregannoAttrFreeList(). */

struct oregannoAttr *oregannoAttrLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all oregannoAttr from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with oregannoAttrFreeList(). */

void oregannoAttrSaveToDb(struct sqlConnection *conn, struct oregannoAttr *el, char *tableName, int updateSize);
/* Save oregannoAttr as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use oregannoAttrSaveToDbEscaped() */

void oregannoAttrSaveToDbEscaped(struct sqlConnection *conn, struct oregannoAttr *el, char *tableName, int updateSize);
/* Save oregannoAttr as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than oregannoAttrSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct oregannoAttr *oregannoAttrCommaIn(char **pS, struct oregannoAttr *ret);
/* Create a oregannoAttr out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new oregannoAttr */

void oregannoAttrFree(struct oregannoAttr **pEl);
/* Free a single dynamically allocated oregannoAttr such as created
 * with oregannoAttrLoad(). */

void oregannoAttrFreeList(struct oregannoAttr **pList);
/* Free a list of dynamically allocated oregannoAttr's */

void oregannoAttrOutput(struct oregannoAttr *el, FILE *f, char sep, char lastSep);
/* Print out oregannoAttr.  Separate fields with sep. Follow last field with lastSep. */

#define oregannoAttrTabOut(el,f) oregannoAttrOutput(el,f,'\t','\n');
/* Print out oregannoAttr as a line in a tab-separated file. */

#define oregannoAttrCommaOut(el,f) oregannoAttrOutput(el,f,',',',');
/* Print out oregannoAttr as a comma separated list including final comma. */

#define OREGANNOLINK_NUM_COLS 4

struct oregannoLink
/* links for ORegAnno regulatory region */
    {
    struct oregannoLink *next;  /* Next in singly linked list. */
    char *id;	/* key into the oreganno table */
    char *attribute;	/* name of attribute being listed */
    char *raKey;	/* key into RA file, tells how to link */
    char *attrAcc;	/* accession used by link */
    };

void oregannoLinkStaticLoad(char **row, struct oregannoLink *ret);
/* Load a row from oregannoLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct oregannoLink *oregannoLinkLoad(char **row);
/* Load a oregannoLink from row fetched with select * from oregannoLink
 * from database.  Dispose of this with oregannoLinkFree(). */

struct oregannoLink *oregannoLinkLoadAll(char *fileName);
/* Load all oregannoLink from whitespace-separated file.
 * Dispose of this with oregannoLinkFreeList(). */

struct oregannoLink *oregannoLinkLoadAllByChar(char *fileName, char chopper);
/* Load all oregannoLink from chopper separated file.
 * Dispose of this with oregannoLinkFreeList(). */

#define oregannoLinkLoadAllByTab(a) oregannoLinkLoadAllByChar(a, '\t');
/* Load all oregannoLink from tab separated file.
 * Dispose of this with oregannoLinkFreeList(). */

struct oregannoLink *oregannoLinkLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all oregannoLink from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with oregannoLinkFreeList(). */

void oregannoLinkSaveToDb(struct sqlConnection *conn, struct oregannoLink *el, char *tableName, int updateSize);
/* Save oregannoLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use oregannoLinkSaveToDbEscaped() */

void oregannoLinkSaveToDbEscaped(struct sqlConnection *conn, struct oregannoLink *el, char *tableName, int updateSize);
/* Save oregannoLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than oregannoLinkSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct oregannoLink *oregannoLinkCommaIn(char **pS, struct oregannoLink *ret);
/* Create a oregannoLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new oregannoLink */

void oregannoLinkFree(struct oregannoLink **pEl);
/* Free a single dynamically allocated oregannoLink such as created
 * with oregannoLinkLoad(). */

void oregannoLinkFreeList(struct oregannoLink **pList);
/* Free a list of dynamically allocated oregannoLink's */

void oregannoLinkOutput(struct oregannoLink *el, FILE *f, char sep, char lastSep);
/* Print out oregannoLink.  Separate fields with sep. Follow last field with lastSep. */

#define oregannoLinkTabOut(el,f) oregannoLinkOutput(el,f,'\t','\n');
/* Print out oregannoLink as a line in a tab-separated file. */

#define oregannoLinkCommaOut(el,f) oregannoLinkOutput(el,f,',',',');
/* Print out oregannoLink as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* OREGANNO_H */

