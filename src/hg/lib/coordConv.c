/** coordConv.c was originally generated by the autoSql program, which also 
 * generated coordConv.h and coordConv.sql.  This module links the database and
 * the RAM representation of objects. */
#include "common.h"
#include "linefile.h"
#include "nib.h"
#include "jksql.h"
#include "coordConv.h"
#include "fa.h"
#include "genoFind.h"
#include "psl.h"
#include "portable.h"
#include "hdb.h"
#include "dbDb.h"

/* #define DEBUG */

struct possibleCoord 
/* info on a possible coord for new seq */
{
    struct possibleCoord *next;
    struct psl *midPsl,*upPsl,*downPsl;
    int d1,d2;
};

/************************Generated By auto-sql below here ****************/

/** Load a row from coordConv table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
void coordConvStaticLoad(char **row, struct coordConv *ret)
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->version = row[3];
ret->date = row[4];
ret->nibDir = row[5];
ret->optional = row[6];
}

/** Load a coordConv from row fetched with select * from coordConv
 * from database.  Dispose of this with coordConvFree(). */
struct coordConv *coordConvLoad(char **row)
{
struct coordConv *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->version = cloneString(row[3]);
ret->date = cloneString(row[4]);
ret->nibDir = cloneString(row[5]);
ret->optional = cloneString(row[6]);
return ret;
}

/** Load all coordConv from a tab-separated file.
 * Dispose of this with coordConvFreeList(). */
struct coordConv *coordConvLoadAll(char *fileName) 
{
struct coordConv *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = coordConvLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

/** Create a coordConv out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new coordConv */
struct coordConv *coordConvCommaIn(char **pS, struct coordConv *ret)
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->version = sqlStringComma(&s);
ret->date = sqlStringComma(&s);
ret->nibDir = sqlStringComma(&s);
ret->optional = sqlStringComma(&s);
*pS = s;
return ret;
}

/** Free a single dynamically allocated coordConv such as created
 * with coordConvLoad(). */
void coordConvFree(struct coordConv **pEl)
{
struct coordConv *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->version);
freeMem(el->date);
freeMem(el->nibDir);
freeMem(el->optional);
freez(pEl);
}

/** Free a list of dynamically allocated coordConv's */
void coordConvFreeList(struct coordConv **pList)
{
struct coordConv *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    coordConvFree(&el);
    }
*pList = NULL;
}

/** Print out coordConv.  Separate fields with sep. Follow last field with lastSep. */
void coordConvOutput(struct coordConv *el, FILE *f, char sep, char lastSep) 
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->version);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->date);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->nibDir);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->optional);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* ----------------------- End of autoSql generated portion ------------------*/

struct coordConvRep *createCoordConvRep()
/* create a coordinate conversion report, allocating
   coordConv structures as well */
{
struct coordConvRep *ccr = NULL;
AllocVar(ccr);
AllocVar(ccr->from);
AllocVar(ccr->to);
return ccr;
}

