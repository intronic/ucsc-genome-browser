/* gfClient - A client for the genomic finding program that produces a .psl file. */
#include "common.h"
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "linefile.h"
#include "fa.h"
#include "fuzzyFind.h"
#include "supStitch.h"
#include "genoFind.h"
#include "errabort.h"
#include "nib.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "gfClient - A client for the genomic finding program that produces a .psl file\n"
  "usage:\n"
  "   gfClient host port nibDir in.fa out.psl\n");
}

struct sockaddr_in sai;		/* Some system socket info. */

int setupSocket(char *hostName, char *portName)
/* Set up our socket. */
{
int port;
int sd;
struct hostent *hostent;

if (!isdigit(portName[0]))
    errAbort("Expecting a port number got %s", portName);
port = atoi(portName);
hostent = gethostbyname(hostName);
if (hostent == NULL)
    {
    herror("");
    errAbort("Couldn't find host %s. h_errno %d", hostName, h_errno);
    }
sai.sin_family = AF_INET;
sai.sin_port = htons(port);
memcpy(&sai.sin_addr.s_addr, hostent->h_addr_list[0], sizeof(sai.sin_addr.s_addr));
sd = socket(AF_INET, SOCK_STREAM, 0);
if (sd < 0)
    errnoAbort("Couldn't setup socket %s %s", hostName, portName);
return sd;
}

struct gfRange
/* A range of bases found by genoFind.  Recursive
 * data structure.  Lowest level roughly corresponds
 * to an exon. */
    {
    struct gfRange *next;  /* Next in singly linked list. */
    int qStart;	/* Start in query */
    int qEnd;	/* End in query */
    char *tName;	/* Target name */
    int tStart;	/* Start in target */
    int tEnd;	/* End in target */
    int hitCount;	/* Number of hits */
    struct gfRange *components;	/* Components of range. */
    };

void gfRangeFreeList(struct gfRange **pList);
/* Free a list of dynamically allocated gfRange's */

void gfRangeFree(struct gfRange **pEl)
/* Free a single dynamically allocated gfRange such as created
 * with gfRangeLoad(). */
{
struct gfRange *el;

if ((el = *pEl) == NULL) return;
freeMem(el->tName);
if (el->components != NULL)
    gfRangeFreeList(&el->components);
freez(pEl);
}

void gfRangeFreeList(struct gfRange **pList)
/* Free a list of dynamically allocated gfRange's */
{
struct gfRange *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    gfRangeFree(&el);
    }
*pList = NULL;
}

struct gfRange *gfRangeLoad(char **row)
/* Load a gfRange from row fetched with select * from gfRange
 * from database.  Dispose of this with gfRangeFree(). */
{
struct gfRange *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->qStart = atoi(row[0]);
ret->qEnd = atoi(row[1]);
ret->tName = cloneString(row[2]);
ret->tStart = atoi(row[3]);
ret->tEnd = atoi(row[4]);
ret->hitCount = atoi(row[5]);
return ret;
}

static int gfRangeCmpTarget(const void *va, const void *vb)
/* Compare to sort based on hit count. */
{
const struct gfRange *a = *((struct gfRange **)va);
const struct gfRange *b = *((struct gfRange **)vb);
int diff;

diff = strcmp(a->tName, b->tName);
if (diff == 0)
    diff = a->tStart - b->tStart;
return diff;
}

struct gfRange *gfQuerySeq(char *hostName, char *portName, struct dnaSeq *seq)
/* Ask server for places sequence hits. */
{
struct gfRange *rangeList = NULL, *range;
char buf[256], *row[6];
int rowSize;

/* Connect to server. */
int sd = setupSocket(hostName, portName);
if (connect(sd, &sai, sizeof(sai)) == -1)
    errnoAbort("Couldn't connect to socket in oneStrand");

/* Do start of query. */
sprintf(buf, "%squery %d", gfSignature(), seq->size);
write(sd, buf, strlen(buf));
read(sd, buf, 1);
if (buf[0] != 'Y')
    errAbort("Expecting 'Y' from server, got %c", buf[0]);
write(sd, seq->dna, seq->size);

/* Read results line by line and save in list, and return. */
for (;;)
    {
    gfRecieveString(sd, buf);
    if (sameString(buf, "end"))
	break;
    else
	{
	rowSize = chopLine(buf, row);
	if (rowSize < 6)
	    errAbort("Expecting 6 words from server got %d", rowSize);
	range = gfRangeLoad(row);
	slAddHead(&rangeList, range);
	}
    }
close(sd);
slReverse(&rangeList);
return rangeList;
}

