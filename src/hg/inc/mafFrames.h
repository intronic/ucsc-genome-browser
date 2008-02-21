/* mafFrames.h was originally generated by the autoSql program, which also 
 * generated mafFrames.c and mafFrames.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MAFFRAMES_H
#define MAFFRAMES_H

#define MAFFRAMES_NUM_COLS 11

struct mafFrames
/* codon frame assignment for MAF components */
    {
    struct mafFrames *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start range in chromosome */
    unsigned chromEnd;	/* End range in chromosome */
    char *src;	/* Name of sequence source in MAF */
    unsigned char frame;	/* frame (0,1,2) for first base(+) or last bast(-) */
    char strand[2];	/* + or - */
    char *name;	/* Name of gene used to define frame */
    int prevFramePos;	/* target position of the previous base (in transcription direction) that continues this frame, or -1 if none, or frame not contiguous */
    int nextFramePos;	/* target position of the next base (in transcription direction) that continues this frame, or -1 if none, or frame not contiguous */
    unsigned char isExonStart;	/* does this start the CDS portion of an exon? */
    unsigned char isExonEnd;	/* does this end the CDS portion of an exon? */
    };

void mafFramesStaticLoad(char **row, struct mafFrames *ret);
/* Load a row from mafFrames table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct mafFrames *mafFramesLoad(char **row);
/* Load a mafFrames from row fetched with select * from mafFrames
 * from database.  Dispose of this with mafFramesFree(). */

struct mafFrames *mafFramesLoadAll(char *fileName);
/* Load all mafFrames from whitespace-separated file.
 * Dispose of this with mafFramesFreeList(). */

struct mafFrames *mafFramesLoadAllByChar(char *fileName, char chopper);
/* Load all mafFrames from chopper separated file.
 * Dispose of this with mafFramesFreeList(). */

#define mafFramesLoadAllByTab(a) mafFramesLoadAllByChar(a, '\t');
/* Load all mafFrames from tab separated file.
 * Dispose of this with mafFramesFreeList(). */

struct mafFrames *mafFramesCommaIn(char **pS, struct mafFrames *ret);
/* Create a mafFrames out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mafFrames */

void mafFramesFree(struct mafFrames **pEl);
/* Free a single dynamically allocated mafFrames such as created
 * with mafFramesLoad(). */

void mafFramesFreeList(struct mafFrames **pList);
/* Free a list of dynamically allocated mafFrames's */

void mafFramesOutput(struct mafFrames *el, FILE *f, char sep, char lastSep);
/* Print out mafFrames.  Separate fields with sep. Follow last field with lastSep. */

#define mafFramesTabOut(el,f) mafFramesOutput(el,f,'\t','\n');
/* Print out mafFrames as a line in a tab-separated file. */

#define mafFramesCommaOut(el,f) mafFramesOutput(el,f,',',',');
/* Print out mafFrames as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

char *mafFramesGetSql(char *table, unsigned options, int chromIdxLen);
/* Get sql to create the table.  Returned string should be freed.  No options
 * are currently defined.*/

#endif /* MAFFRAMES_H */

