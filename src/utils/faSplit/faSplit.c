/* faSplit - Split an fa file into several files.. */
#include "common.h"
#include "portable.h"
#include "linefile.h"
#include "dystring.h"
#include "dnautil.h"
#include "obscure.h"
#include "fa.h"
#include "options.h"
#include "bits.h"

static char const rcsid[] = "$Id: faSplit.c,v 1.19 2004/09/14 04:40:58 baertsch Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "faSplit - Split an fa file into several files.\n"
  "usage:\n"
  "   faSplit how input.fa count outRoot\n"
  "where how is either 'base' 'sequence' or 'size'.  Files\n"
  "split by sequence will be broken at the nearest\n"
  "fa record boundary, while those split by base will\n"
  "be broken at any base.  Files broken by size will\n"
  "be broken every count bases.\n"
  "Examples:\n"
  "   faSplit sequence estAll.fa 100 est\n"
  "This will break up estAll.fa into 100 files\n"
  "(numbered est001.fa est002.fa, ... est100.fa\n"
  "Files will only be broken at fa record boundaries\n"
  "   faSplit base chr1.fa 10 1_\n"
  "This will break up chr1.fa into 10 files\n"
  "   faSplit size input.fa 2000 outRoot\n"
  "This breaks up input.fa into 2000 base chunks\n"
  "   faSplit about est.fa 20000 outRoot\n"
  "This will break up est.fa into files of about 20000 bytes each by record.\n"
  "   faSplit byname scaffolds.fa outRoot \n"
  "This breaks up scaffolds.fa using sequence names as file names.\n"
  "   faSplit gap chrN.fa 20000 outRoot\n"
  "This breaks up chrN.fa into files of at most 20000 bases each, \n"
  "at gap boundaries if possible.\n"
  "\n"
  "Options:\n"
  "    -verbose=2 - Write names of each file created\n"
  "    -maxN=N - Suppress pieces with more than maxN n's.  Only used with size.\n"
  "              default is size-1 (only suppresses pieces that are all N).\n"
  "    -oneFile - Put output in one file. Only used with size\n"
  "    -out=outFile Get masking from outfile.  Only used with size.\n"
  "    -lift=file.lft Put info on how to reconstruct sequence from\n"
  "                   pieces in file.lft.  Only used with size and gap.\n"
  "    -minGapSize=X Consider a block of Ns to be a gap if block size >= X.\n"
  "                  Only used with gap.\n"
  "    -outDirDepth=n Create n-levels of output directory under outRoot.\n"
  "                   This helps prevent NFS problems with a large number of\n"
  "                   file in a directory.  Using -outDirDepth=3 would\n"
  "                   produce rootDir/1/2/3/foo.123.fa.\n"
  "    -prefixLength=N - used with byname option. create a separate output\n"
  "                   file for each group of sequences names with same prefix\n"
  "                   of length N.\n"
);

}

/* command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"maxN", OPTION_INT},
    {"oneFile", OPTION_BOOLEAN},
    {"out", OPTION_STRING},
    {"lift", OPTION_STRING},
    {"minGapSize", OPTION_INT},
    {"outDirDepth", OPTION_INT},
    {"prefixLength", OPTION_INT},
    {NULL, 0}
};

int outDirDepth = 0;

unsigned long estimateFaSize(char *fileName)
/* Estimate number of bases from file size. */
{
unsigned long size = fileSize(fileName);
return 0.5 + size * 0.99;
}

unsigned long calcNextEnd(int fileIx, int totalFiles, unsigned long estSize)
/* Return next end to break at. */
{
if (fileIx == totalFiles)
     return 0xefffffff;	/* bignum */
else
    {
    unsigned long nextEnd = round((double)fileIx*(double)estSize/(double)totalFiles/16.0);
    return nextEnd<<4;
    }
}

void mkOutPath(char *outPath, char* outRoot, int digits, int fileCount)
/* generate output file name */
{
char dir[PATH_LEN], fname[PATH_LEN];
splitPath(outRoot, dir, fname, NULL);

strcpy(outPath, dir);
if (outDirDepth > 0)
    {
    /* add directory levels, using training digits for names */
    char fcntStr[64], dirBuf[3];
    int i, iDir;
    safef(fcntStr, sizeof(fcntStr), "%0*d", outDirDepth, fileCount);
    iDir = strlen(fcntStr)-outDirDepth;
    strcpy(dirBuf, "X/");
    for (i = 0; i < outDirDepth; i++, iDir++)
        {
        dirBuf[0] = fcntStr[iDir];
        strcat(outPath, dirBuf);
        makeDir(outPath);
        }
    }

sprintf(outPath+strlen(outPath), "%s%0*d.fa", fname, digits, fileCount);
}

