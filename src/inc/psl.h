/* psl.h was originally generated by the autoSql program, which also 
 * generated psl.c and psl.sql.  This header links the database and 
 * the RAM representation of objects.   Additional functions were
 * added later. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#ifndef PSL_H
#define PSL_H

#ifndef LOCALMEM_H
#include "localmem.h"
#endif 

#ifndef LINEFILE_H
#include "linefile.h"
#endif

#ifndef FUZZYFIND_H
#include "fuzzyFind.h"
#endif

#ifndef DNASEQ_H
#include "dnaseq.h"
#endif

struct psl
/* Summary info about a patSpace alignment */
    {
    struct psl *next;  /* Next in singly linked list. */
    unsigned match;	/* Number of bases that match that aren't repeats */
    unsigned misMatch;	/* Number of bases that don't match */
    unsigned repMatch;	/* Number of bases that match but are part of repeats */
    unsigned nCount;	/* Number of 'N' bases */
    unsigned qNumInsert;	/* Number of inserts in query */
    int qBaseInsert;	/* Number of bases inserted in query */
    unsigned tNumInsert;	/* Number of inserts in target */
    int tBaseInsert;	/* Number of bases inserted in target */
    char strand[3];	/* + or - for strand */
    char *qName;	/* Query sequence name */
    unsigned qSize;	/* Query sequence size */
    int qStart;	/* Alignment start position in query */
    int qEnd;	/* Alignment end position in query */
    char *tName;	/* Target sequence name */
    unsigned tSize;	/* Target sequence size */
    int tStart;	/* Alignment start position in target */
    int tEnd;	/* Alignment end position in target */
    unsigned blockCount;	/* Number of blocks in alignment */
    unsigned *blockSizes;	/* Size of each block */
    unsigned *qStarts;	/* Start of each block in query. */
    unsigned *tStarts;	/* Start of each block in target. */
    };

struct psl *pslLoad(char **row);
/* Load a psl from row fetched with select * from psl
 * from database.  Dispose of this with pslFree(). */

struct psl *pslCommaIn(char **pS, struct psl *ret);
/* Create a psl out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new psl */

void pslFree(struct psl **pEl);
/* Free a single dynamically allocated psl such as created
 * with pslLoad(). */

void pslFreeList(struct psl **pList);
/* Free a list of dynamically allocated psl's */

void pslOutput(struct psl *el, FILE *f, char sep, char lastSep);
/* Print out psl.  Separate fields with sep. Follow last field with lastSep. */

#define pslTabOut(el,f) pslOutput(el,f,'\t','\n')
/* Print out psl as a line in a tab-separated file. */

#define pslCommaOut(el,f) pslOutput(el,f,',',',')
/* Print out psl as a comma separated list including final comma. */

/* ----- end autoSql generated part --------------- */

struct psl *pslLoadAll(char *fileName);
/* Load all psl's in file. */

struct psl *pslNext(struct lineFile *lf);
/* Read next line from file and convert it to psl.  Return
 * NULL at eof. */

struct psl *pslLoadLm(char **row, struct lm *lm);
/* Load row into local memory psl. */

void pslWriteHead(FILE *f);
/* Write head of psl. */

void pslxWriteHead(FILE *f, enum gfType qType, enum gfType tType);
/* Write head of pslx (extended psl). */

void pslWriteAll(struct psl *pslList, char *fileName, boolean writeHeader);
/* Write a psl file from list. */

struct lineFile *pslFileOpen(char *fileName);
/* Read header part of psl and make sure it's right. 
 * Return line file handle to it. */

void pslxFileOpen(char *fileName, enum gfType *retQueryType, 
	enum gfType *retTargetType, struct lineFile **retLf);
/* Read header part of psl and make sure it's right.  Return
 * sequence types and file handle. */

int pslCmpQuery(const void *va, const void *vb);
/* Compare to sort based on query. */

int pslCmpTarget(const void *va, const void *vb);
/* Compare to sort based on target. */

int pslCalcMilliBad(struct psl *psl, boolean isMrna);
/* Calculate badness in parts per thousand. */

int pslScore(const struct psl *psl);
/* Return score for psl. */

struct ffAli *pslToFfAli(struct psl *psl, struct dnaSeq *query, struct dnaSeq *target,
	int targetOffset);
/* Convert from psl to ffAli format. */

struct ffAli *pslToFakeFfAli(struct psl *psl, DNA *needle, DNA *haystack);
/* Convert from psl to ffAli format.  In some cases you can pass NULL
 * for needle and haystack - depending what the post-processing is going
 * to be. */

struct psl *pslFromFakeFfAli(struct ffAli *ff, 
	DNA *needle, DNA *haystack, char strand,
	char *qName, int qSize, char *tName, int tSize);
/* This will create a basic psl structure from a sorted series of ffAli
 * blocks.  The fields that would need actual sequence to be filled in
 * are left zero however - fields including match, repMatch, mismatch. */

int pslOrientation(struct psl *psl);
/* Translate psl strand + or - to orientation +1 or -1 */

int pslWeightedIntronOrientation(struct psl *psl, struct dnaSeq *genoSeq, int offset);
/* Return >0 if introns make it look like alignment is on + strand,
 *        <0 if introns make it look like alignment is on - strand,
 *        0 if can't tell.  The absolute value of the return indicates
 * how many splice sites we've seen supporting the orientation. */

int pslIntronOrientation(struct psl *psl, struct dnaSeq *genoSeq, int offset);
/* Return 1 if introns make it look like alignment is on + strand,
 *       -1 if introns make it look like alignment is on - strand,
 *        0 if can't tell. */

void pslTailSizes(struct psl *psl, int *retStartTail, int *retEndTail);
/* Find the length of "tails" (rather than extensions) implied by psl. */

void pslRcBoth(struct psl *psl);
/* Swap around things in psl so it works as if the alignment
 * was done on the reverse strand of the target. */

void pslTargetOffset(struct psl *psl, int offset);
/* Add offset to target positions in psl. */

void pslDump(struct psl *psl, FILE *f);
/* Dump most of PSL to file - for debugging. */

struct psl *pslTrimToTargetRange(struct psl *oldPsl, int tMin, int tMax);
/* Return psl trimmed to fit inside tMin/tMax.  Note this does not
 * update the match/misMatch and related fields. */

#endif /* PSL_H */

