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
    int start, end;	/* Start/end half open zero based. */
    };

struct featureBits *fbGetRange(char *trackQualifier, char *chrom,
    int chromStart, int chromEnd);
/* Get features in range. */

boolean fbUnderstandTrack(char *track);
/* Return TRUE if can turn track into a set of ranges or bits. */

void fbOrTableBits(Bits *bits, char *trackQualifier, char *chrom, 
	int chromSize, struct sqlConnection *conn);
/* Ors in features in track on chromosome into bits.  */

#endif /* FEATUREBITS_H */