void splitByBase(char *inName, int splitCount, char *outRoot, unsigned long estSize)
/* Split into a file base by base. */
{
struct lineFile *lf = lineFileOpen(inName, TRUE);
int lineSize;
char *line;
char c;
char dir[PATH_LEN], seqName[128], outFile[128], outPathName[PATH_LEN];
int digits = digitsBaseTen(splitCount);
boolean warnedMultipleRecords = FALSE;
int fileCount = 0;
unsigned long nextEnd = 0;
unsigned long curPos = 0;
FILE *f = NULL;
int linePos = 0;
int outLineSize = 50;


if (!lineFileNext(lf, &line, &lineSize))
    errAbort("%s is empty", inName);
if (line[0] == '>')
    {
    line = firstWordInLine(line+1);
    if (line == NULL)
        errAbort("Empty initial '>' line in %s", inName);
    strncpy(seqName, line, sizeof(seqName));
    }
else
    {
    splitPath(inName, dir, seqName, NULL);
    lineFileReuse(lf);
    }
splitPath(outRoot, NULL, outFile, NULL);
while (lineFileNext(lf, &line, &lineSize))
    {
    if (line[0] == '>')
        {
	if (!warnedMultipleRecords)
	    {
	    warnedMultipleRecords = TRUE;
	    warn("More than one record in FA file line %d of %s", 
	    	lf->lineIx, lf->fileName);
	    continue;
	    }
	}
    while ((c = *line++) != 0)
        {
	if (isdigit(c) || isspace(c))
	    continue;
	if (!isalpha(c))
	    errAbort("Weird %c (0x%x) line %d of %s", c, c, lf->lineIx, lf->fileName);
	if (++curPos >= nextEnd)
	    {
	    if (f != NULL)
	        {
		if (linePos != 0)
		    fputc('\n', f);
		fclose(f);
		}
            mkOutPath(outPathName, outRoot, digits, fileCount);
	    verbose(2, "writing %s\n", outPathName);
	    f = mustOpen(outPathName, "w");
	    fprintf(f, ">%s%0*d\n", outFile, digits, fileCount);
	    ++fileCount;
	    linePos = 0;
	    nextEnd = calcNextEnd(fileCount, splitCount, estSize);
	    }
	fputc(c, f);
	if (++linePos >= outLineSize)
	    {
	    fputc('\n', f);
	    linePos = 0;
	    }
	}
    }
if (f != NULL)
    {
    if (linePos != 0)
	fputc('\n', f);
    fclose(f);
    }
lineFileClose(&lf);
}

void splitByRecord(char *inName, int splitCount, char *outRoot, unsigned long estSize)
/* Split into a file base by base. */
{
struct dnaSeq seq;
struct lineFile *lf = lineFileOpen(inName, TRUE);
int digits = digitsBaseTen(splitCount);
unsigned long nextEnd = 0;
unsigned long curPos = 0;
int fileCount = 0;
FILE *f = NULL;
char outPath[PATH_LEN];
ZeroVar(&seq);

while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    curPos += seq.size;
    if (curPos > nextEnd)
        {
	carefulClose(&f);
        mkOutPath(outPath, outRoot, digits, fileCount++);
	verbose(2, "writing %s\n", outPath);
	f = mustOpen(outPath, "w");
	nextEnd = calcNextEnd(fileCount, splitCount, estSize);
	}
    faWriteNext(f, seq.name, seq.dna, seq.size);
    }
carefulClose(&f);
lineFileClose(&lf);
}

void splitAbout(char *inName, unsigned long approxSize, char *outRoot)
/* Split into chunks of about approxSize.  Don't break up
 * sequence though. */
{
struct dnaSeq seq;
struct lineFile *lf = lineFileOpen(inName, TRUE);
int digits = 2;
unsigned long curPos = approxSize;
int fileCount = 0;
FILE *f = NULL;
char outPath[PATH_LEN];
ZeroVar(&seq);

while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    if (curPos >= approxSize)
        {
	carefulClose(&f);
	curPos = 0;
        mkOutPath(outPath, outRoot, digits, fileCount++);
	verbose(2, "writing %s\n", outPath);
	f = mustOpen(outPath, "w");
	}
    curPos += seq.size;
    faWriteNext(f, seq.name, seq.dna, seq.size);
    }
