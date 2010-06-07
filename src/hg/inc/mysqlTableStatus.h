/* mysqlTableStatus.h was originally generated by the autoSql program, which also 
 * generated mysqlTableStatus.c and mysqlTableStatus.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MYSQLTABLESTATUS_H
#define MYSQLTABLESTATUS_H

#define MYSQLTABLESTATUS_NUM_COLS 13

struct mysqlTableStatus
/* Table status info from MySQL.  Just includes stuff common to 3.1 & 4.1 */
    {
    struct mysqlTableStatus *next;  /* Next in singly linked list. */
    char *name;	/* Name of table */
    char *type;	/* Type - MyISAM, InnoDB, etc. */
    char *rowFormat;	/* Row storage format: Fixed, Dynamic, Compressed */
    int rowCount;	/* Number of rows */
    int averageRowLength;	/* Average row length */
    int dataLength;	/* Length of data file */
    int maxDataLength;	/* Maximum length of data file */
    int indexLength;	/* Length of index file */
    int dataFree;	/* Number of allocated but not used bytes of data */
    char *autoIncrement;	/* Next autoincrement value (or NULL) */
    char *createTime;	/* Table creation time */
    char *updateTime;	/* Table last update time */
    char *checkTime;	/* Table last checked time (or NULL) */
    };

void mysqlTableStatusStaticLoad(char **row, struct mysqlTableStatus *ret);
/* Load a row from mysqlTableStatus table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct mysqlTableStatus *mysqlTableStatusLoad(char **row);
/* Load a mysqlTableStatus from row fetched with select * from mysqlTableStatus
 * from database.  Dispose of this with mysqlTableStatusFree(). */

void mysqlTableStatusFree(struct mysqlTableStatus **pEl);
/* Free a single dynamically allocated mysqlTableStatus such as created
 * with mysqlTableStatusLoad(). */

void mysqlTableStatusFreeList(struct mysqlTableStatus **pList);
/* Free a list of dynamically allocated mysqlTableStatus's */

void mysqlTableStatusOutput(struct mysqlTableStatus *el, FILE *f, char sep, char lastSep);
/* Print out mysqlTableStatus.  Separate fields with sep. Follow last field with lastSep. */

#define mysqlTableStatusTabOut(el,f) mysqlTableStatusOutput(el,f,'\t','\n');
/* Print out mysqlTableStatus as a line in a tab-separated file. */

#define mysqlTableStatusCommaOut(el,f) mysqlTableStatusOutput(el,f,',',',');
/* Print out mysqlTableStatus as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* MYSQLTABLESTATUS_H */

