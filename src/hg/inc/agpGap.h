/* agpGap.h was originally generated by the autoSql program, which also 
 * generated agpGap.c and agpGap.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef AGPGAP_H
#define AGPGAP_H

struct agpGap
/* Gaps in golden path */
    {
    struct agpGap *next;  /* Next in singly linked list. */
    char *chrom;	/* which chromosome */
    unsigned chromStart;	/* start position in chromosome */
    unsigned chromEnd;	/* end position in chromosome */
    int ix;	/* ix of this fragment (useless) */
    char n[2];	/* always 'N' */
    unsigned size;	/* size of gap */
    char *type;	/* contig, clone, fragment, etc. */
    char *bridge;	/* yes, no, mrna, bacEndPair, etc. */
    };

void agpGapStaticLoad(char **row, struct agpGap *ret);
/* Load a row from agpGap table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct agpGap *agpGapLoad(char **row);
/* Load a agpGap from row fetched with select * from agpGap
 * from database.  Dispose of this with agpGapFree(). */

struct agpGap *agpGapCommaIn(char **pS, struct agpGap *ret);
/* Create a agpGap out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new agpGap */

void agpGapFree(struct agpGap **pEl);
/* Free a single dynamically allocated agpGap such as created
 * with agpGapLoad(). */

void agpGapFreeList(struct agpGap **pList);
/* Free a list of dynamically allocated agpGap's */

void agpGapOutput(struct agpGap *el, FILE *f, char sep, char lastSep);
/* Print out agpGap.  Separate fields with sep. Follow last field with lastSep. */

#define agpGapTabOut(el,f) agpGapOutput(el,f,'\t','\n');
/* Print out agpGap as a line in a tab-separated file. */

#define agpGapCommaOut(el,f) agpGapOutput(el,f,',',',');
/* Print out agpGap as a comma separated list including final comma. */

#endif /* AGPGAP_H */

