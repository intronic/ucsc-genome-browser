/* Nib - nibble (4 bit) representation of nucleotide sequences. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "dnautil.h"
#include "dnaseq.h"
#include "nib.h"
#include "sig.h"

static char *findNibSubrange(char *fileName)
/* find the colon starting a nib seq name/subrange in a nib file name, or NULL
 * if none */
{
char *baseName = strrchr(fileName, '/');
baseName = (baseName == NULL) ? fileName : baseName+1;
return strchr(baseName, ':');
}

static void parseSubrange(char *subrange, char *name, int *start, int *end)
/* parse the subrange specification */
{
char *rangePart = strchr(subrange+1, ':');
if (rangePart != NULL)
    {
    /* :seqId:start-end form */
    *rangePart = '\0';
    strcpy(name, subrange+1);
    *rangePart = ':';
    rangePart++;
    }
else
    {
    /* :start-end form */
    rangePart = subrange+1;
    strcpy(name, ""); 
    }
if ((sscanf(rangePart, "%u-%u", start, end) != 2) || (*start > *end))
    errAbort("can't parse nib file subsequence specification: %s",
             subrange);
}

static void parseNibName(unsigned options, char *fileSpec, char *filePath,
                         char *name, unsigned *start, unsigned *end)
/* Parse the nib name, getting the file name, seq name to use, and
 * optionally the start and end positions. Zero is return for start
 * and end if they are not specified. Return the path to the file
S * and the name to use for the sequence. */
{
char *subrange = findNibSubrange(fileSpec);
if (subrange != NULL)
    {
    *subrange = '\0';
    parseSubrange(subrange, name, start, end);
    strcpy(filePath, fileSpec);
    *subrange = ':';
    if (strlen(name) == 0)
        {
        /* no name in spec, generate one */
        if (options & NIB_BASE_NAME)
            splitPath(filePath, NULL, name, NULL);
        else
            strcpy(name, filePath);
        sprintf(name+strlen(name), ":%u-%u", *start, *end);
        }
    }
else
    {
    *start = 0;
    *end = 0;
    strcpy(filePath, fileSpec);
    if (options & NIB_BASE_NAME)
        splitPath(fileSpec, NULL, name, NULL);
    else
        strcpy(name, fileSpec);
    }
}

void nibOpenVerify(char *fileName, FILE **retFile, int *retSize)
/* Open file and verify it's in good nibble format. */
{
bits32 size;
bits32 sig;
FILE *f = mustOpen(fileName, "rb");

dnaUtilOpen();
mustReadOne(f, sig);
mustReadOne(f, size);
if (sig != nibSig)
    {
    sig = byteSwap32(sig);
    size = byteSwap32(size);
    if (sig != nibSig)
	errAbort("%s is not a good .nib file.",  fileName);
    }
*retSize = size;
*retFile = f;
}

static struct dnaSeq *nibInput(int options, char *fileName, char *seqName,
                               FILE *f, int seqSize, int start, int size)
/* Load part of an open .nib file. */
{
int end;
DNA *d;
int bVal;
DNA *valToNtTbl = ((options &  NIB_MASK_MIXED) ? valToNtMasked : valToNt);
struct dnaSeq *seq;
Bits* mask = NULL;
int bytePos, byteSize;
int maskIdx = 0;

assert(start >= 0);
assert(size >= 0);

end = start+size;
if (end > seqSize)
    errAbort("nib read past end of file (%d %d) in file: %s", 
	     end, seqSize, (fileName != NULL ? fileName : "(NULL)"));

AllocVar(seq);
seq->size = size;
seq->name = cloneString(seqName);
seq->dna = d = needLargeMem(size+1);
if (options & NIB_MASK_MIXED)
    seq->mask = mask = bitAlloc(size);

bytePos = (start>>1);
fseek(f, bytePos + 2*sizeof(bits32), SEEK_SET);
if (start & 1)
    {
    bVal = getc(f);
    if (bVal < 0)
	{
	errAbort("Read error 1 in %s", fileName);
	}
    *d++ = valToNtTbl[(bVal&0xf)];
    size -= 1;
    if (mask != NULL)
        {
        if ((bVal&0xf&MASKED_BASE_BIT) == 0)
            bitSetOne(mask, maskIdx);
        maskIdx++;
        }
    }
byteSize = (size>>1);
while (--byteSize >= 0)
    {
    bVal = getc(f);
    if (bVal < 0)
	errAbort("Read error 2 in %s", fileName);
    d[0] = valToNtTbl[(bVal>>4)];
    d[1] = valToNtTbl[(bVal&0xf)];
    d += 2;
    if (mask != NULL)
        {
        if (((bVal>>4)&0xf) == 0)
            bitSetOne(mask, maskIdx);
        if ((bVal&0xf) == 0)
            bitSetOne(mask, maskIdx+1);
        maskIdx += 2;
        }
    }
if (size&1)
    {
    bVal = getc(f);
    if (bVal < 0)
	errAbort("Read error 3 in %s", fileName);
    *d++ = valToNtTbl[(bVal>>4)];
    if (mask != NULL)
        {
        if ((bVal>>4) == 0)
            bitSetOne(mask, maskIdx);
        maskIdx++;
        }
    }
*d = 0;
return seq;
}

