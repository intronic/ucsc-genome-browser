/* snpMap.h was originally generated by the autoSql program, which also 
 * generated snpMap.c and snpMap.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef SNPMAP_H
#define SNPMAP_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define SNPMAP_NUM_COLS 6

struct snpMap
/* SNP positions from various sources */
    {
    struct snpMap *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome or 'unknown' */
    int chromStart;	/* Start position in chrom - negative 1 if unpositioned */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Name of SNP */
    char *source;	/* BAC_OVERLAP | MIXED | RANDOM | OTHER */
    char *type;	/* SNP | INDEL | SEGMENTAL */
    };

void snpMapStaticLoad(char **row, struct snpMap *ret);
/* Load a row from snpMap table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snpMap *snpMapLoad(char **row);
/* Load a snpMap from row fetched with select * from snpMap
 * from database.  Dispose of this with snpMapFree(). */

struct snpMap *snpMapLoadAll(char *fileName);
/* Load all snpMap from whitespace-separated file.
 * Dispose of this with snpMapFreeList(). */

struct snpMap *snpMapLoadAllByChar(char *fileName, char chopper);
/* Load all snpMap from chopper separated file.
 * Dispose of this with snpMapFreeList(). */

#define snpMapLoadAllByTab(a) snpMapLoadAllByChar(a, '\t');
/* Load all snpMap from tab separated file.
 * Dispose of this with snpMapFreeList(). */

struct snpMap *snpMapLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all snpMap from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with snpMapFreeList(). */

void snpMapSaveToDb(struct sqlConnection *conn, struct snpMap *el, char *tableName, int updateSize);
/* Save snpMap as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use snpMapSaveToDbEscaped() */

void snpMapSaveToDbEscaped(struct sqlConnection *conn, struct snpMap *el, char *tableName, int updateSize);
/* Save snpMap as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than snpMapSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct snpMap *snpMapCommaIn(char **pS, struct snpMap *ret);
/* Create a snpMap out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snpMap */

void snpMapFree(struct snpMap **pEl);
/* Free a single dynamically allocated snpMap such as created
 * with snpMapLoad(). */

void snpMapFreeList(struct snpMap **pList);
/* Free a list of dynamically allocated snpMap's */

void snpMapOutput(struct snpMap *el, FILE *f, char sep, char lastSep);
/* Print out snpMap.  Separate fields with sep. Follow last field with lastSep. */

#define snpMapTabOut(el,f) snpMapOutput(el,f,'\t','\n');
/* Print out snpMap as a line in a tab-separated file. */

#define snpMapCommaOut(el,f) snpMapOutput(el,f,',',',');
/* Print out snpMap as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SNPMAP_H */

