/* hgSeq - Fetch and format DNA sequence data (to stdout) for gene records. */
#include "common.h"
#include "dnautil.h"
#include "dnaseq.h"
#include "cart.h"
#include "cheapcgi.h"
#include "hdb.h"
#include "web.h"
#include "rmskOut.h"
#include "fa.h"
#include "genePred.h"
#include "bed.h"

static char const rcsid[] = "$Id: hgSeq.c,v 1.17 2003/05/06 07:22:22 kate Exp $";

/* I don't like using this global, but don't want to do a zillion 
 * hChromSizes in addFeature and don't want to add it as a param of 
 * every call to addFeature. */
static int chromSize = 0;

void hgSeqFeatureRegionOptions(struct cart *cart, boolean canDoUTR,
			       boolean canDoIntrons)
/* Print out HTML FORM entries for feature region options. */
{
char *exonStr = canDoIntrons ? " Exons" : "";
char *setting;

puts("\n<H3> Sequence Retrieval Region Options: </H3>\n");

if (canDoIntrons || canDoUTR)
    {
    cgiMakeCheckBox("hgSeq.promoter",
		    cartCgiUsualBoolean(cart, "hgSeq.promoter", FALSE));
    puts("Promoter/Upstream by ");
    setting = cartCgiUsualString(cart, "hgSeq.promoterSize", "1000");
    cgiMakeTextVar("hgSeq.promoterSize", setting, 5);
    puts("bases <BR>");
    }

if (canDoUTR)
    {
    cgiMakeCheckBox("hgSeq.utrExon5",
		    cartCgiUsualBoolean(cart, "hgSeq.utrExon5", TRUE));
    printf("5' UTR%s <BR>\n", exonStr);
    }

if (canDoIntrons)
    {
    cgiMakeCheckBox("hgSeq.cdsExon",
		    cartCgiUsualBoolean(cart, "hgSeq.cdsExon", TRUE));
    if (canDoUTR)
	printf("CDS Exons <BR>\n");
    else
	printf("Blocks <BR>\n");
    }
else if (canDoUTR)
    {
    cgiMakeCheckBox("hgSeq.cdsExon",
		    cartCgiUsualBoolean(cart, "hgSeq.cdsExon", TRUE));
    printf("CDS <BR>\n");
    }
else
    {
    cgiMakeHiddenVar("hgSeq.cdsExon", "1");
    }

if (canDoUTR)
    {
    cgiMakeCheckBox("hgSeq.utrExon3",
 		    cartCgiUsualBoolean(cart, "hgSeq.utrExon3", TRUE));
   printf("3' UTR%s <BR>\n", exonStr);
    }

if (canDoIntrons)
    {
    cgiMakeCheckBox("hgSeq.intron",
		    cartCgiUsualBoolean(cart, "hgSeq.intron", TRUE));
    if (canDoUTR)
	puts("Introns <BR>");
    else
	puts("Regions between blocks <BR>");
    }

if (canDoIntrons || canDoUTR)
    {
    cgiMakeCheckBox("hgSeq.downstream",
		    cartCgiUsualBoolean(cart, "hgSeq.downstream", FALSE));
    puts("Downstream by ");
    setting = cartCgiUsualString(cart, "hgSeq.downstreamSize", "1000");
    cgiMakeTextVar("hgSeq.downstreamSize", setting, 5);
    puts("bases <BR>");
    }

if (canDoIntrons || canDoUTR)
    {
    setting = cartCgiUsualString(cart, "hgSeq.granularity", "gene");
    cgiMakeRadioButton("hgSeq.granularity", "gene",
		       sameString(setting, "gene"));
    if (canDoUTR)
	puts("One FASTA record per gene. <BR>");
    else
	puts("One FASTA record per item. <BR>");
    cgiMakeRadioButton("hgSeq.granularity", "feature",
		       sameString(setting, "feature"));
    if (canDoUTR)
	puts("One FASTA record per region (exon, intron, etc.) with ");
    else
	puts("One FASTA record per region (block/between blocks) with ");
    }
else
    {
    puts("Add ");
    }
setting = cartCgiUsualString(cart, "hgSeq.padding5", "0");
cgiMakeTextVar("hgSeq.padding5", setting, 5);
puts("extra bases upstream (5') and ");
setting = cartCgiUsualString(cart, "hgSeq.padding3", "0");
cgiMakeTextVar("hgSeq.padding3", setting, 5);
puts("extra downstream (3') <BR>");
if (canDoIntrons && canDoUTR)
    {
    puts("&nbsp;&nbsp;&nbsp;");
    cgiMakeCheckBox("hgSeq.splitCDSUTR",
		    cartCgiUsualBoolean(cart, "hgSeq.splitCDSUTR", FALSE));
    puts("Split UTR and CDS parts of an exon into separate FASTA records");
    }
puts("<P>\n");

}


