/* hgMut.h was originally generated by the autoSql program, which also 
 * generated hgMut.c and hgMut.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HGMUT_H
#define HGMUT_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define HGMUT_NUM_COLS 11

struct hgMut
/* track for human mutation data */
    {
    struct hgMut *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* HGVS style description of mutation. */
    char *mutId;	/* unique ID for this mutation */
    unsigned short srcId;	/* source ID for this mutation */
    char hasPhenData[2];	/* y or n, does this have phenotype data linked */
    char *baseChangeType;	/* enum('insertion', 'deletion', 'substitution','duplication','complex','unknown'). */
    char *location;	/* enum('intron', 'exon', '5'' UTR', '3'' UTR', 'not within known transcription unit'). */
    unsigned char coordinateAccuracy;	/* 0=estimated, 1=definite, others? */
    unsigned short bin; /* A field to speed indexing */
    };

void hgMutStaticLoad(char **row, struct hgMut *ret);
/* Load a row from hgMut table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMut *hgMutLoad(char **row);
/* Load a hgMut from row fetched with select * from hgMut
 * from database.  Dispose of this with hgMutFree(). */

struct hgMut *hgMutLoadAll(char *fileName);
/* Load all hgMut from whitespace-separated file.
 * Dispose of this with hgMutFreeList(). */

struct hgMut *hgMutLoadAllByChar(char *fileName, char chopper);
/* Load all hgMut from chopper separated file.
 * Dispose of this with hgMutFreeList(). */

#define hgMutLoadAllByTab(a) hgMutLoadAllByChar(a, '\t');
/* Load all hgMut from tab separated file.
 * Dispose of this with hgMutFreeList(). */

struct hgMut *hgMutLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMut from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutFreeList(). */

void hgMutSaveToDb(struct sqlConnection *conn, struct hgMut *el, char *tableName, int updateSize);
/* Save hgMut as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutSaveToDbEscaped() */

void hgMutSaveToDbEscaped(struct sqlConnection *conn, struct hgMut *el, char *tableName, int updateSize);
/* Save hgMut as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMut *hgMutCommaIn(char **pS, struct hgMut *ret);
/* Create a hgMut out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMut */

void hgMutFree(struct hgMut **pEl);
/* Free a single dynamically allocated hgMut such as created
 * with hgMutLoad(). */

void hgMutFreeList(struct hgMut **pList);
/* Free a list of dynamically allocated hgMut's */

void hgMutOutput(struct hgMut *el, FILE *f, char sep, char lastSep);
/* Print out hgMut.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutTabOut(el,f) hgMutOutput(el,f,'\t','\n');
/* Print out hgMut as a line in a tab-separated file. */

#define hgMutCommaOut(el,f) hgMutOutput(el,f,',',',');
/* Print out hgMut as a comma separated list including final comma. */

#define HGMUTSRC_NUM_COLS 3

struct hgMutSrc
/* sources for human mutation track */
    {
    struct hgMutSrc *next;  /* Next in singly linked list. */
    unsigned short srcId;	/* key into hgMut table */
    char *src;	/* name of genome wide source or LSDB */
    char *details;	/* for LSDB name of actual source DB */
    };

void hgMutSrcStaticLoad(char **row, struct hgMutSrc *ret);
/* Load a row from hgMutSrc table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutSrc *hgMutSrcLoad(char **row);
/* Load a hgMutSrc from row fetched with select * from hgMutSrc
 * from database.  Dispose of this with hgMutSrcFree(). */

struct hgMutSrc *hgMutSrcLoadAll(char *fileName);
/* Load all hgMutSrc from whitespace-separated file.
 * Dispose of this with hgMutSrcFreeList(). */

struct hgMutSrc *hgMutSrcLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutSrc from chopper separated file.
 * Dispose of this with hgMutSrcFreeList(). */

#define hgMutSrcLoadAllByTab(a) hgMutSrcLoadAllByChar(a, '\t');
/* Load all hgMutSrc from tab separated file.
 * Dispose of this with hgMutSrcFreeList(). */

struct hgMutSrc *hgMutSrcLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutSrc from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutSrcFreeList(). */

void hgMutSrcSaveToDb(struct sqlConnection *conn, struct hgMutSrc *el, char *tableName, int updateSize);
/* Save hgMutSrc as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutSrcSaveToDbEscaped() */

