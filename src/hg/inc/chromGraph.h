/* chromGraph.h was originally generated by the autoSql program, which also 
 * generated chromGraph.c and chromGraph.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CHROMGRAPH_H
#define CHROMGRAPH_H

#ifndef TRACKDB_H
#include "trackDb.h"
#endif

#ifndef CART_H
#include "cart.h"
#endif

#define CHROMGRAPH_NUM_COLS 3

struct chromGraph
/* A graph over a chromosome - best used for low resolution graphs */
    {
    struct chromGraph *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    int chromStart;	/* Start coordinate */
    double val;	/* Value at coordinate */
    };

void chromGraphStaticLoad(char **row, struct chromGraph *ret);
/* Load a row from chromGraph table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct chromGraph *chromGraphLoad(char **row);
/* Load a chromGraph from row fetched with select * from chromGraph
 * from database.  Dispose of this with chromGraphFree(). */

struct chromGraph *chromGraphLoadAll(char *fileName);
/* Load all chromGraph from whitespace-separated file.
 * Dispose of this with chromGraphFreeList(). */

struct chromGraph *chromGraphLoadAllByChar(char *fileName, char chopper);
/* Load all chromGraph from chopper separated file.
 * Dispose of this with chromGraphFreeList(). */

#define chromGraphLoadAllByTab(a) chromGraphLoadAllByChar(a, '\t');
/* Load all chromGraph from tab separated file.
 * Dispose of this with chromGraphFreeList(). */

struct chromGraph *chromGraphCommaIn(char **pS, struct chromGraph *ret);
/* Create a chromGraph out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chromGraph */

void chromGraphFree(struct chromGraph **pEl);
/* Free a single dynamically allocated chromGraph such as created
 * with chromGraphLoad(). */

void chromGraphFreeList(struct chromGraph **pList);
/* Free a list of dynamically allocated chromGraph's */

void chromGraphOutput(struct chromGraph *el, FILE *f, char sep, char lastSep);
/* Print out chromGraph.  Separate fields with sep. Follow last field with lastSep. */

#define chromGraphTabOut(el,f) chromGraphOutput(el,f,'\t','\n');
/* Print out chromGraph as a line in a tab-separated file. */

#define chromGraphCommaOut(el,f) chromGraphOutput(el,f,',',',');
/* Print out chromGraph as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

int chromGraphCmp(const void *va, const void *vb);
/* Compare to sort based on query chromStart. */

struct chromGraphSettings
/* Settings */
    {
    int maxGapToFill;		/* Maximum gap to fill with line */
    double minVal, maxVal;	/* Max/min data range */
    int linesAtCount;		/* Number of horizontal labeling lines */
    double *linesAt;		/* Data values to label with horizontal line */
    int minPixels;		/* Minimum allowed pixels. */
    int pixels;			/* Actual pixels. */
    int maxPixels;		/* Maximum allowed pixels. */
    };

struct chromGraphSettings *chromGraphSettingsGet(char *trackName,
	struct sqlConnection *conn, struct trackDb *tdb, struct cart *cart);
/* Get settings for chromGraph track.  If you pass in all NULLs
 * you'll get a reasonable default. */

#define chromGraphVarNameMaxSize 512

void chromGraphVarName(char *track, char *var, 
	char output[chromGraphVarNameMaxSize]);
/* Fill in output with name of cart variable. */

void chromGraphDataRange(char *trackName, struct sqlConnection *conn,
	double *retMin, double *retMax);
/* Get min/max values observed from metaChromGraph table */

struct slName *chromGraphListAll(struct sqlConnection *conn);
/* Return list of all chrom graph tables. */
	
char *chromGraphBinaryFileName(char *trackName, struct sqlConnection *conn);
/* Get binary file name associated with chromGraph track. Returns NULL
 * if no such file or track. FreeMem result when done. */

void chromGraphToBin(struct chromGraph *list, char *fileName);
/* Create binary representation of chromGraph list, which should
 * be sorted. */

/* chromGraphBin interface.  The general calling sequence is:
 *   struct chromGraphBin *cgb = chromGraphBinOpen(path);
 *   while (chromGraphBinNextChrom(cgb))
 *      {
 *      char *chrom = cgb->chrom;
 *      while (chromGraphBinNextVal(cgb))
 *          {
 *          // process cgb->pos, cgb->val 
 *          }
 *      }
 *     chromGraphBinFree(&cgb);
 */
	  
struct chromGraphBin
/* A handle to binary representation to chrom graph */
    {
    struct chromGraphBin *next;	/* Next in list if any */
    char *fileName;	/* Name of file. */
    FILE *f;		/* File handle. */
    boolean isSwapped;	/* Need to swap? */
    char chrom[256];	/* Current chromosome. */
    bits32 chromStart;	/* Current position. */
    double val;		/* Current value. */
    };

struct chromGraphBin *chromGraphBinOpen(char *path);
/* Open up a chromGraphBin file */

void chromGraphBinFree(struct chromGraphBin **pCgb);
/* Close down and free up chromGraphBin. */

boolean chromGraphBinNextChrom(struct chromGraphBin *cgb);
/* Fetch next chromosome, or FALSE if at end of file. */

boolean chromGraphBinNextVal(struct chromGraphBin *cgb);
/* Fetch next pos/val or FALSE if at end of chromosome. */

#endif /* CHROMGRAPH_H */

