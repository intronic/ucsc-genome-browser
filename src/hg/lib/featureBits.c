/* FeatureBits - convert features tables to bitmaps. */
#include "common.h"
#include "jksql.h"
#include "hdb.h"
#include "bits.h"
#include "cheapcgi.h"
#include "trackDb.h"
#include "bed.h"
#include "psl.h"
#include "genePred.h"
#include "rmskOut.h"
#include "featureBits.h"

/* By default, clip features to the search range.  It's important to clip 
 * when featureBits output will be used to populate Bits etc.  But allow 
 * the user to turn off clipping if they don't want it. */
boolean clipToWin = TRUE;

void featureBitsFree(struct featureBits **pBits)
/* Free up feature bits. */
{
struct featureBits *bits;
if ((bits = *pBits) != NULL)
    {
    freeMem(bits->name);
    freeMem(bits->chrom);
    freez(pBits);
    }
}

void featureBitsFreeList(struct featureBits **pList)
/* Free up a list of featureBits */
{
struct featureBits *el, *next;
for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    freeMem(el);
    }
*pList = NULL;
}

static boolean fetchQualifiers(char *type, char *qualifier, char *extra, int *retSize)
/* Return true if qualifier is of type.  Convert extra to *retSize. */
{
if (qualifier != NULL && sameWord(qualifier, type))
    {
    int size = 0;
    if (extra != NULL)
        size = atoi(extra);
    *retSize = size;
    return TRUE;
    }
else
    return FALSE;
}

static boolean scoreQualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a score qualifier. */
{
return fetchQualifiers("score", qualifier, extra, retSize);
}


static boolean upstreamQualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a upstream qualifier. */
{
return fetchQualifiers("upstream", qualifier, extra, retSize);
}


static boolean exonQualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a exon qualifier. */
{
return fetchQualifiers("exon", qualifier, extra, retSize);
}

static boolean intronQualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a exon qualifier. */
{
return fetchQualifiers("intron", qualifier, extra, retSize);
}

static boolean cdsQualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a cds qualifier. */
{
return fetchQualifiers("cds", qualifier, extra, retSize);
}

static boolean endQualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's an end qualifier. */
{
boolean res = fetchQualifiers("downstream", qualifier, extra, retSize);
if (res)
    return res;
return fetchQualifiers("end", qualifier, extra, retSize);
}

static boolean utr3Qualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a utr3 qualifier. */
{
return fetchQualifiers("utr3", qualifier, extra, retSize);
}

static boolean utr5Qualifier(char *qualifier, char *extra, int *retSize)
/* Return TRUE if it's a utr5 qualifier. */
{
return fetchQualifiers("utr5", qualifier, extra, retSize);
}



boolean fbUnderstandTrackDb(char *db, char *track)
/* Return TRUE if can turn track into a set of ranges or bits. */
{
struct hTableInfo *hti = hFindTableInfoDb(db, NULL, track);

if (hti == NULL)
    return FALSE;
else
    return hti->isPos;
}

boolean fbUnderstandTrack(char *track)
/* Return TRUE if can turn track into a set of ranges or bits. */
{
return(fbUnderstandTrackDb(hGetDb(), track));
}

static void fbAddFeature(struct featureBits **pList, char *name,
	char *chrom, int start, int size, char strand, 
	int winStart, int winEnd)
/* Add new feature to head of list.  Name can be NULL. */
{
struct featureBits *fb;
int s, e;
char nameBuf[512];

if (name == NULL)
    sprintf(nameBuf, "%s:%d-%d", chrom, start+1, start+size);
else
    sprintf(nameBuf, "%s %s:%d-%d", name, chrom, start+1, start+size);
s = start;
e = s + size;
if (clipToWin)
    {
    if (s < winStart) s = winStart;
    if (e > winEnd) e = winEnd;
    }
if (s < e)
    {
    AllocVar(fb);
    fb->name = cloneString(nameBuf);
    fb->chrom = cloneString(chrom);
    fb->start = s;
    fb->end = e;
    fb->strand = strand;
    slAddHead(pList, fb);
    }
}

