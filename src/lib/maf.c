static char const rcsid[] = "$Id: maf.c,v 1.9 2003/05/05 07:26:12 kent Exp $";

/* maf.c - Read/write maf format. */
#include "common.h"
#include "linefile.h"
#include "errabort.h"
#include "obscure.h"
#include "dnautil.h"
#include "axt.h"
#include "maf.h"

struct mafFile *mafMayOpen(char *fileName)
/* Open up a maf file and verify header. */
{
struct mafFile *mf;
struct lineFile *lf;
char *line, *word;
char *sig = "##maf";

if ((lf = lineFileMayOpen(fileName, TRUE)) == NULL)
    return NULL;
AllocVar(mf);
mf->lf = lf;

lineFileNeedNext(lf, &line, NULL);
if (!startsWith(sig, line))
    errAbort("%s does not start with %s", fileName, sig);
line += strlen(sig);

while ((word = nextWord(&line)) != NULL)
    {
    /* Parse name=val. */
    char *name = word;
    char *val = strchr(word, '=');
    if (val == NULL)
       errAbort("Missing = after %s line 1 of %s\n", name, fileName);
    *val++ = 0;

    if (sameString(name, "version"))
        mf->version = atoi(val);
    else if (sameString(name, "scoring"))
        mf->scoring = cloneString(val);
    }
if (mf->version == 0)
    errAbort("No version line 1 of %s\n", fileName);
return mf;
}

struct mafFile *mafOpen(char *fileName)
/* Open up a maf file.  Squawk and die if there's a problem. */
{
struct mafFile *mf = mafMayOpen(fileName);
if (mf == NULL)
    errnoAbort("Couldn't open %s\n", fileName);
return mf;
}

static boolean nextLine(struct lineFile *lf, char **pLine)
/* Get next line that is not a comment. */
{
for (;;)
    {
    if (!lineFileNext(lf, pLine, NULL))
        return FALSE;
    if (**pLine != '#')
        return TRUE;
    }
}

struct mafAli *mafNextWithPos(struct mafFile *mf, off_t *retOffset)
/* Return next alignment in FILE or NULL if at end.  If retOffset is
 * nonNULL, return start offset of record in file. */
{
struct lineFile *lf = mf->lf;
struct mafAli *ali;
char *line, *word;

/* Loop until get an alignment paragraph or reach end of file. */
for (;;)
    {
    /* Get alignment header line.  If it's not there assume end of file. */
    if (!nextLine(lf, &line))
	{
	lineFileClose(&mf->lf);
	return NULL;
	}

    /* Parse alignment header line. */
    word = nextWord(&line);
    if (word == NULL)
	continue;	/* Ignore blank lines. */
	
    if (sameString(word, "a"))
	{
	if (retOffset != NULL)
	    *retOffset = lineFileTell(mf->lf);
	AllocVar(ali);
	while ((word = nextWord(&line)) != NULL)
	    {
	    /* Parse name=val. */
	    char *name = word;
	    char *val = strchr(word, '=');
	    if (val == NULL)
	       errAbort("Missing = after %s line 1 of %s", name, lf->fileName);
	    *val++ = 0;

	    if (sameString(name, "score"))
		ali->score = atof(val);
	    }

	/* Parse alignment components until blank line. */
	for (;;)
	    {
	    if (!nextLine(lf, &line))
		errAbort("Unexpected end of file %s", lf->fileName);
	    word = nextWord(&line);
	    if (word == NULL)
		break;
	    if (sameString(word, "s"))
		{
		struct mafComp *comp;
		int wordCount;
		char *row[7];
		int textSize;

		/* Chop line up by white space.  This involves a few +-1's because
		 * have already chopped out first word. */
		row[0] = word;
		wordCount = chopByWhite(line, row+1, ArraySize(row)-1) + 1; /* +-1 because of "s" */
		lineFileExpectWords(lf, ArraySize(row), wordCount);
		AllocVar(comp);

		/* Convert ascii text representation to mafComp structure. */
		comp->src = cloneString(row[1]);
		comp->srcSize = lineFileNeedNum(lf, row, 5);
		comp->strand = row[4][0];
		comp->start = lineFileNeedNum(lf, row, 2);
		comp->size = lineFileNeedNum(lf, row, 3);
		comp->text = cloneString(row[6]);
		textSize = strlen(comp->text);

		/* Fill in ali->text size. */
		if (ali->textSize == 0)
		    ali->textSize = textSize;
		else if (ali->textSize != textSize)
		    errAbort("Text size inconsistent (%d vs %d) line %d of %s",
			textSize, ali->textSize, lf->lineIx, lf->fileName);

		/* Do some sanity checking. */
		if (comp->srcSize <= 0 || comp->size <= 0)
		     errAbort("Got a zero or negative size line %d of %s", lf->lineIx, lf->fileName);
		if (comp->start < 0 || comp->start + comp->size > comp->srcSize)
		     errAbort("Coordinates out of range line %d of %s", lf->lineIx, lf->fileName);
		  
		/* Add component to head of list. */
		slAddHead(&ali->components, comp);
		}
	    }
	slReverse(&ali->components);
	return ali;
	}
    else  /* Skip over paragraph we don't understand. */
	{
	for (;;)
	    {
	    if (!nextLine(lf, &line))
		return NULL;
            if (nextWord(&line) == NULL)
		break;
	    }
	}
    }
}


