/* xAli.h was originally generated by the autoSql program, which also 
 * generated xAli.c and xAli.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef XALI_H
#define XALI_H

struct xAli
/* An alignment - like psl but includes the sequence itself */
    {
    struct xAli *next;  /* Next in singly linked list. */
    unsigned match;	/* Number of bases that match that aren't repeats */
    unsigned misMatch;	/* Number of bases that don't match */
    unsigned repMatch;	/* Number of bases that match but are part of repeats */
    unsigned nCount;	/* Number of 'N' bases */
    unsigned qNumInsert;	/* Number of inserts in query */
    int qBaseInsert;	/* Number of bases inserted in query */
    unsigned tNumInsert;	/* Number of inserts in target */
    int tBaseInsert;	/* Number of bases inserted in target */
    char strand[3];	/* + or - for strand. First character query, second target (optional) */
    char *qName;	/* Query sequence name */
    unsigned qSize;	/* Query sequence size */
    unsigned qStart;	/* Alignment start position in query */
    unsigned qEnd;	/* Alignment end position in query */
    char *tName;	/* Target sequence name */
    unsigned tSize;	/* Target sequence size */
    unsigned tStart;	/* Alignment start position in target */
    unsigned tEnd;	/* Alignment end position in target */
    unsigned blockCount;	/* Number of blocks in alignment */
    unsigned *blockSizes;	/* Size of each block */
    unsigned *qStarts;	/* Start of each block in query. */
    unsigned *tStarts;	/* Start of each block in target. */
    char **qSeq;	/* Query sequence for each block. */
    char **tSeq;	/* Target sequence for each block. */
    };

struct xAli *xAliLoad(char **row);
/* Load a xAli from row fetched with select * from xAli
 * from database.  Dispose of this with xAliFree(). */

struct xAli *xAliLoadAll(char *fileName);
/* Load all xAli from a tab-separated file.
 * Dispose of this with xAliFreeList(). */

struct xAli *xAliCommaIn(char **pS, struct xAli *ret);
/* Create a xAli out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new xAli */

void xAliFree(struct xAli **pEl);
/* Free a single dynamically allocated xAli such as created
 * with xAliLoad(). */

void xAliFreeList(struct xAli **pList);
/* Free a list of dynamically allocated xAli's */

void xAliOutput(struct xAli *el, FILE *f, char sep, char lastSep);
/* Print out xAli.  Separate fields with sep. Follow last field with lastSep. */

#define xAliTabOut(el,f) xAliOutput(el,f,'\t','\n');
/* Print out xAli as a line in a tab-separated file. */

#define xAliCommaOut(el,f) xAliOutput(el,f,',',',');
/* Print out xAli as a comma separated list including final comma. */

#endif /* XALI_H */

