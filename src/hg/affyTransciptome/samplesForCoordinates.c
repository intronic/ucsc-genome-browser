/* Download the coordinates for a range of human coordinates of
   form chrN:1-100. Designed for getting probe values for exons. */
#include "common.h"
#include "hdb.h"
#include "jksql.h"
#include "hgConfig.h"
#include "genePred.h"
#include "cheapcgi.h"
#include "sample.h"

struct genomeBit
/* Piece of the genome */
{
    struct genomeBit *next;
    char *fileName;
    char *chrom;
    int chromStart;
    int chromEnd;
};

void usage() 
{
errAbort("samplesForCoordinates - Small program that takes coordinates for hg7 build\n"
	 "and cuts out the results from the affy transcriptome tracks. Can also be used\n"
	 "for loading all the exons froma given genePred record.\n"
	 "usage:\n\t"
	 "samplesForCoordinates <chrN:100-200> <optionally gpFile=fileName>\n");
}

struct genomeBit *parseGenomeBit(char *name)
/* take a name like chrN:1-1000 and parse it into a genome bit */
{
struct genomeBit *gp = NULL;
char *pos1 = NULL, *pos2 = NULL;
AllocVar(gp);
pos1 = strstr(name, ":");
 if(pos1 == NULL)
   errAbort("runSlam::parseGenomeBit() - %s doesn't look like chrN:1-1000", name);
gp->chrom = name;
*pos1 = '\0';
gp->chrom = cloneString(gp->chrom);
pos1++;
pos2 = strstr(pos1, "-");
 if(pos2 == NULL)
   errAbort("runSlam::parseGenomeBit() - %s doesn't look like chrN:1-1000, name");
*pos2 = '\0';
pos2++;
gp->chromStart = atoi(pos1);
gp->chromEnd = atoi(pos2);
return gp;
}

struct sqlConnection *getHgdbtestConn(char *db) 
/* Read .hg.conf and return connection. */
{
char *host = cfgOption( "hgdbtest.host");
char *user = cfgOption( "hgdbtest.user");
char *password = cfgOption( "hgdbtest.password");
hSetDbConnect(host,db,user,password);
return sqlConnectRemote(host, user,password, db);
}

int sampleCmpName(const void *va, const void *vb)
{
/* Compare to sort based on name, then on chromStart */
const struct sample *a = *((struct sample **)va);
const struct sample *b = *((struct sample **)vb);
int dif;
dif = strcmp(a->name, b->name);
if (dif == 0)
    dif = a->chromStart - b->chromEnd;
return dif;
}

char *getNameForExp(int num)
{
switch(num)
    {
    case 1 : 
	return cloneString("A-375");
    case 2 : 
	return cloneString("CCRF-Cem");
    case 3 : 
	return cloneString("COLO-205");
    case 4 : 
	return cloneString("FHs-738Lu");
    case 5 : 
	return cloneString("HepG2");
    case 6 : 
	return cloneString("Jurkat");
    case 7 : 
	return cloneString("NCCIT");
    case 8 : 
	return cloneString("NIH_OVCAR-3");
    case 9 : 
	return cloneString("PC3");
    case 10 : 
	return cloneString("SK-N-AS");
    case 11 : 
	return cloneString("U-87MG");
    default:
	errAbort("Don't recognize num %d");
    }
}

char *getHumanName(char *name)
{
char *s = cloneString(name);
char *mark = strstr(s,".");
int exp = 0;
char buff[256];
assert(mark);
*mark = '\0';
exp = atoi(s);
freez(&s);
s = getNameForExp(exp);
snprintf(buff, sizeof(buff), "%s", s, name);
return cloneString(buff);
}

