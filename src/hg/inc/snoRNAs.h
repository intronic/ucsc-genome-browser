/* snoRNAs.h was originally generated by the autoSql program, which also 
 * generated snoRNAs.c and snoRNAs.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef SNORNAS_H
#define SNORNAS_H

#define SNORNAS_NUM_COLS 18

struct snoRNAs
/* C/D box snoRNA genes */
    {
    struct snoRNAs *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* snoRNA RNA gene name */
    unsigned score;	/* Score from 900-1000.  1000 is best */
    char strand[2];	/* Value should be + or - */
    float snoScore;	/* snoscan score */
    char *targetList;	/* Target RNAs */
    char *orthologs;	/* Possible gene orthologs in closely-related species */
    char *guideLen;	/* Length of guide region */
    char *guideStr;	/* Guide region string */
    char *guideScore;	/* Guide region score */
    char *cBox;	/* C box feature */
    char *dBox;	/* D box feature */
    char *cpBox;	/* C' box feature */
    char *dpBox;	/* D' box feature */
    float hmmScore;	/* score to HMM snoRNA model */
    char *snoscanOutput;	/* Full snoscan output for snoRNA */
    };

void snoRNAsStaticLoad(char **row, struct snoRNAs *ret);
/* Load a row from snoRNAs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snoRNAs *snoRNAsLoad(char **row);
/* Load a snoRNAs from row fetched with select * from snoRNAs
 * from database.  Dispose of this with snoRNAsFree(). */

struct snoRNAs *snoRNAsLoadAll(char *fileName);
/* Load all snoRNAs from whitespace-separated file.
 * Dispose of this with snoRNAsFreeList(). */

struct snoRNAs *snoRNAsLoadAllByChar(char *fileName, char chopper);
/* Load all snoRNAs from chopper separated file.
 * Dispose of this with snoRNAsFreeList(). */

#define snoRNAsLoadAllByTab(a) snoRNAsLoadAllByChar(a, '\t');
/* Load all snoRNAs from tab separated file.
 * Dispose of this with snoRNAsFreeList(). */

struct snoRNAs *snoRNAsCommaIn(char **pS, struct snoRNAs *ret);
/* Create a snoRNAs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snoRNAs */

void snoRNAsFree(struct snoRNAs **pEl);
/* Free a single dynamically allocated snoRNAs such as created
 * with snoRNAsLoad(). */

void snoRNAsFreeList(struct snoRNAs **pList);
/* Free a list of dynamically allocated snoRNAs's */

void snoRNAsOutput(struct snoRNAs *el, FILE *f, char sep, char lastSep);
/* Print out snoRNAs.  Separate fields with sep. Follow last field with lastSep. */

#define snoRNAsTabOut(el,f) snoRNAsOutput(el,f,'\t','\n');
/* Print out snoRNAs as a line in a tab-separated file. */

#define snoRNAsCommaOut(el,f) snoRNAsOutput(el,f,',',',');
/* Print out snoRNAs as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SNORNAS_H */

