/* bed.h was originally generated by the autoSql program, which also 
 * generated bed.c and bed.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef BED_H
#define BED_H

#ifndef PSL_H
#include "psl.h"
#endif

#ifndef GENEPRED_H
#include "genePred.h"
#endif

#ifndef HASH_H
#include "hash.h"
#endif

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


    int expCount;	/* Experiment count */
    int *expIds;		/* Comma separated list of Experiment ids */
    float *expScores;	/* Comma separated list of Experiment scores. */
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

int bedCmpPlusScore(const void *va, const void *vb);
/* Compare to sort based on chrom, chromStart and score - lowest first. */

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

struct bed *bedLoadNAllChrom(char *fileName, int numFields, char* chrom);
/* Load bed entries from a tab-separated file that have the given chrom.
 * Dispose of this with bedFreeList(). */

struct bed *bedLoadNAll(char *fileName, int numFields);
/* Load all bed from a tab-separated file.
 * Dispose of this with bedFreeList(). */

struct bed *bedLoadAll(char *fileName);
/* Determines how many fields are in a bedFile and load all beds from
 * a tab-separated file.  Dispose of this with bedFreeList(). */

struct bed *bedLoadNBin(char *row[], int wordCount);
/* Convert a row of strings to a bed. */

void bedOutputN(struct bed *el, int wordCount, FILE *f, char sep, char lastSep);
/* Write a bed of wordCount fields. */

#define bedTabOutN(el,wordCount, f) bedOutputN(el,wordCount,f,'\t','\n')
/* Print out bed as a line in a tab-separated file. */

#define bedCommaOutN(el,wordCount, f) bedOutputN(el,wordCount,f,',',',')
/* Print out bed as a comma separated list including final comma. */

struct bed *bedFromPsl(struct psl *psl);
/* Convert a single psl to a bed structure */

struct bed *bedFromGenePred(struct genePred *genePred);
/* Convert a single genePred to a bed structure */

struct bed *cloneBed(struct bed *bed);
/* Make an all-newly-allocated copy of a single bed record. */

struct bed *cloneBedList(struct bed *bed);
/* Make an all-newly-allocated list copied from bed. */

/* Constraints that can be placed on bed fields: */
enum charFilterType
    {
    cftIgnore = 0,
    cftSingleLiteral = 1,
    cftMultiLiteral = 2,
    };
enum stringFilterType
    {
    sftIgnore = 0,
    sftSingleLiteral = 1,
    sftMultiLiteral = 2,
    sftSingleRegexp = 3,
    sftMultiRegexp = 4,
    };
enum numericFilterType
    {
    nftIgnore = 0,
    nftLessThan = 1,
    nftLTE = 2,
    nftEqual = 3,
    nftNotEqual = 4,
    nftGTE = 5,
    nftGreaterThan = 6,
    nftInRange = 7,
    nftNotInRange = 8,
    };
struct bedFilter
    {
    enum stringFilterType chromFilter;
    char **chromVals;
    boolean chromInvert;
    enum numericFilterType chromStartFilter;
    int *chromStartVals;
    enum numericFilterType chromEndFilter;
    int *chromEndVals;
    enum stringFilterType nameFilter;
    char **nameVals;
    boolean nameInvert;
    enum numericFilterType scoreFilter;
    int *scoreVals;
    enum charFilterType strandFilter;
    char *strandVals;
    boolean strandInvert;
    enum numericFilterType thickStartFilter;
    int *thickStartVals;
    enum numericFilterType thickEndFilter;
    int *thickEndVals;
    enum numericFilterType blockCountFilter;
    int *blockCountVals;
    enum numericFilterType chromLengthFilter;
    int *chromLengthVals;
    enum numericFilterType thickLengthFilter;
    int *thickLengthVals;
    enum numericFilterType compareStartsFilter;
    enum numericFilterType compareEndsFilter;
    };

struct bed *bedFilterListInRange(struct bed *bedListIn, struct bedFilter *bf,
				 char *chrom, int winStart, int winEnd);
/* Given a bed list, a position range, and a bedFilter which specifies
 * constraints on bed fields, return the list of bed items that meet
 * the constraints.  If chrom is NULL, position range is ignored. */

struct bed *bedFilterList(struct bed *bedListIn, struct bedFilter *bf);
/* Given a bed list and a bedFilter which specifies constraints on bed 
 * fields, return the list of bed items that meet the constraints. */

struct bed *bedFilterByNameHash(struct bed *bedList, struct hash *nameHash);
/* Given a bed list and a hash of names to keep, return the list of bed 
 * items whose name is in nameHash. */

struct bed *bedFilterByWildNames(struct bed *bedList, struct slName *wildNames);
/* Given a bed list and a list of names that may include wildcard characters,
 * return the list of bed items whose name matches at least one wildName. */


struct bed *bedCommaInN(char **pS, struct bed *ret, int fieldCount);
/* Create a bed out of a comma separated string looking for fieldCount
 * fields. This will fill in ret if non-null, otherwise will return a
 * new bed */

struct hash *readBedToBinKeeper(char *sizeFileName, char *bedFileName, int wordCount);
/* read a list of beds and return results in hash of binKeeper structure for fast query*/


#endif /* BED_H */

