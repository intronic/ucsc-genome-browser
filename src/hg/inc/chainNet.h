/* chainNet - read/write and free nets which are constructed
 * of chains of genomic alignments. */

#ifndef CHAINNET_H
#define CHAINNET_H

struct chainNet
/* A net on one chromosome. */
    {
    struct chainNet *next;
    char *name;			/* Chromosome name. */
    int size;			/* Chromosome size. */
    struct cnFill *fillList;	/* Top level fills. */
    struct hash *nameHash; 	/* Hash of all strings in fillList. */
    };

struct cnFill
/* Filling sequence or a gap. */
    {
	/* Required fields */
    struct cnFill *next;	   /* Next in list. */
    int tStart, tSize;	   /* Range in target chromosome. */
    char *qName;	   /* Other chromosome (not allocated here) */
    char qStrand;	   /* Orientation + or - in other chrom. */
    int qStart,	qSize;	   /* Range in query chromosome. */
    struct cnFill *children; /* List of child gaps. */
	/* Optional fields. */
    int chainId;   /* Chain id.  0 for a gap. */
    double score;  /* Score of associated chain. 0 if undefined. */
    int tN;	   /* Count of N's in target chromosome or -1 */
    int qN;	   /* Count of N's in query chromosome or -1 */
    int tR;	   /* Count of repeats in target chromosome or -1 */
    int qR;	   /* Count of repeats in query chromosome or -1 */
    int tNewR;	   /* Count of new (lineage specific) repeats in target */
    int qNewR;	   /* Count of new (lineage specific) repeats in query */
    int tOldR;	   /* Count of ancient repeats (pre-split) in target */
    int qOldR;	   /* Count of ancient repeats (pre-split) in query */
    int qTrf;	   /* Count of simple repeats, period 12 or less. */
    int tTrf;	   /* Count of simple repeats, period 12 or less. */
    char *type;    /* One word description.  Not allocated here. */
    };

void chromNetFree(struct chainNet **pNet);
/* Free up a chromosome net. */

void chromNetFreeList(struct chainNet **pList);
/* Free up a list of chainNet. */

void chromNetWrite(struct chainNet *net, FILE *f);
/* Write out chain net. */

struct chainNet *chromNetRead(struct lineFile *lf);
/* Read next net from file. Return NULL at end of file.*/


struct cnFill *cnFillNew();
/* Return fill structure with some basic stuff filled in */

void cnFillFree(struct cnFill **pFill);
/* Free up a fill structure and all of it's children. */

void cnFillFreeList(struct cnFill **pList);
/* Free up a list of fills. */

struct cnFill *cnFillRead(struct chainNet *net, struct lineFile *lf);
/* Recursively read in list and children from file. */

void cnFillWrite(struct cnFill *fillList, FILE *f, int depth);
/* Recursively write out fill list. */

#endif /* CHAINNET_H */