static void setRangePlusExtra(struct featureBits **pList, 
	char *name, char *chrom, int s, int e, char strand, 
	int extraStart, int extraEnd, 
	int winStart, int winEnd)
/* Set range between s and e plus possibly some extra. */
{
int w;
s -= extraStart;
e += extraEnd;
w = e - s;
fbAddFeature(pList, name, chrom, s, w, strand, winStart,winEnd);
}


char frForStrand(char strand)
/* Return 'r' for '-', else 'f' */
{
if (strand == '-')
    return 'r';
else
    return 'f';
}

static void parseTrackQualifier(char *trackQualifier, char **retTrack, 
	char **retQualifier, char **retExtra)
/* Parse track:qualifier:extra. */
{
char *words[4];
int wordCount;
words[1] = words[2] = words[3] = 0;
wordCount = chopString(trackQualifier, ":", words, ArraySize(words));
if (wordCount < 1)
    errAbort("empty trackQualifier");
*retTrack = words[0];
*retQualifier = words[1];
*retExtra = words[2];
}

void fbOptionsDb(char *db, char *track)
/* Print out an HTML table with radio buttons for featureBits options. */
{
struct sqlConnection *conn;
struct trackDb *tdb;
struct hTableInfo *hti;
boolean isGene;

hti = hFindTableInfoDb(db, NULL, track);
if (hti == NULL)
    errAbort("Could not find table info for table %s", track);

if (sameString("psl", hti->type))
    isGene = FALSE;
else
    isGene = TRUE;

puts("<TABLE><TR><TD>\n");
cgiMakeRadioButton("fbQual", "whole", TRUE);
if (isGene)
    puts(" Whole Gene </TD><TD> ");
else
    puts(" Whole Alignment </TD><TD> ");
puts(" </TD></TR><TR><TD>\n");
cgiMakeRadioButton("fbQual", "upstream", FALSE);
puts(" Upstream by </TD><TD> ");
cgiMakeTextVar("fbUpBases", "200", 8);
puts(" bases </TD></TR><TR><TD>\n");
if (hti->hasBlocks)
    {
    cgiMakeRadioButton("fbQual", "exon", FALSE);
    if (isGene)
	puts(" Exons plus </TD><TD> ");
    else
	puts(" Blocks plus </TD><TD> ");
    cgiMakeTextVar("fbExonBases", "0", 8);
    puts(" bases at each end </TD></TR><TR><TD>\n");
    cgiMakeRadioButton("fbQual", "intron", FALSE);
    if (isGene)
	puts(" Introns plus </TD><TD> ");
    else
	puts(" Regions between blocks plus </TD><TD> ");
    cgiMakeTextVar("fbIntronBases", "0", 8);
    puts(" bases at each end </TD></TR><TR><TD>\n");
    }
if (hti->hasBlocks && hti->hasCDS)
    {
    cgiMakeRadioButton("fbQual", "utr5", FALSE);
    puts(" 5' UTR Exons </TD><TD> ");
    puts(" </TD></TR><TR><TD>\n");
    cgiMakeRadioButton("fbQual", "cds", FALSE);
    puts(" Coding Exons </TD><TD> ");
    puts(" </TD></TR><TR><TD>\n");
    cgiMakeRadioButton("fbQual", "utr3", FALSE);
    puts(" 3' UTR Exons </TD><TD> ");
    puts(" </TD></TR><TR><TD>\n");
    }
else if (hti->hasCDS)
    {
    cgiMakeRadioButton("fbQual", "utr5", FALSE);
    puts(" 5' UTR  </TD><TD> ");
    puts(" </TD></TR><TR><TD>\n");
    cgiMakeRadioButton("fbQual", "cds", FALSE);
    puts(" CDS </TD><TD> ");
    puts(" </TD></TR><TR><TD>\n");
    cgiMakeRadioButton("fbQual", "utr3", FALSE);
    puts(" 3' UTR </TD><TD> ");
    puts(" </TD></TR><TR><TD>\n");
    }
cgiMakeRadioButton("fbQual", "end", FALSE);
puts(" Downstream by </TD><TD> ");
cgiMakeTextVar("fbDownBases", "200", 8);
puts(" bases </TD></TR></TABLE>");
}