carefulClose(&f);
lineFileClose(&lf);
}

void splitByName(char *inName, char *outRoot)
/* Split into chunks using sequence names.  */
{
struct dnaSeq seq;
struct lineFile *lf = lineFileOpen(inName, TRUE);
FILE *f = NULL;
char outDir[256], outFile[128], ext[64], outPath[512];
ZeroVar(&seq);

splitPath(outRoot, outDir, outFile, ext);
while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    carefulClose(&f);
    sprintf(outPath, "%s%s.fa", outDir, seq.name);
    verbose(2, "writing %s\n", outPath);
    f = mustOpen(outPath, "w");
    faWriteNext(f, seq.name, seq.dna, seq.size);
    }
carefulClose(&f);
lineFileClose(&lf);
}

void splitByNamePrefix(char *inName, char *outRoot, int preFixCount)
/* Split into chunks using prefix of sequence names.  */
{
struct dnaSeq seq;
struct lineFile *lf = lineFileOpen(inName, TRUE);
FILE *f = NULL;
char outDir[256], outFile[128], ext[64], outPath[512], preFix[512];
ZeroVar(&seq);

splitPath(outRoot, outDir, outFile, ext);
assert(preFixCount < sizeof(preFix));

while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    carefulClose(&f);
    strncpy(preFix, seq.name, preFixCount);
    preFix[preFixCount] = '\0';
    sprintf(outPath, "%s%s.fa", outDir, preFix);
    verbose(2, "writing %s\n", outPath);
    f = mustOpen(outPath, "a");
    faWriteNext(f, seq.name, seq.dna, seq.size);
    }
carefulClose(&f);
lineFileClose(&lf);
}
int countN(char *s, int size)
/* Count number of N's from s[0] to s[size-1].
 * Treat any parts past end of string as N's. */
{
int goodCount = 0;
int i;
char c;

for (i=0; i<size; ++i)
    {
    c = s[i];
    if (c == 0)
        break;
    if (c != 'n')
        ++goodCount;
    }
return size - goodCount;
}

void bitsForOut(char *fileName, int seqSize, Bits *bits)
/* Get bitmap that corresponds to outFile. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line, *words[8];
int wordCount;
boolean firstTime = TRUE;
int start,end;

/* Check and skip over three line header */
if (!lineFileNext(lf, &line, NULL))
    errAbort("%s is empty", fileName);
line = skipLeadingSpaces(line);
if (!startsWith("SW", line))
    errAbort("%s is not a RepeatMasker .out file", fileName);
lineFileNext(lf, &line, NULL);
if (!startsWith("score", line))
    errAbort("%s is not a RepeatMasker .out file", fileName);
lineFileNext(lf, &line, NULL);

for (;;)
    {
    if (!lineFileNext(lf, &line, NULL))
        break;
    wordCount = chopLine(line, words);
    if (wordCount < 8)
	errAbort("Short line %d of %s\n", lf->lineIx, lf->fileName);
    start = lineFileNeedNum(lf, words, 5) - 1;
    end = lineFileNeedNum(lf, words, 6);
    if (start > end)
        errAbort("Start after end line %d of %s", lf->lineIx, lf->fileName);
    if (firstTime)
        {
	char *s = words[7];
	if (s[0] != '(' || !isdigit(s[1]))
	    errAbort("Expected parenthesized number line %d of %s", lf->lineIx, lf->fileName);
	if (seqSize != end + atoi(s+1))
	    errAbort("Size mismatch line %d of %s", lf->lineIx, lf->fileName);
	firstTime = FALSE;
	}
    if (end > seqSize)
        errAbort("End past bounds line %d of %s", lf->lineIx, lf->fileName);
    bitSetRange(bits, start, end-start);
    }
lineFileClose(&lf);
}

void setBitsN(DNA *dna, int size, Bits *bits)
/* Set bits in bitmap where there are N's in DNA. */
{
int i;
for (i=0; i<size; ++i)
    {
    if (dna[i] == 'n' || dna[i] == 'N')
	bitSetOne(bits, i);
    }
}