void hgMutSrcSaveToDbEscaped(struct sqlConnection *conn, struct hgMutSrc *el, char *tableName, int updateSize);
/* Save hgMutSrc as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutSrcSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutSrc *hgMutSrcCommaIn(char **pS, struct hgMutSrc *ret);
/* Create a hgMutSrc out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutSrc */

void hgMutSrcFree(struct hgMutSrc **pEl);
/* Free a single dynamically allocated hgMutSrc such as created
 * with hgMutSrcLoad(). */

void hgMutSrcFreeList(struct hgMutSrc **pList);
/* Free a list of dynamically allocated hgMutSrc's */

void hgMutSrcOutput(struct hgMutSrc *el, FILE *f, char sep, char lastSep);
/* Print out hgMutSrc.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutSrcTabOut(el,f) hgMutSrcOutput(el,f,'\t','\n');
/* Print out hgMutSrc as a line in a tab-separated file. */

#define hgMutSrcCommaOut(el,f) hgMutSrcOutput(el,f,',',',');
/* Print out hgMutSrc as a comma separated list including final comma. */

#define HGMUTEXTLINK_NUM_COLS 3

struct hgMutExtLink
/* accessions and sources for links */
    {
    struct hgMutExtLink *next;  /* Next in singly linked list. */
    char *mutId;	/* mutation ID */
    char *acc;	/* accession or ID used by link */
    int linkId;	/* link ID, foreign key into hgMutLink */
    };

void hgMutExtLinkStaticLoad(char **row, struct hgMutExtLink *ret);
/* Load a row from hgMutExtLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutExtLink *hgMutExtLinkLoad(char **row);
/* Load a hgMutExtLink from row fetched with select * from hgMutExtLink
 * from database.  Dispose of this with hgMutExtLinkFree(). */

struct hgMutExtLink *hgMutExtLinkLoadAll(char *fileName);
/* Load all hgMutExtLink from whitespace-separated file.
 * Dispose of this with hgMutExtLinkFreeList(). */

struct hgMutExtLink *hgMutExtLinkLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutExtLink from chopper separated file.
 * Dispose of this with hgMutExtLinkFreeList(). */

#define hgMutExtLinkLoadAllByTab(a) hgMutExtLinkLoadAllByChar(a, '\t');
/* Load all hgMutExtLink from tab separated file.
 * Dispose of this with hgMutExtLinkFreeList(). */

struct hgMutExtLink *hgMutExtLinkLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutExtLink from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutExtLinkFreeList(). */

void hgMutExtLinkSaveToDb(struct sqlConnection *conn, struct hgMutExtLink *el, char *tableName, int updateSize);
/* Save hgMutExtLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutExtLinkSaveToDbEscaped() */

void hgMutExtLinkSaveToDbEscaped(struct sqlConnection *conn, struct hgMutExtLink *el, char *tableName, int updateSize);
/* Save hgMutExtLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutExtLinkSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutExtLink *hgMutExtLinkCommaIn(char **pS, struct hgMutExtLink *ret);
/* Create a hgMutExtLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutExtLink */

void hgMutExtLinkFree(struct hgMutExtLink **pEl);
/* Free a single dynamically allocated hgMutExtLink such as created
 * with hgMutExtLinkLoad(). */

void hgMutExtLinkFreeList(struct hgMutExtLink **pList);
/* Free a list of dynamically allocated hgMutExtLink's */

void hgMutExtLinkOutput(struct hgMutExtLink *el, FILE *f, char sep, char lastSep);
/* Print out hgMutExtLink.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutExtLinkTabOut(el,f) hgMutExtLinkOutput(el,f,'\t','\n');
/* Print out hgMutExtLink as a line in a tab-separated file. */

#define hgMutExtLinkCommaOut(el,f) hgMutExtLinkOutput(el,f,',',',');
/* Print out hgMutExtLink as a comma separated list including final comma. */

#define HGMUTLINK_NUM_COLS 3

struct hgMutLink
/* links for human mutation detail page */
    {
    struct hgMutLink *next;  /* Next in singly linked list. */
    int linkId;	/* ID for this source, links to hgMutExtLink table. */
    char *linkDisplayName;	/* Display name for this link. */
    char *url;	/* url to substitute ID in for links. */
    };

void hgMutLinkStaticLoad(char **row, struct hgMutLink *ret);
/* Load a row from hgMutLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutLink *hgMutLinkLoad(char **row);
/* Load a hgMutLink from row fetched with select * from hgMutLink
 * from database.  Dispose of this with hgMutLinkFree(). */

