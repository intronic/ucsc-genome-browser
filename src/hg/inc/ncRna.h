/* ncRna.h was originally generated by the autoSql program, which also 
 * generated ncRna.c and ncRna.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef NCRNA_H
#define NCRNA_H

#define NCRNA_NUM_COLS 11

struct ncRna
/* non-protein-coding genes */
    {
    struct ncRna *next;  /* Next in singly linked list. */
    short bin;	/* bin for browser speed up */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* genomic start position */
    unsigned chromEnd;	/* gnomic end position */
    char *name;	/* Name of gene */
    int score;	/* score */
    char strand[2];	/* + or - for strand */
    unsigned thickStart;	/* unused */
    unsigned thickEnd;	/* unused */
    char *type;	/* type of gene */
    char *extGeneId;	/* external gene ID */
    };

void ncRnaStaticLoad(char **row, struct ncRna *ret);
/* Load a row from ncRna table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct ncRna *ncRnaLoad(char **row);
/* Load a ncRna from row fetched with select * from ncRna
 * from database.  Dispose of this with ncRnaFree(). */

struct ncRna *ncRnaLoadAll(char *fileName);
/* Load all ncRna from whitespace-separated file.
 * Dispose of this with ncRnaFreeList(). */

struct ncRna *ncRnaLoadAllByChar(char *fileName, char chopper);
/* Load all ncRna from chopper separated file.
 * Dispose of this with ncRnaFreeList(). */

#define ncRnaLoadAllByTab(a) ncRnaLoadAllByChar(a, '\t');
/* Load all ncRna from tab separated file.
 * Dispose of this with ncRnaFreeList(). */

struct ncRna *ncRnaCommaIn(char **pS, struct ncRna *ret);
/* Create a ncRna out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ncRna */

void ncRnaFree(struct ncRna **pEl);
/* Free a single dynamically allocated ncRna such as created
 * with ncRnaLoad(). */

void ncRnaFreeList(struct ncRna **pList);
/* Free a list of dynamically allocated ncRna's */

void ncRnaOutput(struct ncRna *el, FILE *f, char sep, char lastSep);
/* Print out ncRna.  Separate fields with sep. Follow last field with lastSep. */

#define ncRnaTabOut(el,f) ncRnaOutput(el,f,'\t','\n');
/* Print out ncRna as a line in a tab-separated file. */

#define ncRnaCommaOut(el,f) ncRnaOutput(el,f,',',',');
/* Print out ncRna as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* NCRNA_H */