static void nibOutput(int options, struct dnaSeq *seq, char *fileName)
/* Write out file in format of four bits per nucleotide, with control over
 * handling of masked positions. */
{
int i;
UBYTE byte;
DNA *dna = seq->dna;
int dVal1, dVal2;
bits32 size = seq->size;
int byteCount = (size>>1);
bits32 sig = nibSig;
int *ntValTbl = ((options & NIB_MASK_MIXED) ? ntValMasked : ntVal5);
Bits* mask = ((options & NIB_MASK_MAP) ? seq->mask : NULL);
int maskIdx = 0;
FILE *f = mustOpen(fileName, "w");

assert(sizeof(bits32) == 4);

writeOne(f, sig);
writeOne(f, seq->size);

printf("Writing %d bases in %d bytes\n", seq->size, ((seq->size+1)/2) + 8);
while (--byteCount >= 0)
    {
    dVal1 = ntValTbl[dna[0]];
    dVal2 = ntValTbl[dna[1]];
    /* Set from mask, remember bit in character is opposite sense of bit
     * in mask. */
    if (mask != NULL)
        {
        if (!bitReadOne(mask, maskIdx))
            dVal1 |= MASKED_BASE_BIT;
        if (!bitReadOne(mask, maskIdx+1))
            dVal2 |= MASKED_BASE_BIT;
        maskIdx += 2;
        }
    byte = (dVal1<<4) | dVal2;
    if (putc(byte, f) < 0)
	{
	perror("");
	errAbort("Couldn't write all of %s", fileName);
	}
    dna += 2;
    }
if (size & 1)
    {
    dVal1 = ntValTbl[dna[0]];
    if ((mask != NULL) && !bitReadOne(mask, maskIdx))
        dVal1 |= MASKED_BASE_BIT;
    byte = (dVal1<<4);
    putc(byte, f);
    }
}

struct dnaSeq *nibLdPartMasked(int options, char *fileName, FILE *f, int seqSize, int start, int size)
/* Load part of an open .nib file, with control over handling of masked
 * positions. */
{
char nameBuf[512];
sprintf(nameBuf, "%s:%d-%d", fileName, start, start+size);
return nibInput(options, fileName, nameBuf, f, seqSize, start, size);
}

struct dnaSeq *nibLdPart(char *fileName, FILE *f, int seqSize, int start, int size)
/* Load part of an open .nib file. */
{
return nibLdPartMasked(0, fileName, f, seqSize, start, size);
}

struct dnaSeq *nibLoadPartMasked(int options, char *fileName, int start, int size)
/* Load part of an .nib file, with control over handling of masked positions */
{
struct dnaSeq *seq;
FILE *f;
int seqSize;
nibOpenVerify(fileName, &f, &seqSize);
seq = nibLdPartMasked(options, fileName, f, seqSize, start, size);
fclose(f);
return seq;
}

struct dnaSeq *nibLoadPart(char *fileName, int start, int size)
/* Load part of an .nib file. */
{
return nibLoadPartMasked(0, fileName, start, size);
}

