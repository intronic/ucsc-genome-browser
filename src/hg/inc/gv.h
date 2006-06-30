/* gv.h was originally generated by the autoSql program, which also 
 * generated gv.c and gv.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef GV_H
#define GV_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define GV_NUM_COLS 6

struct gv
/* track for mutation data */
    {
    struct gv *next;  /* Next in singly linked list. */
    char *id;	/* unique ID for this mutation */
    char *name;	/* Official nomenclature description of mutation */
    char *srcId;	/* source ID for this mutation */
    char *baseChangeType;	/* enum('insertion', 'deletion', 'substitution','duplication','complex','unknown') */
    char *location;	/* enum('intron', 'exon', '5'' UTR', '3'' UTR', 'not within known transcription unit') */
    unsigned char coordinateAccuracy;	/* 0=estimated, 1=definite, others? */
    };

void gvStaticLoad(char **row, struct gv *ret);
/* Load a row from gv table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gv *gvLoad(char **row);
/* Load a gv from row fetched with select * from gv
 * from database.  Dispose of this with gvFree(). */

struct gv *gvLoadAll(char *fileName);
/* Load all gv from whitespace-separated file.
 * Dispose of this with gvFreeList(). */

struct gv *gvLoadAllByChar(char *fileName, char chopper);
/* Load all gv from chopper separated file.
 * Dispose of this with gvFreeList(). */

#define gvLoadAllByTab(a) gvLoadAllByChar(a, '\t');
/* Load all gv from tab separated file.
 * Dispose of this with gvFreeList(). */

struct gv *gvLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gv from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gvFreeList(). */

void gvSaveToDb(struct sqlConnection *conn, struct gv *el, char *tableName, int updateSize);
/* Save gv as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gvSaveToDbEscaped() */

void gvSaveToDbEscaped(struct sqlConnection *conn, struct gv *el, char *tableName, int updateSize);
/* Save gv as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gvSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gv *gvCommaIn(char **pS, struct gv *ret);
/* Create a gv out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gv */

void gvFree(struct gv **pEl);
/* Free a single dynamically allocated gv such as created
 * with gvLoad(). */

void gvFreeList(struct gv **pList);
/* Free a list of dynamically allocated gv's */

void gvOutput(struct gv *el, FILE *f, char sep, char lastSep);
/* Print out gv.  Separate fields with sep. Follow last field with lastSep. */

#define gvTabOut(el,f) gvOutput(el,f,'\t','\n');
/* Print out gv as a line in a tab-separated file. */

#define gvCommaOut(el,f) gvOutput(el,f,',',',');
/* Print out gv as a comma separated list including final comma. */

#define GVPOS_NUM_COLS 5

struct gvPos
/* location of mutation */
    {
    struct gvPos *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* ID for this mutation */
    char *id;   /* Added field to hold ID if change name */
    unsigned short bin; /* A field to speed indexing */
    };

void gvPosStaticLoad(char **row, struct gvPos *ret);
/* Load a row from gvPos table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gvPos *gvPosLoad(char **row);
/* Load a gvPos from row fetched with select * from gvPos
 * from database.  Dispose of this with gvPosFree(). */

struct gvPos *gvPosLoadAll(char *fileName);
/* Load all gvPos from whitespace-separated file.
 * Dispose of this with gvPosFreeList(). */

struct gvPos *gvPosLoadAllByChar(char *fileName, char chopper);
/* Load all gvPos from chopper separated file.
 * Dispose of this with gvPosFreeList(). */

#define gvPosLoadAllByTab(a) gvPosLoadAllByChar(a, '\t');
/* Load all gvPos from tab separated file.
 * Dispose of this with gvPosFreeList(). */

struct gvPos *gvPosLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gvPos from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gvPosFreeList(). */

void gvPosSaveToDb(struct sqlConnection *conn, struct gvPos *el, char *tableName, int updateSize);
/* Save gvPos as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gvPosSaveToDbEscaped() */

void gvPosSaveToDbEscaped(struct sqlConnection *conn, struct gvPos *el, char *tableName, int updateSize);
/* Save gvPos as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gvPosSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gvPos *gvPosCommaIn(char **pS, struct gvPos *ret);
/* Create a gvPos out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gvPos */

void gvPosFree(struct gvPos **pEl);
/* Free a single dynamically allocated gvPos such as created
 * with gvPosLoad(). */

void gvPosFreeList(struct gvPos **pList);
/* Free a list of dynamically allocated gvPos's */

void gvPosOutput(struct gvPos *el, FILE *f, char sep, char lastSep);
/* Print out gvPos.  Separate fields with sep. Follow last field with lastSep. */

#define gvPosTabOut(el,f) gvPosOutput(el,f,'\t','\n');
/* Print out gvPos as a line in a tab-separated file. */

#define gvPosCommaOut(el,f) gvPosOutput(el,f,',',',');
/* Print out gvPos as a comma separated list including final comma. */

#define GVSRC_NUM_COLS 3