void fbOptions(char *track)
/* Print out an HTML table with radio buttons for featureBits options. */
{
fbOptionsDb(hGetDb(), track);
}

char *fbOptionsToQualifier()
/* Translate CGI variable created by fbOptions() to a featureBits qualifier. */
{
char qual[128];
char *fbQual  = cgiOptionalString("fbQual");

if (fbQual == NULL)
    return NULL;

if (sameString(fbQual, "whole"))
    qual[0] = 0;
else if (sameString(fbQual, "exon"))
    snprintf(qual, sizeof(qual), "%s:%s", fbQual, cgiString("fbExonBases"));
else if (sameString(fbQual, "intron"))
    snprintf(qual, sizeof(qual), "%s:%s", fbQual,
			 cgiString("fbIntronBases"));
else if (sameString(fbQual, "upstream"))
    snprintf(qual, sizeof(qual), "%s:%s", fbQual, cgiString("fbUpBases"));
else if (sameString(fbQual, "end"))
    snprintf(qual, sizeof(qual), "%s:%s", fbQual, cgiString("fbDownBases"));
else
    strcpy(qual, fbQual);
return(cloneString(qual));
}

struct featureBits *fbGetRangeQueryDb(char *db, char *trackQualifier,
	char *chrom, int chromStart, int chromEnd, char *sqlConstraints,
	boolean clipToWindow, boolean filterOutNoUTR)
