/* bed.h was originally generated by the autoSql program, which also 
 * generated bed.c and bed.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef BED_H
#define BED_H

struct bed
/* Browser extensible data */
    {
    struct bed *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */

    /* The following items are not loaded by   the bedLoad routines. */
    int score; /* Score - 0-1000 */
    char strand[2];  /* + or -.  */
    unsigned thickStart; /* Start of where display should be thick (start codon for genes) */
    unsigned thickEnd;   /* End of where display should be thick (stop codon for genes) */
    unsigned reserved;   /* Always zero for now. */
    unsigned blockCount; /* Number of blocks. */
    int *blockSizes;     /* Comma separated list of block sizes.  */
    int *chromStarts;    /* Start positions inside chromosome.  Relative to chromStart*/
    };

void bedStaticLoad(char **row, struct bed *ret);
/* Load a row from bed table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct bed *bedLoad(char **row);
/* Load a bed from row fetched with select * from bed
 * from database.  Dispose of this with bedFree(). 
 * This loads first four fields. */

struct bed *bedCommaIn(char **pS, struct bed *ret);
/* Create a bed out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed */

void bedFree(struct bed **pEl);
/* Free a single dynamically allocated bed such as created
 * with bedLoad(). */

void bedFreeList(struct bed **pList);
/* Free a list of dynamically allocated bed's */

void bedOutput(struct bed *el, FILE *f, char sep, char lastSep);
/* Print out bed.  Separate fields with sep. Follow last field with lastSep. */

#define bedTabOut(el,f) bedOutput(el,f,'\t','\n');
/* Print out bed as a line in a tab-separated file. */

#define bedCommaOut(el,f) bedOutput(el,f,',',',');
/* Print out bed as a comma separated list including final comma. */

/* --------------- End of AutoSQL generated code. --------------- */

int bedCmp(const void *va, const void *vb);
/* Compare to sort based on chrom,chromStart. */

int bedCmpScore(const void *va, const void *vb);
/* Compare to sort based on score - lowest first. */

struct bedLine
/* A line in a bed file with chromosome, start position parsed out. */
    {
    struct bedLine *next;	/* Next in list. */
    char *chrom;                /* Chromosome parsed out. */
    int chromStart;             /* Start position (still in rest of line). */
    char *line;                 /* Rest of line. */
    };

struct bedLine *bedLineNew(char *line);
/* Create a new bedLine based on tab-separated string s. */

void bedLineFree(struct bedLine **pBl);
/* Free up memory associated with bedLine. */

void bedLineFreeList(struct bedLine **pList);
/* Free a list of dynamically allocated bedLine's */

int bedLineCmp(const void *va, const void *vb);
/* Compare to sort based on chrom,chromStart. */

void bedSortFile(char *inFile, char *outFile);
/* Sort a bed file (in place, overwrites old file. */

struct bed *bedLoad3(char **row);
/* Load first three fields of bed. */

struct bed *bedLoad5(char **row);
/* Load first five fields of bed. */

struct bed *bedLoad12(char **row);
/* Load all 12 fields of bed. */

struct bed *bedLoadN(char *row[], int wordCount);
/* Convert a row of strings to a bed. */

void fullBedOutput(struct bed *el, FILE *f, char sep, char lastSep);
/* Print out all 12 fields of bed.   */

#endif /* BED_H */

