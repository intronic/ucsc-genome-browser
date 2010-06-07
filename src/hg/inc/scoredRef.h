/* scoredRef.h was originally generated by the autoSql program, which also 
 * generated scoredRef.c and scoredRef.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef SCOREDREF_H
#define SCOREDREF_H

#define SCOREDREF_NUM_COLS 6

struct scoredRef
/* A score, a range of positions in the genome and a extFile offset */
    {
    struct scoredRef *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome (this species) */
    unsigned chromStart;	/* Start position in chromosome (forward strand) */
    unsigned chromEnd;	/* End position in chromosome */
    unsigned extFile;	/* Pointer to associated external file */
    long long offset;	/* Offset in external file */
    float score;	/* Overall score */
    };

void scoredRefStaticLoad(char **row, struct scoredRef *ret);
/* Load a row from scoredRef table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct scoredRef *scoredRefLoad(char **row);
/* Load a scoredRef from row fetched with select * from scoredRef
 * from database.  Dispose of this with scoredRefFree(). */

struct scoredRef *scoredRefLoadAll(char *fileName);
/* Load all scoredRef from whitespace-separated file.
 * Dispose of this with scoredRefFreeList(). */

struct scoredRef *scoredRefCommaIn(char **pS, struct scoredRef *ret);
/* Create a scoredRef out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new scoredRef */

void scoredRefFree(struct scoredRef **pEl);
/* Free a single dynamically allocated scoredRef such as created
 * with scoredRefLoad(). */

void scoredRefFreeList(struct scoredRef **pList);
/* Free a list of dynamically allocated scoredRef's */

void scoredRefOutput(struct scoredRef *el, FILE *f, char sep, char lastSep);
/* Print out scoredRef.  Separate fields with sep. Follow last field with lastSep. */

#define scoredRefTabOut(el,f) scoredRefOutput(el,f,'\t','\n');
/* Print out scoredRef as a line in a tab-separated file. */

#define scoredRefCommaOut(el,f) scoredRefOutput(el,f,',',',');
/* Print out scoredRef as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void scoredRefTableCreate(struct sqlConnection *conn, char *tableName, int indexSize);
/* Create a scored-ref table with the given name. */

#endif /* SCOREDREF_H */