struct gvSrc
/* sources for mutation track */
    {
    struct gvSrc *next;  /* Next in singly linked list. */
    char *srcId;	/* key into gv table */
    char *src;	/* name of genome wide source or LSDB */
    char *lsdb;	/* for LSDB name of actual source DB */
    };

void gvSrcStaticLoad(char **row, struct gvSrc *ret);
/* Load a row from gvSrc table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gvSrc *gvSrcLoad(char **row);
/* Load a gvSrc from row fetched with select * from gvSrc
 * from database.  Dispose of this with gvSrcFree(). */

struct gvSrc *gvSrcLoadAll(char *fileName);
/* Load all gvSrc from whitespace-separated file.
 * Dispose of this with gvSrcFreeList(). */

struct gvSrc *gvSrcLoadAllByChar(char *fileName, char chopper);
/* Load all gvSrc from chopper separated file.
 * Dispose of this with gvSrcFreeList(). */

#define gvSrcLoadAllByTab(a) gvSrcLoadAllByChar(a, '\t');
/* Load all gvSrc from tab separated file.
 * Dispose of this with gvSrcFreeList(). */

struct gvSrc *gvSrcLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gvSrc from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gvSrcFreeList(). */

void gvSrcSaveToDb(struct sqlConnection *conn, struct gvSrc *el, char *tableName, int updateSize);
/* Save gvSrc as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gvSrcSaveToDbEscaped() */

void gvSrcSaveToDbEscaped(struct sqlConnection *conn, struct gvSrc *el, char *tableName, int updateSize);
/* Save gvSrc as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gvSrcSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gvSrc *gvSrcCommaIn(char **pS, struct gvSrc *ret);
/* Create a gvSrc out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gvSrc */

void gvSrcFree(struct gvSrc **pEl);
/* Free a single dynamically allocated gvSrc such as created
 * with gvSrcLoad(). */

void gvSrcFreeList(struct gvSrc **pList);
/* Free a list of dynamically allocated gvSrc's */

void gvSrcOutput(struct gvSrc *el, FILE *f, char sep, char lastSep);
/* Print out gvSrc.  Separate fields with sep. Follow last field with lastSep. */

#define gvSrcTabOut(el,f) gvSrcOutput(el,f,'\t','\n');
/* Print out gvSrc as a line in a tab-separated file. */

#define gvSrcCommaOut(el,f) gvSrcOutput(el,f,',',',');
/* Print out gvSrc as a comma separated list including final comma. */

#define GVATTR_NUM_COLS 3

struct gvAttr
/* attributes asssociated with the mutation */
    {
    struct gvAttr *next;  /* Next in singly linked list. */
    char *id;	/* mutation ID */
    char *attrType;	/* attribute type */
    char *attrVal;	/* value for this attribute */
    };

void gvAttrStaticLoad(char **row, struct gvAttr *ret);
/* Load a row from gvAttr table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gvAttr *gvAttrLoad(char **row);
/* Load a gvAttr from row fetched with select * from gvAttr
 * from database.  Dispose of this with gvAttrFree(). */

struct gvAttr *gvAttrLoadAll(char *fileName);
/* Load all gvAttr from whitespace-separated file.
 * Dispose of this with gvAttrFreeList(). */

struct gvAttr *gvAttrLoadAllByChar(char *fileName, char chopper);
/* Load all gvAttr from chopper separated file.
 * Dispose of this with gvAttrFreeList(). */

#define gvAttrLoadAllByTab(a) gvAttrLoadAllByChar(a, '\t');
/* Load all gvAttr from tab separated file.
 * Dispose of this with gvAttrFreeList(). */

struct gvAttr *gvAttrLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gvAttr from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gvAttrFreeList(). */

void gvAttrSaveToDb(struct sqlConnection *conn, struct gvAttr *el, char *tableName, int updateSize);
/* Save gvAttr as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gvAttrSaveToDbEscaped() */

void gvAttrSaveToDbEscaped(struct sqlConnection *conn, struct gvAttr *el, char *tableName, int updateSize);
/* Save gvAttr as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gvAttrSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gvAttr *gvAttrCommaIn(char **pS, struct gvAttr *ret);
/* Create a gvAttr out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gvAttr */

void gvAttrFree(struct gvAttr **pEl);
/* Free a single dynamically allocated gvAttr such as created
 * with gvAttrLoad(). */

void gvAttrFreeList(struct gvAttr **pList);
/* Free a list of dynamically allocated gvAttr's */

void gvAttrOutput(struct gvAttr *el, FILE *f, char sep, char lastSep);
/* Print out gvAttr.  Separate fields with sep. Follow last field with lastSep. */

#define gvAttrTabOut(el,f) gvAttrOutput(el,f,'\t','\n');
/* Print out gvAttr as a line in a tab-separated file. */

#define gvAttrCommaOut(el,f) gvAttrOutput(el,f,',',',');
/* Print out gvAttr as a comma separated list including final comma. */

#define GVLINK_NUM_COLS 5

