/* txRnaAccs.h was originally generated by the autoSql program, which also 
 * generated txRnaAccs.c and txRnaAccs.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef TXRNAACCS_H
#define TXRNAACCS_H

#define TXRNAACCS_NUM_COLS 4

struct txRnaAccs
/* A list of Genbank accessions associated with a transcript */
    {
    struct txRnaAccs *next;  /* Next in singly linked list. */
    char *name;	/* Transcript name */
    char *primary;	/* Primary source of info - contains all exons */
    int accCount;	/* Count of total accessions */
    char **accs;	/* Array of accessions.  Not all of these have all exons */
    };

struct txRnaAccs *txRnaAccsLoad(char **row);
/* Load a txRnaAccs from row fetched with select * from txRnaAccs
 * from database.  Dispose of this with txRnaAccsFree(). */

struct txRnaAccs *txRnaAccsLoadAll(char *fileName);
/* Load all txRnaAccs from whitespace-separated file.
 * Dispose of this with txRnaAccsFreeList(). */

struct txRnaAccs *txRnaAccsLoadAllByChar(char *fileName, char chopper);
/* Load all txRnaAccs from chopper separated file.
 * Dispose of this with txRnaAccsFreeList(). */

#define txRnaAccsLoadAllByTab(a) txRnaAccsLoadAllByChar(a, '\t');
/* Load all txRnaAccs from tab separated file.
 * Dispose of this with txRnaAccsFreeList(). */

struct txRnaAccs *txRnaAccsCommaIn(char **pS, struct txRnaAccs *ret);
/* Create a txRnaAccs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new txRnaAccs */

void txRnaAccsFree(struct txRnaAccs **pEl);
/* Free a single dynamically allocated txRnaAccs such as created
 * with txRnaAccsLoad(). */

void txRnaAccsFreeList(struct txRnaAccs **pList);
/* Free a list of dynamically allocated txRnaAccs's */

void txRnaAccsOutput(struct txRnaAccs *el, FILE *f, char sep, char lastSep);
/* Print out txRnaAccs.  Separate fields with sep. Follow last field with lastSep. */

#define txRnaAccsTabOut(el,f) txRnaAccsOutput(el,f,'\t','\n');
/* Print out txRnaAccs as a line in a tab-separated file. */

#define txRnaAccsCommaOut(el,f) txRnaAccsOutput(el,f,',',',');
/* Print out txRnaAccs as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* TXRNAACCS_H */

