/* refLink.h was originally generated by the autoSql program, which also 
 * generated refLink.c and refLink.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef REFLINK_H
#define REFLINK_H

struct refLink
/* Link together a refseq mRNA and other stuff */
    {
    struct refLink *next;  /* Next in singly linked list. */
    char *name;	/* Name displayed in UI */
    char *product;	/* Name of protein product */
    char *mrnaAcc;	/* mRNA accession */
    char *protAcc;	/* protein accession */
    unsigned geneId;	/* pointer to geneName table */
    unsigned prodId;	/* pointer to prodName table */
    unsigned locusLinkId;	/* Locus Link ID */
    unsigned omimId;	/* OMIM ID */
    };

void refLinkStaticLoad(char **row, struct refLink *ret);
/* Load a row from refLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct refLink *refLinkLoad(char **row);
/* Load a refLink from row fetched with select * from refLink
 * from database.  Dispose of this with refLinkFree(). */

struct refLink *refLinkLoadAll(char *fileName);
/* Load all refLink from a tab-separated file.
 * Dispose of this with refLinkFreeList(). */

struct refLink *refLinkCommaIn(char **pS, struct refLink *ret);
/* Create a refLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new refLink */

void refLinkFree(struct refLink **pEl);
/* Free a single dynamically allocated refLink such as created
 * with refLinkLoad(). */

void refLinkFreeList(struct refLink **pList);
/* Free a list of dynamically allocated refLink's */

void refLinkOutput(struct refLink *el, FILE *f, char sep, char lastSep);
/* Print out refLink.  Separate fields with sep. Follow last field with lastSep. */

#define refLinkTabOut(el,f) refLinkOutput(el,f,'\t','\n');
/* Print out refLink as a line in a tab-separated file. */

#define refLinkCommaOut(el,f) refLinkOutput(el,f,',',',');
/* Print out refLink as a comma separated list including final comma. */

#endif /* REFLINK_H */