/* Get features in range that match sqlConstraints. */
{
struct hTableInfo *hti;
struct bed *bedList, *bed;
struct featureBits *fbList = NULL, *fbItem;
char itemName[128];
char nameBuf[512];
char *fName;
char *track, *qualifier, *extra;
boolean doUp = FALSE, doEnd = FALSE, doCds = FALSE, doExon = FALSE,
	doUtr3 = FALSE, doUtr5 = FALSE, doIntron = FALSE, doScore = FALSE;
int promoSize = 0, extraSize = 0, endSize = 0, scoreThreshold = 0;
boolean canDoIntrons, canDoUTR, canDoScore;
boolean oldClipToWin = clipToWin;
int i, count, *starts, *sizes;
int s, e;

clipToWin = clipToWindow;

trackQualifier = cloneString(trackQualifier);
parseTrackQualifier(trackQualifier, &track, &qualifier, &extra);
hti = hFindTableInfoDb(db, chrom, track);
if (hti == NULL)
    errAbort("Could not find table info for table %s", track);
canDoUTR = hti->hasCDS;
canDoIntrons = hti->hasBlocks;
canDoScore = (hti->scoreField[0] != 0);

if ((doScore = scoreQualifier(qualifier, extra, &scoreThreshold)) != FALSE)
    {
    if (! canDoScore)
	errAbort("Can't handle score on table %s, sorry", track);
    }

if ((doUp = upstreamQualifier(qualifier, extra, &promoSize)) != FALSE)
    {
    }
else if ((doEnd = endQualifier(qualifier, extra, &endSize)) != FALSE)
    {
    }
else if ((doExon = exonQualifier(qualifier, extra, &extraSize)) != FALSE)
    {
    if (! canDoIntrons)
	errAbort("Can't handle exon on table %s, sorry", track);
    }
else if ((doIntron = intronQualifier(qualifier, extra, &extraSize)) != FALSE)
    {
    if (! canDoIntrons)
	errAbort("Can't handle intron on table %s, sorry", track);
    }
else if ((doCds = cdsQualifier(qualifier, extra, &extraSize)) != FALSE)
    {
    if (! canDoUTR)
	errAbort("Can't handle cds on table %s, sorry", track);
    }
else if ((doUtr3 = utr3Qualifier(qualifier, extra, &extraSize)) != FALSE)
    {
    if (! canDoUTR)
	errAbort("Can't handle utr3 on table %s, sorry", track);
    }
else if ((doUtr5 = utr5Qualifier(qualifier, extra, &extraSize)) != FALSE)
    {
    if (! canDoUTR)
	errAbort("Can't handle utr5 on table %s, sorry", track);
    }

bedList = hGetBedRangeDb(db, track, chrom, chromStart, chromEnd,
			 sqlConstraints);
for (bed = bedList;  bed != NULL;  bed = bed->next)
    {
    if (doUp)
        {
	if (!canDoUTR || !filterOutNoUTR ||
	    ((bed->chromStart != bed->thickStart) &&
	     (bed->chromEnd != bed->thickEnd)))
	    {
	    if (bed->strand[0] == '-')
		{
		s = bed->chromEnd;
		e = s + promoSize;
		}
	    else
		{
		e = bed->chromStart;
		s = e - promoSize;
		}
	    sprintf(nameBuf, "%s_up_%d_%s_%d_%c", 
		    bed->name, promoSize, bed->chrom, s+1,
		    frForStrand(bed->strand[0]));
	    fbAddFeature(&fbList, nameBuf, bed->chrom, s, e - s,
			 bed->strand[0], chromStart, chromEnd);
	    }
	}
    else if (doEnd)
        {
	if (!canDoUTR || !filterOutNoUTR ||
	    ((bed->chromStart != bed->thickStart) &&
	     (bed->chromEnd != bed->thickEnd)))
	    {
	    if (bed->strand[0] == '-')
		{
		e = bed->chromStart;
		s = e - endSize;
		}
	    else
		{
		s = bed->chromEnd;
		e = s + endSize;
		}
	    sprintf(nameBuf, "%s_end_%d_%s_%d_%c", 
		    bed->name, endSize, bed->chrom, s+1,
		    frForStrand(bed->strand[0]));
	    fbAddFeature(&fbList, nameBuf, bed->chrom, s, e - s,
			 bed->strand[0], chromStart, chromEnd);
	    }
	}
    else if (doIntron)
        {
	count  = bed->blockCount;
	starts = bed->chromStarts;
	sizes  = bed->blockSizes;
	for (i=1; i<count; ++i)
	    {
	    s = bed->chromStart + starts[i-1] + sizes[i-1];
	    e = bed->chromStart + starts[i];
	    sprintf(nameBuf, "%s_intron_%d_%d_%s_%d_%c", 
		    bed->name, i-1, extraSize, bed->chrom, s+1,
		    frForStrand(bed->strand[0]));
	    setRangePlusExtra(&fbList, nameBuf, bed->chrom, s, e,
			      bed->strand[0], extraSize, extraSize,
			      chromStart, chromEnd);
	    }
	}
    else
        {
	if (canDoIntrons)
	    {
	    // doExon is the default action.
	    count  = bed->blockCount;
	    starts = bed->chromStarts;
	    sizes  = bed->blockSizes;
	    for (i=0; i<count; ++i)
		{
		s = bed->chromStart + starts[i];
		e = bed->chromStart + starts[i] + sizes[i];
		if (doCds)
		    {
		    if ((e < bed->thickStart) || (s > bed->thickEnd)) continue;
		    if (s < bed->thickStart) s = bed->thickStart;
		    if (e > bed->thickEnd) e = bed->thickEnd;
		    fName = "cds";
		    }
		else if (doUtr5)
		    {
		    if (bed->strand[0] == '-')
			{
			if (e < bed->thickEnd) continue;
			if (s < bed->thickEnd) s = bed->thickEnd;
			}
		    else
			{
			if (s > bed->thickStart) continue;
			if (e > bed->thickStart) e = bed->thickStart;
			}
		    fName = "utr5";
		    }
		else if (doUtr3)
		    {
		    if (bed->strand[0] == '-')
			{
			if (s > bed->thickStart) continue;
			if (e > bed->thickStart) e = bed->thickStart;
			}
		    else
			{
			if (e < bed->thickEnd) continue;
			if (s < bed->thickEnd) s = bed->thickEnd;
			}
		    fName = "utr3";
		    }
		else
		    {
		    fName = "exon";
		    }
		if (!doScore || (doScore && bed->score >= scoreThreshold))
		    {
		    sprintf(nameBuf, "%s_%s_%d_%d_%s_%d_%c", 
			    bed->name, fName, i, extraSize, bed->chrom, s+1,
			    frForStrand(bed->strand[0]));
		    setRangePlusExtra(&fbList, nameBuf, bed->chrom, s, e,
				      bed->strand[0], extraSize, extraSize,
				      chromStart, chromEnd);
		    }
		}
	    }
	else
	    {
	    if (doCds)
		{
		s = bed->thickStart;
		e = bed->thickEnd;
		fName = "cds";
		}
	    else if (doUtr5)
		{
		if (bed->strand[0] == '-')
		    {
		    s = bed->thickEnd;
		    e = bed->chromEnd;
		    }
		else
		    {
		    s = bed->chromStart;
		    e = bed->thickStart;
		    }
		fName = "utr5";
		}
	    else if (doUtr3)
		{
		if (bed->strand[0] == '-')
		    {
		    s = bed->chromStart;
		    e = bed->thickStart;
		    }
		else
		    {
		    s = bed->thickEnd;
		    e = bed->chromEnd;
		    }
		fName = "utr3";
		}
	    else
		{
		s = bed->chromStart;
		e = bed->chromEnd;
		fName = "whole";
		}
	    if (!doScore || (doScore && bed->score >= scoreThreshold))
		{
		sprintf(nameBuf, "%s_%s_%d_%s_%d_%c", 
			bed->name, fName, extraSize, bed->chrom, s+1,
			frForStrand(bed->strand[0]));
		setRangePlusExtra(&fbList, nameBuf, bed->chrom, s, e,
				  bed->strand[0], extraSize, extraSize,
				  chromStart, chromEnd);
		}
	    }
	}
    }
clipToWin = oldClipToWin;
bedFreeList(&bedList);
freeMem(trackQualifier);
slReverse(&fbList);
return fbList;
}