struct hgMutLink *hgMutLinkLoadAll(char *fileName);
/* Load all hgMutLink from whitespace-separated file.
 * Dispose of this with hgMutLinkFreeList(). */

struct hgMutLink *hgMutLinkLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutLink from chopper separated file.
 * Dispose of this with hgMutLinkFreeList(). */

#define hgMutLinkLoadAllByTab(a) hgMutLinkLoadAllByChar(a, '\t');
/* Load all hgMutLink from tab separated file.
 * Dispose of this with hgMutLinkFreeList(). */

struct hgMutLink *hgMutLinkLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutLink from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutLinkFreeList(). */

void hgMutLinkSaveToDb(struct sqlConnection *conn, struct hgMutLink *el, char *tableName, int updateSize);
/* Save hgMutLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutLinkSaveToDbEscaped() */

void hgMutLinkSaveToDbEscaped(struct sqlConnection *conn, struct hgMutLink *el, char *tableName, int updateSize);
/* Save hgMutLink as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutLinkSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutLink *hgMutLinkCommaIn(char **pS, struct hgMutLink *ret);
/* Create a hgMutLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutLink */

void hgMutLinkFree(struct hgMutLink **pEl);
/* Free a single dynamically allocated hgMutLink such as created
 * with hgMutLinkLoad(). */

void hgMutLinkFreeList(struct hgMutLink **pList);
/* Free a list of dynamically allocated hgMutLink's */

void hgMutLinkOutput(struct hgMutLink *el, FILE *f, char sep, char lastSep);
/* Print out hgMutLink.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutLinkTabOut(el,f) hgMutLinkOutput(el,f,'\t','\n');
/* Print out hgMutLink as a line in a tab-separated file. */

#define hgMutLinkCommaOut(el,f) hgMutLinkOutput(el,f,',',',');
/* Print out hgMutLink as a comma separated list including final comma. */

#define HGMUTALIAS_NUM_COLS 3

struct hgMutAlias
/* aliases for mutations */
    {
    struct hgMutAlias *next;  /* Next in singly linked list. */
    char *mutId;	/* mutation ID from hgMut table. */
    char *name;	/* Another name for the mutation. */
    char *nameType;	/* common, or ? */
    };

void hgMutAliasStaticLoad(char **row, struct hgMutAlias *ret);
/* Load a row from hgMutAlias table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutAlias *hgMutAliasLoad(char **row);
/* Load a hgMutAlias from row fetched with select * from hgMutAlias
 * from database.  Dispose of this with hgMutAliasFree(). */

struct hgMutAlias *hgMutAliasLoadAll(char *fileName);
/* Load all hgMutAlias from whitespace-separated file.
 * Dispose of this with hgMutAliasFreeList(). */

struct hgMutAlias *hgMutAliasLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutAlias from chopper separated file.
 * Dispose of this with hgMutAliasFreeList(). */

#define hgMutAliasLoadAllByTab(a) hgMutAliasLoadAllByChar(a, '\t');
/* Load all hgMutAlias from tab separated file.
 * Dispose of this with hgMutAliasFreeList(). */

struct hgMutAlias *hgMutAliasLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutAlias from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutAliasFreeList(). */

void hgMutAliasSaveToDb(struct sqlConnection *conn, struct hgMutAlias *el, char *tableName, int updateSize);
/* Save hgMutAlias as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutAliasSaveToDbEscaped() */

void hgMutAliasSaveToDbEscaped(struct sqlConnection *conn, struct hgMutAlias *el, char *tableName, int updateSize);
/* Save hgMutAlias as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutAliasSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutAlias *hgMutAliasCommaIn(char **pS, struct hgMutAlias *ret);
/* Create a hgMutAlias out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutAlias */

void hgMutAliasFree(struct hgMutAlias **pEl);
/* Free a single dynamically allocated hgMutAlias such as created
 * with hgMutAliasLoad(). */

void hgMutAliasFreeList(struct hgMutAlias **pList);
/* Free a list of dynamically allocated hgMutAlias's */

void hgMutAliasOutput(struct hgMutAlias *el, FILE *f, char sep, char lastSep);
/* Print out hgMutAlias.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutAliasTabOut(el,f) hgMutAliasOutput(el,f,'\t','\n');
/* Print out hgMutAlias as a line in a tab-separated file. */

