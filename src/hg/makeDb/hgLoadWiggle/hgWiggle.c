/* hgWiggle - fetch wiggle data from database or .wig file */
#include "common.h"
#include "options.h"
#include "linefile.h"
#include "obscure.h"
#include "hash.h"
#include "cheapcgi.h"
#include "jksql.h"
#include "dystring.h"
#include "wiggle.h"
#include "hdb.h"
#include "portable.h"

static char const rcsid[] = "$Id: hgWiggle.c,v 1.8 2004/08/04 20:35:47 hiram Exp $";

/* Command line switches. */
static boolean silent = FALSE;	/*	no data points output */
static boolean timing = FALSE;	/*	turn timing on	*/
static boolean skipDataRead = FALSE;	/*	do not read the wib data */
static char *dataConstraint;	/*	one of < = >= <= == != 'in range' */

/* command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"db", OPTION_STRING},
    {"chr", OPTION_STRING},
    {"dataConstraint", OPTION_STRING},
    {"silent", OPTION_BOOLEAN},
    {"timing", OPTION_BOOLEAN},
    {"skipDataRead", OPTION_BOOLEAN},
    {"span", OPTION_INT},
    {"ll", OPTION_FLOAT},
    {"ul", OPTION_FLOAT},
    {NULL, 0}
};

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgWiggle - fetch wiggle data from data base or file\n"
  "usage:\n"
  "   hgWiggle [options] <track names ...>\n"
  "options:\n"
  "   -db=<database> - use specified database\n"
  "   -chr=chrN - examine data only on chrN\n"
  "   -silent - no output, scanning data only\n"
  "   -timing - display timing statistics\n"
  "   -skipDataRead - do not read the .wib data (for no-read speed check)\n"
  "   -dataConstraint='DC' - where DC is one of < = >= <= == != 'in range'\n"
  "   -ll=<F> - lowerLimit compare data values to F (float) (all but 'in range')\n"
  "   -ul=<F> - upperLimit compare data values to F (float)\n\t\t(need both ll and ul when 'in range')\n"
  "   When no database is specified, track names will refer to .wig files\n"
  "   example using the file gc5Base.wig:\n"
  "                hgWiggle -chr=chrM gc5Base\n"
  "   example using the database table hg17.gc5Base:\n"
  "                hgWiggle -chr=chrM -db=hg17 gc5Base"
  );
}

static boolean wigNextRow(struct wiggleDataStream *wDS, char *row[],
	int maxRow)
/*	read next wig row from sql query or lineFile	*/
{
int numCols;

if (wDS->isFile)
    {
    numCols = lineFileChopNextTab(wDS->lf, row, maxRow);
    if (numCols != maxRow) return FALSE;
    verbose(3, "#\tnumCols = %d, row[0]: %s, row[1]: %s, row[%d]: %s\n",
	numCols, row[0], row[1], maxRow-1, row[maxRow-1]);
    }
else
    {
    int i;
    char **sqlRow;
    sqlRow = sqlNextRow(wDS->sr);
    if (sqlRow == NULL)
	return FALSE;
    /*	skip the bin column sqlRow[0]	*/
    for (i=1; i <= maxRow; ++i)
	{
	row[i-1] = sqlRow[i];
	}
    }
return TRUE;
}

static void hgWiggle(struct wiggleDataStream *wDS, int trackCount,
	char *tracks[])