void hgSeqDisplayOptions(struct cart *cart, boolean canDoUTR,
			 boolean canDoIntrons, boolean offerRevComp)
/* Print out HTML FORM entries for sequence display options. */
{
char *casing, *repMasking;

puts("\n<H3> Sequence Formatting Options: </H3>\n");

casing = cartCgiUsualString(cart, "hgSeq.casing", "exon");
if (canDoIntrons)
    {
    cgiMakeRadioButton("hgSeq.casing", "exon", sameString(casing, "exon"));
    if (canDoUTR)
	puts("Exons in upper case, everything else in lower case. <BR>");
    else
	puts("Blocks in upper case, everything else in lower case. <BR>");
    }
if (canDoUTR)
    {
    if (sameString(casing, "exon") && !canDoIntrons)
	casing = "cds";
    cgiMakeRadioButton("hgSeq.casing", "cds", sameString(casing, "cds"));
    puts("CDS in upper case, UTR in lower case. <BR>");
    }
if ((sameString(casing, "exon") && !canDoIntrons) ||
    (sameString(casing, "cds") && !canDoUTR))
    casing = "upper";
cgiMakeRadioButton("hgSeq.casing", "upper", sameString(casing, "upper"));
puts("All upper case. <BR>");
cgiMakeRadioButton("hgSeq.casing", "lower", sameString(casing, "lower"));
puts("All lower case. <BR>");

cgiMakeCheckBox("hgSeq.maskRepeats",
		cartCgiUsualBoolean(cart, "hgSeq.maskRepeats", FALSE));
puts("Mask repeats: ");

repMasking = cartCgiUsualString(cart, "hgSeq.repMasking", "lower");
cgiMakeRadioButton("hgSeq.repMasking", "lower",
		   sameString(repMasking, "lower"));
puts(" to lower case ");
cgiMakeRadioButton("hgSeq.repMasking", "N", sameString(repMasking, "N"));
puts(" to N <BR>");
if (offerRevComp)
    {
    cgiMakeCheckBox("hgSeq.revComp",
		    cartCgiUsualBoolean(cart, "hgSeq.revComp", FALSE));
    puts("Reverse complement (get \'-\' strand sequence)");
    }
}


void hgSeqOptionsHtiCart(struct hTableInfo *hti, struct cart *cart)
/* Print out HTML FORM entries for gene region and sequence display options. 
 * Use defaults from CGI and cart. */
{
boolean canDoUTR, canDoIntrons, offerRevComp;

if (hti == NULL)
    {
    canDoUTR = canDoIntrons = FALSE;
    offerRevComp = TRUE;
    }
else
    {
    canDoUTR = hti->hasCDS;
    canDoIntrons = hti->hasBlocks;
    offerRevComp = FALSE;
    }
hgSeqFeatureRegionOptions(cart, canDoUTR, canDoIntrons);
hgSeqDisplayOptions(cart, canDoUTR, canDoIntrons, offerRevComp);
}


void hgSeqOptionsHti(struct hTableInfo *hti)
/* Print out HTML FORM entries for gene region and sequence display options.
 * Use defaults from CGI. */
{
hgSeqOptionsHtiCart(hti, NULL);
}


void hgSeqOptionsDb(char *db, char *table)
/* Print out HTML FORM entries for gene region and sequence display options. */
{
struct hTableInfo *hti;
char chrom[32];
char rootName[256];

if ((table == NULL) || (table[0] == 0))
    {
    hti = NULL;
    }
else
    {
    hParseTableName(table, rootName, chrom);
    hti = hFindTableInfoDb(db, chrom, rootName);
    if (hti == NULL)
	webAbort("Error", "Could not find table info for table %s (%s)",
		 rootName, table);
    }
hgSeqOptionsHti(hti);
}


