/* altSpliceSite.h was originally generated by the autoSql program, which also 
 * generated altSpliceSite.c and altSpliceSite.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ALTSPLICESITE_H
#define ALTSPLICESITE_H

#define ALTSPLICESITE_NUM_COLS 18

struct altSpliceSite
/* Structre to hold information about one splice site in graph. */
{
    struct altSpliceSite *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome. */
    unsigned chromStart;	/* Chrom start. */
    unsigned chromEnd;	/* End. */
    char *agName;	/* Name of altGraphX that edge is from.*/ 
    char strand[3];	/* Strand. */
    unsigned index;	/* Index into altGraphX records. */
    unsigned type;	/* Type of splice site i.e. ggHardStart,ggHardEnd. */
    unsigned altMax; /* Maximum number of altCount. */
    unsigned altCount;	/* Number of alternative ways out of splice site. */
    unsigned *vIndexes;	/* Index into altGraphX record for alt vertexes. */
    unsigned *altStarts;	/* Chromosome starts of alternative outs. */
    unsigned *altTypes;	/* Types of vertexes connecting to. */
    unsigned *spliceTypes;	/* Types of of splice sites. */
    unsigned *support;	/* Number of mRNAs supporting this edge. */
    unsigned *altBpStarts;	/* Start of alternatively spliced base pairs, type independent. */
    unsigned *altBpEnds;	/* End of alternatively spliced base pairs, type independent. */
    float *altCons;	/* Average s-score of altSpilce region. */
    float *upStreamCons;	/* Average s-score 100bp to start of altSpilce. */
    float *downStreamCons;	/* Average s-score 100bp past end of altSpilce. */
    };

struct altSpliceSite *altSpliceSiteLoad(char **row);
/* Load a altSpliceSite from row fetched with select * from altSpliceSite
 * from database.  Dispose of this with altSpliceSiteFree(). */

struct altSpliceSite *altSpliceSiteLoadAll(char *fileName);
/* Load all altSpliceSite from whitespace-separated file.
 * Dispose of this with altSpliceSiteFreeList(). */

struct altSpliceSite *altSpliceSiteLoadAllByChar(char *fileName, char chopper);
/* Load all altSpliceSite from chopper separated file.
 * Dispose of this with altSpliceSiteFreeList(). */

#define altSpliceSiteLoadAllByTab(a) altSpliceSiteLoadAllByChar(a, '\t');
/* Load all altSpliceSite from tab separated file.
 * Dispose of this with altSpliceSiteFreeList(). */

struct altSpliceSite *altSpliceSiteCommaIn(char **pS, struct altSpliceSite *ret);
/* Create a altSpliceSite out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new altSpliceSite */

void altSpliceSiteFree(struct altSpliceSite **pEl);
/* Free a single dynamically allocated altSpliceSite such as created
 * with altSpliceSiteLoad(). */

void altSpliceSiteFreeList(struct altSpliceSite **pList);
/* Free a list of dynamically allocated altSpliceSite's */

void altSpliceSiteOutput(struct altSpliceSite *el, FILE *f, char sep, char lastSep);
/* Print out altSpliceSite.  Separate fields with sep. Follow last field with lastSep. */

#define altSpliceSiteTabOut(el,f) altSpliceSiteOutput(el,f,'\t','\n');
/* Print out altSpliceSite as a line in a tab-separated file. */

#define altSpliceSiteCommaOut(el,f) altSpliceSiteOutput(el,f,',',',');
/* Print out altSpliceSite as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ALTSPLICESITE_H */