/* hgWiggle - dump wiggle data from database or .wig file */
{
int i;
struct wiggle *wiggle;
unsigned long long totalValidData = 0;
unsigned long fileNoDataBytes = 0;
long fileEt = 0;
unsigned long long totalRows = 0;
unsigned long long valuesMatched = 0;
unsigned long long bytesRead = 0;
unsigned long long bytesSkipped = 0;


verbose(2, "#\texamining tracks:");
for (i=0; i<trackCount; ++i)
    verbose(2, " %s", tracks[i]);
verbose(2, "\n");

for (i=0; i<trackCount; ++i)
    {
    char *row[WIGGLE_NUM_COLS];
    unsigned long long rowCount = 0;
    unsigned long long chrRowCount = 0;
    long startClock = clock1000();
    long endClock;
    long chrStartClock = clock1000();
    long chrEndClock;
    unsigned long long validData = 0;
    unsigned long chrBytesRead = 0;
    unsigned long noDataBytes = 0;
    unsigned long chrNoDataBytes = 0;

    wDS->openWigConn(wDS, tracks[i]);
    while (wigNextRow(wDS, row, WIGGLE_NUM_COLS))
	{
	++rowCount;
	++chrRowCount;
	wiggle = wiggleLoad(row);
	if (wDS->isFile)
	    {
	    if (wDS->chrName)
		if (differentString(wDS->chrName,wiggle->chrom))
		    continue;
	    if (wDS->spanLimit)
		if (wDS->spanLimit != wiggle->span)
		    continue;
	    }

	if ( (wDS->currentSpan != wiggle->span) || 
		(wDS->currentChrom && differentString(wDS->currentChrom, wiggle->chrom)))
	    {
	    if (wDS->currentChrom && differentString(wDS->currentChrom, wiggle->chrom))
		{
		long et;
		chrEndClock = clock1000();
		et = chrEndClock - chrStartClock;
		if (timing)
 verbose(1,"#\t%s.%s %lu data bytes, %lu no-data bytes, %ld ms, %llu rows\n",
		    tracks[i], wDS->currentChrom, chrBytesRead, chrNoDataBytes, et,
			chrRowCount);
    		chrRowCount = chrNoDataBytes = chrBytesRead = 0;
		chrStartClock = clock1000();
		}
	    freeMem(wDS->currentChrom);
	    wDS->currentChrom=cloneString(wiggle->chrom);
	    if (!silent)
		printf ("variableStep chrom=%s", wDS->currentChrom);
	    wDS->currentSpan = wiggle->span;
	    if (!silent && (wDS->currentSpan > 1))
		printf (" span=%u\n", wDS->currentSpan);
	    else if (!silent)
		printf ("\n");
	    }

	verbose(3, "#\trow: %llu, start: %u, data range: %g: [%g:%g]\n",
		rowCount, wiggle->chromStart, wiggle->dataRange,
		wiggle->lowerLimit, wiggle->lowerLimit+wiggle->dataRange);
	verbose(3, "#\tresolution: %g per bin\n",
		wiggle->dataRange/(double)MAX_WIG_VALUE);
	if (wDS->useDataConstraint)
	    {
	    if (!wDS->cmpDouble(wDS, wiggle->lowerLimit,
		    (wiggle->lowerLimit + wiggle->dataRange)))
		{
		bytesSkipped += wiggle->count;
		continue;
		}
	    wDS->setCompareByte(wDS, wiggle->lowerLimit, wiggle->dataRange);
	    }
	if (!skipDataRead)
	    {
	    int j;	/*	loop counter through ReadData	*/
	    unsigned char *datum;    /* to walk through ReadData bytes */
	    unsigned char *ReadData;    /* the bytes read in from the file */
	    wDS->openWibFile(wDS, wiggle->file);
			/* possibly open a new wib file */
	    ReadData = (unsigned char *) needMem((size_t) (wiggle->count + 1));
	    bytesRead += wiggle->count;
	    lseek(wDS->wibFH, wiggle->offset, SEEK_SET);
	    read(wDS->wibFH, ReadData,
		(size_t) wiggle->count * (size_t) sizeof(unsigned char));

    verbose(3, "#\trow: %llu, reading: %u bytes\n", rowCount, wiggle->count);

	    datum = ReadData;
	    for (j = 0; j < wiggle->count; ++j)
		{
		if (*datum != WIG_NO_DATA)
		    {
		    ++validData;
		    ++chrBytesRead;
		    if (wDS->useDataConstraint)
			{
			if (wDS->cmpByte(wDS, datum))
			    {
			    ++valuesMatched;
			    if (!silent)
				{
				double datumOut =
 wiggle->lowerLimit+(((double)*datum/(double)MAX_WIG_VALUE)*wiggle->dataRange);
				if (verboseLevel() > 2)
				    printf("%d\t%g\t%d\n", 1 +
					wiggle->chromStart + (j * wiggle->span),
						datumOut, *datum);
				else
				    printf("%d\t%g\n", 1 +
					wiggle->chromStart + (j * wiggle->span),
						datumOut);
				}
			    }
			}
		    else
			{
			if (!silent)
			    {
			    double datumOut =
 wiggle->lowerLimit+(((double)*datum/(double)MAX_WIG_VALUE)*wiggle->dataRange);
			    if (verboseLevel() > 2)
				printf("%d\t%g\t%d\n",
				  1 + wiggle->chromStart + (j * wiggle->span),
					    datumOut, *datum);
			    else
				printf("%d\t%g\n",
				  1 + wiggle->chromStart + (j * wiggle->span),
					    datumOut);
			    }
			}
		    }
		else
		    {
		    ++noDataBytes;
		    ++chrNoDataBytes;
		    }
		++datum;
		}
	    freeMem(ReadData);
	    }	/*	if (!skipDataRead)	*/
	wiggleFree(&wiggle);
	}	/*	while (wigNextRow())	*/
    endClock = clock1000();
    totalRows += rowCount;
    if (timing)
	{
	long et;
	chrEndClock = clock1000();
	et = chrEndClock - chrStartClock;
	/*	file per chrom output already happened above except for
	 *	the last one */
	if (!wDS->isFile || (wDS->isFile && ((i+1)==trackCount)))
 verbose(1,"#\t%s.%s %lu data bytes, %lu no-data bytes, %ld ms, %llu rows, %llu matched\n",
		    tracks[i], wDS->currentChrom, chrBytesRead, chrNoDataBytes, et,
			rowCount, valuesMatched);
    	chrNoDataBytes = chrBytesRead = 0;
	chrStartClock = clock1000();
	et = endClock - startClock;
	fileEt += et;
 verbose(1,"#\t%s %llu valid bytes, %lu no-data bytes, %ld ms, %llu rows, %llu matched\n",
	tracks[i], validData, noDataBytes, et, totalRows, valuesMatched);
	}
    totalValidData += validData;
    fileNoDataBytes += noDataBytes;
    }	/*	for (i=0; i<trackCount; ++i)	*/

if (wDS->isFile)
    {
    lineFileClose(&wDS->lf);
    if (timing)
 verbose(1,"#\ttotal %llu valid bytes, %lu no-data bytes, %ld ms, %llu rows\n#\t%llu matched = %% %.2f, wib bytes: %llu, bytes skipped: %llu\n",
	totalValidData, fileNoDataBytes, fileEt, totalRows, valuesMatched,
	100.0 * (float)valuesMatched / (float)totalValidData, bytesRead, bytesSkipped);
    }

wDS->closeWigConn(wDS);

}	/*	void hgWiggle()	*/


