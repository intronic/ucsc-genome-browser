/* pushQ.h was originally generated by the autoSql program, which also 
 * generated pushQ.c and pushQ.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef PUSHQ_H
#define PUSHQ_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define PUSHQ_NUM_COLS 29

struct pushQ
/* Push Queue */
    {
    struct pushQ *next;  /* Next in singly linked list. */
    char qid[7];	/* Queue Id */
    char pqid[7];	/* Parent Queue Id if split off another */
    char priority[2];	/* Priority */
    unsigned rank;	/* Rank for display sort */
    char qadate[11];	/* QA (re)-submission date */
    char newYN[2];	/* new (track)? */
    char *track;	/* Track */
    char *dbs;	/* Databases */
    char *tbls;	/* Tables */
    char *cgis;	/* CGI(s) */
    char *files;	/* File(s) */
    unsigned sizeMB;	/* Size MB */
    char currLoc[21];	/* Current Location */
    char makeDocYN[2];	/* MakeDoc verified? */
    char onlineHelp[51];	/* Online Help */
    char ndxYN[2];	/* Index verified? */
    char joinerYN[2];	/* all.joiner verified? */
    char *stat;	/* Status */
    char sponsor[51];	/* Sponsor */
    char reviewer[51];	/* QA Reviewer */
    char extSource[51];	/* External Source */
    char *openIssues;	/* Open issues */
    char *notes;	/* Notes */
    char pushState[2];	/* Push State = NYD = No, Yes, Done */
    char initdate[11];	/* Initial Submission Date */
    unsigned bounces;	/* Bounce-count (num times bounced back to developer) */
    char lockUser[9];	/* User who has locked record, or empty string */
    char lockDateTime[17];	/* When user placed lock, or empty string */
    char *releaseLog;	/* Release Log entry */
    };

void pushQStaticLoad(char **row, struct pushQ *ret);
/* Load a row from pushQ table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct pushQ *pushQLoad(char **row);
/* Load a pushQ from row fetched with select * from pushQ
 * from database.  Dispose of this with pushQFree(). */

struct pushQ *pushQLoadAll(char *fileName);
/* Load all pushQ from whitespace-separated file.
 * Dispose of this with pushQFreeList(). */

struct pushQ *pushQLoadAllByChar(char *fileName, char chopper);
/* Load all pushQ from chopper separated file.
 * Dispose of this with pushQFreeList(). */

#define pushQLoadAllByTab(a) pushQLoadAllByChar(a, '\t');
/* Load all pushQ from tab separated file.
 * Dispose of this with pushQFreeList(). */

struct pushQ *pushQLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all pushQ from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with pushQFreeList(). */

void pushQSaveToDb(struct sqlConnection *conn, struct pushQ *el, char *tableName, int updateSize);
/* Save pushQ as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use pushQSaveToDbEscaped() */

void pushQSaveToDbEscaped(struct sqlConnection *conn, struct pushQ *el, char *tableName, int updateSize);
/* Save pushQ as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than pushQSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct pushQ *pushQCommaIn(char **pS, struct pushQ *ret);
/* Create a pushQ out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pushQ */

void pushQFree(struct pushQ **pEl);
/* Free a single dynamically allocated pushQ such as created
 * with pushQLoad(). */

void pushQFreeList(struct pushQ **pList);
/* Free a list of dynamically allocated pushQ's */

void pushQOutput(struct pushQ *el, FILE *f, char sep, char lastSep);
/* Print out pushQ.  Separate fields with sep. Follow last field with lastSep. */

#define pushQTabOut(el,f) pushQOutput(el,f,'\t','\n');
/* Print out pushQ as a line in a tab-separated file. */

#define pushQCommaOut(el,f) pushQOutput(el,f,',',',');
/* Print out pushQ as a comma separated list including final comma. */

#define USERS_NUM_COLS 4

struct users
/* PushQ Users */
    {
    struct users *next;  /* Next in singly linked list. */
    char user[9];	/* User */
    char password[35];	/* Password */
    char role[9];	/* Role=admin,dev,qa */
    char *contents;	/* pushq-cart contents */
    };

void usersStaticLoad(char **row, struct users *ret);
/* Load a row from users table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct users *usersLoad(char **row);
/* Load a users from row fetched with select * from users
 * from database.  Dispose of this with usersFree(). */

struct users *usersLoadAll(char *fileName);
/* Load all users from whitespace-separated file.
 * Dispose of this with usersFreeList(). */

struct users *usersLoadAllByChar(char *fileName, char chopper);
/* Load all users from chopper separated file.
 * Dispose of this with usersFreeList(). */

#define usersLoadAllByTab(a) usersLoadAllByChar(a, '\t');
/* Load all users from tab separated file.
 * Dispose of this with usersFreeList(). */

struct users *usersLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all users from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with usersFreeList(). */

void usersSaveToDb(struct sqlConnection *conn, struct users *el, char *tableName, int updateSize);
/* Save users as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use usersSaveToDbEscaped() */

void usersSaveToDbEscaped(struct sqlConnection *conn, struct users *el, char *tableName, int updateSize);
/* Save users as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than usersSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct users *usersCommaIn(char **pS, struct users *ret);
/* Create a users out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new users */

void usersFree(struct users **pEl);
/* Free a single dynamically allocated users such as created
 * with usersLoad(). */

void usersFreeList(struct users **pList);
/* Free a list of dynamically allocated users's */

void usersOutput(struct users *el, FILE *f, char sep, char lastSep);
/* Print out users.  Separate fields with sep. Follow last field with lastSep. */

#define usersTabOut(el,f) usersOutput(el,f,'\t','\n');
/* Print out users as a line in a tab-separated file. */

#define usersCommaOut(el,f) usersOutput(el,f,',',',');
/* Print out users as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* PUSHQ_H */

