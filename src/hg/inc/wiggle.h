/* wiggle.h was originally generated by the autoSql program, which also 
 * generated wiggle.c and wiggle.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef WIGGLE_H
#define WIGGLE_H

#define WIGGLE_NUM_COLS 13

struct wiggle
/* Wiggle track values to display as y-values (first 6 fields are bed6) */
    {
    struct wiggle *next;	/* Next in singly linked list. */
    char *chrom;		/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;		/* End position in chromosome */
    char *name;			/* Name of item */
    unsigned span;	/* each value spans this many bases */
    unsigned count;	/* number of values in this block */
    unsigned offset;	/* offset in File to fetch data */
    char *file;	/* path name to data file, one byte per value */
    double lowerLimit;	/* lowest data value in this block */
    double dataRange;	/* lowerLimit + dataRange = upperLimit */
    unsigned validCount;	/* number of valid data values in this block */
    double sumData;	/* sum of the data points, for average and stddev calc */
    double sumSquares;	/* sum of data points squared, for stddev calc */
    };

void wiggleStaticLoad(char **row, struct wiggle *ret);
/* Load a row from wiggle table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wiggle *wiggleLoad(char **row);
/* Load a wiggle from row fetched with select * from wiggle
 * from database.  Dispose of this with wiggleFree(). */

struct wiggle *wiggleLoadAll(char *fileName);
/* Load all wiggle from whitespace-separated file.
 * Dispose of this with wiggleFreeList(). */

struct wiggle *wiggleLoadAllByChar(char *fileName, char chopper);
/* Load all wiggle from chopper separated file.
 * Dispose of this with wiggleFreeList(). */

#define wiggleLoadAllByTab(a) wiggleLoadAllByChar(a, '\t');
/* Load all wiggle from tab separated file.
 * Dispose of this with wiggleFreeList(). */

struct wiggle *wiggleCommaIn(char **pS, struct wiggle *ret);
/* Create a wiggle out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wiggle */

void wiggleFree(struct wiggle **pEl);
/* Free a single dynamically allocated wiggle such as created
 * with wiggleLoad(). */

void wiggleFreeList(struct wiggle **pList);
/* Free a list of dynamically allocated wiggle's */

void wiggleOutput(struct wiggle *el, FILE *f, char sep, char lastSep);
/* Print out wiggle.  Separate fields with sep. Follow last field with lastSep. */

#define wiggleTabOut(el,f) wiggleOutput(el,f,'\t','\n');
/* Print out wiggle as a line in a tab-separated file. */

#define wiggleCommaOut(el,f) wiggleOutput(el,f,',',',');
/* Print out wiggle as a comma separated list including final comma. */

/* -------------- End autoSql Generated Code -------------------------- */

#define DEFAULT_MIN_Yv	0.0
#define DEFAULT_MAX_Yv	127.0
#define DEFAULT_HEIGHT_PER	"128"
#define MIN_HEIGHT_PER	11
#define WIG_NO_DATA	128
#define MAX_WIG_VALUE	127
#define MAX_OPT_STRLEN	128

struct wiggleDatum
/* a single instance of a wiggle data value */
    {
    unsigned chromStart;    /* Start position in chromosome, 0 relative */
    double value;
    };

struct asciiDatum
/* a single instance of a wiggle data value (trying float here to save space */
    {
    unsigned chromStart;    /* Start position in chromosome, 0 relative */
    float value;
    };

struct wiggleData
/* linked list of wiggle data values */
    {
    struct wiggleData *next;  /* Next in singly linked list. */
    char *chrom;	/* chromosome or contig */
    unsigned chromStart;    /* Start position, first value */
    unsigned chromEnd;      /* End position, last value */
    unsigned span;	/* each value spans this many bases */
    unsigned count;	/* number of values in this block */
    double lowerLimit;	/* lowest data value in this block */
    double dataRange;	/* lowerLimit + dataRange = upperLimit */
    double sumData;   /* sum of the data points, for average and stddev calc */
    double sumSquares;	/* sum of data points squared, for stddev calc */
    struct wiggleDatum *data;	/* many individual data items here */
    };

struct wiggleStats
/* linked list of wiggle statistics */
    {
    struct wiggleStats *next;  /* Next in singly linked list. */
    char *chrom;	/* chromosome or contig */
    unsigned chromStart;    /* Start position, first value */
    unsigned chromEnd;      /* End position, last value */
    unsigned span;	/* each value spans this many bases */
    unsigned count;	/* number of values in this block */
    double lowerLimit;	/* lowest data value in this block */
    double dataRange;	/* lowerLimit + dataRange = upperLimit */
    double mean;	/* mean of data points */
    double variance;	/* variance of data points */
    double stddev;	/* standard deviation of data points */
    };

struct wigAsciiData
/* linked list of wiggle data in ascii form */
    {
    struct wigAsciiData *next;	/*	next in singly linked list	*/
    char *chrom;		/*	chrom name for this set of data */
    unsigned span;		/*	span for this set of data	*/
    unsigned count;		/*	number of values in this block */
    struct asciiDatum *data;	/*	individual data items here */
    };