int main(int argc, char *argv[])
/* Process command line. */
{
struct wiggleDataStream *wDS = NULL;
float lowerLimit = -1 * INFINITY;  /*	for constraint comparison	*/
float upperLimit = INFINITY;	/*	for constraint comparison	*/
unsigned span = 0;	/*	select for this span only	*/
char *chr = NULL;		/* work on this chromosome only */

optionInit(&argc, argv, optionSpecs);

wDS = newWigDataStream();

wDS->db = optionVal("db", NULL);
chr = optionVal("chr", NULL);
dataConstraint = optionVal("dataConstraint", NULL);
silent = optionExists("silent");
timing = optionExists("timing");
skipDataRead = optionExists("skipDataRead");
span = optionInt("span", 0);
lowerLimit = optionFloat("ll", -1 * INFINITY);
upperLimit = optionFloat("ul", INFINITY);

if (wDS->db)
    {
    wDS->isFile = FALSE;
    verbose(2, "#\tdatabase: %s\n", wDS->db);
    }
else
    {
    wDS->isFile = TRUE;
    verbose(2, "#\tno database specified, using .wig files\n");
    }
if (chr)
    {
    wDS->addChromConstraint(wDS, chr);
    verbose(2, "#\tchrom constraint: (%s)\n", wDS->sqlConstraint);
    }
if (silent)
    verbose(2, "#\tsilent option on, no data points output\n");
if (timing)
    verbose(2, "#\ttiming option on\n");
if (skipDataRead)
    verbose(2, "#\tskipDataRead option on, do not read .wib data\n");
if (span)
    {
    wDS->addSpanConstraint(wDS, span);
    verbose(2, "#\tspan constraint: (%s)\n", wDS->sqlConstraint);
    }
if (dataConstraint)
    {
    if (sameString(dataConstraint, "in range"))
	{
	if (!(optionExists("ll") && optionExists("ul")))
	    {
	    warn("ERROR: dataConstraint 'in range' specified without both -ll=<F> and -ul=<F>");
	    usage();
	    }
	}
    else if (!optionExists("ll"))
	{
	warn("ERROR: dataConstraint specified without -ll=<F>");
	usage();
	}

    wDS->setDataConstraint(wDS, dataConstraint, lowerLimit, upperLimit);

    if (sameString(dataConstraint, "in range"))
	verbose(2, "#\tdataConstraint: %s [%f : %f]\n", wDS->dataConstraint,
		wDS->limit_0, wDS->limit_1);
    else
	verbose(2, "#\tdataConstraint: data values %s %f\n",
		wDS->dataConstraint, wDS->limit_0);
    }
else if (optionExists("ll") || optionExists("ul"))
    {
    warn("ERROR: ll or ul options specified without -dataConstraint");
    usage();
    }

if (argc < 2)
    usage();

hgWiggle(wDS, argc-1, argv+1);
return 0;
}
