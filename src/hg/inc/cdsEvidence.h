/* cdsEvidence.h was originally generated by the autoSql program, which also 
 * generated cdsEvidence.c and cdsEvidence.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CDSEVIDENCE_H
#define CDSEVIDENCE_H

#ifndef BED_H
#include "bed.h"
#endif

#define CDSEVIDENCE_NUM_COLS 11

struct cdsEvidence
/* Evidence for CDS boundary within a transcript */
    {
    struct cdsEvidence *next;  /* Next in singly linked list. */
    char *name;	/* Name of transcript */
    int start;	/* CDS start within transcript, zero based */
    int end;	/* CDS end, non-inclusive */
    char *source;	/* Source of evidence */
    char *accession;	/* Genbank/uniProt accession */
    double score;	/* 0-1000, higher is better */
    unsigned char startComplete;	/* Starts with ATG? */
    unsigned char endComplete;	/* Ends with stop codon? */
    int cdsCount;	/* Number of CDS blocks */
    int *cdsStarts;	/* Start positions of CDS blocks */
    int *cdsSizes;	/* Sizes of CDS blocks */
    };

struct cdsEvidence *cdsEvidenceLoad(char **row);
/* Load a cdsEvidence from row fetched with select * from cdsEvidence
 * from database.  Dispose of this with cdsEvidenceFree(). */

struct cdsEvidence *cdsEvidenceLoadAll(char *fileName);
/* Load all cdsEvidence from whitespace-separated file.
 * Dispose of this with cdsEvidenceFreeList(). */

struct cdsEvidence *cdsEvidenceLoadAllByChar(char *fileName, char chopper);
/* Load all cdsEvidence from chopper separated file.
 * Dispose of this with cdsEvidenceFreeList(). */

#define cdsEvidenceLoadAllByTab(a) cdsEvidenceLoadAllByChar(a, '\t');
/* Load all cdsEvidence from tab separated file.
 * Dispose of this with cdsEvidenceFreeList(). */

struct cdsEvidence *cdsEvidenceCommaIn(char **pS, struct cdsEvidence *ret);
/* Create a cdsEvidence out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cdsEvidence */

void cdsEvidenceFree(struct cdsEvidence **pEl);
/* Free a single dynamically allocated cdsEvidence such as created
 * with cdsEvidenceLoad(). */

void cdsEvidenceFreeList(struct cdsEvidence **pList);
/* Free a list of dynamically allocated cdsEvidence's */

void cdsEvidenceOutput(struct cdsEvidence *el, FILE *f, char sep, char lastSep);
/* Print out cdsEvidence.  Separate fields with sep. Follow last field with lastSep. */

#define cdsEvidenceTabOut(el,f) cdsEvidenceOutput(el,f,'\t','\n');
/* Print out cdsEvidence as a line in a tab-separated file. */

#define cdsEvidenceCommaOut(el,f) cdsEvidenceOutput(el,f,',',',');
/* Print out cdsEvidence as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct hash *cdsEvidenceReadAllIntoHash(char *fileName);
/* Return hash full of cdsEvidence keyed by transcript name. */

void cdsEvidenceSetBedThick(struct cdsEvidence *cds, struct bed *bed, 
			    const boolean freeOfCdsErrors);
/* Set thickStart/thickEnd on bed from cdsEvidence. */

int cdsEvidenceCmpScore(const void *va, const void *vb);
/* Compare to sort based on score (descending). */

#endif /* CDSEVIDENCE_H */

