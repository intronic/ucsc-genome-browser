/*	wiggleUtils - operations to fetch data from .wib files
 *	first used in hgText
 */
#include "common.h"
#include "jksql.h"
#include "dystring.h"
#include "hdb.h"
#include "wiggle.h"

static char const rcsid[] = "$Id: wiggleUtils.c,v 1.1 2004/03/17 22:59:46 hiram Exp $";

static char *currentFile = (char *) NULL;	/* the binary file name */
static FILE *f = (FILE *) NULL;			/* file handle to binary file */

static void openWibFile(char *fileName)
/*  smart open of the .wib file - only if it isn't already open */
{
if (currentFile)
    {
    if (differentString(currentFile,fileName))
	{
	if (f != (FILE *) NULL)
	    {
	    fclose(f);
	    freeMem(currentFile);
	    }
	currentFile = cloneString(fileName);
	f = mustOpen(currentFile, "r");
	}
    }
else
    {
    currentFile = cloneString(fileName);
    f = mustOpen(currentFile, "r");
    }
}

static void closeWibFile(FILE *f)
/*	smart close of .wib file - close only if open	*/
{
if (f != (FILE *) NULL)
    {
    fclose(f);
    freeMem(currentFile);
    currentFile = (char *) NULL;
    f = (FILE *) NULL;
    }
}

static struct wiggleData * readWigDataRow(struct wiggle *wiggle,
    int winStart, int winEnd)
/*  read one row of wiggle data, return data values between winStart, winEnd */
{
unsigned char *readData = (unsigned char *) NULL;
size_t itemsRead = 0;
int dataOffset = 0;
int noData = 0;
struct wiggleData *ret = (struct wiggleData *) NULL;
struct wiggleDatum *data = (struct wiggleDatum *)NULL;
struct wiggleDatum *dataPtr = (struct wiggleDatum *)NULL;
unsigned chromPosition = 0;
unsigned validCount = 0;
long int chromStart = -1;
unsigned chromEnd = 0;
double lowerLimit = 1.0e+300;
double upperLimit = -1.0e+300;
double sumData = 0.0;
double sumSquares = 0.0;
unsigned dataArea;

openWibFile(wiggle->file);
fseek(f, wiggle->offset, SEEK_SET);
readData = (unsigned char *) needMem((size_t) (wiggle->count + 1));
itemsRead = fread(readData, (size_t) wiggle->count,
	(size_t) sizeof(unsigned char), f);
if (itemsRead != sizeof(unsigned char))
    errAbort("readWigDataRow: can not read %u bytes from %s at offset %u",
	wiggle->count, wiggle->file, wiggle->offset);

/*	need at most this amount, perhaps less	*/
dataArea = sizeof(struct wiggleDatum)*wiggle->validCount;

data = (struct wiggleDatum *) needMem((size_t)
    (sizeof(struct wiggleDatum)*wiggle->validCount));

dataPtr = data;
chromPosition = wiggle->chromStart;

for (dataOffset = 0; dataOffset < wiggle->count; ++dataOffset)
    {
    unsigned char datum = readData[dataOffset];
    if (datum == WIG_NO_DATA)
	++noData;
    else
	{
	if (chromPosition >= winStart && chromPosition < winEnd)
	    {
	    dataPtr->chromStart = chromPosition;
	    dataPtr->value = wiggle->lowerLimit +
		(((double)datum/(double)MAX_WIG_VALUE)*wiggle->dataRange);
	    ++validCount;
	    if (chromStart < 0)
		chromStart = chromPosition;
	    chromEnd = chromPosition + 1 + wiggle->span;
	    if (lowerLimit > dataPtr->value)
		lowerLimit = dataPtr->value;
	    if (upperLimit < dataPtr->value)
		upperLimit = dataPtr->value;
	    sumData += dataPtr->value;
	    sumSquares += dataPtr->value * dataPtr->value;
	    ++dataPtr;
	    }
	}
    chromPosition += wiggle->span;
    }
if (validCount)
    {
    double dataRange = upperLimit - lowerLimit;
    AllocVar(ret);
    ret->next = (struct wiggleData *) NULL;
    ret->chrom = wiggle->chrom;
    ret->chromStart = chromStart;
    ret->chromEnd = chromEnd;
    ret->span = wiggle->span;
    ret->count = validCount;
    ret->lowerLimit = lowerLimit;
    ret->dataRange = dataRange;
    ret->sumData = sumData;
    ret->sumSquares = sumSquares;
    ret->data = data;
    }

return (ret);	/* may be null if validCount < 1	*/
}	/*	static struct wiggleData * readWigDataRow()	*/