struct gfRange *gfRangesBundle(struct gfRange *exonList, int maxIntron)
/* Bundle a bunch of 'exons' into plausable 'genes'.  It's
 * not necessary to be precise here.  The main thing is to
 * group together exons that are close to each other in the
 * same target sequence. */
{
struct gfRange *geneList = NULL, *gene = NULL, *lastExon = NULL, *exon, *nextExon;

for (exon = exonList; exon != NULL; exon = nextExon)
    {
    nextExon = exon->next;
    if (lastExon == NULL || !sameString(lastExon->tName, exon->tName) 
        || exon->tStart - lastExon->tEnd > maxIntron)
	{
	AllocVar(gene);
	gene->tStart = exon->tStart;
	gene->tEnd = exon->tEnd;
	gene->tName = cloneString(exon->tName);
	gene->qStart = exon->qStart;
	gene->qEnd = exon->qEnd;
	gene->hitCount = exon->hitCount;
	slAddHead(&gene->components, exon);
	slAddHead(&geneList, gene);
	}
    else
        {
	if (exon->qStart < gene->qStart) gene->qStart = exon->qStart;
	if (exon->qEnd > gene->qEnd) gene->qEnd = exon->qEnd;
	if (exon->tStart < gene->tStart) gene->tStart = exon->tStart;
	if (exon->tEnd > gene->tEnd) gene->tEnd = exon->tEnd;
	gene->hitCount += exon->hitCount;
	slAddTail(&gene->components, exon);
	}
    lastExon = exon;
    }
slReverse(&geneList);
return geneList;
}

struct dnaSeq *expandAndLoad(struct gfRange *range, char *nibDir, struct dnaSeq *query, int *retNibSize)
/* Expand range to cover an additional 500 bases on either side.
 * Load up target sequence and return. (Done together because don't
 * know target size before loading.) */
{
struct dnaSeq *target = NULL;
int extra = 500;
int x;
char nibFileName[512];
FILE *f = NULL;
int nibSize;

/* Open file and get size of target. */
sprintf(nibFileName, "%s/%s", nibDir, range->tName);
nibOpenVerify(nibFileName, &f, &nibSize);

/* Add to ends of range, clipping to fit inside sequence. */
x = range->qStart - extra;
if (x < 0) x = 0;
range->qStart = x;

x = range->qEnd + extra;
if (x > query->size) x = query->size;
range->qEnd = x;

x = range->tStart - extra;
if (x < 0) x = 0;
range->tStart = x;

x = range->tEnd + extra;
if (x > nibSize) x = nibSize;
range->tEnd = x;

target = nibLdPart(nibFileName, f, nibSize, range->tStart, range->tEnd - range->tStart);
fclose(f);
*retNibSize = nibSize;
return target;
}

void alignComponents(struct gfRange *combined, struct ssBundle *bun, 
	enum ffStringency stringency)
/* Align each piece of combined->components and put result in
 * bun->ffList. */
{
struct gfRange *range;
struct dnaSeq *qSeq = bun->qSeq, *tSeq = bun->genoSeq;
struct ssFfItem *ffi;
struct ffAli *ali;
int qStart, qEnd, tStart, tEnd;
int extra = 250;

for (range = combined->components; range != NULL; range = range->next)
    {
    /* Expand to include some extra sequence around range. */
    qStart = range->qStart - extra;
    tStart = range->tStart - extra;
    qEnd = range->qEnd + extra;
    tEnd = range->tEnd + extra;
    if (range == combined->components)
	{
        qStart -= extra;
	tStart -= extra;
	}
    if (range->next == NULL)
        {
	qEnd += extra;
	tEnd += extra;
	}
    if (qStart < combined->qStart) qStart = combined->qStart;
    if (tStart < combined->tStart) tStart = combined->tStart;
    if (qEnd > combined->qEnd) qEnd = combined->qEnd;
    if (tEnd > combined->tEnd) tEnd = combined->tEnd;

    ali = ffFind(qSeq->dna + qStart - combined->qStart,
                 qSeq->dna + qEnd - combined->qStart,
		 tSeq->dna + tStart - combined->tStart,
		 tSeq->dna + tEnd - combined->tStart,
		 stringency);
    if (ali != NULL)
        {
	AllocVar(ffi);
	ffi->ff = ali;
	ffi->score = ffScore(ali, stringency);
	slAddHead(&bun->ffList, ffi);
	}
    }
}