void loadUpDnaSeqs(struct coordConvRep *ccr)
/* get the three dnaSeqs that we are going to align */
{
char query[128];
char nibFileName[512];
struct sqlConnection *conn = sqlConnect(ccr->from->version);
FILE *nib = NULL;
int chromSize;
int querySize=0,midPos=0,upStream=0,downStream=0;
struct dnaSeq *seqList=NULL,*seq=NULL;
int chromStart,chromEnd,nibStart=0;
nibFileName[0] = '\0';
sprintf(query, "select fileName from chromInfo where chrom='%s'", ccr->from->chrom);
sqlQuickQuery(conn, query, nibFileName, sizeof(nibFileName));
if(strlen(nibFileName) == 0)
    errAbort("coordConv::loadUpDnaSeqs() - can't find file for chromosome %s.", ccr->from->chrom);
nibOpenVerify(nibFileName, &nib, &chromSize);

/* 
   Now we want to get three pieces of dna representing the middle, 
   and two ends of the sequence in question. If the sequence is smaller
   than 6kb use endpoints of 6kb, otherwise use a 1000bp off of each end.
*/

chromStart = ccr->from->chromStart;
chromEnd =  ccr->from->chromEnd;
querySize = chromEnd - chromStart;
midPos = (chromEnd + chromStart)/2;
if(querySize < 6000)
    {
    /* First the upstream (5') */
    nibStart = midPos -3000;
    if(nibStart < 0) nibStart =0;
    ccr->upSeq = nibLdPart(nibFileName, nib, chromSize,nibStart, ccr->seqSize);
    ccr->upStart = nibStart;
    
    /* Downstream (3') seq */
    nibStart = midPos +2000;
    if(nibStart > (chromSize-ccr->seqSize)) nibStart = chromSize - ccr->seqSize;
    ccr->downSeq = nibLdPart(nibFileName, nib, chromSize,nibStart, ccr->seqSize);
    ccr->downStart = nibStart;

    /* Middle seq */
    nibStart = midPos - (ccr->seqSize/2);
    if(nibStart > (chromSize - ccr->seqSize)) nibStart = chromSize- ccr->seqSize;
    ccr->midSeq = nibLdPart(nibFileName, nib, chromSize,nibStart, ccr->seqSize);
    ccr->midStart = nibStart;
    }
else 
    {
    /* First the upstream (5') */
    nibStart = chromStart;
    if(nibStart < 0) nibStart =0;
    ccr->upSeq = nibLdPart(nibFileName, nib, chromSize,nibStart, ccr->seqSize);
    ccr->upStart = nibStart;

    /* Downstream (3') seq */
    nibStart = chromEnd-1000;
    if(nibStart > (chromSize -ccr->seqSize)) nibStart = chromSize -ccr->seqSize;
    ccr->downSeq = nibLdPart(nibFileName, nib, chromSize,nibStart, ccr->seqSize);
    ccr->downStart = nibStart;

    /* Middle seq */
    nibStart = midPos - (ccr->seqSize/2);
    if(nibStart > (chromSize - ccr->seqSize)) nibStart = chromSize- ccr->seqSize;
    ccr->midSeq = nibLdPart(nibFileName, nib, chromSize,nibStart, ccr->seqSize);
    ccr->midStart = nibStart;
    }
carefulClose(&nib);
sqlDisconnect(&conn);
}

struct psl* doDnaAlignment(struct dnaSeq *seq, char *db, char *blatHost, char *port, char *nibDir) 
/* get the alignment from the blat host for this sequence */
{
struct psl *pslList = NULL;
int conn =0, count =0;
struct tempName pslTn;
FILE *f = NULL;
static struct gfSavePslxData outForm;


if(seq == NULL || db == NULL)
    errAbort("coordConv::doDnaAlignment() - dnaSeq and/or db can't be NULL.");
if(strlen(seq->dna) != seq->size)
    errAbort("coordConv::doDnaAlignment() - there seems to be something fishy about %s: the size doesn't equal the length", seq->name);
/* if there are too many n's it can cause the blat server to hang */
if(strstr(seq->dna, "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn") )
  return NULL; 
makeTempName(&pslTn,"ccR", ".psl");
f = mustOpen(pslTn.forCgi, "w");
pslxWriteHead(f, gftDna, gftDna);

/* align to genome, both strands */
outForm.f = f;
outForm.minGood = 920;
conn = gfConnect(blatHost, port);
gfAlignStrand(conn, nibDir, seq, FALSE, 20, gfSavePslx, &outForm);
close(conn);
reverseComplement(seq->dna, seq->size);
conn = gfConnect(blatHost, port);
gfAlignStrand(conn, nibDir, seq, TRUE, 20 , gfSavePslx, &outForm);
close(conn);
carefulClose(&f);
pslList = pslLoadAll(pslTn.forCgi);
remove(pslTn.forCgi);
return pslList;
}

