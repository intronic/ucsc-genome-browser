/* ncbiTabAlign.h was originally generated by the autoSql program, which also 
 * generated ncbiTabAlign.c and ncbiTabAlign.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef NCBITABALIGN_H
#define NCBITABALIGN_H

#define NCBITABALIGN_NUM_COLS 11

struct ncbiTabAlign
/* NCBI tab-seperated alignment file */
    {
    struct ncbiTabAlign *next;  /* Next in singly linked list. */
    char *target;	/* target name */
    int alnId;	/* arbitrary alignment id */
    char *type;	/* EXON = aligned region, GAP = query gap */
    int tStart;	/* 1-based start position in target */
    int tStop;	/* end position in target */
    char *query;	/* query name */
    char unused[2];	/* unused */
    int qStart;	/* 1-based start position in query */
    int qStop;	/* end position in query */
    float perId;	/* percent identity */
    float score;	/* score */
    };

void ncbiTabAlignStaticLoad(char **row, struct ncbiTabAlign *ret);
/* Load a row from ncbiTabAlign table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct ncbiTabAlign *ncbiTabAlignLoad(char **row);
/* Load a ncbiTabAlign from row fetched with select * from ncbiTabAlign
 * from database.  Dispose of this with ncbiTabAlignFree(). */

struct ncbiTabAlign *ncbiTabAlignLoadAll(char *fileName);
/* Load all ncbiTabAlign from whitespace-separated file.
 * Dispose of this with ncbiTabAlignFreeList(). */

struct ncbiTabAlign *ncbiTabAlignLoadAllByChar(char *fileName, char chopper);
/* Load all ncbiTabAlign from chopper separated file.
 * Dispose of this with ncbiTabAlignFreeList(). */

#define ncbiTabAlignLoadAllByTab(a) ncbiTabAlignLoadAllByChar(a, '\t');
/* Load all ncbiTabAlign from tab separated file.
 * Dispose of this with ncbiTabAlignFreeList(). */

struct ncbiTabAlign *ncbiTabAlignCommaIn(char **pS, struct ncbiTabAlign *ret);
/* Create a ncbiTabAlign out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ncbiTabAlign */

void ncbiTabAlignFree(struct ncbiTabAlign **pEl);
/* Free a single dynamically allocated ncbiTabAlign such as created
 * with ncbiTabAlignLoad(). */

void ncbiTabAlignFreeList(struct ncbiTabAlign **pList);
/* Free a list of dynamically allocated ncbiTabAlign's */

void ncbiTabAlignOutput(struct ncbiTabAlign *el, FILE *f, char sep, char lastSep);
/* Print out ncbiTabAlign.  Separate fields with sep. Follow last field with lastSep. */

#define ncbiTabAlignTabOut(el,f) ncbiTabAlignOutput(el,f,'\t','\n');
/* Print out ncbiTabAlign as a line in a tab-separated file. */

#define ncbiTabAlignCommaOut(el,f) ncbiTabAlignOutput(el,f,',',',');
/* Print out ncbiTabAlign as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* NCBITABALIGN_H */