int calcMilliBad(int qAliSize, int tAliSize, 
	int qNumInserts, int tNumInserts,
	int match, int repMatch, int misMatch, boolean isMrna)
/* Express mismatches and other problems in parts per thousand. 
 * Stolen straight from psLayout. */
{
int aliSize;
int milliBad;
int sizeDif;
int insertFactor;

aliSize = min(qAliSize, tAliSize);
sizeDif = qAliSize - tAliSize;
if (sizeDif < 0)
    {
    if (isMrna)
	sizeDif = 0;
    else
	sizeDif = -sizeDif;
    }
insertFactor = qNumInserts;
if (!isMrna)
    insertFactor += tNumInserts;

milliBad = (1000 * (misMatch + insertFactor + sizeDif)) / (match + repMatch);
return milliBad;
}

void filterSaveAli(
	char *chromName, int chromSize, int tOffset,
	struct ffAli *left, struct dnaSeq *genoSeq, struct dnaSeq *otherSeq, 
	boolean isRc, enum ffStringency stringency, FILE *out, int minMatch)
/* Analyse one alignment and if it looks good enough write it out to file. 
 * This stolen from psLayout and slightly modified (mostly because we don't
 * have repeat data). */
{
struct ffAli *ff, *nextFf;
struct ffAli *right = ffRightmost(left);
DNA *needle = otherSeq->dna;
DNA *hay = genoSeq->dna;
int nStart = left->nStart - needle;
int nEnd = right->nEnd - needle;
int hStart = left->hStart - hay + tOffset;
int hEnd = right->hEnd - hay + tOffset;
int nSize = nEnd - nStart;
int hSize = hEnd - hStart;
int nInsertBaseCount = 0;
int nInsertCount = 0;
int hInsertBaseCount = 0;
int hInsertCount = 0;
int matchCount = 0;
int mismatchCount = 0;
int repMatch = 0;
int countNs = 0;
DNA *np, *hp, n, h;
int blockSize;
int i;
int badScore;
int milliBad;
int passIt;

/* Count up matches, mismatches, inserts, etc. */
for (ff = left; ff != NULL; ff = nextFf)
    {
    int hStart;
    nextFf = ff->right;
    blockSize = ff->nEnd - ff->nStart;
    np = ff->nStart;
    hp = ff->hStart;
    hStart = hp - hay;
    for (i=0; i<blockSize; ++i)
	{
	n = np[i];
	h = hp[i];
	if (n == 'n' || h == 'n')
	    ++countNs;
	else
	    {
	    if (n == h)
		++matchCount;
	    else
		++mismatchCount;
	    }
	}
    if (nextFf != NULL)
	{
	if (ff->nEnd != nextFf->nStart)
	    {
	    ++nInsertCount;
	    nInsertBaseCount += nextFf->nStart - ff->nEnd;
	    }
	if (ff->hEnd != nextFf->hStart)
	    {
	    ++hInsertCount;
	    hInsertBaseCount += nextFf->hStart - ff->hEnd;
	    }
	}
    }

/* See if it looks good enough to output. */
milliBad = calcMilliBad(nEnd - nStart, hEnd - hStart, nInsertCount, hInsertCount, 
	matchCount, repMatch, mismatchCount, stringency == ffCdna);
passIt = (milliBad < 100 && matchCount >= minMatch);
if (passIt)
    {
    if (isRc)
	{
	int temp;
	int oSize = otherSeq->size;
	temp = nStart;
	nStart = oSize - nEnd;
	nEnd = oSize - temp;
	}
    fprintf(out, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t"
                 "%c\t"
		 "%s\t%d\t%d\t%d\t"
		 "%s\t%d\t%d\t%d\t%d\t",
	matchCount, mismatchCount, repMatch, countNs, nInsertCount, nInsertBaseCount, hInsertCount, hInsertBaseCount,
	(isRc ? '-' : '+'),
	otherSeq->name, otherSeq->size, nStart, nEnd,
	chromName, chromSize, hStart, hEnd,
	ffAliCount(left));
    for (ff = left; ff != NULL; ff = ff->right)
	fprintf(out, "%d,", ff->nEnd - ff->nStart);
    fprintf(out, "\t");
    for (ff = left; ff != NULL; ff = ff->right)
	fprintf(out, "%d,", ff->nStart - needle);
    fprintf(out, "\t");
    for (ff = left; ff != NULL; ff = ff->right)
	fprintf(out, "%d,", ff->hStart - hay + tOffset);
    fprintf(out, "\n");
    if (ferror(out))
	{
	perror("");
	errAbort("Write error to .psl");
	}
    }
}