struct mafAli *mafNext(struct mafFile *mf)
/* Return next alignment in FILE or NULL if at end. */
{
return mafNextWithPos(mf, NULL);
}


struct mafFile *mafReadAll(char *fileName)
/* Read all elements in a maf file */
{
struct mafFile *mf = mafOpen(fileName);
struct mafAli *ali;
while ((ali = mafNext(mf)) != NULL)
    {
    slAddHead(&mf->alignments, ali);
    }
slReverse(&mf->alignments);
return mf;
}

void mafWriteStart(FILE *f, char *scoring)
/* Write maf header and scoring scheme name (may be null) */
{
fprintf(f, "##maf version=1");
if (scoring != NULL)
    fprintf(f, " scoring=%s", scoring);
fprintf(f, "\n");
}

void mafWrite(FILE *f, struct mafAli *ali)
/* Write next alignment to file. */
{
struct mafComp *comp;
int srcChars = 0, startChars = 0, sizeChars = 0, srcSizeChars = 0;

/* Write out alignment header */
fputc('a', f);
if (ali->score != 0.0)
    fprintf(f, " score=%f", ali->score);
fputc('\n', f);

/* Figure out length of each field. */
for (comp = ali->components; comp != NULL; comp = comp->next)
    {
    int len = strlen(comp->src);
    if (srcChars < len)
        srcChars = len;
    len = digitsBaseTen(comp->start);
    if (startChars < len)
        startChars = len;
    len = digitsBaseTen(comp->size);
    if (sizeChars < len)
        sizeChars = len;
    len = digitsBaseTen(comp->srcSize);
    if (srcSizeChars < len)
        srcSizeChars = len;
    }

/* Write out each component. */
for (comp = ali->components; comp != NULL; comp = comp->next)
    {
    fprintf(f, "s %-*s %*d %*d %c %*d %s\n", 
    	srcChars, comp->src, startChars, comp->start, 
	sizeChars, comp->size, comp->strand, 
	srcSizeChars, comp->srcSize, comp->text);
    }

/* Write out blank separator line. */
fprintf(f, "\n");
}

void mafWriteEnd(FILE *f)
/* Write maf footer. In this case nothing */
{
}

void mafWriteAll(struct mafFile *mf, char *fileName)
/* Write out full mafFile. */
{
FILE *f = mustOpen(fileName, "w");
struct mafAli *ali;
mafWriteStart(f, mf->scoring);
for (ali = mf->alignments; ali != NULL; ali = ali->next)
    mafWrite(f, ali);
mafWriteEnd(f);
carefulClose(&f);
}

void mafCompFree(struct mafComp **pObj)
/* Free up a maf component. */
{
struct mafComp *obj = *pObj;
if (obj == NULL)
    return;
freeMem(obj->src);
freeMem(obj->text);
freez(pObj);
}

void mafCompFreeList(struct mafComp **pList)
/* Free up a list of maf components. */
{
struct mafComp *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    mafCompFree(&el);
    }
*pList = NULL;
}

int mafPlusStart(struct mafComp *comp)
/* Return start relative to plus strand of src. */
{
if (comp->strand == '-') 
    return comp->srcSize - (comp->start + comp->size);
else
    return comp->start;
}

void mafAliFree(struct mafAli **pObj)
/* Free up a maf alignment. */
{
struct mafAli *obj = *pObj;
if (obj == NULL)
    return;
mafCompFreeList(&obj->components);
freez(pObj);
}