void outputSamples(struct sample *sList, FILE *out, int chromStart, int chromEnd)
/* Output the sample results in tab delimited form where first column
   is experiment name and following columns are the probe results for
   that experiment. */
{
struct sample *s = NULL;
char *name = NULL;
char *outputName = NULL;
int i;
int start, end;
slSort(&sList, sampleCmpName);
assert(sList);
name = sList->name;
outputName = getHumanName(name);
fprintf(out, "%s", outputName);
for(s = sList; s != NULL; s = s->next)
    {
    start = s->chromStart;
    end = s->chromEnd;
    if(differentString(name, s->name))
	{
	name = s->name;
	outputName = getHumanName(name);
	fprintf(out,"\n%s",outputName);
	}
    for(i=0; i< s->sampleCount; i++)
	{
	if(start + s->samplePosition[i] >= chromStart && start + s->samplePosition[i] <= chromEnd)
	    fprintf(out, "\t%d", s->sampleHeight[i]);
	}
    }
fprintf(out,"\n");
}

struct sample *loadSamples(char *chrom, int chromStart, int chromEnd, struct sqlConnection *conn)
/* Load up all the sample from a given region. */
{
struct sqlResult *sr = NULL;
char **row;
struct sample *sList=NULL,*s=NULL;
int rowOffset;
sr = hRangeQuery(conn, "affyTrans", chrom, chromStart, chromEnd, NULL, &rowOffset);
while((row = sqlNextRow(sr)) != NULL)
    {
    s = sampleLoad(row+rowOffset);
    slAddHead(&sList, s);
    }
sqlFreeResult(&sr);
return sList;
}

void getSamples(char *coordinates, FILE *out)
/* Select the samples and output. */
{
struct genomeBit *gb = parseGenomeBit(coordinates);
struct sqlConnection *conn = getHgdbtestConn("sugnet");
struct sqlResult *sr = NULL;
char **row;
struct sample *sList=NULL,*s=NULL;
int rowOffset;
warn("Starting query for coordiantes %s:%d-%d, size %d", gb->chrom, gb->chromStart, gb->chromEnd, (gb->chromEnd-gb->chromStart));
sr = hRangeQuery(conn, "affyTrans", gb->chrom, gb->chromStart, gb->chromEnd, NULL, &rowOffset);
warn("Putting into samples list");
while((row = sqlNextRow(sr)) != NULL)
    {
    s = sampleLoad(row+rowOffset);
    slAddHead(&sList, s);
    }
sqlFreeResult(&sr);
sqlDisconnect(&conn);
warn("Outputting results");
outputSamples(sList, out, gb->chromStart, gb->chromEnd);
sampleFreeList(&sList);
}

boolean sameWordOrRoot(char *s1, char *s2)
/* Return true if s1 and s2 are the same word, or if they are the
   same up to last '.' character. */
{
char *t1 = cloneString(s1);
char *t2 = cloneString(s2);
boolean retVal = FALSE;
chopSuffix(t1);
chopSuffix(t2);
if(sameWord(s1, s2) || sameWord(t1,t2))
    retVal = TRUE;
freez(&t1);
freez(&t2);
return retVal;
}

int countDifferentNameOrRoot(struct sample *sList) 
/* Count how many samples have different names or roots of names. */
{
struct sample *s = NULL;
int count =1; // the first one is different from NULL
for(s=sList; s->next != NULL; s=s->next)
    {
    if(!sameWordOrRoot(s->name, s->next->name))
	count++;
    }
return count;
}

void groupSamplesByName(struct sample **pSamp, struct sample *sList, int expCount)
/* Put samples that are replicates (have the same name) into the
   different lists of pSamp. */
{
struct sample *s = NULL, *sNext = NULL;
struct sample *tmpList = NULL;
int current = 0;
int i;
struct sample *last = NULL;
assert(pSamp);
last = sList;
s = sList->next;
slAddHead(&pSamp[current],last);
sNext = s->next;
for(; sNext != NULL; s=sNext)
    {
    sNext = s->next;
    if(sameWordOrRoot(last->name, s->name))
	{
	slAddHead(&pSamp[current], s);
	last = s;
	}
    else
	{
	current++;
	assert(current < expCount);
	slAddHead(&pSamp[current], s);
	last = s;
	}
    }
for(i=0;i<expCount; i++) 
    slReverse(&pSamp[i]);
}

