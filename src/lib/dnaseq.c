/* dnaSeq.c - stuff to manage DNA sequences. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "dnaseq.h"
#include "bits.h"

static char const rcsid[] = "$Id: dnaseq.c,v 1.13 2003/05/06 07:33:42 kate Exp $";


struct dnaSeq *newDnaSeq(DNA *dna, int size, char *name)
/* Create a new DNA seq. */
{
struct dnaSeq *seq;

seq = needMem(sizeof(*seq));
if (name != NULL)
    seq->name = cloneString(name);
seq->dna = dna;
seq->size = size;
seq->mask = NULL;
return seq;
}

struct dnaSeq *cloneDnaSeq(struct dnaSeq *orig)
/* Duplicate dna sequence in RAM. */
{
struct dnaSeq *seq = CloneVar(orig);
seq->name = cloneString(seq->name);
seq->dna = needHugeMem(seq->size+1);
memcpy(seq->dna, orig->dna, seq->size+1);
seq->mask = NULL;
if (orig->mask != NULL)
    {
    seq->mask = bitClone(orig->mask, seq->size);
    }
return seq;
}

void freeDnaSeq(struct dnaSeq **pSeq)
/* Free up DNA seq. (And unlink underlying resource node.) */
{
struct dnaSeq *seq = *pSeq;
if (seq == NULL)
    return;
freeMem(seq->name);
freeMem(seq->dna);
bitFree(&seq->mask);
freez(pSeq);
}

void freeDnaSeqList(struct dnaSeq **pSeqList)
/* Free up list of DNA sequences. */
{
struct dnaSeq *seq, *next;

for (seq = *pSeqList; seq != NULL; seq = next)
    {
    next = seq->next;
    freeDnaSeq(&seq);
    }
*pSeqList = NULL;
}

boolean seqIsDna(bioSeq *seq)
/* Make educated guess whether sequence is DNA or protein. */
{
int size = seq->size, i;
char *poly = seq->dna;

dnaUtilOpen();
for (i=0; i<size; ++i)
    {
    if (!ntChars[poly[i]]) 
    	return FALSE;
    }
return TRUE;
}


aaSeq *translateSeq(struct dnaSeq *inSeq, int offset, boolean stop)
/* Return a translated sequence.  Offset is position of first base to
 * translate. If stop is TRUE then stop at first stop codon.  (Otherwise 
 * represent stop codons as 'Z'). */
{
aaSeq *seq;
DNA *dna = inSeq->dna;
AA *pep, aa;
int inSize = inSeq->size;
int i, lastCodon = inSize - 3;
int txSize = (inSize-offset)/3;
int actualSize = 0;
char buf[256];

AllocVar(seq);
seq->dna = pep = needLargeMem(txSize+1);
for (i=offset; i <= lastCodon; i += 3)
    {
    aa = lookupCodon(dna+i);
    if (aa == 0)
	{
        if (stop)
	    break;
	else
	    aa = 'Z';
	}
    *pep++ = aa;
    ++actualSize;
    }
*pep = 0;
assert(actualSize <= txSize);
seq->size = actualSize;
seq->name = cloneString(inSeq->name);
return seq;
}

bioSeq *whichSeqIn(bioSeq **seqs, int seqCount, char *letters)
/* Figure out which if any sequence letters is in. */
{
aaSeq *seq;
int i;

for (i=0; i<seqCount; ++i)
    {
    seq = seqs[i];
    if (seq->dna <= letters && letters < seq->dna + seq->size)
        return seq;
    }
internalErr();
return NULL;
}

Bits *maskFromUpperCaseSeq(bioSeq *seq)
/* Allocate a mask for sequence and fill it in based on
 * sequence case. */
{
int size = seq->size, i;
char *poly = seq->dna;
Bits *b = bitAlloc(size);
for (i=0; i<size; ++i)
    {
    if (isupper(poly[i]))
        bitSetOne(b, i);
    }
return b;
}