struct featureBits *fbGetRangeQuery(char *trackQualifier,
	char *chrom, int chromStart, int chromEnd, char *sqlConstraints,
	boolean clipToWindow, boolean filterOutNoUTR)
/* Get features in range that match sqlConstraints. */
{
return(fbGetRangeQueryDb(hGetDb(), trackQualifier,
			 chrom, chromStart, chromEnd, sqlConstraints,
			 clipToWindow, filterOutNoUTR));
}


struct featureBits *fbGetRange(char *trackQualifier, char *chrom,
	int chromStart, int chromEnd)
/* Get features in range that match sqlConstraints. */
{
return(fbGetRangeQueryDb(hGetDb(), trackQualifier,
			 chrom, chromStart, chromEnd, NULL, TRUE, TRUE));
}


void fbOrBits(Bits *bits, int bitSize, struct featureBits *fbList, int bitOffset)
/* Or in bits.   Bits should have bitSize bits.  */
{
int s, e, w;
struct featureBits *fb;

for (fb = fbList; fb != NULL; fb = fb->next)
    {
    s = fb->start - bitOffset;
    e = fb->end - bitOffset;
    if (e > bitSize) e = bitSize;
    if (s < 0) s = 0;
    w = e - s;
    if (w > 0)
	bitSetRange(bits, s , w);
    }
}

void fbOrTableBits(Bits *bits, char *trackQualifier, char *chrom, 
	int chromSize, struct sqlConnection *conn)
{
struct featureBits *fbList = fbGetRange(trackQualifier, chrom, 0, chromSize);
fbOrBits(bits, chromSize, fbList, 0);
slFreeList(&fbList);
}