int outputSampleRows(struct sample *sList, FILE *out, char *chrom, 
		     int chromStart, int chromEnd, int exon, int probeCount, char *name)
/* Write out a sample row in R's data frame format... */
{
struct sample *s = NULL, *tail = NULL;
char *outputName = NULL;
int probeOffset = 0;
int start, end;
int i;
int markStart = -1;
/* Loop through each set of samples. */
for(tail = sList; tail != NULL; tail = tail->next)
    {
    /* But we don't want to get the same replicates multiple times.... */
    if(tail->chromStart != markStart)
	{
	for(i=0; i< tail->sampleCount; i++)
	    {
	    start = tail->chromStart;
	    end = tail->chromEnd;
	    if(start + tail->samplePosition[i] >= chromStart && start +tail->samplePosition[i] <= chromEnd)
		{
		int sampStart = start + tail->samplePosition[i];
		int sampEnd = start + tail->samplePosition[i] +1;
		for(s=tail; s != NULL; s=s->next)
		    {
		    if(s->chromStart == start && s->chromEnd == end)
			{
			outputName = getHumanName(s->name);
			fprintf(out,"%s\t%d\t%d\t%d\t%s\t%s\t%d\t%d\t%s\n", 
				outputName, exon, probeCount+probeOffset, s->sampleHeight[i], 
				name, s->chrom, sampStart, sampEnd, s->strand);
			freez(&outputName);
			}
		    }
		probeOffset++;
		}
	    }
	markStart = tail->chromStart;
	}
    }
return probeOffset;
}

void printOutHeader(FILE *out)
{
printf("exp\texon\tprobe\tresponse\tname\tchrom\tchromStart\tchromEnd\tstrand\n");
}

void outputGpSamples(struct genePred *gp, struct sqlConnection *conn, FILE *out)
/* Select the probes for each exon. */
{
int exonCount = 0;
struct sample *sList = NULL;
int i;
int probeCount =1;
int probesInExon =0;
printOutHeader(out);
for(i=0; i<gp->exonCount; i++)
    {
    struct sample **pSamp = NULL;
    int expCount = 0;
    int j;
    sList = loadSamples(gp->chrom, gp->exonStarts[i], gp->exonEnds[i], conn);
    slSort(&sList, sampleCmpName);
    expCount = countDifferentNameOrRoot(sList);
    pSamp = AllocArray(pSamp, expCount);
    groupSamplesByName(pSamp, sList, expCount);
    for(j=0; j<expCount; j++)
	{
	probesInExon = outputSampleRows(pSamp[j], out, gp->chrom, 
					gp->exonStarts[i], gp->exonEnds[i],(i+1), probeCount, gp->name);
	sampleFreeList(&pSamp[j]);
	}
    probeCount += probesInExon;
    freez(&pSamp);
    }
}

void loadGenePredSamples(char *fileName, FILE *out)
/* Load all of the genePreds from a file and output their samples. */
{
struct sqlConnection *conn = getHgdbtestConn("sugnet");
struct genePred *gpList = genePredLoadAll(fileName);
struct genePred *gp = NULL;
for(gp = gpList; gp != NULL; gp = gp->next)
    {
    outputGpSamples(gp, conn, out);
    }
genePredFreeList(&gpList);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
{
char *gpFileName = NULL;
if(argc == 1)
    usage();
cgiSpoof(&argc, argv);
gpFileName = cgiOptionalString("gpFile");
warn("Loading Records");
if(gpFileName != NULL)
    loadGenePredSamples(gpFileName, stdout);
else
    getSamples(argv[1],stdout);
warn("Done");
return 0;
}









