/* flyBaseSwissProt.h was originally generated by the autoSql program, which also 
 * generated flyBaseSwissProt.c and flyBaseSwissProt.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef FLYBASESWISSPROT_H
#define FLYBASESWISSPROT_H

#define FLYBASESWISSPROT_NUM_COLS 4

struct flyBaseSwissProt
/* FlyBase acc to SwissProt acc, plus some other SwissProt info */
    {
    struct flyBaseSwissProt *next;  /* Next in singly linked list. */
    char *flyBaseId;	/* FlyBase FBgn ID */
    char *swissProtId;	/* SwissProt ID */
    char *spGeneName;	/* (long) gene name from SwissProt */
    char *spSymbol;	/* symbolic-looking gene id from SwissProt */
    };

void flyBaseSwissProtStaticLoad(char **row, struct flyBaseSwissProt *ret);
/* Load a row from flyBaseSwissProt table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct flyBaseSwissProt *flyBaseSwissProtLoad(char **row);
/* Load a flyBaseSwissProt from row fetched with select * from flyBaseSwissProt
 * from database.  Dispose of this with flyBaseSwissProtFree(). */

struct flyBaseSwissProt *flyBaseSwissProtLoadAll(char *fileName);
/* Load all flyBaseSwissProt from whitespace-separated file.
 * Dispose of this with flyBaseSwissProtFreeList(). */

struct flyBaseSwissProt *flyBaseSwissProtLoadAllByChar(char *fileName, char chopper);
/* Load all flyBaseSwissProt from chopper separated file.
 * Dispose of this with flyBaseSwissProtFreeList(). */

#define flyBaseSwissProtLoadAllByTab(a) flyBaseSwissProtLoadAllByChar(a, '\t');
/* Load all flyBaseSwissProt from tab separated file.
 * Dispose of this with flyBaseSwissProtFreeList(). */

struct flyBaseSwissProt *flyBaseSwissProtCommaIn(char **pS, struct flyBaseSwissProt *ret);
/* Create a flyBaseSwissProt out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new flyBaseSwissProt */

void flyBaseSwissProtFree(struct flyBaseSwissProt **pEl);
/* Free a single dynamically allocated flyBaseSwissProt such as created
 * with flyBaseSwissProtLoad(). */

void flyBaseSwissProtFreeList(struct flyBaseSwissProt **pList);
/* Free a list of dynamically allocated flyBaseSwissProt's */

void flyBaseSwissProtOutput(struct flyBaseSwissProt *el, FILE *f, char sep, char lastSep);
/* Print out flyBaseSwissProt.  Separate fields with sep. Follow last field with lastSep. */

#define flyBaseSwissProtTabOut(el,f) flyBaseSwissProtOutput(el,f,'\t','\n');
/* Print out flyBaseSwissProt as a line in a tab-separated file. */

#define flyBaseSwissProtCommaOut(el,f) flyBaseSwissProtOutput(el,f,',',',');
/* Print out flyBaseSwissProt as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* FLYBASESWISSPROT_H */

