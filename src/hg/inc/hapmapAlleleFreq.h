/* hapmapAlleleFreq.h was originally generated by the autoSql program, which also 
 * generated hapmapAlleleFreq.c and hapmapAlleleFreq.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HAPMAPALLELEFREQ_H
#define HAPMAPALLELEFREQ_H

#define HAPMAPALLELEFREQ_NUM_COLS 14

struct hapmapAlleleFreq
/* HapMap SNPs */
    {
    struct hapmapAlleleFreq *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position */
    unsigned chromEnd;	/* End position */
    char *name;	/* rsId or id */
    unsigned score;	/* Minor allele frequency from 500 to 1000 */
    char strand[2];	/* Strand */
    char *center;	/* Sequencing center */
    char refAllele[2];	/* Reference allele */
    char otherAllele[2];	/* Variant allele */
    float refAlleleFreq;	/* Reference allele frequency (between 0.0 and 1.0) */
    float otherAlleleFreq;	/* Variant allele frequency (between 0.0 and 1.0) */
    float minorAlleleFreq;	/* Smaller of the 2 frequencies (between 0.0 and 0.5) */
    unsigned totalCount;	/* Count of individuals */
    float derivedAlleleFreq;	/* Derived allele frequency */
    };

void hapmapAlleleFreqStaticLoad(char **row, struct hapmapAlleleFreq *ret);
/* Load a row from hapmapAlleleFreq table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hapmapAlleleFreq *hapmapAlleleFreqLoad(char **row);
/* Load a hapmapAlleleFreq from row fetched with select * from hapmapAlleleFreq
 * from database.  Dispose of this with hapmapAlleleFreqFree(). */

struct hapmapAlleleFreq *hapmapAlleleFreqLoadAll(char *fileName);
/* Load all hapmapAlleleFreq from whitespace-separated file.
 * Dispose of this with hapmapAlleleFreqFreeList(). */

struct hapmapAlleleFreq *hapmapAlleleFreqLoadAllByChar(char *fileName, char chopper);
/* Load all hapmapAlleleFreq from chopper separated file.
 * Dispose of this with hapmapAlleleFreqFreeList(). */

#define hapmapAlleleFreqLoadAllByTab(a) hapmapAlleleFreqLoadAllByChar(a, '\t');
/* Load all hapmapAlleleFreq from tab separated file.
 * Dispose of this with hapmapAlleleFreqFreeList(). */

struct hapmapAlleleFreq *hapmapAlleleFreqCommaIn(char **pS, struct hapmapAlleleFreq *ret);
/* Create a hapmapAlleleFreq out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hapmapAlleleFreq */

void hapmapAlleleFreqFree(struct hapmapAlleleFreq **pEl);
/* Free a single dynamically allocated hapmapAlleleFreq such as created
 * with hapmapAlleleFreqLoad(). */

void hapmapAlleleFreqFreeList(struct hapmapAlleleFreq **pList);
/* Free a list of dynamically allocated hapmapAlleleFreq's */

void hapmapAlleleFreqOutput(struct hapmapAlleleFreq *el, FILE *f, char sep, char lastSep);
/* Print out hapmapAlleleFreq.  Separate fields with sep. Follow last field with lastSep. */

#define hapmapAlleleFreqTabOut(el,f) hapmapAlleleFreqOutput(el,f,'\t','\n');
/* Print out hapmapAlleleFreq as a line in a tab-separated file. */

#define hapmapAlleleFreqCommaOut(el,f) hapmapAlleleFreqOutput(el,f,',',',');
/* Print out hapmapAlleleFreq as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* HAPMAPALLELEFREQ_H */