void getAlignmentsForSeqs(struct coordConvRep *ccr, char *blatHost, char *port, char *nibDir)
/* Do alignments for the the dnaSeqs in a coordConvRep */
{
if(!(ccr->midSeq && ccr->upSeq && ccr-> downSeq))
    errAbort("coordConv::getAlignmentsForSeqs() - can't have any NULL dnaSeqs.");
ccr->midPsl = doDnaAlignment(ccr->midSeq, ccr->to->version, blatHost, port, nibDir);
ccr->upPsl =doDnaAlignment(ccr->upSeq, ccr->to->version, blatHost, port, nibDir);
ccr->downPsl =doDnaAlignment(ccr->downSeq, ccr->to->version, blatHost, port, nibDir);
}

int pslCmpScore(const void *va, const void *vb)
/* Compare to sort based on query then score. */
{
const struct psl *a = *((struct psl **)va);
const struct psl *b = *((struct psl **)vb);
return pslScore(b) - pslScore(a);
}

void dropNotBest(struct psl **pslList)
/* Sorts list and drops any score less than the best score */
{
struct psl *tail = NULL;
int bestScore =0;
slSort(pslList, pslCmpScore);
if(pslList == NULL) 
    return;
bestScore = pslScore(*pslList);
for(tail=*pslList; tail != NULL; tail=tail->next)
    {
    if(tail->next == NULL)
	break;
    if(pslScore(tail->next) < bestScore)
	{
	struct psl *tmp = tail->next;
	tail->next = NULL;
	pslFreeList(&tmp);
	break;
	}
    }
}

struct possibleCoord *findBestMatch(struct coordConvRep *ccr, struct possibleCoord *pcList)
/* find the best match given a series of possible coordinates */
{
struct possibleCoord *pc=NULL,*bestPc=NULL;
int origD1 = abs(ccr->upStart - ccr->midStart);
int origD2 = abs(ccr->midStart - ccr->downStart);
/* Here we want the one with the lowest difference between
   the original coordinates and the new coordinates */
double aveD1 = 0.0;
double bestD1 =0.0;
double secondBestD1 = -1;
for(pc=bestPc=pcList; pc != NULL; pc=pc->next)
    {
    aveD1 = fabs( ((pc->d1 - origD1) + (pc->d2 - origD2))/2 );
    bestD1 =  fabs( ((bestPc->d1 - origD1) + (bestPc->d2 - origD2))/2 );
    if(bestD1  == aveD1 && pc != bestPc)
	secondBestD1 = aveD1;
    if(bestD1 >= aveD1)
	bestPc = pc;
    }
/* in case there are perfect duplicates in the genome give up */
if(bestD1 == secondBestD1) 
    bestPc = NULL;
return bestPc;
}

void fillInNewCoord(struct coordConvRep *ccr, struct possibleCoord *pc) 
/* fill in coordinates of the report given possible coordinates */
{
unsigned s, midPos, e;
midPos = pc->midPsl->tStart - pc->midPsl->qStarts[0] + (ccr->seqSize/2);
ccr->to->chrom = cloneString(pc->midPsl->tName);
ccr->to->chromStart = midPos - (ccr->from->chromEnd - ccr->from->chromStart)/2;
ccr->to->chromEnd = midPos +(ccr->from->chromEnd - ccr->from->chromStart)/2;
if((ccr->from->chromEnd + ccr->from->chromStart) % 2)
    {
    if(ccr->from->chromEnd %2)
	ccr->to->chromEnd++;
    if(ccr->from->chromStart %2)
	ccr->to->chromEnd++;
    }
ccr->good = TRUE;
ccr->msg = cloneString("Sucessfully Converted.");
}