#define hgMutAliasCommaOut(el,f) hgMutAliasOutput(el,f,',',',');
/* Print out hgMutAlias as a comma separated list including final comma. */

#define HGMUTATTR_NUM_COLS 4

struct hgMutAttr
/* attributes asssociated with the mutation */
    {
    struct hgMutAttr *next;  /* Next in singly linked list. */
    char *mutId;	/* mutation ID. */
    int mutAttrClassId;	/* id for attribute class or category, foreign key. */
    int mutAttrNameId;	/* id for attribute name, foreign key. */
    char *mutAttrVal;	/* value for this attribute */
    };

void hgMutAttrStaticLoad(char **row, struct hgMutAttr *ret);
/* Load a row from hgMutAttr table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutAttr *hgMutAttrLoad(char **row);
/* Load a hgMutAttr from row fetched with select * from hgMutAttr
 * from database.  Dispose of this with hgMutAttrFree(). */

struct hgMutAttr *hgMutAttrLoadAll(char *fileName);
/* Load all hgMutAttr from whitespace-separated file.
 * Dispose of this with hgMutAttrFreeList(). */

struct hgMutAttr *hgMutAttrLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutAttr from chopper separated file.
 * Dispose of this with hgMutAttrFreeList(). */

#define hgMutAttrLoadAllByTab(a) hgMutAttrLoadAllByChar(a, '\t');
/* Load all hgMutAttr from tab separated file.
 * Dispose of this with hgMutAttrFreeList(). */

struct hgMutAttr *hgMutAttrLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutAttr from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutAttrFreeList(). */

void hgMutAttrSaveToDb(struct sqlConnection *conn, struct hgMutAttr *el, char *tableName, int updateSize);
/* Save hgMutAttr as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutAttrSaveToDbEscaped() */

void hgMutAttrSaveToDbEscaped(struct sqlConnection *conn, struct hgMutAttr *el, char *tableName, int updateSize);
/* Save hgMutAttr as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutAttrSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutAttr *hgMutAttrCommaIn(char **pS, struct hgMutAttr *ret);
/* Create a hgMutAttr out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutAttr */

void hgMutAttrFree(struct hgMutAttr **pEl);
/* Free a single dynamically allocated hgMutAttr such as created
 * with hgMutAttrLoad(). */

void hgMutAttrFreeList(struct hgMutAttr **pList);
/* Free a list of dynamically allocated hgMutAttr's */

void hgMutAttrOutput(struct hgMutAttr *el, FILE *f, char sep, char lastSep);
/* Print out hgMutAttr.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutAttrTabOut(el,f) hgMutAttrOutput(el,f,'\t','\n');
/* Print out hgMutAttr as a line in a tab-separated file. */

#define hgMutAttrCommaOut(el,f) hgMutAttrOutput(el,f,',',',');
/* Print out hgMutAttr as a comma separated list including final comma. */

#define HGMUTATTRCLASS_NUM_COLS 3

struct hgMutAttrClass
/* classes or categories of attributes */
    {
    struct hgMutAttrClass *next;  /* Next in singly linked list. */
    int mutAttrClassId;	/* id for attribute class. */
    char *mutAttrClass;	/* class */
    int displayOrder;	/* order to display the classes in on the detail page. */
    };

void hgMutAttrClassStaticLoad(char **row, struct hgMutAttrClass *ret);
/* Load a row from hgMutAttrClass table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutAttrClass *hgMutAttrClassLoad(char **row);
/* Load a hgMutAttrClass from row fetched with select * from hgMutAttrClass
 * from database.  Dispose of this with hgMutAttrClassFree(). */

struct hgMutAttrClass *hgMutAttrClassLoadAll(char *fileName);
/* Load all hgMutAttrClass from whitespace-separated file.
 * Dispose of this with hgMutAttrClassFreeList(). */

struct hgMutAttrClass *hgMutAttrClassLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutAttrClass from chopper separated file.
 * Dispose of this with hgMutAttrClassFreeList(). */

#define hgMutAttrClassLoadAllByTab(a) hgMutAttrClassLoadAllByChar(a, '\t');
/* Load all hgMutAttrClass from tab separated file.
 * Dispose of this with hgMutAttrClassFreeList(). */

struct hgMutAttrClass *hgMutAttrClassLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutAttrClass from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutAttrClassFreeList(). */

