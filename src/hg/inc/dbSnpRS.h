/* dbSnpRS.h was originally generated by the autoSql program, which also 
 * generated dbSnpRS.c and dbSnpRS.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef DBSNPRS_H
#define DBSNPRS_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define DBSNPRS_NUM_COLS 8

struct dbSnpRS
/* Information from dbSNP at the reference SNP level */
    {
    struct dbSnpRS *next;  /* Next in singly linked list. */
    char *rsId;	/* dbSnp reference snp (rs) identifier */
    float avHet;	/* the average heterozygosity from all observations */
    float avHetSE;	/* the Standard Error for the average heterozygosity */
    char *valid;	/* the validation status of the SNP */
    char *allele1;	/* the sequence of the first allele */
    char *allele2;	/* the sequence of the second allele */
    char *assembly;	/* the sequence in the assembly */
    char *alternate;	/* the sequence of the alternate allele */
    };

void dbSnpRSStaticLoad(char **row, struct dbSnpRS *ret);
/* Load a row from dbSnpRS table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct dbSnpRS *dbSnpRSLoad(char **row);
/* Load a dbSnpRS from row fetched with select * from dbSnpRS
 * from database.  Dispose of this with dbSnpRSFree(). */

struct dbSnpRS *dbSnpRSLoadAll(char *fileName);
/* Load all dbSnpRS from whitespace-separated file.
 * Dispose of this with dbSnpRSFreeList(). */

struct dbSnpRS *dbSnpRSLoadAllByChar(char *fileName, char chopper);
/* Load all dbSnpRS from chopper separated file.
 * Dispose of this with dbSnpRSFreeList(). */

#define dbSnpRSLoadAllByTab(a) dbSnpRSLoadAllByChar(a, '\t');
/* Load all dbSnpRS from tab separated file.
 * Dispose of this with dbSnpRSFreeList(). */

struct dbSnpRS *dbSnpRSLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all dbSnpRS from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with dbSnpRSFreeList(). */

void dbSnpRSSaveToDb(struct sqlConnection *conn, struct dbSnpRS *el, char *tableName, int updateSize);
/* Save dbSnpRS as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use dbSnpRSSaveToDbEscaped() */

void dbSnpRSSaveToDbEscaped(struct sqlConnection *conn, struct dbSnpRS *el, char *tableName, int updateSize);
/* Save dbSnpRS as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than dbSnpRSSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct dbSnpRS *dbSnpRSCommaIn(char **pS, struct dbSnpRS *ret);
/* Create a dbSnpRS out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dbSnpRS */

void dbSnpRSFree(struct dbSnpRS **pEl);
/* Free a single dynamically allocated dbSnpRS such as created
 * with dbSnpRSLoad(). */

void dbSnpRSFreeList(struct dbSnpRS **pList);
/* Free a list of dynamically allocated dbSnpRS's */

void dbSnpRSOutput(struct dbSnpRS *el, FILE *f, char sep, char lastSep);
/* Print out dbSnpRS.  Separate fields with sep. Follow last field with lastSep. */

#define dbSnpRSTabOut(el,f) dbSnpRSOutput(el,f,'\t','\n');
/* Print out dbSnpRS as a line in a tab-separated file. */

#define dbSnpRSCommaOut(el,f) dbSnpRSOutput(el,f,',',',');
/* Print out dbSnpRS as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* DBSNPRS_H */

