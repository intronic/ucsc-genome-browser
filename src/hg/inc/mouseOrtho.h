/* mouseOrtho.h was originally generated by the autoSql program, which also 
 * generated mouseOrtho.c and mouseOrtho.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MOUSEORTHO_H
#define MOUSEORTHO_H

struct mouseOrtho
/* Synteny between mouse and human chromosomes. */
    {
    struct mouseOrtho *next;  /* Next in singly linked list. */
    char *chrom;	/* Name of chromosome */
    unsigned chromStart;	/* Start in chromosome */
    unsigned chromEnd;	/* End in chromosome */
    char *name;	/* Name of gene  */
    unsigned score;	/* alignment score blat */
    char strand[2];	/* + or - for strand */
    unsigned thickStart;	/* Coding region start */
    unsigned thickEnd;	/* Coding region end */
    unsigned exonCount;	/* Number of exons */
    unsigned *exonStarts;	/* Exon start positions */
    unsigned *exonEnds;	/* Exon end positions */
    int segment;	/* Number of segment */
    };

struct mouseOrtho *mouseOrthoLoad(char **row);
/* Load a mouseOrtho from row fetched with select * from mouseOrtho
 * from database.  Dispose of this with mouseOrthoFree(). */

struct mouseOrtho *mouseOrthoLoadAll(char *fileName);
/* Load all mouseOrtho from a tab-separated file.
 * Dispose of this with mouseOrthoFreeList(). */

struct mouseOrtho *mouseOrthoLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all mouseOrtho from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with mouseOrthoFreeList(). */

struct mouseOrtho *mouseOrthoCommaIn(char **pS, struct mouseOrtho *ret);
/* Create a mouseOrtho out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mouseOrtho */

void mouseOrthoFree(struct mouseOrtho **pEl);
/* Free a single dynamically allocated mouseOrtho such as created
 * with mouseOrthoLoad(). */

void mouseOrthoFreeList(struct mouseOrtho **pList);
/* Free a list of dynamically allocated mouseOrtho's */

void mouseOrthoOutput(struct mouseOrtho *el, FILE *f, char sep, char lastSep);
/* Print out mouseOrtho.  Separate fields with sep. Follow last field with lastSep. */

#define mouseOrthoTabOut(el,f) mouseOrthoOutput(el,f,'\t','\n');
/* Print out mouseOrtho as a line in a tab-separated file. */

#define mouseOrthoCommaOut(el,f) mouseOrthoOutput(el,f,',',',');
/* Print out mouseOrtho as a comma separated list including final comma. */

#endif /* MOUSEORTHO_H */