void hgMutAttrClassSaveToDb(struct sqlConnection *conn, struct hgMutAttrClass *el, char *tableName, int updateSize);
/* Save hgMutAttrClass as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutAttrClassSaveToDbEscaped() */

void hgMutAttrClassSaveToDbEscaped(struct sqlConnection *conn, struct hgMutAttrClass *el, char *tableName, int updateSize);
/* Save hgMutAttrClass as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutAttrClassSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutAttrClass *hgMutAttrClassCommaIn(char **pS, struct hgMutAttrClass *ret);
/* Create a hgMutAttrClass out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutAttrClass */

void hgMutAttrClassFree(struct hgMutAttrClass **pEl);
/* Free a single dynamically allocated hgMutAttrClass such as created
 * with hgMutAttrClassLoad(). */

void hgMutAttrClassFreeList(struct hgMutAttrClass **pList);
/* Free a list of dynamically allocated hgMutAttrClass's */

void hgMutAttrClassOutput(struct hgMutAttrClass *el, FILE *f, char sep, char lastSep);
/* Print out hgMutAttrClass.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutAttrClassTabOut(el,f) hgMutAttrClassOutput(el,f,'\t','\n');
/* Print out hgMutAttrClass as a line in a tab-separated file. */

#define hgMutAttrClassCommaOut(el,f) hgMutAttrClassOutput(el,f,',',',');
/* Print out hgMutAttrClass as a comma separated list including final comma. */

#define HGMUTATTRNAME_NUM_COLS 3

struct hgMutAttrName
/* Names of attributes */
    {
    struct hgMutAttrName *next;  /* Next in singly linked list. */
    int mutAttrNameId;	/* id for attribute name. */
    int mutAttrClassId;	/* id for class this name belongs to. */
    char *mutAttrName;	/* name */
    };

void hgMutAttrNameStaticLoad(char **row, struct hgMutAttrName *ret);
/* Load a row from hgMutAttrName table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgMutAttrName *hgMutAttrNameLoad(char **row);
/* Load a hgMutAttrName from row fetched with select * from hgMutAttrName
 * from database.  Dispose of this with hgMutAttrNameFree(). */

struct hgMutAttrName *hgMutAttrNameLoadAll(char *fileName);
/* Load all hgMutAttrName from whitespace-separated file.
 * Dispose of this with hgMutAttrNameFreeList(). */

struct hgMutAttrName *hgMutAttrNameLoadAllByChar(char *fileName, char chopper);
/* Load all hgMutAttrName from chopper separated file.
 * Dispose of this with hgMutAttrNameFreeList(). */

#define hgMutAttrNameLoadAllByTab(a) hgMutAttrNameLoadAllByChar(a, '\t');
/* Load all hgMutAttrName from tab separated file.
 * Dispose of this with hgMutAttrNameFreeList(). */

struct hgMutAttrName *hgMutAttrNameLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hgMutAttrName from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hgMutAttrNameFreeList(). */

void hgMutAttrNameSaveToDb(struct sqlConnection *conn, struct hgMutAttrName *el, char *tableName, int updateSize);
/* Save hgMutAttrName as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use hgMutAttrNameSaveToDbEscaped() */

void hgMutAttrNameSaveToDbEscaped(struct sqlConnection *conn, struct hgMutAttrName *el, char *tableName, int updateSize);
/* Save hgMutAttrName as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than hgMutAttrNameSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct hgMutAttrName *hgMutAttrNameCommaIn(char **pS, struct hgMutAttrName *ret);
/* Create a hgMutAttrName out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgMutAttrName */

void hgMutAttrNameFree(struct hgMutAttrName **pEl);
/* Free a single dynamically allocated hgMutAttrName such as created
 * with hgMutAttrNameLoad(). */

void hgMutAttrNameFreeList(struct hgMutAttrName **pList);
/* Free a list of dynamically allocated hgMutAttrName's */

void hgMutAttrNameOutput(struct hgMutAttrName *el, FILE *f, char sep, char lastSep);
/* Print out hgMutAttrName.  Separate fields with sep. Follow last field with lastSep. */

#define hgMutAttrNameTabOut(el,f) hgMutAttrNameOutput(el,f,'\t','\n');
/* Print out hgMutAttrName as a line in a tab-separated file. */

#define hgMutAttrNameCommaOut(el,f) hgMutAttrNameOutput(el,f,',',',');
/* Print out hgMutAttrName as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* HGMUT_H */

