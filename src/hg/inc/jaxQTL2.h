/* jaxQTL2.h was originally generated by the autoSql program, which also 
 * generated jaxQTL2.c and jaxQTL2.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef JAXQTL2_H
#define JAXQTL2_H

struct jaxQTL2
/* Quantitative Trait Loci from Jackson Labs / Mouse Genome Informatics */
    {
    struct jaxQTL2 *next;  /* Next in singly linked list. */
    char *chrom;	/* chromosome */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 (bed6 compat.) */
    char strand[2];	/* + or - (bed6 compat.) */
    unsigned thickStart;	/* start of thick region */
    unsigned thickEnd;	/* start of thick region */
    char *marker;	/* MIT SSLP Marker w/highest correlation */
    char *mgiID;	/* MGI ID */
    char *description;	/* MGI description */
    float cMscore;	/* cM position of marker associated with peak LOD score */
    };

void jaxQTL2StaticLoad(char **row, struct jaxQTL2 *ret);
/* Load a row from jaxQTL2 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct jaxQTL2 *jaxQTL2Load(char **row);
/* Load a jaxQTL2 from row fetched with select * from jaxQTL2
 * from database.  Dispose of this with jaxQTL2Free(). */

struct jaxQTL2 *jaxQTL2LoadAll(char *fileName);
/* Load all jaxQTL2 from a tab-separated file.
 * Dispose of this with jaxQTL2FreeList(). */

struct jaxQTL2 *jaxQTL2CommaIn(char **pS, struct jaxQTL2 *ret);
/* Create a jaxQTL2 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new jaxQTL2 */

void jaxQTL2Free(struct jaxQTL2 **pEl);
/* Free a single dynamically allocated jaxQTL2 such as created
 * with jaxQTL2Load(). */

void jaxQTL2FreeList(struct jaxQTL2 **pList);
/* Free a list of dynamically allocated jaxQTL2's */

void jaxQTL2Output(struct jaxQTL2 *el, FILE *f, char sep, char lastSep);
/* Print out jaxQTL2.  Separate fields with sep. Follow last field with lastSep. */

#define jaxQTL2TabOut(el,f) jaxQTL2Output(el,f,'\t','\n');
/* Print out jaxQTL2 as a line in a tab-separated file. */

#define jaxQTL2CommaOut(el,f) jaxQTL2Output(el,f,',',',');
/* Print out jaxQTL2 as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* JAXQTL2_H */

