/* pseudoGeneLink.h was originally generated by the autoSql program, which also 
 * generated pseudoGeneLink.c and pseudoGeneLink.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef PSEUDOGENELINK_H
#define PSEUDOGENELINK_H

#define PSEUDOGENELINK_NUM_COLS 27

struct pseudoGeneLink
/* links a gene/pseudogene prediction to an ortholog or paralog. */
    {
    struct pseudoGeneLink *next;  /* Next in singly linked list. */
    short bin;	/* bin for browser speedup */
    char *chrom;	/* Chromosome name for pseudogene */
    unsigned chromStart;	/* pseudogene alignment start position */
    unsigned chromEnd;	/* pseudogene alignment end position */
    char *name;	/* Name of pseudogene */
    unsigned score;	/* score of pseudogene with gene */
    char *strand;	/* strand of pseudoegene */
    char *assembly;	/* assembly for gene */
    char *geneTable;	/* mysql table of gene */
    char *gene;	/* Name of gene */
    char *gChrom;	/* Chromosome name */
    unsigned gStart;	/* gene alignment start position */
    unsigned gEnd;	/* gene alignment end position */
    unsigned score2;	/* intron score of pseudogene with gap */
    unsigned score3;	/* intron score of pseudogene */
    unsigned chainId;	/* chain id of gene/pseudogene alignment */
    char *gStrand;	/* strand of gene */
    unsigned polyA;	/* length of polyA */
    unsigned polyAstart;	/* start f polyA */
    unsigned exonCover;	/* number of exons in Gene covered */
    unsigned intronCount;	/* number of introns in pseudogene */
    unsigned bestAliCount;	/* number of good mrnas aligning */
    unsigned matches;	/* matches + repMatches */
    unsigned qSize;	/* aligning bases in pseudogene */
    unsigned tReps;	/* repeats in gene */
    unsigned qReps;	/* repeats in pseudogene */
    unsigned overlapDiag;	/* bases on the diagonal to mouse */
    };

void pseudoGeneLinkStaticLoad(char **row, struct pseudoGeneLink *ret);
/* Load a row from pseudoGeneLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct pseudoGeneLink *pseudoGeneLinkLoad(char **row);
/* Load a pseudoGeneLink from row fetched with select * from pseudoGeneLink
 * from database.  Dispose of this with pseudoGeneLinkFree(). */

struct pseudoGeneLink *pseudoGeneLinkLoadAll(char *fileName);
/* Load all pseudoGeneLink from whitespace-separated file.
 * Dispose of this with pseudoGeneLinkFreeList(). */

struct pseudoGeneLink *pseudoGeneLinkLoadAllByChar(char *fileName, char chopper);
/* Load all pseudoGeneLink from chopper separated file.
 * Dispose of this with pseudoGeneLinkFreeList(). */

#define pseudoGeneLinkLoadAllByTab(a) pseudoGeneLinkLoadAllByChar(a, '\t');
/* Load all pseudoGeneLink from tab separated file.
 * Dispose of this with pseudoGeneLinkFreeList(). */

struct pseudoGeneLink *pseudoGeneLinkCommaIn(char **pS, struct pseudoGeneLink *ret);
/* Create a pseudoGeneLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pseudoGeneLink */

void pseudoGeneLinkFree(struct pseudoGeneLink **pEl);
/* Free a single dynamically allocated pseudoGeneLink such as created
 * with pseudoGeneLinkLoad(). */

void pseudoGeneLinkFreeList(struct pseudoGeneLink **pList);
/* Free a list of dynamically allocated pseudoGeneLink's */

void pseudoGeneLinkOutput(struct pseudoGeneLink *el, FILE *f, char sep, char lastSep);
/* Print out pseudoGeneLink.  Separate fields with sep. Follow last field with lastSep. */

#define pseudoGeneLinkTabOut(el,f) pseudoGeneLinkOutput(el,f,'\t','\n');
/* Print out pseudoGeneLink as a line in a tab-separated file. */

#define pseudoGeneLinkCommaOut(el,f) pseudoGeneLinkOutput(el,f,',',',');
/* Print out pseudoGeneLink as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* PSEUDOGENELINK_H */