static void insertOrderedWigData(struct wiggleData * *wdList, struct wiggleData *wd)
/* insert wiggle data, keeping in chrom position order */
{
struct wiggleData *wdPtr;
struct wiggleData *wdPrev;

/*	If first one, it becomes the list	*/
if (*wdList == (struct wiggleData *)NULL)
    {
    wd->next = (struct wiggleData *)NULL;	/*	redundant safety */
    *wdList = wd;
    return;			/*	!! RETURN !!	*/
    }

wdPrev = *wdList;
/*	Not first one, find position to insert	*/
for (wdPtr = *wdList; wdPtr != (struct wiggleData *)NULL; wdPtr = wdPtr->next)
    {
    if (wd->chromEnd <= wdPtr->chromStart)
	{
	wd->next = wdPtr;
	wdPrev->next = wd;
	return;			/*	!! RETURN !!	*/
	}
    wdPrev = wdPtr;
    }
/*	It is last on the list	*/
wdPrev->next = wd;
wd->next = (struct wiggleData *)NULL;	/*	redundant safety	*/
}

struct wiggleData *fetchWigData(char *db, char *tableName, char *chromName,
    int winStart, int winEnd)
/*  return linked list of wiggle data between winStart, winEnd */
{
struct wiggleData *ret = (struct wiggleData *) NULL;
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
char *whereNULL = NULL;
int rowOffset;
int rowCount = 0;
struct wiggle *wiggle;
struct hash *spans = NULL;      /* List of spans encountered during load */
char spanName[128];
char whereSpan[128];
struct hashEl *el, *elList;
int leastSpan = BIGNUM;
int mostSpan = 0;
int spanCount = 0;
struct hashCookie cookie;
struct wiggleData *wigData = (struct wiggleData *) NULL;
struct wiggleData *wdList = (struct wiggleData *) NULL;

spans = newHash(0);	/*	a listing of all spans found here	*/

/*	the first thing we need to do is check for spans	*/
sr = hRangeQuery(conn, tableName, chromName, winStart, winEnd,
        whereNULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
{
    ++rowCount;
    wiggle = wiggleLoad(row + rowOffset);

    snprintf(spanName, sizeof(spanName), "%d", wiggle->span);
    el = hashLookup(spans, spanName);
    if ( el == NULL)
	{
	if (wiggle->span > mostSpan) mostSpan = wiggle->span;
	if (wiggle->span < leastSpan) leastSpan = wiggle->span;
	++spanCount;
	hashAddInt(spans, spanName, wiggle->span);
	}

    wiggleFree(&wiggle);
    }
sqlFreeResult(&sr);


/*	Now, using that span list, go through each span, fetching data	*/
cookie = hashFirst(spans);
while ((el = hashNext(&cookie)) != NULL)
    {
    snprintf(whereSpan, sizeof(whereSpan), "span = %s", el->name);
    sr = hRangeQuery(conn, tableName, chromName, winStart, winEnd,
        whereSpan, &rowOffset);
    rowCount = 0;
    while ((row = sqlNextRow(sr)) != NULL)
	{
	++rowCount;
	wiggle = wiggleLoad(row + rowOffset);
	wigData = readWigDataRow(wiggle, winStart, winEnd);
	if (wigData)
	    {
	    insertOrderedWigData(&wdList,wigData);
	    }
	}
    sqlFreeResult(&sr);
    }
closeWibFile(f);

hFreeConn(&conn);

return(wdList);
}	/*	struct wiggleData *fetchWigData()	*/
