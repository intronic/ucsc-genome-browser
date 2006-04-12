/* taxonName.h was originally generated by the autoSql program, which also 
 * generated taxonName.c and taxonName.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef TAXONNAME_H
#define TAXONNAME_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define TAXONNAME_NUM_COLS 3

struct taxonName
/* ncbi taxonomy species names and id including synonmyms */
    {
    struct taxonName *next;  /* Next in singly linked list. */
    char *name;	/* ncbi name */
    unsigned taxon;	/* ncbi id of node associated with this name */
    char *class;	/* type of name: synonym, common name, scientific name, misspelling */
    };

void taxonNameStaticLoad(char **row, struct taxonName *ret);
/* Load a row from taxonName table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct taxonName *taxonNameLoad(char **row);
/* Load a taxonName from row fetched with select * from taxonName
 * from database.  Dispose of this with taxonNameFree(). */

struct taxonName *taxonNameLoadAll(char *fileName);
/* Load all taxonName from whitespace-separated file.
 * Dispose of this with taxonNameFreeList(). */

struct taxonName *taxonNameLoadAllByChar(char *fileName, char chopper);
/* Load all taxonName from chopper separated file.
 * Dispose of this with taxonNameFreeList(). */

#define taxonNameLoadAllByTab(a) taxonNameLoadAllByChar(a, '\t');
/* Load all taxonName from tab separated file.
 * Dispose of this with taxonNameFreeList(). */

struct taxonName *taxonNameLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all taxonName from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with taxonNameFreeList(). */

void taxonNameSaveToDb(struct sqlConnection *conn, struct taxonName *el, char *tableName, int updateSize);
/* Save taxonName as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use taxonNameSaveToDbEscaped() */

void taxonNameSaveToDbEscaped(struct sqlConnection *conn, struct taxonName *el, char *tableName, int updateSize);
/* Save taxonName as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than taxonNameSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct taxonName *taxonNameCommaIn(char **pS, struct taxonName *ret);
/* Create a taxonName out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new taxonName */

void taxonNameFree(struct taxonName **pEl);
/* Free a single dynamically allocated taxonName such as created
 * with taxonNameLoad(). */

void taxonNameFreeList(struct taxonName **pList);
/* Free a list of dynamically allocated taxonName's */

void taxonNameOutput(struct taxonName *el, FILE *f, char sep, char lastSep);
/* Print out taxonName.  Separate fields with sep. Follow last field with lastSep. */

#define taxonNameTabOut(el,f) taxonNameOutput(el,f,'\t','\n');
/* Print out taxonName as a line in a tab-separated file. */

#define taxonNameCommaOut(el,f) taxonNameOutput(el,f,',',',');
/* Print out taxonName as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* TAXONNAME_H */

