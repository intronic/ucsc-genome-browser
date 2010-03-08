/* pgSiftPred.h was originally generated by the autoSql program, which also 
 * generated pgSiftPred.c and pgSiftPred.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef PGSIFTPRED_H
#define PGSIFTPRED_H

#define PGSIFTPRED_NUM_COLS 11

struct pgSiftPred
/* sift predictions for pgSnp tracks */
    {
    struct pgSiftPred *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned *chromStart;	/* Start position in chrom */
    unsigned *chromEnd;	/* End position in chrom */
    char *prediction;	/* sifts prediction, damaging, tolerated,... */
    char *geneId;	/* gene ID, Ensembl */
    char *geneName;	/* gene name */
    char *geneDesc;	/* gene description */
    char *protFamDesc;	/* protein family description */
    char *omimDisease;	/* OMIM disease */
    char *aveAlleleFreq;	/* Average allele frequencies */
    char *ceuAlleleFreq;	/* Caucasian allele frequencies */
    };

void pgSiftPredStaticLoadWithNull(char **row, struct pgSiftPred *ret);
/* Load a row from pgSiftPred table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct pgSiftPred *pgSiftPredLoadWithNull(char **row);
/* Load a pgSiftPred from row fetched with select * from pgSiftPred
 * from database.  Dispose of this with pgSiftPredFree(). */

struct pgSiftPred *pgSiftPredLoadAll(char *fileName);
/* Load all pgSiftPred from whitespace-separated file.
 * Dispose of this with pgSiftPredFreeList(). */

struct pgSiftPred *pgSiftPredLoadAllByChar(char *fileName, char chopper);
/* Load all pgSiftPred from chopper separated file.
 * Dispose of this with pgSiftPredFreeList(). */

#define pgSiftPredLoadAllByTab(a) pgSiftPredLoadAllByChar(a, '\t');
/* Load all pgSiftPred from tab separated file.
 * Dispose of this with pgSiftPredFreeList(). */

struct pgSiftPred *pgSiftPredCommaIn(char **pS, struct pgSiftPred *ret);
/* Create a pgSiftPred out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pgSiftPred */

void pgSiftPredFree(struct pgSiftPred **pEl);
/* Free a single dynamically allocated pgSiftPred such as created
 * with pgSiftPredLoad(). */

void pgSiftPredFreeList(struct pgSiftPred **pList);
/* Free a list of dynamically allocated pgSiftPred's */

void pgSiftPredOutput(struct pgSiftPred *el, FILE *f, char sep, char lastSep);
/* Print out pgSiftPred.  Separate fields with sep. Follow last field with lastSep. */

#define pgSiftPredTabOut(el,f) pgSiftPredOutput(el,f,'\t','\n');
/* Print out pgSiftPred as a line in a tab-separated file. */

#define pgSiftPredCommaOut(el,f) pgSiftPredOutput(el,f,',',',');
/* Print out pgSiftPred as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */
#include "pgSnp.h"

void printPgSiftPred (char *db, char *tableName, struct pgSnp *item);
/* print the predictions for an hgc item click for a pgSnp track */

#endif /* PGSIFTPRED_H */

