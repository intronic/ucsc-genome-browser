/* expRecord.h was originally generated by the autoSql program, which also 
 * generated expRecord.c and expRecord.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef EXPRECORD_H
#define EXPRECORD_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

struct expRecord
/* minimal descriptive data for an experiment in the browser */
    {
    struct expRecord *next;  /* Next in singly linked list. */
    unsigned id;	/* internal id of experiment */
    char *name;	/* name of experiment */
    char *description;	/* description of experiment */
    char *url;	/* url relevant to experiment */
    char *ref;	/* reference for experiment */
    char *credit;	/* who to credit with experiment */
    unsigned numExtras;	/* number of extra things */
    char **extras;	/* extra things of interest, i.e. classifications */
    };

struct expRecord *expRecordLoad(char **row);
/* Load a expRecord from row fetched with select * from expRecord
 * from database.  Dispose of this with expRecordFree(). */

struct expRecord *expRecordLoadAll(char *fileName);
/* Load all expRecord from a tab-separated file.
 * Dispose of this with expRecordFreeList(). */

struct expRecord *expRecordLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all expRecord from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with expRecordFreeList(). */

void expRecordSaveToDb(struct sqlConnection *conn, struct expRecord *el, char *tableName, int updateSize);
/* Save expRecord as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use expRecordSaveToDbEscaped() */

void expRecordSaveToDbEscaped(struct sqlConnection *conn, struct expRecord *el, char *tableName, int updateSize);
/* Save expRecord as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than expRecordSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct expRecord *expRecordCommaIn(char **pS, struct expRecord *ret);
/* Create a expRecord out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new expRecord */

void expRecordFree(struct expRecord **pEl);
/* Free a single dynamically allocated expRecord such as created
 * with expRecordLoad(). */

void expRecordFreeList(struct expRecord **pList);
/* Free a list of dynamically allocated expRecord's */

void expRecordOutput(struct expRecord *el, FILE *f, char sep, char lastSep);
/* Print out expRecord.  Separate fields with sep. Follow last field with lastSep. */

#define expRecordTabOut(el,f) expRecordOutput(el,f,'\t','\n');
/* Print out expRecord as a line in a tab-separated file. */

#define expRecordCommaOut(el,f) expRecordOutput(el,f,',',',');
/* Print out expRecord as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void expRecordCreateTable(struct sqlConnection *conn, char *table);
/* Create expression record format table of given name. */

struct expRecord *expRecordLoadTable(struct sqlConnection *conn, char *table);
/* Load expression record format table of given name. */

struct expRecord *expRecordConnectAndLoadTable(char *database, char *table);
/* Load expression record format table of given name. */

/* cart variables for filters */

#define UCSF_DEMO_ER "ucsfdemoER"
#define UCSF_DEMO_ER_DEFAULT "no filter"
#define UCSF_DEMO_PR "ucsfdemoPR"
#define UCSF_DEMO_PR_DEFAULT "no filter"


#endif /* EXPRECORD_H */

