/* dnaMotif.h was originally generated by the autoSql program, which also 
 * generated dnaMotif.c and dnaMotif.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef DNAMOTIF_H
#define DNAMOTIF_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

struct dnaMotif
/* A gapless DNA motif */
    {
    struct dnaMotif *next;  /* Next in singly linked list. */
    char *name;	/* Motif name. */
    int columnCount;	/* Count of columns in motif. */
    float *aProb;	/* Probability of A's in each column. */
    float *cProb;	/* Probability of C's in each column. */
    float *gProb;	/* Probability of G's in each column. */
    float *tProb;	/* Probability of T's in each column. */
    };

#define dnaMotifRowSize 6  /* Number of stored fields, not including next */

struct dnaMotif *dnaMotifLoad(char **row);
/* Load a dnaMotif from row fetched with select * from dnaMotif
 * from database.  Dispose of this with dnaMotifFree(). */

struct dnaMotif *dnaMotifLoadAll(char *fileName);
/* Load all dnaMotif from a tab-separated file.
 * Dispose of this with dnaMotifFreeList(). */

struct dnaMotif *dnaMotifLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all dnaMotif from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with dnaMotifFreeList(). */

struct dnaMotif *dnaMotifCommaIn(char **pS, struct dnaMotif *ret);
/* Create a dnaMotif out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dnaMotif */

void dnaMotifFree(struct dnaMotif **pEl);
/* Free a single dynamically allocated dnaMotif such as created
 * with dnaMotifLoad(). */

void dnaMotifFreeList(struct dnaMotif **pList);
/* Free a list of dynamically allocated dnaMotif's */

void dnaMotifOutput(struct dnaMotif *el, FILE *f, char sep, char lastSep);
/* Print out dnaMotif.  Separate fields with sep. Follow last field with lastSep. */

#define dnaMotifTabOut(el,f) dnaMotifOutput(el,f,'\t','\n');
/* Print out dnaMotif as a line in a tab-separated file. */

#define dnaMotifCommaOut(el,f) dnaMotifOutput(el,f,',',',');
/* Print out dnaMotif as a comma separated list including final comma. */

#endif /* DNAMOTIF_H */

