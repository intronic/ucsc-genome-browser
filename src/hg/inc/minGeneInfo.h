/* minGeneInfo.h was originally generated by the autoSql program, which also 
 * generated minGeneInfo.c and minGeneInfo.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MINGENEINFO_H
#define MINGENEINFO_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define MINGENEINFO_NUM_COLS 3

struct minGeneInfo
/* Auxilliary info about a gene (less than the knownInfo) */
    {
    struct minGeneInfo *next;  /* Next in singly linked list. */
    char *name;	/* gene name */
    char *product;	/* gene product */
    char *note;	/* gene note */
    };

void minGeneInfoStaticLoad(char **row, struct minGeneInfo *ret);
/* Load a row from minGeneInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct minGeneInfo *minGeneInfoLoad(char **row);
/* Load a minGeneInfo from row fetched with select * from minGeneInfo
 * from database.  Dispose of this with minGeneInfoFree(). */

struct minGeneInfo *minGeneInfoLoadAll(char *fileName);
/* Load all minGeneInfo from whitespace-separated file.
 * Dispose of this with minGeneInfoFreeList(). */

struct minGeneInfo *minGeneInfoLoadAllByChar(char *fileName, char chopper);
/* Load all minGeneInfo from chopper separated file.
 * Dispose of this with minGeneInfoFreeList(). */

#define minGeneInfoLoadAllByTab(a) minGeneInfoLoadAllByChar(a, '\t');
/* Load all minGeneInfo from tab separated file.
 * Dispose of this with minGeneInfoFreeList(). */

struct minGeneInfo *minGeneInfoLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all minGeneInfo from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with minGeneInfoFreeList(). */

void minGeneInfoSaveToDb(struct sqlConnection *conn, struct minGeneInfo *el, char *tableName, int updateSize);
/* Save minGeneInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use minGeneInfoSaveToDbEscaped() */

void minGeneInfoSaveToDbEscaped(struct sqlConnection *conn, struct minGeneInfo *el, char *tableName, int updateSize);
/* Save minGeneInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than minGeneInfoSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct minGeneInfo *minGeneInfoCommaIn(char **pS, struct minGeneInfo *ret);
/* Create a minGeneInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new minGeneInfo */

void minGeneInfoFree(struct minGeneInfo **pEl);
/* Free a single dynamically allocated minGeneInfo such as created
 * with minGeneInfoLoad(). */

void minGeneInfoFreeList(struct minGeneInfo **pList);
/* Free a list of dynamically allocated minGeneInfo's */

void minGeneInfoOutput(struct minGeneInfo *el, FILE *f, char sep, char lastSep);
/* Print out minGeneInfo.  Separate fields with sep. Follow last field with lastSep. */

#define minGeneInfoTabOut(el,f) minGeneInfoOutput(el,f,'\t','\n');
/* Print out minGeneInfo as a line in a tab-separated file. */

#define minGeneInfoCommaOut(el,f) minGeneInfoOutput(el,f,',',',');
/* Print out minGeneInfo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* MINGENEINFO_H */

