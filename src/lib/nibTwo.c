/* nibTwo - Something to let you transparently access either
 * .2bit or .nib files. */

#include "common.h"
#include "hash.h"
#include "dnaseq.h"
#include "nib.h"
#include "twoBit.h"
#include "nibTwo.h"

static char const rcsid[] = "$Id: nibTwo.c,v 1.2 2004/10/24 04:03:39 kent Exp $";

struct nibTwoCache *nibTwoCacheNew(char *pathName)
/* Get something that will more or less transparently get sequence from 
 * nib files or .2bit. */ 
{
struct nibTwoCache *ntc;
AllocVar(ntc);
ntc->pathName = cloneString(pathName);
ntc->isTwoBit = twoBitIsFile(pathName);
if (ntc->isTwoBit)
    ntc->tbf = twoBitOpen(pathName);
else
    ntc->nibHash = newHash(10);
return ntc;
}

void nibTwoCacheFree(struct nibTwoCache **pNtc)
/* Free up resources associated with nibTwoCache. */
{
struct nibTwoCache *ntc = *pNtc;
if (ntc != NULL)
    {
    freez(&ntc->pathName);
    if (ntc->isTwoBit)
        twoBitClose(&ntc->tbf);
    else
        {
	struct hashEl *el, *list = hashElListHash(ntc->nibHash);
	struct nibInfo *nib;
	for (el = list; el != NULL; el = el->next)
	     {
	     nib = el->val;
	     nibInfoFree(&nib);
	     }
	hashElFreeList(&list);
	hashFree(&ntc->nibHash);
	}
    freez(pNtc);
    }
}

struct dnaSeq *nibTwoCacheSeq(struct nibTwoCache *ntc, char *seqName)
/* Return all of sequence. This will have repeats in lower case. */
{
if (ntc->isTwoBit)
    return twoBitReadSeqFrag(ntc->tbf, seqName, 0, 0);
else
    {
    struct nibInfo *nib = nibInfoFromCache(ntc->nibHash, ntc->pathName, seqName);
    return nibLdPart(nib->fileName, nib->f, 0, nib->size, nib->size);
    }
}

struct dnaSeq *nibTwoCacheSeqPart(struct nibTwoCache *ntc, char *seqName, int start, int size,
	int *retFullSeqSize)
/* Return part of sequence. If *retFullSeqSize is non-null then return full size of
 * sequence (not just loaded part) there. This will have repeats in lower case. */
{
if (ntc->isTwoBit)
    {
    if (retFullSeqSize != NULL)
        *retFullSeqSize = twoBitSeqSize(ntc->tbf, seqName);
    return twoBitReadSeqFrag(ntc->tbf, seqName, start, start+size);
    }
else
    {
    struct nibInfo *nib = nibInfoFromCache(ntc->nibHash, ntc->pathName, seqName);
    if (retFullSeqSize != NULL)
        *retFullSeqSize = nib->size;
    return nibLdPartMasked(NIB_MASK_MIXED, nib->fileName, nib->f, nib->size, start, size);
    }
}

struct dnaSeq *nibTwoLoadOne(char *pathName, char *seqName)
/* Return sequence from a directory full of nibs or a .2bit file. 
 * The sequence will have repeats in lower case. */
{
struct dnaSeq *seq;
if (twoBitIsFile(pathName))
    {
    struct twoBitFile *tbf = twoBitOpen(pathName);
    seq = twoBitReadSeqFrag(tbf, seqName, 0, 0);
    twoBitClose(&tbf);
    }
else
    {
    char path[512];
    sprintf(path, "%s/%s.nib", pathName, seqName);
    seq = nibLoadAllMasked(NIB_MASK_MIXED, path);
    }
return seq;
}