void saveAlignments(char *chromName, int chromSize, int tOffset, struct ssBundle *bun, 
	FILE *out, boolean isRc, 
	enum ffStringency stringency)
/* Save significant alignments to file in .psl format. */
{
struct dnaSeq *tSeq = bun->genoSeq, *qSeq = bun->qSeq;
struct ssFfItem *ffi;
int minMatch = 50;

if (minMatch < qSeq->size/2) minMatch = qSeq->size/2;
for (ffi = bun->ffList; ffi != NULL; ffi = ffi->next)
    {
    struct ffAli *ff = ffi->ff;
    filterSaveAli(chromName, chromSize, tOffset, ff, tSeq, qSeq, isRc, stringency, out, minMatch);
    }
}

void oneStrand(char *hostName, char *portName, char *nibDir, struct dnaSeq *seq,
    boolean isRc,  enum ffStringency stringency, FILE *out)
/* Search one strand of other sequence. */
{
struct ssBundle *bun;
struct gfRange *rangeList = NULL, *range;
struct dnaSeq *targetSeq;
char dir[256], chromName[128], ext[64];
int chromSize;

rangeList = gfQuerySeq(hostName, portName, seq);
slSort(&rangeList, gfRangeCmpTarget);
for (range = rangeList; range != NULL; range = range->next)
    {
    uglyf("%c %s %d-%d %s %d-%d %d\n", (isRc ? '-' : '+'), 
    	seq->name, range->qStart, range->qEnd, 
	range->tName, range->tStart, range->tEnd, range->hitCount);
    }
rangeList = gfRangesBundle(rangeList, 100000);
uglyf("Bundled down to %d\n", slCount(rangeList));
for (range = rangeList; range != NULL; range = range->next)
    {
    uglyf("%c %s %d-%d %s %d-%d %d\n", (isRc ? '-' : '+'), 
    	seq->name, range->qStart, range->qEnd, 
	range->tName, range->tStart, range->tEnd, range->hitCount);
    splitPath(range->tName, dir, chromName, ext);
    targetSeq = expandAndLoad(range, nibDir, seq, &chromSize);
    AllocVar(bun);
    bun->qSeq = seq;
    bun->genoSeq = targetSeq;
    bun->data = range;
    alignComponents(range, bun, stringency);
    uglyf("Aligned components ok\n");
    ssStitch(bun, stringency);
    uglyf("Stitched ok\n");
    saveAlignments(chromName, chromSize, range->tStart, bun, out, isRc, stringency);
    ssBundleFree(&bun);
    uglyf("Freed bundle ok\n");
    freeDnaSeq(&targetSeq);
    }
gfRangeFreeList(&rangeList);
}


void gfClient(char *hostName, char *portName, char *nibDir, char *inName, char *outName)
/* gfClient - A client for the genomic finding program that produces a .psl file. */
{
struct lineFile *lf = lineFileOpen(inName, TRUE);
static struct dnaSeq seq;
struct ssBundle *bundleList;
enum ffStringency stringency = ffCdna;
FILE *out = mustOpen(outName, "w");

while (faSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name))
    {
    oneStrand(hostName, portName, nibDir, &seq, FALSE, stringency, out);
    reverseComplement(seq.dna, seq.size);
    oneStrand(hostName, portName, nibDir, &seq, TRUE,  stringency, out);
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
if (argc != 6)
    usage();
gfClient(argv[1], argv[2], argv[3], argv[4], argv[5]);
return 0;
}