void splitByCount(char *inName, int pieceSize, char *outRoot, unsigned long estSize)
/* Split up file into pieces pieceSize long. */
{
unsigned long pieces = (estSize + pieceSize-1)/pieceSize;
int digits = digitsBaseTen(pieces);
int maxN = optionInt("maxN", pieceSize-1);
boolean oneFile = optionExists("oneFile");
char fileName[PATH_LEN];
char dirOnly[PATH_LEN], noPath[128];
int pos, pieceIx = 0, writeCount = 0;
struct dnaSeq seq;
struct lineFile *lf = lineFileOpen(inName, TRUE);
FILE *f = NULL;
Bits *bits = NULL;
int seqCount = 0;
char *outFile = optionVal("out", NULL);
char *liftFile = optionVal("lift", NULL);
FILE *lift = NULL;
ZeroVar(&seq);

splitPath(outRoot, dirOnly, noPath, NULL);
if (oneFile)
    {
    sprintf(fileName, "%s.fa", outRoot);
    f = mustOpen(fileName, "w");
    }
if (liftFile)
    lift = mustOpen(liftFile, "w");


/* Count number of N's from s[0] to s[size-1].
 * Treat any parts past end of string as N's. */
while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    bits = bitAlloc(seq.size);
    setBitsN(seq.dna, seq.size, bits);
    ++seqCount;
    if (outFile != NULL)
        {
	if (seqCount > 1)
	    errAbort("Can only handle in files with one sequence using out option");
	bitsForOut(outFile, seq.size, bits);
	}
    for (pos = 0; pos < seq.size; pos += pieceSize)
        {
	char numOut[128];
	int thisSize = seq.size - pos;
	if (thisSize > pieceSize) 
	    thisSize = pieceSize;
	if (bitCountRange(bits, pos, thisSize) <= maxN)
	    {
	    if (!oneFile)
	        {
                mkOutPath(fileName, outRoot, digits, pieceIx);
		f = mustOpen(fileName, "w");
		}
            sprintf(numOut, "%s%0*d", noPath, digits, pieceIx);
	    faWriteNext(f, numOut, seq.dna + pos, thisSize);
	    if (lift)
	        fprintf(lift, "%d\t%s\t%d\t%s\t%d\n",
		    pos, numOut, thisSize, seq.name, seq.size);
	    ++writeCount;
	    if (!oneFile)
	        carefulClose(&f);
	    }
        pieceIx++;
	}
    bitFree(&bits);
    }
carefulClose(&f);
carefulClose(&lift);
lineFileClose(&lf);
printf("%d pieces of %d written\n", writeCount, pieceIx);
}

boolean findLastGap(DNA *dna, int stopOffset, int endOffset, int minGapSize,
		    int *retGapStart, int *retGapSize)
/* Find the last gap (block of at least minGapSize N's) in dna that starts 
 * before stopOffset.  If we reach stopOffset while in a block of N's, keep 
 * counting N's unless we hit endOffset.  If a gap is found, return true. */
{
boolean gotGap = FALSE;
int gapStart = 0;
int gapSize = 0;
int nStart = 0;
int nSize = 0;
int i = 0;

for (i=0;  i < stopOffset;  i++)
    {
    if (dna[i] == 'n' || dna[i] == 'N')
	{
	if (nSize == 0)
	    nStart = i;
	nSize++;
	}
    else
	{
	if (nSize > 0)
	    {
	    if (nSize > minGapSize)
		{
		gotGap   = TRUE;
		gapStart = nStart;
		gapSize  = nSize;
		}
	    nSize = 0;
	    }
	}
    }

if (nSize > 0)
    {
    while ((dna[i] == 'n' || dna[i] == 'N') &&
	   i < endOffset)
	{
	nSize++;
	i++;
	}
    if (nSize > minGapSize)
	{
	gotGap   = TRUE;
	gapStart = nStart;
	gapSize  = nSize;
	}
    }

*retGapStart = gapStart;
*retGapSize  = gapSize;
return(gotGap);
}

