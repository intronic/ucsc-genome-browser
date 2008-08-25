/* encodePeak.h was originally generated by the autoSql program, which also 
 * generated encodePeak.c and encodePeak.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ENCODEPEAK_H
#define ENCODEPEAK_H

#define ENCODEPEAK_NUM_COLS 9

struct encodePeak
/* Peaks format (BED 3+) */
    {
    struct encodePeak *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    float signalValue;	/* Measurement of average enrichment for the region */
    float pValue;	/* Statistical significance of signal value (-log10) */
    int peak;	/* Point-source called for this peak; 0-based offset from chromStart (use -1 if no point-source called) */
    unsigned blockCount;	/* Number of blocks */
    unsigned *blockSizes;	/* Comma separated list of block sizes */
    unsigned *chromStarts;	/* Start positions relative to chromStart */
    };

struct encodePeak *encodePeakLoad(char **row);
/* Load a encodePeak from row fetched with select * from encodePeak
 * from database.  Dispose of this with encodePeakFree(). */

struct encodePeak *encodePeakLoadAll(char *fileName);
/* Load all encodePeak from whitespace-separated file.
 * Dispose of this with encodePeakFreeList(). */

struct encodePeak *encodePeakLoadAllByChar(char *fileName, char chopper);
/* Load all encodePeak from chopper separated file.
 * Dispose of this with encodePeakFreeList(). */

#define encodePeakLoadAllByTab(a) encodePeakLoadAllByChar(a, '\t');
/* Load all encodePeak from tab separated file.
 * Dispose of this with encodePeakFreeList(). */

struct encodePeak *encodePeakCommaIn(char **pS, struct encodePeak *ret);
/* Create a encodePeak out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodePeak */

void encodePeakFree(struct encodePeak **pEl);
/* Free a single dynamically allocated encodePeak such as created
 * with encodePeakLoad(). */

void encodePeakFreeList(struct encodePeak **pList);
/* Free a list of dynamically allocated encodePeak's */

void encodePeakOutput(struct encodePeak *el, FILE *f, char sep, char lastSep);
/* Print out encodePeak.  Separate fields with sep. Follow last field with lastSep. */

#define encodePeakTabOut(el,f) encodePeakOutput(el,f,'\t','\n');
/* Print out encodePeak as a line in a tab-separated file. */

#define encodePeakCommaOut(el,f) encodePeakOutput(el,f,',',',');
/* Print out encodePeak as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

/* Handy little enum for classification of the different peak types. */
enum encodePeakType
{
    narrowPeak = 1,
    broadPeak = 2,
    gappedPeak = 3,
    encodePeak5 = 4,
    encodePeak6 = 5,
    encodePeak9 = 6
};

int encodePeakNumFields(char *trackName);
/* Just quickly count th number of fields. */

#endif /* ENCODEPEAK_H */

