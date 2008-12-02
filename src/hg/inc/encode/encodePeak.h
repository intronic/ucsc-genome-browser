/* encodePeak.h was originally generated by the autoSql program, which also 
 * generated encodePeak.c and encodePeak.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ENCODEPEAK_H
#define ENCODEPEAK_H

#ifndef TRACKDB_H
#include "trackDb.h"
#endif

#define ENCODEPEAK_NUM_COLS 13

struct encodePeak
/* Peaks format (BED 6+) */
    {
    struct encodePeak *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* The name... probably just a period */
    unsigned score;	/* Score 1-1000 */
    char strand[3];	/* +, -, or . */
    float signalValue;	/* Measurement of average enrichment for the region */
    float pValue;	/* Statistical significance of signal value (-log10) */
    float qValue;	/* Statistical significance normalized/corrected across datasets. Use -1 if no qValue is assigned. */
    int peak;	/* Point-source called for this peak; 0-based offset from chromStart (use -1 if no point-source called) */
    unsigned blockCount;	/* Number of blocks */
    unsigned *blockSizes;	/* Comma separated list of block sizes */
    unsigned *blockStarts;	/* Start positions relative to chromStart */
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

#define ENCODE_PEAK_KNOWN_FIELDS 15

#define ENCODE_PEAK_NARROW_PEAK_FIELDS 10
#define ENCODE_PEAK_BROAD_PEAK_FIELDS 9
#define ENCODE_PEAK_GAPPED_PEAK_FIELDS 15

/* Handy little enum for classification of the different peak types. */
enum encodePeakType
{
    invalid = 0,
    narrowPeak = 1,
    broadPeak = 2,
    gappedPeak = 3,
    encodePeak = 4,
};

enum encodePeakType encodePeakInferType(int numFields, char *type);
/* Given a track type string and number of fields in a row, infer the type */

int encodePeakNumFields(char *db, char *trackName);
/* Just quickly count the number of fields. */

enum encodePeakType encodePeakInferTypeFromTable(char *db, char *table, char *tdbType);
/* Given the trackDb figure out the peak type. Returns zero on error. */ 

struct encodePeak *encodePeakGeneralLoad(char **row, enum encodePeakType pt);
/* Make a new encodePeak and return it. */

struct encodePeak *narrowPeakLoad(char **row);
/* Load a narrowPeak */

struct encodePeak *broadPeakLoad(char **row);
/* Load a broadPeak */

struct encodePeak *gappedPeakLoad(char **row);
/* Load a gappedPeak */

struct encodePeak *encodePeakLineFileLoad(char **row, enum encodePeakType pt, struct lineFile *lf);
/* From a linefile line, load an encodePeak row.  Errors outputted */
/* have line numbers, etc. Does more error checking as well. */

void encodePeakOutputWithType(struct encodePeak *el, enum encodePeakType pt, FILE *f);
/* Print out encodePeak different ways depending on narrowPeak, broadPeak, etc. */
/* but make it tab-separated. */

#define ENCODE_PEAK_PVAL_FILTER_SUFFIX "pValFilter"
#define ENCODE_PEAK_QVAL_FILTER_SUFFIX "qValFilter"
#define ENCODE_PEAK_SCORE_FILTER_SUFFIX "scoreFilter"

#endif /* ENCODEPEAK_H */