#include "hdb.h"

#define HTI_IS_WIGGLE (hti->spanField[0] !=0)

/*	anonymous declaration of track to take care of the use of this struct
 *	in the wigSetCart definition below.  Prevents compiler warnings on
 *	lib/wiggleCart.c and doesn't disturb the usage in hg/hgTracks/...c
 */
struct track;

/*	in hgTracks/wigTrack.c	*/
void wigSetCart(struct track *track, char *dataID, void *dataValue);
    /*	set one of the variables in the wigCart	*/

/*	in lib/wiggleUtils.c	*/
struct wiggleData *wigFetchData(char *db, char *table, char *chromName,
    int winStart, int winEnd, boolean summaryOnly, boolean freeData,
	int tableId, boolean (*wiggleCompare)(int tableId, double value,
	    boolean summaryOnly, struct wiggle *wiggle),
		char *constraints, struct bed **bedList,
		    unsigned maxBedElements, struct wiggleStats **wsList);
/*  return linked list of wiggle data between winStart, winEnd
 *	summaryOnly TRUE will not look at all the data, just the
 *	summaries in the table rows
 */
#define	WIG_SUMMARY_ONLY	TRUE
#define	WIG_DATA_NOT_RETURNED	TRUE
#define	WIG_RETURN_DATA	FALSE
#define	WIG_ALL_DATA		FALSE
void wigFreeData(struct wiggleData **wigData);
/* free everything in the wiggleData structure */

#define wiggleDataFreeList(a) wigFreeData(a)

extern void wigFetchMinMaxY(struct trackDb *tdb, double *min,
    double *max, double *tDbMin, double *tDbMax, int wordCount, char *words[]);
/* return min,max Y ranges from trackDb or cart, in lib/wiggleCart.c */
extern void wigFetchMinMaxPixels(struct trackDb *tdb, int *Min, int *Max,
    int *Default);
/* return pixels heights allowable from trackDb or cart in lib/wiggleCart.c */
extern enum wiggleGridOptEnum wigFetchHorizontalGrid(struct trackDb *tdb,
    char **optString);
/* return horizontalGrid setting	*/
extern enum wiggleScaleOptEnum wigFetchAutoScale(struct trackDb *tdb,
    char **optString);
/* return autoScale setting	*/
extern enum wiggleGraphOptEnum wigFetchGraphType(struct trackDb *tdb,
	char **optString);
/* return graph type, line(points) or bar graph	*/
extern enum wiggleWindowingEnum wigFetchWindowingFunction(struct trackDb *tdb,
	char **optString);
/* return windowing function, Maximum, Mean, Minimum */
extern enum wiggleSmoothingEnum wigFetchSmoothingWindow(struct trackDb *tdb,
	char **optString);
/* return smoothing window size N: [0:16]	*/
extern enum wiggleYLineMarkEnum wigFetchYLineMark(struct trackDb *tdb,
    char **optString);
/* return On/Off status of y= line marker	*/
void wigFetchYLineMarkValue(struct trackDb *tdb, double *tDbYMark);
/* return value for the y line marker to be drawn	*/

/*	cart and trackDb strings	*/
#define VIEWLIMITS "viewLimits"
#define DEFAULTVIEWLIMITS "defaultViewLimits"
#define MIN_Y "minY"
#define MAX_Y "maxY"
#define MAXHEIGHTPIXELS "maxHeightPixels"
#define HEIGHTPER "heightPer"
#define HORIZGRID "horizGrid"
#define GRIDDEFAULT "gridDefault"
#define AUTOSCALE "autoScale"
#define AUTOSCALEDEFAULT "autoScaleDefault"
#define LINEBAR "lineBar"
#define GRAPHTYPE "graphType"
#define GRAPHTYPEDEFAULT "graphTypeDefault"
#define WINDOWINGFUNCTION "windowingFunction"
#define SMOOTHINGWINDOW "smoothingWindow"
#define YLINEONOFF "yLineOnOff"
#define YLINEMARK "yLineMark"

/*	compute the data value give the bin	*/
#define BIN_TO_VALUE(bin,lowerLimit,range) ((range > 0.0) ? \
	lowerLimit+((double)bin/(double)MAX_WIG_VALUE*range) : lowerLimit)
/*	This needs to be adjusted for data with positive and
 *	negative values, a special bin for 0 needs to be set aside and
 *	the positive and negative values kept separately
 */

#if defined(DEBUG)	/*	dbg	*/
extern void wigProfileEnter();
extern long wigProfileLeave();
#define DBGMSGSZ	1023
extern char dbgMsg[DBGMSGSZ+1];
extern void wigDebugPrint(char * name);
#endif

/*	in lib/wigAsciiToBinary.c	*/
void wigAsciiToBinary(char *wigAscii, char *wigFile, char *wibFile,
   double *upperLimit, double *lowerLimit );
/*	given the three file names, read the wigAscii file and produce
 *	the wigFile and wibFile outputs, return the upper and lower
 *	limits for all the data
 */