void checkPcAndTranslateCoordinates(struct coordConvRep *ccr, struct possibleCoord *pc)
/* check to make sure that the possibleCoord given is within
   a reasonable distance and then get the orignial coordinates */
{
int origD1 = abs(ccr->upStart - ccr->midStart);
int origD2 = abs(ccr->midStart - ccr->downStart);
char *nns =  "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
if(pc == NULL)
    {
    ccr->to->chrom= cloneString("unknown");
    ccr->good = FALSE;
    if(strstr(ccr->midSeq->dna, nns)  ||
       strstr(ccr->upSeq->dna,  nns) ||
       strstr(ccr->downSeq->dna,nns))
	ccr->msg = cloneString("Couldn't find a match for this coordinate range. Sequence has a few n's in it.");
    else
	ccr->msg = cloneString("Couldn't place this sequence on new draft. Part of it appears to have moved or be duplicated in the new draft.");
    }
else if( origD1 > (1.2*pc->d1) || origD1 < (.8*pc->d1))
    {
    ccr->to->chrom= cloneString("unknown");
    ccr->good = FALSE;
    ccr->msg = cloneString("Found a match, however original upstream start and aligned upstream start are too far apart to trust.");
    }
else if( origD2 > (1.2*pc->d2) || origD2 < (.8*pc->d2))
    {
    ccr->to->chrom= cloneString("unknown");
    ccr->good = FALSE;
    ccr->msg = cloneString("Found a match, however original downstream start and aligned downstream start are too far apart to trust.");
    }
else 
    {
    fillInNewCoord(ccr,pc);
    }
}

void evaluateAlignments(struct coordConvRep *ccr) 
/* Figure out if the alignments of the up,mid, and down seqs indicates
that we can shift coordinates. If yes, do conversion otherwise set
error message and ccr->good = FALSE; At first I'm going to be
pretty stringent. They all have to use one of the best scoring
psls to:
   - Line up all on the same strand.
   - Line up in the original order (or reverse if on opposite strand).
   - Line up withing 20% of the original distance from eachother.
*/
{
struct psl *upPsl=NULL,*midPsl=NULL,*downPsl=NULL;
struct possibleCoord *pcList = NULL, *pc=NULL;

if(ccr->upPsl && ccr->midPsl && ccr->downPsl)
    {
    /* Drop the psls that don't have the best score */
    dropNotBest(&ccr->upPsl);
    dropNotBest(&ccr->midPsl);
    dropNotBest(&ccr->downPsl);
    
    for(upPsl=ccr->upPsl; upPsl != NULL; upPsl=upPsl->next)
	{
	for(midPsl=ccr->midPsl; midPsl != NULL; midPsl=midPsl->next)
	    {
	    /* make sure that we're on the same strand and chromosome */
	    if(!(sameString(midPsl->strand,upPsl->strand)) || !(sameString(midPsl->tName, upPsl->tName)))
		continue;
	    for(downPsl=ccr->downPsl; downPsl!=NULL; downPsl=downPsl->next) 
		{
		/* make sure that we're on the same strand and chromosome */
		if(!(sameString(upPsl->strand,downPsl->strand)) || !(sameString(upPsl->tName,downPsl->tName)))
		    continue;
		/* check to make sure we're in the correct relative order */
		if(strstr(upPsl->strand,"+"))
		    {
		    /* make sure that we're in the right realtive order if it is + strand */
		    if( (upPsl->tEnd > midPsl->tStart) || (midPsl->tEnd > downPsl->tStart))
			continue;
		    }
		else if(strstr(upPsl->strand,"-")) 
		    {
		    /* make sure that we're in the right realtive order if it is + strand */
		    if( (downPsl->tStart > midPsl->tEnd) ||  (midPsl->tStart > upPsl->tEnd))
			continue;
		    }
		else 
		    {
		    errAbort("coordConv::evaluateAlignments() - alignment doesn't appear to be on '+' or '-' strand.");
		    }
		
		/* Whew, we've got a candidate, record the distances and add it to our
		   possibleCoord structure */
		AllocVar(pc);
		pc->midPsl = midPsl;
		pc->upPsl = upPsl;
		pc->downPsl = downPsl;
		pc->d1 = abs(upPsl->tStart - midPsl->tStart);
		pc->d2 = abs(midPsl->tStart - downPsl->tStart);
		slSafeAddHead(&pcList,pc);
		pc = NULL;
		}
	    }
	}
    }
else 
    {
#ifdef DEBUG
    FILE *f = mustOpen("bad.fa", "a");
    fprintf(f,"#\n");
    faWriteNext(f,ccr->upSeq->name,ccr->upSeq->dna, ccr->upSeq->size);
    faWriteNext(f,ccr->midSeq->name,ccr->midSeq->dna, ccr->midSeq->size);
    faWriteNext(f,ccr->downSeq->name,ccr->downSeq->dna, ccr->downSeq->size);
    carefulClose(&f);
#endif
    }
pc = findBestMatch(ccr,pcList);
checkPcAndTranslateCoordinates(ccr,pc);
}

