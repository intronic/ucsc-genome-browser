/* hgClonePos - create clonePos table in browser database. */
#include "common.h"
#include "portable.h"
#include "linefile.h"
#include "dystring.h"
#include "hash.h"
#include "hCommon.h"
#include "jksql.h"
#include "glDbRep.h"
#include "clonePos.h"
#include "gsSeqInfo.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgClonePos - create clonePos table in browser database\n"
  "usage:\n"
  "   hgClonePos database ooDir ffa/sequence.inf gsDir\n");
}

char *createClonePos = 
"CREATE TABLE clonePos (\n"
"   name varchar(255) not null,	# Name of clone including version\n"
"   seqSize int unsigned not null,	# base count not including gaps\n"
"   phase tinyint unsigned not null,	# htg phase\n"
"   chrom varchar(255) not null,	# Chromosome name\n"
"   chromStart int unsigned not null,	# Start in chromosome\n"
"   chromEnd int unsigned not null,	# End in chromosome\n"
"   stage char(1) not null,	# F/D/P for finished/draft/predraft\n"
"   faFile varchar(255) not null,	# File with sequence.\n"
"             #Indices\n"
"   PRIMARY KEY(name(12)),\n"
"   INDEX(chrom(12),chromStart),\n"
"   INDEX(chrom(12),chromEnd)\n"
")\n";

void addCloneInfo(char *glFileName, struct hash *cloneHash, struct clonePos **pCloneList)
/* Add in clone info from one .gl file. */
{
char dir[256], chrom[128], ext[64];
struct gl gl;
struct lineFile *lf = lineFileOpen(glFileName, TRUE);
struct clonePos *clone;
char *line, *words[8];
int lineSize, wordCount;
char cloneVerName[128];
char cloneName[128];

printf("Processing %s\n", glFileName);
splitPath(glFileName, dir, chrom, ext);
while (lineFileNext(lf, &line, &lineSize))
    {
    wordCount = chopLine(line, words);
    if (wordCount == 0)
        continue;
    if (wordCount != 4)
        errAbort("Expecting %d words line %d of %s", 4, lf->lineIx, lf->fileName);
    glStaticLoad(words, &gl);
    fragToCloneVerName(gl.frag, cloneVerName);
    fragToCloneName(gl.frag, cloneName);
    if ((clone = hashFindVal(cloneHash, cloneName)) == NULL)
        {
	AllocVar(clone);
	clone->name = cloneString(cloneVerName);
	clone->chrom = cloneString(chrom);
	clone->chromStart = gl.start;
	clone->chromEnd = gl.end;
	slAddHead(pCloneList, clone);
	hashAdd(cloneHash, cloneName, clone);
	}
    else
        {
	if (!sameString(clone->chrom, chrom))
	    {
	    warn("Clone %s is on chromosomes %s and %s.  Ignoring %s",
	        cloneName, clone->chrom, chrom, chrom);
	    continue;
	    }
	if (clone->chromStart > gl.start)
	    clone->chromStart = gl.start;
	if (clone->chromEnd < gl.end)
	    clone->chromEnd = gl.end;
	}
    }
lineFileClose(&lf);
}

