/* pseudoGeneLink.h was originally generated by the autoSql program, which also 
 * generated pseudoGeneLink.c and pseudoGeneLink.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef PSEUDOGENELINK_H
#define PSEUDOGENELINK_H

#define PSEUDOGENELINK_NUM_COLS 56

struct pseudoGeneLink
/* links a gene/pseudogene prediction to an ortholog or paralog. */
    {
    struct pseudoGeneLink *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome name for pseudogene */
    unsigned chromStart;	/* pseudogene alignment start position */
    unsigned chromEnd;	/* pseudogene alignment end position */
    char *name;	/* Name of pseudogene */
    unsigned score;	/* score of pseudogene with gene */
    char strand[3];	/* + or - */
    unsigned thickStart;	/* Start of where display should be thick (start codon) */
    unsigned thickEnd;	/* End of where display should be thick (stop codon) */
    unsigned reserved;	/* Always zero for now */
    int blockCount;	/* Number of blocks */
    int *blockSizes;	/* Comma separated list of block sizes */
    int *chromStarts;	/* Start positions relative to chromStart */
    float trfRatio;	/* ratio of tandem repeats */
    char *type;	/* type of evidence */
    int axtScore;	/* blastz score, gene mrna aligned to pseudogene */
    char *gChrom;	/* Chromosome name */
    int gStart;	/* gene alignment start position */
    int gEnd;	/* gene alignment end position */
    char gStrand[3];	/* strand of gene */
    unsigned exonCount;	/* # of exons in gene  */
    unsigned geneOverlap;	/* bases overlapping */
    unsigned polyA;	/* count of As in polyA */
    int polyAstart;	/* start of polyA, relative to end of pseudogene */
    unsigned exonCover;	/* number of exons in Gene covered */
    unsigned intronCount;	/* number of introns in pseudogene */
    unsigned bestAliCount;	/* number of good mrnas aligning */
    unsigned matches;	/* matches + repMatches */
    unsigned qSize;	/* aligning bases in pseudogene */
    unsigned qEnd;	/* end of cdna alignment */
    unsigned tReps;	/* repeats in gene */
    unsigned qReps;	/* repeats in pseudogene */
    unsigned overlapDiag;	/* bases on the diagonal to mouse */
    unsigned coverage;	/* bases on the diagonal to mouse */
    int label;	/* 1=pseudogene,-1 not pseudogene */
    unsigned milliBad;	/* milliBad score, pseudogene aligned to genome */
    unsigned oldScore;	/* another heuristic */
    int oldIntronCount;	/* old simple intron count */
    int conservedIntrons;	/* conserved intron count */
    char *intronScores;	/* Intron sizes in gene/pseudogene */
    int maxOverlap;	/* largest overlap with another mrna */
    char *refSeq;	/* Name of closest regSeq to gene */
    int rStart;	/* refSeq alignment start position */
    int rEnd;	/* refSeq alignment end position */
    char *mgc;	/* Name of closest mgc to gene */
    int mStart;	/* mgc alignment start position */
    int mEnd;	/* mgc alignment end position */
    char *kgName;	/* Name of closest knownGene to gene */
    int kStart;	/* kg alignment start position */
    int kEnd;	/* kg alignment end position */
    char *overName;	/* name of overlapping mrna */
    int overStart;	/* overlapping mrna start position */
    int overEnd;	/* overlapping mrna end position */
    char overStrand[3];	/* strand of overlapping mrna */
    int adaBoost;	/* adaBoost label */
    float posConf;	/* pvalue for positive */
    unsigned polyAlen;	/* length of polyA */
    };

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

