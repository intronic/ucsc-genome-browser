/* bactigPos.h was originally generated by the autoSql program, which also 
 * generated bactigPos.c and bactigPos.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef BACTIGPOS_H
#define BACTIGPOS_H

struct bactigPos
/* Bactig positions in chromosome coordinates (bed 4 +). */
    {
    struct bactigPos *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Bactig */
    char *startContig;	/* First contig in this bactig */
    char *endContig;	/* Last contig in this bactig */
    };

void bactigPosStaticLoad(char **row, struct bactigPos *ret);
/* Load a row from bactigPos table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct bactigPos *bactigPosLoad(char **row);
/* Load a bactigPos from row fetched with select * from bactigPos
 * from database.  Dispose of this with bactigPosFree(). */

struct bactigPos *bactigPosLoadAll(char *fileName);
/* Load all bactigPos from a tab-separated file.
 * Dispose of this with bactigPosFreeList(). */

struct bactigPos *bactigPosCommaIn(char **pS, struct bactigPos *ret);
/* Create a bactigPos out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bactigPos */

void bactigPosFree(struct bactigPos **pEl);
/* Free a single dynamically allocated bactigPos such as created
 * with bactigPosLoad(). */

void bactigPosFreeList(struct bactigPos **pList);
/* Free a list of dynamically allocated bactigPos's */

void bactigPosOutput(struct bactigPos *el, FILE *f, char sep, char lastSep);
/* Print out bactigPos.  Separate fields with sep. Follow last field with lastSep. */

#define bactigPosTabOut(el,f) bactigPosOutput(el,f,'\t','\n');
/* Print out bactigPos as a line in a tab-separated file. */

#define bactigPosCommaOut(el,f) bactigPosOutput(el,f,',',',');
/* Print out bactigPos as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* BACTIGPOS_H */