void splitByGap(char *inName, int pieceSize, char *outRoot, int estSize)
/* Split up file into pieces at most pieceSize bases long, at gap boundaries 
 * if possible. */
{
unsigned long pieces = (estSize + pieceSize-1)/pieceSize;
int digits = digitsBaseTen(pieces);
int minGapSize = optionInt("minGapSize", 1000);
int maxN = optionInt("maxN", pieceSize-1);
boolean oneFile = optionExists("oneFile");
char fileName[512];
char dirOnly[256], noPath[128];
int pos, pieceIx = 0, writeCount = 0;
struct dnaSeq seq;
struct lineFile *lf = lineFileOpen(inName, TRUE);
FILE *f = NULL;
Bits *bits = NULL;
int seqCount = 0;
char *outFile = optionVal("out", NULL);
char *liftFile = optionVal("lift", NULL);
FILE *lift = NULL;
ZeroVar(&seq);

splitPath(outRoot, dirOnly, noPath, NULL);
if (oneFile)
    {
    sprintf(fileName, "%s.fa", outRoot);
    f = mustOpen(fileName, "w");
    }
if (liftFile)
    lift = mustOpen(liftFile, "w");

while (faMixedSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    bits = bitAlloc(seq.size);
    setBitsN(seq.dna, seq.size, bits);
    ++seqCount;
    if (outFile != NULL)
        {
	if (seqCount > 1)
	    errAbort("Can only handle in files with one sequence using out option");
	bitsForOut(outFile, seq.size, bits);
	}
    pos = 0;
    while (pos < seq.size)
        {
	char numOut[128];
	boolean gotGap = FALSE;
	int gapStart = 0;
	int gapSize  = 0;
	int endSize  = seq.size - pos;
	int thisSize = min(endSize, pieceSize);
	gotGap = findLastGap(&(seq.dna[pos]), thisSize, endSize,
			     minGapSize, &gapStart, &gapSize);
	if (gotGap)
	    thisSize = gapStart;

	sprintf(numOut, "%s%0*d", noPath, digits, pieceIx++);
	if (thisSize > 0 && bitCountRange(bits, pos, thisSize) <= maxN)
	    {
	    if (!oneFile)
	        {
                mkOutPath(fileName, outRoot, digits, pieceIx);
		f = mustOpen(fileName, "w");
		}
            sprintf(numOut, "%s%0*d", noPath, digits, pieceIx);
	    faWriteNext(f, numOut, seq.dna + pos, thisSize);
	    if (lift)
	        fprintf(lift, "%d\t%s\t%d\t%s\t%d\n",
		    pos, numOut, thisSize, seq.name, seq.size);
	    ++writeCount;
	    if (!oneFile)
	        carefulClose(&f);
	    }
        pieceIx++;
	pos += thisSize;
	if (gotGap)
	    pos += gapSize;
	}
    bitFree(&bits);
    }
carefulClose(&f);
carefulClose(&lift);
lineFileClose(&lf);
printf("%d pieces of %d written\n", writeCount, pieceIx);
}

int main(int argc, char *argv[])
/* Process command line. */
{
unsigned long estSize;
char *how = NULL;
char *inName = NULL;
int count;
char *outRoot;
int prefixLength = 0;

optionInit(&argc, argv, optionSpecs);
how = argv[1];
inName = argv[2];

if (argc < 4 )
    usage();
outDirDepth = optionInt("outDirDepth", 0);
prefixLength = optionInt("prefixLength", prefixLength);

if (sameWord(how, "byname"))
    {
    if (argc != 4)
        usage();
    dnaUtilOpen();
    outRoot = argv[3];
    if (optionExists("prefixLength"))
        {
        /* no count argument.  Make as many as needed */
        splitByNamePrefix(inName, outRoot, prefixLength);
        }
    else
        {
        /* no count argument.  Make as many as needed */
        splitByName(inName, outRoot);
        }
    }
else
    {
    if (argc != 5 || !isdigit(argv[3][0]))
	usage();

    dnaUtilOpen();
    count = atoi(argv[3]);
    outRoot=argv[4];
    estSize = estimateFaSize(argv[2]);
    if (sameWord(how, "sequence"))
	splitByRecord(inName, count, outRoot, estSize);
    else if (sameWord(how, "base"))
	splitByBase(inName, count, outRoot, estSize);
    else if (sameWord(how, "size"))
	splitByCount(inName, count, outRoot, estSize);
    else if (sameWord(how, "about"))
	splitAbout(inName, count, outRoot);
    else if (sameWord(how, "gap"))
	splitByGap(inName, count, outRoot, estSize);
    else
	usage();
    }
return 0;
}