int cmpClonePos(const void *va, const void *vb)
/* Compare to sort based on chromosome and chromStart. */
{
const struct clonePos *a = *((struct clonePos **)va);
const struct clonePos *b = *((struct clonePos **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}

struct clonePos *readClonesFromOoDir(char *ooDir, struct hash *cloneHash)
/* Read in clones from ooDir. */
{
struct clonePos *cloneList = NULL;
struct fileInfo *chrFiList = NULL, *chrFi; 
struct fileInfo *glFiList = NULL, *glFi;
char pathName[512];

uglyf("readClonesFromOoDir %s\n", ooDir);
chrFiList = listDirX(ooDir, "*", FALSE);
for (chrFi = chrFiList; chrFi != NULL; chrFi = chrFi->next)
    {
    if (chrFi->isDir && strlen(chrFi->name) <= 2)
        {
	sprintf(pathName, "%s/%s", ooDir, chrFi->name);
	glFiList = listDirX(pathName, "*.gl2", TRUE);
	for (glFi = glFiList; glFi != NULL; glFi = glFi->next)
	    addCloneInfo(glFi->name, cloneHash, &cloneList);
	slFreeList(&glFiList);
        }
    }
slFreeList(&chrFiList);
slReverse(&cloneList);
slSort(&cloneList, cmpClonePos);
if (slCount(cloneList) < 0)
   errAbort("No .gl2 files in %s\n", ooDir);
return cloneList;
}

void addStageInfo(char *gsDir, struct hash *cloneHash)
/* Add info about which file and what stage clone is in. */
{
static char *subDirs[] = {"fin/fa", "draft/fa", "predraft/fa"};
static char stages[] = "FDP";
int numStages = strlen(stages);
int i;
char pathName[512];
struct slName *dirList, *dirEl;
char *subDir, stage;
int warnsLeft = 10;	/* Only give first 10 warnings about missing clones. */
char cloneName[256];
struct clonePos *clone;

for (i=0; i<numStages; ++i)
   {
   subDir = subDirs[i];
   stage = stages[i];
   sprintf(pathName, "%s/%s", gsDir, subDir);
   printf("Processing %s\n", pathName);
   dirList = listDir(pathName, "*.fa");
   uglyf("Got %d fa files in %s\n", slCount(dirList), pathName);
   for (dirEl = dirList; dirEl != NULL; dirEl = dirEl->next)
       {
       strcpy(cloneName, dirEl->name);
       chopSuffix(cloneName);
       if ((clone = hashFindVal(cloneHash, cloneName)) == NULL)
            {
	    if (warnsLeft > 0)
	       {
	       --warnsLeft;
	       warn("%s not in ooDir", cloneName);
	       }
	    continue;
	    }
       sprintf(pathName, "%s/%s/%s", gsDir, subDir, dirEl->name);
       clone->faFile = cloneString(pathName);
       clone->stage[0] = stage;
       }
   }
}

void addSeqInfo(char *seqInfoName, struct hash *cloneHash)
/* Add in information from sequence.info file. */
{
struct lineFile *lf = lineFileOpen(seqInfoName, TRUE);
char *line, *words[16];
int lineSize, wordCount;
struct clonePos *clone;
struct gsSeqInfo gs;
int warnsLeft = 10;	/* Only give first 10 warnings about missing clones. */

printf("Processing %s\n", seqInfoName);
while (lineFileNext(lf, &line, &lineSize))
    {
    if (line[0] == '#')
        continue;
    wordCount = chopLine(line, words);
    if (wordCount == 0)
        continue;
    if (wordCount != 8)
        errAbort("Expecting 8 words line %d of %s", lf->lineIx, lf->fileName);
    gsSeqInfoStaticLoad(words, &gs);
    if (gs.phase != 0)
        {
	chopSuffix(gs.acc);
	if ((clone = hashFindVal(cloneHash, gs.acc)) == NULL)
	    {
	    if (warnsLeft > 0)
	       {
	       --warnsLeft;
	       warn("%s not in ooDir", gs.acc);
	       }
	    continue;
	    }
	clone->seqSize = gs.size;
	clone->phase = gs.phase;
	}
    }
lineFileClose(&lf);
}

void checkClonePos(struct clonePos *cloneList)
/* Make sure that all necessary bits of cloneList are filled in. */
{
struct clonePos *clone;
int errCount = 0;

for (clone = cloneList; clone != NULL; clone = clone->next)
    {
    if (clone->seqSize == 0)
        {
	if (errCount < 20)
	    warn("Missing size info on %s (not in sequence.inf)", clone->name);
	++errCount;
	}
    if (clone->faFile == NULL)
        {
	if (errCount < 20)
	    warn("Missing file location on %s (not under gsDir)", clone->name);
	++errCount;
	}
    }
if (errCount > 0)
    {
    errAbort("Aborting with %d errors\n", errCount);
    }
}

void saveClonePos(struct clonePos *cloneList, char *database)
/* Save sorted clone position list to database. */
{
struct sqlConnection *conn = sqlConnect(database);
struct clonePos *clone;
char tabFileName[L_tmpnam];
FILE *f;
struct dyString *ds = newDyString(2048);

/* Create tab file from clone list. */
printf("Creating tab file\n");
tmpnam(tabFileName);
f = mustOpen(tabFileName, "w");
for (clone = cloneList; clone != NULL; clone = clone->next)
    clonePosTabOut(clone, f);
fclose(f);

/* Create table if it doesn't exist, delete whatever is
 * already in it, and fill it up from tab file. */
printf("Loading clonePos table\n");
sqlMaybeMakeTable(conn, "clonePos", createClonePos);
sqlUpdate(conn, "DELETE from clonePos");
dyStringPrintf(ds, "LOAD data local infile '%s' into table clonePos", 
    tabFileName);
sqlUpdate(conn, ds->string);

/* Clean up. */
remove(tabFileName);
sqlDisconnect(&conn);
}

void hgClonePos(char *database, char *ooDir, char *seqInfoName, char *gsDir)
/* hgClonePos - create clonePos table in browser database. */
{
struct hash *cloneHash = newHash(16);
struct clonePos *cloneList = NULL;

cloneList = readClonesFromOoDir(ooDir, cloneHash);
addStageInfo(gsDir, cloneHash);
addSeqInfo(seqInfoName, cloneHash);
checkClonePos(cloneList);
saveClonePos(cloneList, database);
freeHash(&cloneHash);
clonePosFreeList(&cloneList);
}

int main(int argc, char *argv[])
/* Process command line. */
{
if (argc != 5)
    usage();
hgClonePos(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
