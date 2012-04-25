/* gbMembers.h was originally generated by the autoSql program, which also 
 * generated gbMembers.c and gbMembers.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef GBMEMBERS_H
#define GBMEMBERS_H

#include "jksql.h"
#define GBMEMBERS_NUM_COLS 8

struct gbMembers
/* UCSC Genome Browser members */
    {
    struct gbMembers *next;  /* Next in singly linked list. */
    unsigned idx;	/* auto-increment unique ID */
    char *userName;	/* Name used to login */
    char *realName;	/* Full name */
    char *password;	/* Encrypted password */
    char *email;	/* Email address */
    char *lastUse;	/* Last date the user log in */
    char activated[2];	/* Account activated? Y or N */
    char *dateAuthenticated;	/* Date the account activated via email */
    };

void gbMembersStaticLoad(char **row, struct gbMembers *ret);
/* Load a row from gbMembers table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gbMembers *gbMembersLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all gbMembers from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with gbMembersFreeList(). */

void gbMembersSaveToDb(struct sqlConnection *conn, struct gbMembers *el, char *tableName, int updateSize);
/* Save gbMembers as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use gbMembersSaveToDbEscaped() */

void gbMembersSaveToDbEscaped(struct sqlConnection *conn, struct gbMembers *el, char *tableName, int updateSize);
/* Save gbMembers as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than gbMembersSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct gbMembers *gbMembersLoad(char **row);
/* Load a gbMembers from row fetched with select * from gbMembers
 * from database.  Dispose of this with gbMembersFree(). */

struct gbMembers *gbMembersLoadAll(char *fileName);
/* Load all gbMembers from whitespace-separated file.
 * Dispose of this with gbMembersFreeList(). */

struct gbMembers *gbMembersLoadAllByChar(char *fileName, char chopper);
/* Load all gbMembers from chopper separated file.
 * Dispose of this with gbMembersFreeList(). */

#define gbMembersLoadAllByTab(a) gbMembersLoadAllByChar(a, '\t');
/* Load all gbMembers from tab separated file.
 * Dispose of this with gbMembersFreeList(). */

struct gbMembers *gbMembersCommaIn(char **pS, struct gbMembers *ret);
/* Create a gbMembers out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gbMembers */

void gbMembersFree(struct gbMembers **pEl);
/* Free a single dynamically allocated gbMembers such as created
 * with gbMembersLoad(). */

void gbMembersFreeList(struct gbMembers **pList);
/* Free a list of dynamically allocated gbMembers's */

void gbMembersOutput(struct gbMembers *el, FILE *f, char sep, char lastSep);
/* Print out gbMembers.  Separate fields with sep. Follow last field with lastSep. */

#define gbMembersTabOut(el,f) gbMembersOutput(el,f,'\t','\n');
/* Print out gbMembers as a line in a tab-separated file. */

#define gbMembersCommaOut(el,f) gbMembersOutput(el,f,',',',');
/* Print out gbMembers as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* GBMEMBERS_H */

