/* FeatureBits - convert features tables to bitmaps. */
#ifndef FEATUREBITS_H
#define FEATUREBITS_H

#ifndef BITS_H
#include "bits.h"
#endif

struct featureBits
/* A part of a sequence. */
    {
    struct featureBits *next;
    char *name;		/* Symbolic name for feature. */
    char *chrom;	/* Chromosome name. */
    int start, end;	/* Start/end half open zero based. */
    char strand;	/* + or - or ? */
    };

void featureBitsFree(struct featureBits **pBits);
/* Free up feature bits. */

void featureBitsFreeList(struct featureBits **pList);
/* Free up a list of featureBits */

struct featureBits *fbGetRange(char *trackQualifier, char *chrom,
    int chromStart, int chromEnd);
/* Get features in range. */

struct featureBits *fbGetRangeQuery(char *trackQualifier,
	char *chrom, int chromStart, int chromEnd, char *sqlConstraints,
	boolean clipToWindow, boolean filterOutNoUTR);
/* Get features in range that match sqlConstraints. */

struct featureBits *fbGetRangeQueryDb(char *db, char *trackQualifier,
	char *chrom, int chromStart, int chromEnd, char *sqlConstraints,
	boolean clipToWindow, boolean filterOutNoUTR);
/* Get features in range that match sqlConstraints. */

boolean fbUnderstandTrack(char *track);
/* Return TRUE if can turn track into a set of ranges or bits. */

boolean fbUnderstandTrackDb(char *db, char *track);
/* Return TRUE if can turn track into a set of ranges or bits. */

void fbOrBits(Bits *bits, int bitSize, struct featureBits *fbList,
	int bitOffset);
/* Or in bits.   Bits should have bitSize bits.  */

void fbOrTableBits(Bits *bits, char *trackQualifier, char *chrom, 
	int chromSize, struct sqlConnection *conn);
/* Ors in features in track on chromosome into bits.  */

void fbOrTableBitsQuery(Bits *bits, char *trackQualifier, char *chrom, 
	int chromSize, struct sqlConnection *conn, char *sqlConstraints,
	boolean clipToWindow, boolean filterOutNoUTR);
/* Ors in features matching sqlConstraints in track on chromosome into bits. */

void fbOptions(char *track);
/* Print out an HTML table with radio buttons for featureBits options. */

void fbOptionsDb(char *db, char *track);
/* Print out an HTML table with radio buttons for featureBits options. */

void fbOptionsHti(struct hTableInfo *hti);
/* Print out an HTML table with radio buttons for featureBits options. */

char *fbOptionsToQualifier();
/* Translate CGI variable created by fbOptions() to a featureBits qualifier. */

struct featureBits *fbFromBed(char *trackQualifier, struct hTableInfo *hti,
	struct bed *bedList, int chromStart, int chromEnd,
	boolean clipToWindow, boolean filterOutNoUTR);
/* Translate a list of bed items into featureBits. */

#endif /* FEATUREBITS_H */