struct dnaSeq *nibLoadAllMasked(int options, char *fileName)
/* Load part of a .nib file, with control over handling of masked
 * positions. Subranges of nib files may specified in the file name
 * using the syntax:
 *    /path/file.nib:seqid:start-end
 * or\n"
 *    /path/file.nib:start-end
 * With the first form, seqid becomes the id of the subrange, with the second
 * form, a sequence id of file:start-end will be used.
 */
{
struct dnaSeq *seq;
FILE *f;
int seqSize;
char filePath[PATH_LEN];
char name[PATH_LEN];
int start, end;

parseNibName(options, fileName, filePath, name, &start, &end);
nibOpenVerify(filePath, &f, &seqSize);
if (end == 0)
    end = seqSize;
seq = nibInput(options, fileName, name, f, seqSize, start, end-start);
fclose(f);
return seq;
}

struct dnaSeq *nibLoadAll(char *fileName)
/* Load part of an .nib file. */
{
return nibLoadAllMasked(0, fileName);
}

void nibWriteMasked(int options, struct dnaSeq *seq, char *fileName)
/* Write out file in format of four bits per nucleotide, with control over
 * handling of masked positions. */
{
    nibOutput(options, seq, fileName);
}

void nibWrite(struct dnaSeq *seq, char *fileName)
/* Write out file in format of four bits per nucleotide. */
{
    nibWriteMasked(0, seq, fileName);
}

struct nibStream
/* Struct to help write a nib file one base at a time. 
 * The routines that do this aren't very fast, but they
 * aren't used much currently. */
    {
    struct nibStream *next;
    char *fileName;	/* Name of file - allocated here. */
    FILE *f;		/* File handle. */
    bits32 size;	/* Current size. */
    UBYTE byte;		/* Two nibble's worth of data. */
    };

struct nibStream *nibStreamOpen(char *fileName)
/* Create a new nib stream.  Open file and stuff. */
{
struct nibStream *ns;
FILE *f;

dnaUtilOpen();
AllocVar(ns);
ns->f = f = mustOpen(fileName, "wb");
ns->fileName = cloneString(fileName);

/* Write header - initially zero.  Will fix it up when we close. */
writeOne(f, ns->size);
writeOne(f, ns->size);

return ns;
}

void nibStreamClose(struct nibStream **pNs)
/* Close a nib stream.  Flush last nibble if need be.  Fix up header. */
{
struct nibStream *ns = *pNs;
FILE *f;
bits32 sig = nibSig;
if (ns == NULL)
    return;
f = ns->f;
if (ns->size&1)
    writeOne(f, ns->byte);
fseek(f,  0L, SEEK_SET);
writeOne(f, sig);
writeOne(f, ns->size);
fclose(f);
freeMem(ns->fileName);
freez(pNs);
}

void nibStreamOne(struct nibStream *ns, DNA base)
/* Write out one base to nibStream. */
{
UBYTE ub = ntVal5[base];

if ((++ns->size&1) == 0)
    {
    ub += ns->byte;
    writeOne(ns->f, ub);
    }
else
    {
    ns->byte = (ub<<4);
    }
}

void nibStreamMany(struct nibStream *ns, DNA *dna, int size)
/* Write many bases to nibStream. */
{
int i;
for (i=0; i<size; ++i)
    nibStreamOne(ns, *dna++);
}

boolean isNib(char *fileName)
/* Return TRUE if file is a nib file. */
{
boolean isANib;
char *subrange = findNibSubrange(fileName);
if (subrange != NULL)
    *subrange = '\0';
isANib = endsWith(fileName, ".nib") || endsWith(fileName, ".NIB");
if (subrange != NULL)
    *subrange = ':';
return isANib;
}

boolean isNibSubrange(char *fileName)
/* Return TRUE if file specifies a subrange of a nib file. */
{
boolean isANib;
char *subrange = findNibSubrange(fileName);;
if (subrange == NULL)
    return FALSE;
*subrange = '\0';
isANib = endsWith(fileName, ".nib") || endsWith(fileName, ".NIB");
*subrange = ':';
return isANib;
}

