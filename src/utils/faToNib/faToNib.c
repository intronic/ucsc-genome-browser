/* faToNib - Convert from .fa to .nib format. */
#include "common.h"
#include "options.h"
#include "nib.h"
#include "fa.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "faToNib - Convert from .fa to .nib format\n"
  "usage:\n"
  "   faToNib [options] in.fa out.nib\n"
  "options:\n"
  "   -softMask - create nib that soft-masks lower case sequence\n"
  "               Note gfServer/gfClient don't know about this yet\n"
  "   -hardMask - create nib that hard-masks lower case sequence\n");
}

void maskLower(struct dnaSeq *seq)
/* Convert upper case to lower case and lower case to 'n' */
{
DNA *dna = seq->dna, base;
int i, size = seq->size;
for (i=0; i<size; ++i)
    {
    base = dna[size];
    if (isupper(base))
	base = tolower(base);
    else
        base = 'n';
    dna[size] = base;
    }
}

void faToNib(int options, char *in, char *out)
/* faToNib - Convert from .fa to .nib format. */
{
struct dnaSeq *seq = faReadAllMixed(in);
if (optionExists("hardMask"))
    {
    maskLower(seq);
    }
nibWriteMasked(options, seq, out);
}

int main(int argc, char *argv[])
/* Process command line. */
{
int options = 0;
optionHash(&argc, argv);
if (optionExists("softMask"))
    options = NIB_MASK_MIXED;
if (argc != 3)
    usage();
faToNib(options, argv[1], argv[2]);
return 0;
}
