/* encodeRna.h was originally generated by the autoSql program, which also 
 * generated encodeRna.c and encodeRna.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ENCODERNA_H
#define ENCODERNA_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define ENCODERNA_NUM_COLS 14

struct encodeRna
/* Describes RNAs in the encode regions */
    {
    struct encodeRna *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome gene is on */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of gene */
    unsigned score;	/* Score from 0 to 1000 */
    char *strand;	/* Strand + or - */
    char *source;	/* Source as in Sean Eddy's files. */
    char *type;	/* Type - snRNA, rRNA, tRNA, etc. */
    float fullScore;	/* Score as in Sean Eddys files. */
    unsigned isPsuedo;	/* TRUE(1) if psuedo, FALSE(0) otherwise */
    unsigned isRmasked;	/* TRUE(1) if >10% is RepeatMasked, FALSE(0) otherwise */
    unsigned isTranscribed;	/* TRUE(1) if >10% falls within a transfrag or TAR, FALSE(0) otherwise */
    unsigned isPrediction;	/* TRUE(1) if an evoFold prediction, FALSE(0) otherwise */
    char *transcribedIn;	/* List of experiments transcribed in */
    };

void encodeRnaStaticLoad(char **row, struct encodeRna *ret);
/* Load a row from encodeRna table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct encodeRna *encodeRnaLoad(char **row);
/* Load a encodeRna from row fetched with select * from encodeRna
 * from database.  Dispose of this with encodeRnaFree(). */

struct encodeRna *encodeRnaLoadAll(char *fileName);
/* Load all encodeRna from whitespace-separated file.
 * Dispose of this with encodeRnaFreeList(). */

struct encodeRna *encodeRnaLoadAllByChar(char *fileName, char chopper);
/* Load all encodeRna from chopper separated file.
 * Dispose of this with encodeRnaFreeList(). */

#define encodeRnaLoadAllByTab(a) encodeRnaLoadAllByChar(a, '\t');
/* Load all encodeRna from tab separated file.
 * Dispose of this with encodeRnaFreeList(). */

struct encodeRna *encodeRnaLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all encodeRna from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with encodeRnaFreeList(). */

void encodeRnaSaveToDb(struct sqlConnection *conn, struct encodeRna *el, char *tableName, int updateSize);
/* Save encodeRna as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use encodeRnaSaveToDbEscaped() */

void encodeRnaSaveToDbEscaped(struct sqlConnection *conn, struct encodeRna *el, char *tableName, int updateSize);
/* Save encodeRna as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than encodeRnaSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct encodeRna *encodeRnaCommaIn(char **pS, struct encodeRna *ret);
/* Create a encodeRna out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeRna */

void encodeRnaFree(struct encodeRna **pEl);
/* Free a single dynamically allocated encodeRna such as created
 * with encodeRnaLoad(). */

void encodeRnaFreeList(struct encodeRna **pList);
/* Free a list of dynamically allocated encodeRna's */

void encodeRnaOutput(struct encodeRna *el, FILE *f, char sep, char lastSep);
/* Print out encodeRna.  Separate fields with sep. Follow last field with lastSep. */

#define encodeRnaTabOut(el,f) encodeRnaOutput(el,f,'\t','\n');
/* Print out encodeRna as a line in a tab-separated file. */

#define encodeRnaCommaOut(el,f) encodeRnaOutput(el,f,',',',');
/* Print out encodeRna as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ENCODERNA_H */