struct gvLink
/* links both urls and local table lookups */
    {
    struct gvLink *next;  /* Next in singly linked list. */
    char *id;	/* id for attribute link */
    char *attrType;	/* attribute type */
    char *raKey;	/* key into .ra file on how to do link */
    char *acc;	/* accession or id used by link */
    char *displayVal;	/* value to display if different from acc */
    };

void gvLinkStaticLoad(char **row, struct gvLink *ret);
/* Load a row from gvLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gvLink *gvLinkLoad(char **row);
/* Load a gvLink from row fetched with select * from gvLink
 * from database.  Dispose of this with gvLinkFree(). */

struct gvLink *gvLinkLoadAll(char *fileName);
/* Load all gvLink from whitespace-separated file.
 * Dispose of this with gvLinkFreeList(). */

struct gvLink *gvLinkLoadAllByChar(char *fileName, char chopper);
/* Load all gvLink from chopper separated file.
 * Dispose of this with gvLinkFreeList(). */

#define gvLinkLoadAllByTab(a) gvLinkLoadAllByChar(a, '\t');
/* Load all gvLink from tab separated file.
 * Dispose of this with gvLinkFreeList(). */

struct gvLink *gvLinkLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gvLink from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gvLinkFreeList(). */

void gvLinkSaveToDb(struct sqlConnection *conn, struct gvLink *el, char *tableName, int updateSize);
/* Save gvLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gvLinkSaveToDbEscaped() */

void gvLinkSaveToDbEscaped(struct sqlConnection *conn, struct gvLink *el, char *tableName, int updateSize);
/* Save gvLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gvLinkSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gvLink *gvLinkCommaIn(char **pS, struct gvLink *ret);
/* Create a gvLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gvLink */

void gvLinkFree(struct gvLink **pEl);
/* Free a single dynamically allocated gvLink such as created
 * with gvLinkLoad(). */

void gvLinkFreeList(struct gvLink **pList);
/* Free a list of dynamically allocated gvLink's */

void gvLinkOutput(struct gvLink *el, FILE *f, char sep, char lastSep);
/* Print out gvLink.  Separate fields with sep. Follow last field with lastSep. */

#define gvLinkTabOut(el,f) gvLinkOutput(el,f,'\t','\n');
/* Print out gvLink as a line in a tab-separated file. */

#define gvLinkCommaOut(el,f) gvLinkOutput(el,f,',',',');
/* Print out gvLink as a comma separated list including final comma. */

#define GVATTRLONG_NUM_COLS 3

struct gvAttrLong
/* attributes associated with a mutation that need long text */
    {
    struct gvAttrLong *next;  /* Next in singly linked list. */
    char *id;	/* mutation ID */
    char *attrType;	/* attribute type */
    char *attrVal;	/* value for this attribute */
    };

void gvAttrLongStaticLoad(char **row, struct gvAttrLong *ret);
/* Load a row from gvAttrLong table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gvAttrLong *gvAttrLongLoad(char **row);
/* Load a gvAttrLong from row fetched with select * from gvAttrLong
 * from database.  Dispose of this with gvAttrLongFree(). */

struct gvAttrLong *gvAttrLongLoadAll(char *fileName);
/* Load all gvAttrLong from whitespace-separated file.
 * Dispose of this with gvAttrLongFreeList(). */

struct gvAttrLong *gvAttrLongLoadAllByChar(char *fileName, char chopper);
/* Load all gvAttrLong from chopper separated file.
 * Dispose of this with gvAttrLongFreeList(). */

#define gvAttrLongLoadAllByTab(a) gvAttrLongLoadAllByChar(a, '\t');
/* Load all gvAttrLong from tab separated file.
 * Dispose of this with gvAttrLongFreeList(). */

struct gvAttrLong *gvAttrLongLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gvAttrLong from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gvAttrLongFreeList(). */

void gvAttrLongSaveToDb(struct sqlConnection *conn, struct gvAttrLong *el, char *tableName, int updateSize);
/* Save gvAttrLong as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gvAttrLongSaveToDbEscaped() */

void gvAttrLongSaveToDbEscaped(struct sqlConnection *conn, struct gvAttrLong *el, char *tableName, int updateSize);
/* Save gvAttrLong as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gvAttrLongSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gvAttrLong *gvAttrLongCommaIn(char **pS, struct gvAttrLong *ret);
/* Create a gvAttrLong out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gvAttrLong */

void gvAttrLongFree(struct gvAttrLong **pEl);
/* Free a single dynamically allocated gvAttrLong such as created
 * with gvAttrLongLoad(). */

void gvAttrLongFreeList(struct gvAttrLong **pList);
/* Free a list of dynamically allocated gvAttrLong's */

void gvAttrLongOutput(struct gvAttrLong *el, FILE *f, char sep, char lastSep);
/* Print out gvAttrLong.  Separate fields with sep. Follow last field with lastSep. */

#define gvAttrLongTabOut(el,f) gvAttrLongOutput(el,f,'\t','\n');
/* Print out gvAttrLong as a line in a tab-separated file. */

#define gvAttrLongCommaOut(el,f) gvAttrLongOutput(el,f,',',',');
/* Print out gvAttrLong as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* GV_H */

