/* pslShow - stuff to help visual psl format alignments. 
 * This file is copyright 2002-2004 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "dnaseq.h"
#include "htmshell.h"
#include "psl.h"
#include "cda.h"
#include "seqOut.h"

static char const rcsid[] = "$Id: pslShow.c,v 1.2 2004/06/16 22:05:06 braney Exp $";

static void pslShowAlignmentStranded(struct psl *psl, boolean isProt,
	char *qName, bioSeq *qSeq, int qStart, int qEnd,
	char *tName, bioSeq *tSeq, int tStart, int tEnd, FILE *f)
/* Show protein/DNA alignment or translated DNA alignment in HTML format. */
{
boolean tIsRc = (psl->strand[1] == '-');
boolean qIsRc = (psl->strand[0] == '-');
int mulFactor = (isProt ? 3 : 1);
DNA *dna = NULL;	/* Mixed case version of genomic DNA. */
int qSize = qSeq->size;
char *qLetters = cloneString(qSeq->dna);
int qbafStart, qbafEnd, tbafStart, tbafEnd;
int qcfmStart, qcfmEnd, tcfmStart, tcfmEnd;

tbafStart = psl->tStart;
tbafEnd   = psl->tEnd;
tcfmStart = psl->tStart;
tcfmEnd   = psl->tEnd;

qbafStart = qStart;
qbafEnd   = qEnd;
qcfmStart = qStart;
qcfmEnd   = qEnd;

/* Deal with minus strand. */
if (tIsRc)
    {
    int temp;
    reverseComplement(tSeq->dna, tSeq->size);
    temp = psl->tSize - tEnd;
    tEnd = psl->tSize - tStart;
    tStart = temp;
    
    tbafStart = psl->tEnd;
    tbafEnd   = psl->tStart;
    tcfmStart = psl->tEnd;
    tcfmEnd   = psl->tStart;
    }
if (qIsRc)
    {
    int temp;
    reverseComplement(qSeq->dna, qSeq->size);
    reverseComplement(qLetters, qSeq->size);

    qcfmStart = qEnd;
    qcfmEnd   = qStart;
    qbafStart = qEnd;
    qbafEnd   = qStart;
    
    temp = psl->qSize - qEnd;
    qEnd = psl->qSize - qStart;
    qStart = temp;
    }
dna = cloneString(tSeq->dna);

if (qName == NULL) 
    qName = psl->qName;
if (tName == NULL)
    tName = psl->tName;


fprintf(f, "<H4><A NAME=cDNA></A>%s%s</H4>\n", qName, (qIsRc  ? " (reverse complemented)" : ""));
fprintf(f, "<PRE><TT>");
tolowers(qLetters);

/* Display query sequence. */
    {
    struct cfm *cfm;
    char *colorFlags = needMem(qSeq->size);
    int i,j;

    for (i=0; i<psl->blockCount; ++i)
	{
	int qs = psl->qStarts[i] - qStart;
	int ts = psl->tStarts[i] - tStart;
	int sz = psl->blockSizes[i]-1;
	colorFlags[qs] = socBrightBlue;
	qLetters[qs] = toupper(qLetters[qs]);
	colorFlags[qs+sz] = socBrightBlue;
	qLetters[qs+sz] = toupper(qLetters[qs+sz]);
	if (isProt)
	    {
	    for (j=1; j<sz; ++j)
		{
		AA aa = qSeq->dna[qs+j];
		DNA *codon = &tSeq->dna[ts + 3*j];
		AA trans = lookupCodon(codon);
		if (trans != 'X' && trans == aa)
		    {
		    colorFlags[qs+j] = socBlue;
		    qLetters[qs+j] = toupper(qLetters[qs+j]);
		    }
		}
	    }
	else
	    {
	    for (j=1; j<sz; ++j)
		{
		if (qSeq->dna[qs+j] == tSeq->dna[ts+j])
		    {
		    colorFlags[qs+j] = socBlue;
		    qLetters[qs+j] = toupper(qLetters[qs+j]);
		    }
		}
	    }
	}
    cfm = cfmNew(10, 60, TRUE, qIsRc, f, qcfmStart);
    for (i=0; i<qSize; ++i)
	cfmOut(cfm, qLetters[i], seqOutColorLookup[colorFlags[i]]);
    cfmFree(&cfm);
    freez(&colorFlags);
    htmHorizontalLine(f);
    }
fprintf(f, "</TT></PRE>\n");
fprintf(f, "<H4><A NAME=genomic></A>%s %s:</H4>\n", 
	tName, (tIsRc ? "(reverse strand)" : ""));
fprintf(f, "<PRE><TT>");

/* Display DNA sequence. */
    {
    struct cfm *cfm;
    char *colorFlags = needMem(tSeq->size);
    int i,j;
    int curBlock = 0;
    int anchorCount = 0;

    for (i=0; i<psl->blockCount; ++i)
	{
	int qs = psl->qStarts[i] - qStart;
	int ts = psl->tStarts[i] - tStart;
	int sz = psl->blockSizes[i];
	if (isProt)
	    {
	    for (j=0; j<sz; ++j)
		{
		AA aa = qSeq->dna[qs+j];
		int codonStart = ts + 3*j;
		DNA *codon = &tSeq->dna[codonStart];
		AA trans = lookupCodon(codon);
		if (trans != 'X' && trans == aa)
		    {
		    colorFlags[codonStart] = socBlue;
		    colorFlags[codonStart+1] = socBlue;
		    colorFlags[codonStart+2] = socBlue;
		    toUpperN(dna+codonStart, 3);
		    }
		}
	    }
	else
	    {
	    for (j=0; j<sz; ++j)
		{
		if (qSeq->dna[qs+j] == tSeq->dna[ts+j])
		    {
		    colorFlags[ts+j] = socBlue;
		    dna[ts+j] = toupper(dna[ts+j]);
		    }
		}
	    }
	colorFlags[ts] = socBrightBlue;
	colorFlags[ts+sz*mulFactor-1] = socBrightBlue;
	}

    cfm = cfmNew(10, 60, TRUE, tIsRc, f, tcfmStart);
	
    for (i=0; i<tSeq->size; ++i)
	{
	/* Put down "anchor" on first match position in haystack
	 * so user can hop here with a click on the needle. */
	if (curBlock < psl->blockCount && psl->tStarts[curBlock] == (i + tStart) )
	    {
	    fprintf(f, "<A NAME=%d></A>", ++curBlock);
	    /* Watch out for (rare) out-of-order tStarts! */
	    while (curBlock < psl->blockCount &&
		   psl->tStarts[curBlock] <= tStart + i)
		curBlock++;
	    }
	cfmOut(cfm, dna[i], seqOutColorLookup[colorFlags[i]]);
	}
    cfmFree(&cfm);
    freez(&colorFlags);
    htmHorizontalLine(f);
    }

/* Display side by side. */
fprintf(f, "</TT></PRE>\n");
fprintf(f, "<H4><A NAME=ali></A>Side by Side Alignment*</H4>\n");
fprintf(f, "<PRE><TT>");
    {
    struct baf baf;
    int i,j;

    bafInit(&baf, qSeq->dna, qbafStart, qIsRc,
	    tSeq->dna, tbafStart, tIsRc, f, 60, isProt);
		
    if (isProt)
	{
	for (i=0; i<psl->blockCount; ++i)
	    {
	    int qs = psl->qStarts[i] - qStart;
	    int ts = psl->tStarts[i] - tStart;
	    int sz = psl->blockSizes[i];

	    bafSetPos(&baf, qs, ts);
	    bafStartLine(&baf);
	    for (j=0; j<sz; ++j)
		{
		AA aa = qSeq->dna[qs+j];
		int codonStart = ts + 3*j;
		DNA *codon = &tSeq->dna[codonStart];
		bafOut(&baf, ' ', codon[0]);
		bafOut(&baf, aa, codon[1]);
		bafOut(&baf, ' ', codon[2]);
		}
	    bafFlushLine(&baf);
	    }
	fprintf( f, "<I>*when aa is different, BLOSUM positives are in green, BLOSUM negatives in red</I>\n");
	}
    else
	{
	int lastQe = psl->qStarts[0] - qStart;
	int lastTe = psl->tStarts[0] - tStart;
	int maxSkip = 20;
	bafSetPos(&baf, lastQe, lastTe);
	bafStartLine(&baf);
	for (i=0; i<psl->blockCount; ++i)
	    {
	    int qs = psl->qStarts[i] - qStart;
	    int ts = psl->tStarts[i] - tStart;
	    int sz = psl->blockSizes[i];
	    boolean doBreak = TRUE;
	    int qSkip = qs - lastQe;
	    int tSkip = ts - lastTe;

	    if (qSkip >= 0 && qSkip <= maxSkip && tSkip == 0)
		{
		for (j=0; j<qSkip; ++j)
		    bafOut(&baf, qSeq->dna[lastQe+j], '-');
		doBreak = FALSE;
		}
	    else if (tSkip > 0 && tSkip <= maxSkip && qSkip == 0)
		{
		for (j=0; j<tSkip; ++j)
		    bafOut(&baf, '-', tSeq->dna[lastTe+j]);
		doBreak = FALSE;
		}
	    if (doBreak)
		{
		bafFlushLine(&baf);
		bafSetPos(&baf, qs, ts);
		bafStartLine(&baf);
		}
	    for (j=0; j<sz; ++j)
		bafOut(&baf, qSeq->dna[qs+j], tSeq->dna[ts+j]);
	    lastQe = qs + sz;
	    lastTe = ts + sz;
	    }
	bafFlushLine(&baf);

	fprintf( f, "<I>*Aligned Blocks with gaps <= %d bases are merged for this display</I>\n", maxSkip);
	}
    }
fprintf(f, "</TT></PRE>");
if (qIsRc)
    reverseComplement(qSeq->dna, qSeq->size);
if (tIsRc)
    reverseComplement(tSeq->dna, tSeq->size);
freeMem(dna);
freeMem(qLetters);
}

int pslShowAlignment(struct psl *psl, boolean isProt,
	char *qName, bioSeq *qSeq, int qStart, int qEnd,
	char *tName, bioSeq *tSeq, int tStart, int tEnd, FILE *f)
/* Show protein/DNA alignment or translated DNA alignment in HTML format. */
{
/* At this step we just do a little shuffling of the strands for
 * untranslated DNA alignments. */
char origStrand[2];
boolean needsSwap = (psl->strand[0] == '-' && psl->strand[1] == 0);
if (needsSwap)
    {
    memcpy(origStrand, psl->strand, 2);
    pslRcBoth(psl);
    psl->strand[0] = '+';
    psl->strand[1] = '-';
    }
pslShowAlignmentStranded(psl, isProt, qName, qSeq, qStart, qEnd,
    tName, tSeq, tStart, tEnd, f);
if (needsSwap)
    {
    pslRcBoth(psl);
    memcpy(psl->strand, origStrand, 2);
    }
return psl->blockCount;
}