void hgSeqOptions(char *table)
/* Print out HTML FORM entries for gene region and sequence display options. */
{
hgSeqOptionsDb(hGetDb(), table);
}


static void maskRepeats(char *chrom, int chromStart, int chromEnd,
			DNA *dna, boolean soft)
/* Lower case bits corresponding to repeats. */
{
int rowOffset;
struct sqlConnection *conn;
struct sqlResult *sr;
char **row;

conn = hAllocConn();
sr = hRangeQuery(conn, "rmsk", chrom, chromStart, chromEnd, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct rmskOut ro;
    rmskOutStaticLoad(row+rowOffset, &ro);
    if (ro.genoEnd > chromEnd) ro.genoEnd = chromEnd;
    if (ro.genoStart < chromStart) ro.genoStart = chromStart;
    if (soft)
	toLowerN(dna+ro.genoStart-chromStart, ro.genoEnd - ro.genoStart);
    else
        memset(dna+ro.genoStart-chromStart, 'n', ro.genoEnd - ro.genoStart);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
}


void hgSeqConcatRegionsDb(char *db, char *chrom, char strand, char *name,
			  int rCount, unsigned *rStarts, unsigned *rSizes,
			  boolean *exonFlags, boolean *cdsFlags)
/* Concatenate and print out dna for a series of regions. */
{
struct dnaSeq *rSeq;
struct dnaSeq *cSeq;
char recName[256];
int seqStart, seqEnd;
int offset, cSize;
int i;
boolean isRc     = (strand == '-') || cgiBoolean("hgSeq.revComp");
boolean maskRep  = cgiBoolean("hgSeq.maskRepeats");
int padding5     = cgiOptionalInt("hgSeq.padding5", 0);
int padding3     = cgiOptionalInt("hgSeq.padding3", 0);
char *casing     = cgiString("hgSeq.casing");
char *repMasking = cgiString("hgSeq.repMasking");
char *granularity  = cgiOptionalString("hgSeq.granularity");
boolean concatRegions = granularity && sameString("gene", granularity);

if (rCount < 1)
    return;

/* Don't support padding if granularity is gene (i.e. concat'ing all). */
if (concatRegions)
    {
    padding5 = padding3 = 0;
    }

i = rCount - 1;
seqStart = rStarts[0]             - (isRc ? padding3 : padding5);
seqEnd   = rStarts[i] + rSizes[i] + (isRc ? padding5 : padding3);
if (seqEnd <= seqStart)
    {
    printf("# Null range for %s%s%s_%s (range=%s:%d-%d 5'pad=%d 3'pad=%d)\n",
	   db, 
	   (sameString(db, hGetDb()) ? "" : "_"),
	   (sameString(db, hGetDb()) ? "" : hGetDb()),
	   name,
	   chrom, seqStart+1, seqEnd,
	   padding5, padding3);
    return;
    }
rSeq = hDnaFromSeq(chrom, seqStart, seqEnd, dnaLower);

/* Handle casing and compute size of concatenated sequence */
if (sameString(casing, "upper"))
    touppers(rSeq->dna);
cSize = 0;
for (i=0;  i < rCount;  i++)
    {
    if ((sameString(casing, "exon") && exonFlags[i]) ||
	(sameString(casing, "cds") && cdsFlags[i]))
	{
	int rStart = rStarts[i] - seqStart;
	toUpperN(rSeq->dna+rStart, rSizes[i]);
	}
    cSize += rSizes[i];
    }
cSize += (padding5 + padding3);
AllocVar(cSeq);
cSeq->dna = needMem(cSize+1);
cSeq->size = cSize;

if (maskRep)
    if (sameString(repMasking, "lower"))
	maskRepeats(chrom, seqStart, seqEnd, rSeq->dna, TRUE);
    else
	maskRepeats(chrom, seqStart, seqEnd, rSeq->dna, FALSE);

offset = 0;
for (i=0;  i < rCount;  i++)
    {
    int start = rStarts[i] - seqStart;
    int size  = rSizes[i];
    if (i == 0)
	{
	start -= (isRc ? padding3 : padding5);
	assert(start == 0);
	size  += (isRc ? padding3 : padding5);
	}
    if (i == rCount-1)
	{
	size  += (isRc ? padding5 : padding3);
	}
    memcpy(cSeq->dna+offset, rSeq->dna+start, size);
    offset += size;
    }
assert(offset == cSeq->size);
cSeq->dna[offset] = 0;
freeDnaSeq(&rSeq);

if (isRc)
    reverseComplement(cSeq->dna, cSeq->size);

sprintf(recName, "%s%s%s_%s range=%s:%d-%d 5'pad=%d 3'pad=%d revComp=%s strand=%c repeatMasking=%s",
	db, 
	(sameString(db, hGetDb()) ? "" : "_"),
	(sameString(db, hGetDb()) ? "" : hGetDb()),
	name,
	chrom, seqStart+1, seqEnd,
	padding5, padding3,
	(isRc ? "TRUE" : "FALSE"),
	strand,
	(maskRep ? repMasking : "none"));
faWriteNext(stdout, recName, cSeq->dna, cSeq->size);
}


void hgSeqRegionsAdjDb(char *db, char *chrom, char strand, char *name,
		       boolean concatRegions, boolean concatAdjacent,
		       int rCount, unsigned *rStarts, unsigned *rSizes,
		       boolean *exonFlags, boolean *cdsFlags)
/* Concatenate and print out dna for a series of regions, 
 * optionally concatenating adjacent exons. */
{
if (concatRegions || (rCount == 1))
    {
    hgSeqConcatRegionsDb(hGetDb(), chrom, strand, name,
			 rCount, rStarts, rSizes, exonFlags, cdsFlags);
    }
else
    {
    int i, count;
    boolean isRc = (strand == '-');
    char rName[256];
    for (i=0,count=0;  i < rCount;  i++,count++)
	{
	int j, jEnd, len, lo, hi;
	snprintf(rName, sizeof(rName), "%s_%d", name, count);
	j = (isRc ? (rCount - i - 1) : i);
	jEnd = (isRc ? (j - 1) : (j + 1));
	if (concatAdjacent && exonFlags[j])
	    {
	    lo = (isRc ? jEnd       : (jEnd - 1));
	    hi = (isRc ? (jEnd + 1) : jEnd);
	    while ((i < rCount) &&
		   ((rStarts[lo] + rSizes[lo]) == rStarts[hi]) &&
		   exonFlags[jEnd])
		{
		i++;
		jEnd = (isRc ? (jEnd - 1) : (jEnd + 1));
		lo = (isRc ? jEnd       : (jEnd - 1));
		hi = (isRc ? (jEnd + 1) : jEnd);
		}
	    }
	len = (isRc ? (j - jEnd) : (jEnd - j));
	lo  = (isRc ? (jEnd + 1) : j);
	hgSeqConcatRegionsDb(hGetDb(), chrom, strand, rName,
			     len, &rStarts[lo], &rSizes[lo], &exonFlags[lo],
			     &cdsFlags[lo]);
	}
    }
}

void hgSeqRegionsDb(char *db, char *chrom, char strand, char *name,
		    boolean concatRegions,
		    int rCount, unsigned *rStarts, unsigned *rSizes,
		    boolean *exonFlags, boolean *cdsFlags)
/* Concatenate and print out dna for a series of regions. */
{
hgSeqRegionsAdjDb(db, chrom, strand, name, concatRegions, FALSE,
		  rCount, rStarts, rSizes, exonFlags, cdsFlags);
}

void hgSeqRegions(char *chrom, char strand, char *name,
		  boolean concatRegions,
		  int rCount, unsigned *rStarts, unsigned *rSizes,
		  boolean *exonFlags, boolean *cdsFlags)
/* Concatenate and print out dna for a series of regions. */
{
hgSeqRegionsDb(hGetDb(), chrom, strand, name, concatRegions,
	       rCount, rStarts, rSizes, exonFlags, cdsFlags);
}


void addFeature(int *count, unsigned *starts, unsigned *sizes,
		boolean *exonFlags, boolean *cdsFlags,
		int start, int size, boolean eFlag, boolean cFlag)
{
if (size > 0)
    {
    if (start < 0)
	start = 0;
    if (start + size > chromSize)
	size = chromSize - start;
    starts[*count]    = start;
    sizes[*count]     = size;
    exonFlags[*count] = eFlag;
    cdsFlags[*count]  = cFlag;
    (*count)++;
    }
}


void hgSeqRange(char *chrom, int chromStart, int chromEnd, char strand,
		char *name)
/* Print out dna sequence for the given range. */
{
int count = 0;
int starts[1];
int sizes[1];
boolean exonFlags[1];
boolean cdsFlags[1];

chromSize = hChromSize(chrom);
addFeature(&count, starts, sizes, exonFlags, cdsFlags,
	   chromStart, chromEnd - chromStart, FALSE, FALSE);
hgSeqRegions(chrom, strand, name, FALSE, count, starts, sizes, exonFlags,
	     cdsFlags);
}


int hgSeqBedDb(char *db, struct hTableInfo *hti, struct bed *bedList)
/* Print out dna sequence from the given database of all items in bedList.  
 * hti describes the bed-compatibility level of bedList items.  
 * Returns number of FASTA records printed out. */
{
struct bed *bedItem;
char itemName[128];
boolean isRc;
int count;
unsigned *starts = NULL;
unsigned *sizes = NULL;
boolean *exonFlags = NULL;
boolean *cdsFlags = NULL;
int i, rowCount, totalCount;
boolean promoter   = cgiBoolean("hgSeq.promoter");
boolean intron     = cgiBoolean("hgSeq.intron");
boolean utrExon5   = cgiBoolean("hgSeq.utrExon5");
boolean utrIntron5 = utrExon5 && intron;
boolean cdsExon    = cgiBoolean("hgSeq.cdsExon");
boolean cdsIntron  = cdsExon && intron;
boolean utrExon3   = cgiBoolean("hgSeq.utrExon3");
boolean utrIntron3 = utrExon3 && intron;
boolean downstream = cgiBoolean("hgSeq.downstream");
int promoterSize   = cgiOptionalInt("hgSeq.promoterSize", 0);
int downstreamSize = cgiOptionalInt("hgSeq.downstreamSize", 0);
char *granularity  = cgiOptionalString("hgSeq.granularity");
boolean concatRegions = granularity && sameString("gene", granularity);
boolean concatAdjacent = (cgiBooleanDefined("hgSeq.splitCDSUTR") &&
			  (! cgiBoolean("hgSeq.splitCDSUTR")));
boolean isCDS, doIntron;
boolean foundFields;
boolean canDoUTR, canDoIntrons;

/* catch a special case: introns selected, but no exons -> include all introns
 * instead of qualifying intron with exon flags. */
if (intron && !(utrExon5 || cdsExon || utrExon3))
    {
    utrIntron5 = cdsIntron = utrIntron3 = TRUE;
    }

canDoUTR = hti->hasCDS;
canDoIntrons = hti->hasBlocks;

rowCount = totalCount = 0;
for (bedItem = bedList;  bedItem != NULL;  bedItem = bedItem->next)
    {
    rowCount++;
    chromSize = hChromSize(bedItem->chrom);
    // bed: translate relative starts to absolute starts
    for (i=0;  i < bedItem->blockCount;  i++)
	{
	bedItem->chromStarts[i] += bedItem->chromStart;
	}
    isRc = (bedItem->strand[0] == '-');
    // here's the max # of feature regions:
    if (canDoIntrons)
	count = 2 + (2 * bedItem->blockCount);
    else
	count = 5;
    starts    = needMem(sizeof(unsigned) * count);
    sizes     = needMem(sizeof(unsigned) * count);
    exonFlags = needMem(sizeof(boolean) * count);
    cdsFlags  = needMem(sizeof(boolean) * count);
    // build up a list of selected regions
    count = 0;
    if (!isRc && promoter && (promoterSize > 0))
	{
	addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		   (bedItem->chromStart - promoterSize), promoterSize,
		   FALSE, FALSE);
	}
    else if (isRc && downstream && (downstreamSize > 0))
	{
	addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		   (bedItem->chromStart - downstreamSize), downstreamSize,
		   FALSE, FALSE);
	}
    if (canDoIntrons && canDoUTR)
	{
	for (i=0;  i < bedItem->blockCount;  i++)
	    {
	    if ((bedItem->chromStarts[i] + bedItem->blockSizes[i]) <=
		bedItem->thickStart)
		{
		if ((!isRc && utrExon5)   || (isRc && utrExon3))
		    {
		    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			       bedItem->chromStarts[i], bedItem->blockSizes[i],
			       TRUE, FALSE);
		    }
		if (((!isRc && utrIntron5) || (isRc && utrIntron3)) &&
		    (i < bedItem->blockCount - 1))
		    {
		    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			       (bedItem->chromStarts[i] +
				bedItem->blockSizes[i]),
			       (bedItem->chromStarts[i+1] -
				bedItem->chromStarts[i] -
				bedItem->blockSizes[i]),
			       FALSE, FALSE);
		    }
		}
	    else if (bedItem->chromStarts[i] < bedItem->thickEnd)
		{
		if ((bedItem->chromStarts[i] < bedItem->thickStart) &&
		    ((bedItem->chromStarts[i] + bedItem->blockSizes[i]) >
		     bedItem->thickEnd))
		    {
		    if ((!isRc && utrExon5)	  || (isRc && utrExon3))
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->chromStarts[i],
				   (bedItem->thickStart -
				    bedItem->chromStarts[i]),
				   TRUE, FALSE);
			}
		    if (cdsExon)
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->thickStart,
				   (bedItem->thickEnd - bedItem->thickStart),
				   TRUE, TRUE);
			}
		    if ((!isRc && utrExon3)	  || (isRc && utrExon5))
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->thickEnd,
				   (bedItem->chromStarts[i] +
				    bedItem->blockSizes[i] -
				    bedItem->thickEnd),
				   TRUE, FALSE);
			}
		    }
		else if (bedItem->chromStarts[i] < bedItem->thickStart)
		    {
		    if ((!isRc && utrExon5)	  || (isRc && utrExon3))
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->chromStarts[i],
				   (bedItem->thickStart -
				    bedItem->chromStarts[i]),
				   TRUE, FALSE);
			}
		    if (cdsExon)
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->thickStart,
				   (bedItem->chromStarts[i] +
				    bedItem->blockSizes[i] -
				    bedItem->thickStart),
				   TRUE, TRUE);
			}
		    }
		else if ((bedItem->chromStarts[i] + bedItem->blockSizes[i]) >
			 bedItem->thickEnd)
		    {
		    if (cdsExon)
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->chromStarts[i],
				   (bedItem->thickEnd -
				    bedItem->chromStarts[i]),
				   TRUE, TRUE);
			}
		    if ((!isRc && utrExon3)	  || (isRc && utrExon5))
			{
			addFeature(&count, starts, sizes, exonFlags, cdsFlags,
				   bedItem->thickEnd,
				   (bedItem->chromStarts[i] +
				    bedItem->blockSizes[i] -
				    bedItem->thickEnd),
				   TRUE, FALSE);
			}
		    }
		else if (cdsExon)
		    {
		    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			       bedItem->chromStarts[i], bedItem->blockSizes[i],
			       TRUE, TRUE);
		    }
		isCDS = ! ((bedItem->chromStarts[i] + bedItem->blockSizes[i]) >
			   bedItem->thickEnd);
		doIntron = (isCDS ? cdsIntron :
			    ((!isRc) ? utrIntron3 : utrIntron5));
		if (doIntron && (i < bedItem->blockCount - 1))
		    {
		    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			       (bedItem->chromStarts[i] +
				bedItem->blockSizes[i]),
			       (bedItem->chromStarts[i+1] -
				bedItem->chromStarts[i] -
				bedItem->blockSizes[i]),
			       FALSE, isCDS);
		    }
		}
	    else
		{
		if ((!isRc && utrExon3)   || (isRc && utrExon5))
		    {
		    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			       bedItem->chromStarts[i], bedItem->blockSizes[i],
			       TRUE, FALSE);
		    }
		if (((!isRc && utrIntron3) || (isRc && utrIntron5)) &&
		    (i < bedItem->blockCount - 1))
		    {
		    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			       (bedItem->chromStarts[i] +
				bedItem->blockSizes[i]),
			       (bedItem->chromStarts[i+1] -
				bedItem->chromStarts[i] -
				bedItem->blockSizes[i]),
			       FALSE, FALSE);
		    }
		}
	    }
	}
    else if (canDoIntrons)
	{
	for (i=0;  i < bedItem->blockCount;  i++)
	    {
	    if (cdsExon)
		{
		addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			   bedItem->chromStarts[i], bedItem->blockSizes[i],
			   TRUE, FALSE);
		}
	    if (cdsIntron && (i < bedItem->blockCount - 1))
		{
		addFeature(&count, starts, sizes, exonFlags, cdsFlags,
			   (bedItem->chromStarts[i] + bedItem->blockSizes[i]),
			   (bedItem->chromStarts[i+1] -
			    bedItem->chromStarts[i] -
			    bedItem->blockSizes[i]),
			   FALSE, FALSE);
		}
	    }
	}
    else if (canDoUTR)
	{
	if ((!isRc && utrExon5)   || (isRc && utrExon3))
	    {
	    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		       bedItem->chromStart,
		       (bedItem->thickStart - bedItem->chromStart),
		       TRUE, FALSE);
	    }
	if (cdsExon)
	    {
	    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		       bedItem->thickStart,
		       (bedItem->thickEnd - bedItem->thickStart),
		       TRUE, TRUE);
	    }
	if ((!isRc && utrExon3)   || (isRc && utrExon5))
	    {
	    addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		       bedItem->thickEnd,
		       (bedItem->chromEnd - bedItem->thickEnd),
		       TRUE, FALSE);
	    }
	}
    else
	{
	addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		   bedItem->chromStart,
		   (bedItem->chromEnd - bedItem->chromStart),
		   TRUE, FALSE);
	}
    if (!isRc && downstream && (downstreamSize > 0))
	{
	addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		   bedItem->chromEnd, downstreamSize, FALSE, FALSE);
	}
    else if (isRc && promoter && (promoterSize > 0))
	{
	addFeature(&count, starts, sizes, exonFlags, cdsFlags,
		   bedItem->chromEnd, promoterSize, FALSE, FALSE);
	}
    snprintf(itemName, sizeof(itemName), "%s_%s", hti->rootName, bedItem->name);
    hgSeqRegionsAdjDb(db, bedItem->chrom, bedItem->strand[0], itemName,
		      concatRegions, concatAdjacent,
		      count, starts, sizes, exonFlags, cdsFlags);
    totalCount += count;
    freeMem(starts);
    freeMem(sizes);
    freeMem(exonFlags);
    freeMem(cdsFlags);
    }