/*	the object wiggleDataStream is implemented in lib/wigDataStream.c */

enum wigCompare 
/*	type of comparison to be calculated	*/
    {
    wigNoOp_e, wigInRange_e, wigLessThan_e, wigLessEqual_e, wigEqual_e,
	wigNotEqual_e, wigGreaterEqual_e, wigGreaterThan_e,
    };

enum wigDataFetchType
/*	bit masks to specify type of data to fetch via getData()	*/
    {
    wigFetchNoOp = 1, wigFetchAscii = 2, wigFetchBed = 4, wigFetchStats = 8,
    };

struct wiggleDataStream
/*	object definition to access wiggle data, in DB or from file	*/
    {
    char *db;			/*	database name	*/
    char *tblName;		/*	the table or file name	*/
    boolean isFile;		/*	TRUE == it is a file, FALSE == DB */
    struct lineFile *lf;	/*	file handle in case of file	*/
    struct wigAsciiData *ascii;	/*	list of wiggle data values */
    struct bed *bed;		/*	data in bed format	*/
    struct wiggleStats *stats;	/*	list of wiggle stats	*/
    unsigned long long maxOutput;	/*	maximum items fetched	*/
    boolean useDataConstraint;	/*	to simplify checking if it is on */
    enum wigCompare wigCmpSwitch;	/*	for compare function switch */
    char *dataConstraint;	/*	one of < = >= <= == != 'in range' */
    double limit_0;		/*	for constraint comparison	*/
    double limit_1;		/*	for constraint comparison	*/
    unsigned char ucLowerLimit;	/*	for comparison direct to bytes	*/
    unsigned char ucUpperLimit;	/*	for comparison direct to bytes	*/
    char *sqlConstraint;	/*	extra SQL constraints	*/
    unsigned int currentSpan;	/*	for use during reading	*/
    char *currentChrom;		/*	for use during reading	*/
    char *wibFile;		/*	for use during reading	*/
    int wibFH;			/*	wibFile handle	*/
    struct sqlConnection *conn;	/*	SQL connection when talking to db */
    struct sqlResult *sr;	/*	SQL result when talking to db	*/
    char *chrName;		/*	for chrom==chrName on file reads */
    unsigned spanLimit;		/*	for span==spanLimit on file reads */
    int winStart;		/*	for fetches between winStart, winEnd */
    int winEnd;			/*	for fetches between winStart, winEnd */
    unsigned long long rowsRead;     /*	reading stats, SQL rows read */
    unsigned long long validPoints;  /*	reading stats, number of data bytes */
    unsigned long long noDataPoints; /* reading stats, NO_DATA bytes	*/
    unsigned long long bytesRead;    /* reading stats, total wib bytes */
    unsigned long long bytesSkipped; /* reading stats, bytes not examined */
    unsigned long long valuesMatched;  /* reading stats, number of data bytes */
    void (*freeAscii)(struct wiggleDataStream *wDS);
			/*	free the ascii list results 	*/
    void (*freeBed)(struct wiggleDataStream *wDS);
			/*	free the bed list results 	*/
    void (*freeStats)(struct wiggleDataStream *wDS);
			/*	free the stats list results 	*/
    void (*freeConstraints)(struct wiggleDataStream *wDS);
			/*	unset all the constraints	*/
    void (*setPositionConstraint)(struct wiggleDataStream *wDS,
	int winStart, int winEnd);
			/*	work only within specified position */
    void (*setChromConstraint)(struct wiggleDataStream *wDS, char *chr);
			/*	work only on specified chrom 	*/
    void (*setSpanConstraint)(struct wiggleDataStream *wDS, unsigned span);
			/*	work only on specified span 	*/
    void (*setDataConstraint)(struct wiggleDataStream *wDS,
	char *dataConstraint, double lowerLimit, double upperLimit);
			/*	setting data compare limits 	*/
    void (*bedOut)(struct wiggleDataStream *wDS, char *fileName, boolean sort);
			/*	output the bed list results 	*/
    void (*statsOut)(struct wiggleDataStream *wDS,char *fileName, boolean sort);
			/*	output the stats list results 	*/
    void (*asciiOut)(struct wiggleDataStream *wDS,char *fileName, boolean sort);
			/*	output the ascii list results 	*/
    void (*sortResults)(struct wiggleDataStream *wDS);
			/*	sort if you want to, the Outs do this too */
    void (*getDataViaBed)(struct wiggleDataStream *wDS, char *db, char *table,
	int operations, struct bed **bedList);
			/*	fetch data constrained by bedList */
    void (*getData)(struct wiggleDataStream *wDS, char *db, char *table,
	int operations);
			/*	fetch data from db.table */
    /*	PRIVATE attributes, for internal use only	*/
    boolean bedConstrained;	/*	signal to output routines */
    };

/*	in lib/wigDataStream.c	*/
struct wiggleDataStream *newWigDataStream();
void destroyWigDataStream(struct wiggleDataStream **wDS);

#endif /* WIGGLE_H */
