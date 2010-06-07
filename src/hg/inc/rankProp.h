/* rankProp.h was originally generated by the autoSql program, which also 
 * generated rankProp.c and rankProp.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef RANKPROP_H
#define RANKPROP_H

#define RANKPROP_NUM_COLS 3

struct rankProp
/* RankProp protein ranking for a pair of proteins */
    {
    struct rankProp *next;  /* Next in singly linked list. */
    char *query;	/* known genes id of query protein */
    char *target;	/* known genes id of target protein */
    float score;	/* rankp score */
    };

void rankPropStaticLoad(char **row, struct rankProp *ret);
/* Load a row from rankProp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct rankProp *rankPropLoad(char **row);
/* Load a rankProp from row fetched with select * from rankProp
 * from database.  Dispose of this with rankPropFree(). */

struct rankProp *rankPropLoadAll(char *fileName);
/* Load all rankProp from whitespace-separated file.
 * Dispose of this with rankPropFreeList(). */

struct rankProp *rankPropLoadAllByChar(char *fileName, char chopper);
/* Load all rankProp from chopper separated file.
 * Dispose of this with rankPropFreeList(). */

#define rankPropLoadAllByTab(a) rankPropLoadAllByChar(a, '\t');
/* Load all rankProp from tab separated file.
 * Dispose of this with rankPropFreeList(). */

struct rankProp *rankPropCommaIn(char **pS, struct rankProp *ret);
/* Create a rankProp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rankProp */

void rankPropFree(struct rankProp **pEl);
/* Free a single dynamically allocated rankProp such as created
 * with rankPropLoad(). */

void rankPropFreeList(struct rankProp **pList);
/* Free a list of dynamically allocated rankProp's */

void rankPropOutput(struct rankProp *el, FILE *f, char sep, char lastSep);
/* Print out rankProp.  Separate fields with sep. Follow last field with lastSep. */

#define rankPropTabOut(el,f) rankPropOutput(el,f,'\t','\n');
/* Print out rankProp as a line in a tab-separated file. */

#define rankPropCommaOut(el,f) rankPropOutput(el,f,',',',');
/* Print out rankProp as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* RANKPROP_H */