return totalCount;
}


int hgSeqBed(struct hTableInfo *hti, struct bed *bedList)
/* Print out dna sequence from the current database of all items in bedList.  
 * hti describes the bed-compatibility level of bedList items.  
 * Returns number of FASTA records printed out. */
{
return hgSeqBedDb(hGetDb(), hti, bedList);
}


int hgSeqItemsInRangeDb(char *db, char *table, char *chrom, int chromStart,
			int chromEnd, char *sqlConstraints)
/* Print out dna sequence of all items (that match sqlConstraints, if nonNULL) 
   in the given range in table.  Return number of items. */
{
struct hTableInfo *hti;
struct bed *bedList;
char rootName[256];
char parsedChrom[32];
int itemCount;

hParseTableName(table, rootName, parsedChrom);
hti = hFindTableInfoDb(db, chrom, rootName);
if (hti == NULL)
    webAbort("Error", "Could not find table info for table %s (%s)",
	     rootName, table);
bedList = hGetBedRangeDb(db, table, chrom, chromStart, chromEnd,
			 sqlConstraints);

itemCount = hgSeqBedDb(db, hti, bedList);
bedFreeList(&bedList);
return itemCount;
}


int hgSeqItemsInRange(char *table, char *chrom, int chromStart, int chromEnd,
		       char *sqlConstraints)
/* Print out dna sequence of all items (that match sqlConstraints, if nonNULL) 
   in the given range in table.  Return number of items. */
{
return hgSeqItemsInRangeDb(hGetDb(), table, chrom, chromStart, chromEnd,
			   sqlConstraints);
}


