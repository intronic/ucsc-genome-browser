/* trfBig - Mask tandem repeats on a big sequence file.. */
#include "common.h"
#include "linefile.h"
#include "fa.h"
#include "nib.h"
#include "portable.h"
#include "cheapcgi.h"

/* Variables that can be set from command line. */
char *trfExe = "trf";	/* trf executable name. */
boolean doBed = FALSE;	/* Output .bed file as well as masked file. */

void usage()
/* Explain usage and exit. */
{
errAbort(
  "trfBig - Mask tandem repeats on a big sequence file.\n"
  "usage:\n"
  "   trfBig inFile outFile\n"
  "This will repeatedly run trf to mask tandem repeats in infile\n"
  "and put masked results in outFile.  inFile and outFile can be .fa\n"
  "or .nib format.\n"
  "\n"
  "options:\n"
  "   -bed creates a bed file in current dir\n"
  "   -trf=trfExe explicitly specifies trf executable name\n");
}

void writeSomeDatToBed(char *inName, FILE *out, char *chromName, int chromOffset, 
	int start, int end)
/* Read dat file and write bits of it to .bed out file adding offset as necessary. */
{
struct lineFile *lf = lineFileOpen(inName, TRUE);
char *line;
int lineSize;
char *row[14];
boolean gotHead = FALSE;
int s, e, i;

while (lineFileNext(lf, &line, &lineSize))
    {
    if (startsWith("Parameters:", line))
        {
	gotHead = TRUE;
	break;
	}
    }
if (!gotHead) errAbort("%s isn't a recognized trf .dat file\n", inName);

while(lineFileRow(lf, row))
    {
    s = atoi(row[0])-1;
    e = atoi(row[1]);
    if (s < start) s = start;
    if (e > end) e = end;
    if (s < e)
        {
	fprintf(out, "%s\t%d\t%d\ttrf", chromName, s+chromOffset, e+chromOffset);
	for (i=2; i<14; ++i)
	    fprintf(out, "\t%s", row[i]);
	fprintf(out, "\n");
	}
    }
lineFileClose(&lf);
}

void removeWild(char *pat)
/* Remove all files matching wildcard. */
{
char dir[256], fn[128], ext[64];
char wild[256];
struct slName *list, *el;

splitPath(pat, dir, fn, ext);
sprintf(wild, "%s%s", fn, ext);
if (dir[0] == 0) strcpy(dir, ".");

list = listDir(dir, wild);
for (el = list; el != NULL; el = el->next)
    {
    remove(el->name);
    uglyf("Removed %s\n", el->name);
    }
slFreeList(&list);
}

void makeTrfRootName(char trfRootName[512], char *faFile)
/* Make root name of files trf produces from faFile. */
{
sprintf(trfRootName, "%s.2.7.7.80.10.50.500", faFile);
}

void trfSysCall(char *faFile)
/* Invoke trf program on file. */
{
char command[1024];
sprintf(command, "%s %s 2 7 7 80 10 50 500 -m %s", 
	trfExe, faFile, doBed ? "-d" : "");
uglyf("faFile %s, command %s\n", faFile, command);
system(command);
}

void outputWithBreaks(FILE *out, char *s, int size, int lineSize)
/* Print s of given size to file, adding line feeds every now and then. */
{
int i, oneSize;
for (i=0; i<size; i += oneSize)
    {
    oneSize = size - i;
    if (oneSize > lineSize) oneSize = lineSize;
    mustWrite(out, s+i, oneSize);
    fputc('\n', out);
    }
}

void trfBig(char *input, char *output)
/* trfBig - Mask tandem repeats on a big sequence file.. */
{
int maxSize = 5000000;
int overlapSize = 10000;
int start, end, s, e;
int halfOverlapSize = overlapSize/2;
char *tempFile, trfRootName[512], trfTemp[512], bedFileName[512];
char dir[256], chrom[128], ext[64];
FILE *bedFile = NULL;
struct tempName tn;
struct dnaSeq  *maskedSeq = NULL;

if (doBed)
    {
    splitPath(output, dir, chrom, ext);
    sprintf(bedFileName, "%s%s.bed", dir, chrom);
    bedFile = mustOpen(bedFileName, "w");
    }
makeTempName(&tn, "trf", ".fa");
tempFile = tn.forCgi;
tempFile = "trfBigTemp.fa";
if (endsWith(input, ".nib") && endsWith(output, ".nib"))
    {
    int nibSize;
    FILE *in;
    struct nibStream *ns = nibStreamOpen(output);
    struct dnaSeq *seq;

    nibOpenVerify(input, &in, &nibSize);
    for (start = 0; start < nibSize; start = end)
        {
	end = start + maxSize;
	if (end > nibSize) end = nibSize;
	seq = nibLdPart(input, in, nibSize, start, end - start);
	faWrite(tempFile, seq->name, seq->dna, seq->size);
	freeDnaSeq(&seq);
	trfSysCall(tempFile);
	makeTrfRootName(trfRootName, tempFile);
	sprintf(trfTemp, "%s.mask", trfRootName);
	maskedSeq = faReadDna(trfTemp);
	s = (start == 0 ? 0 : halfOverlapSize);
	if (end == nibSize)
	    e = end - start;
	else
	    {
	    e = end - halfOverlapSize - start;
	    end -= overlapSize;
	    }
	nibStreamMany(ns, maskedSeq->dna + s, e-s);
	freeDnaSeq(&maskedSeq);
	if (doBed)
	    {
	    sprintf(trfTemp, "%s.dat", trfRootName);
	    writeSomeDatToBed(trfTemp, bedFile, chrom, start, s, e);
	    }
	}
    nibStreamClose(&ns);
    }
else if (!endsWith(input, ".nib") && !endsWith(output, ".nib"))
    {
    struct lineFile *lf = lineFileOpen(input, TRUE);
    struct dnaSeq seq;
    FILE *f = mustOpen(output, "w");
    FILE *out = mustOpen(output, "w");
    while (faSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
        {
	fprintf(out, ">%s\n", seq.name);
	for (start = 0; start < seq.size; start = end)
	    {
	    end = start + maxSize;
	    if (end > seq.size) end = seq.size;
	    uglyf(">>> start %d, end %d\n", start, end);
	    faWrite(tempFile, seq.name, seq.dna+start, end - start);
	    trfSysCall(tempFile);
	    makeTrfRootName(trfRootName, tempFile);
	    sprintf(trfTemp, "%s.mask", trfRootName);
	    maskedSeq = faReadDna(trfTemp);
	    s = (start == 0 ? 0 : halfOverlapSize);
	    if (end == seq.size)
		e = end - start;
	    else
		{
		e = end - halfOverlapSize - start;
		end -= overlapSize;
		}
	    outputWithBreaks(out, maskedSeq->dna+s, e-s, 50);
	    freeDnaSeq(&maskedSeq);
	    if (doBed)
		{
		sprintf(trfTemp, "%s.dat", trfRootName);
		writeSomeDatToBed(trfTemp, bedFile, seq.name, start, s, e);
		}
	    }
	}
    lineFileClose(&lf);
    carefulClose(&out);
    }
else
    {
    errAbort("Sorry, both input and output must be in same format.");
    }
sprintf(trfTemp, "%s*", tempFile);
removeWild(trfTemp);
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
doBed = cgiBoolean("bed");
trfExe = cgiUsualString("trf", trfExe);
if (argc != 3)
    usage();
trfBig(argv[1], argv[2]);
return 0;
}