void coordConvConvert(struct coordConvRep *ccr, char *blatHost, char *port, char *nibDir)
/* Given a coordConvRep containing old coordinates and new db attempts to 
   calculate the position in genome */
{
loadUpDnaSeqs(ccr);
getAlignmentsForSeqs(ccr, blatHost, port, nibDir);
evaluateAlignments(ccr);
}

struct dbDb *loadDbInformation(char *database)
/* load up the information for a particular draft */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr = NULL;
char **row;
struct dbDb *dbList = NULL, *db = NULL;
char query[256];
snprintf(query, sizeof(query), "select * from dbDb where name='%s'", database);

/* Scan through dbDb table, loading into list */
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    db = dbDbLoad(row);
    slAddHead(&dbList, db);
    }
sqlFreeResult(&sr);
hDisconnectCentral(&conn);
if(slCount(dbList) != 1)
    errAbort("coordConv.c::loadDbInformation() - expecting 1 dbDb record for %s got %d", db, slCount(dbList));
return dbList;
}

struct coordConvRep *coordConvConvertPos(char *chr, unsigned chromStart, unsigned chromEnd, char *oldDb, char *newDb,
					 char *blatHost, char *port, char *nibDir)
/* setup a coordinate conversion report and use it to 
   try and find the new coordinates */
{
struct coordConvRep *ccr = createCoordConvRep();
struct dbDb *newDbRec = NULL, *oldDbRec = NULL;
oldDbRec = loadDbInformation(oldDb);
ccr->from->chrom = cloneString(chr);
ccr->from->chromStart = chromStart;
ccr->from->chromEnd = chromEnd;
ccr->from->version = cloneString(oldDbRec->name);
ccr->from->date = cloneString(oldDbRec->description);
ccr->from->nibDir = cloneString(oldDbRec->nibPath);
ccr->seqSize=1000;
newDbRec = loadDbInformation(newDb);
ccr->to->version = cloneString(newDbRec->name);
ccr->to->date = cloneString(newDbRec->description);
ccr->to->nibDir = cloneString(newDbRec->nibPath);
coordConvConvert(ccr, blatHost, port, nibDir);
dbDbFree(&oldDbRec);
dbDbFree(&newDbRec);
return ccr;
}

void coordConvRepFree(struct coordConvRep **pEl)
/* free an individual coordinate conversion report */
{
struct coordConvRep *el;
if((el = *pEl) == NULL) return;
freeMem(el->msg);
coordConvFree(&el->to);
coordConvFree(&el->from);
freeDnaSeqList(&el->upSeq);
freeDnaSeqList(&el->midSeq);
freeDnaSeqList(&el->downSeq);
pslFreeList(&el->upPsl);
pslFreeList(&el->midPsl);
pslFreeList(&el->downPsl);
}

void coordConvRepFreeList(struct coordConvRep **pList)
/* free a list of coordiate reports */
{
struct coordConvRep *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    coordConvRepFree(&el);
    }
*pList = NULL;
}