void mafAliFreeList(struct mafAli **pList)
/* Free up a list of maf alignmentx. */
{
struct mafAli *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    mafAliFree(&el);
    }
*pList = NULL;
}

void mafFileFree(struct mafFile **pObj)
/* Free up a maf file. */
{
struct mafFile *obj = *pObj;
if (obj == NULL)
    return;
lineFileClose(&obj->lf);
freeMem(obj->scoring);
mafAliFreeList(&obj->alignments);
freez(pObj);
}

void mafFileFreeList(struct mafFile **pList)
/* Free up a list of maf files. */
{
struct mafFile *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    mafFileFree(&el);
    }
*pList = NULL;
}

struct mafComp *mafMayFindComponent(struct mafAli *maf, char *src)
/* Find component of given source. Return NULL if not found. */
{
struct mafComp *mc;
for (mc = maf->components; mc != NULL; mc = mc->next)
    {
    if (sameString(mc->src, src))
        return mc;
    }
return NULL;
}

struct mafComp *mafFindComponent(struct mafAli *maf, char *src)
/* Find component of given source or die trying. */
{
struct mafComp *mc = mafMayFindComponent(maf, src);
if (mc == NULL)
    errAbort("Couldn't find %s in maf", src);
return mc;
}

struct mafAli *mafSubset(struct mafAli *maf, char *componentSource,
	int newStart, int newEnd)
/* Extract subset of maf that intersects a given range
 * in a component sequence.  The newStart and newEnd
 * are given in the forward strand coordinates of the
 * component sequence.  The componentSource is typically
 * something like 'mm3.chr1'.  This will return NULL
 * if maf does not intersect range.  The score field
 * in the returned maf will not be filled in (since
 * we don't know which scoring scheme to use). */
{
struct mafComp *mcMaster = mafFindComponent(maf, componentSource);
struct mafAli *subset;
struct mafComp *mc, *subMc;
char *s, *e;
int textStart, textSize;

/* Reverse complement input range if necessary. */
if (mcMaster->strand == '-')
    reverseIntRange(&newStart, &newEnd, mcMaster->srcSize);

/* Check if any real intersection and return NULL if not. */
if (newStart >= newEnd)
    return NULL;
if (newStart >= mcMaster->start + mcMaster->size)
    return NULL;
if (newEnd <= mcMaster->start)
    return NULL;

/* Clip to bounds of actual data. */
if (newStart < mcMaster->start)
    newStart = mcMaster->start;
if (newEnd > mcMaster->start + mcMaster->size)
    newEnd = mcMaster->start + mcMaster->size;

/* Translate position in master sequence to position in
 * multiple alignment. */
s = skipIgnoringDash(mcMaster->text, newStart - mcMaster->start, TRUE);
e = skipIgnoringDash(s, newEnd - newStart, FALSE);
textStart = s - mcMaster->text;
textSize = e - s;

/* Allocate subset structure and fill it in */
AllocVar(subset);
subset->textSize = textSize;
for (mc = maf->components; mc != NULL; mc = mc->next)
    {
    AllocVar(subMc);
    subMc->src = cloneString(mc->src);
    subMc->srcSize = mc->srcSize;
    subMc->strand = mc->strand;
    subMc->start = mc->start + countNonDash(mc->text, textStart);
    subMc->size = countNonDash(mc->text+textStart, textSize);
    subMc->text = cloneStringZ(mc->text + textStart, textSize);
    slAddHead(&subset->components, subMc);
    }
slReverse(&subset->components);
return subset;
}

boolean mafNeedSubset(struct mafAli *maf, char *componentSource,
	int newStart, int newEnd)
/* Return TRUE if maf only partially fits between newStart/newEnd
 * in given component. */
{
struct mafComp *mcMaster = mafFindComponent(maf, componentSource);

/* Reverse complement input range if necessary. */
if (mcMaster->strand == '-')
    reverseIntRange(&newStart, &newEnd, mcMaster->srcSize);

return newStart > mcMaster->start && newEnd < mcMaster->start + mcMaster->size;
}

void mafFlipStrand(struct mafAli *maf)
/* Reverse complement maf. */
{
struct mafComp *mc;
for (mc = maf->components; mc != NULL; mc = mc->next)
    {
    int s = mc->start, e = mc->start + mc->size;
    reverseIntRange(&s, &e, mc->srcSize);
    mc->start = e;
    reverseComplement(mc->text, maf->textSize);
    if (mc->strand == '-')
        mc->strand = '+';
    else
        mc->strand = '-';
    }
}

