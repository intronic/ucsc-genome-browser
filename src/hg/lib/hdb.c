/* hdb - human genome browser database. */
#include "common.h"
#include "portable.h"
#include "hash.h"
#include "jksql.h"
#include "dnautil.h"
#include "dnaseq.h"
#include "nib.h"
#include "hdb.h"
#include "hgRelate.h"
#include "fa.h"
#include "hgConfig.h"
#include "ctgPos.h"
#include "trackDb.h"
#include "hCommon.h"
#include "hgFind.h"
#include "dbDb.h"
#include "blatServers.h"

static struct sqlConnCache *hdbCc = NULL;
static struct sqlConnCache *centralCc = NULL;

static char *hdbHost;
static char *hdbName = "hg8";
static char *hdbUser;
static char *hdbPassword;

void hDefaultConnect()
/* read in the connection options from config file */
{
hdbHost 	= cfgOption("db.host");
hdbUser 	= cfgOption("db.user");
hdbPassword	= cfgOption("db.password");

if(hdbHost == NULL || hdbUser == NULL || hdbPassword == NULL)
    errAbort("cannot read in connection setting from configuration file.");
}


void hSetDbConnect(char* host, char *db, char *user, char *password)
/* set the connection information for the database */
{
    hdbHost = host;
    hdbName = db;
    hdbUser = user;
    hdbPassword = password;
}

void hSetDb(char *dbName)
/* Set the database name. */
{
if (hdbCc != NULL)
    errAbort("Can't hgSetDb after an hgAllocConn(), sorry.");
hdbName = dbName;
}

char *hGetDb()
/* Return the current database name. */
{
return hdbName;
}

char *hGetDbHost()
/* Return the current database host. */
{
return hdbHost;
}

char *hGetDbName()
/* Return the current database name. */
{
return hdbName;
}

char *hGetDbUser()
/* Return the current database user. */
{
return hdbUser;
}

char *hGetDbPassword()
/* Return the current database password. */
{
return hdbPassword;
}

struct sqlConnection *hAllocConn()
/* Get free connection if possible. If not allocate a new one. */
{
if (hdbCc == NULL)
    hdbCc = sqlNewConnCache(hdbName);
return sqlAllocConnection(hdbCc);
}

void hFreeConn(struct sqlConnection **pConn)
/* Put back connection for reuse. */
{
sqlFreeConnection(hdbCc, pConn);
}

struct sqlConnection *hConnectCentral()
/* Connect to central database where user info and other info
 * not specific to a particular genome lives.  Free this up
 * with hDisconnectCentral(). */
{
if (centralCc == NULL)
    {
    char *database = "hgcentral";
    char *host = cfgOption("central.host");
    char *user = cfgOption("central.user");
    char *password = cfgOption("central.password");;

    if (host == NULL || user == NULL || password == NULL)
	errAbort("Please set central options in the hg.conf file.");
    centralCc = sqlNewRemoteConnCache("hgcentral", host, user, password);
    }
return sqlAllocConnection(centralCc);
}

void hDisconnectCentral(struct sqlConnection **pConn)
/* Put back connection for reuse. */
{
sqlFreeConnection(centralCc, pConn);
}

boolean hTableExists(char *table)
/* Return TRUE if a table exists in database. */
{
struct sqlConnection *conn = hAllocConn();
boolean exists = sqlTableExists(conn, table);
hFreeConn(&conn);
return exists;
}


int hChromSize(char *chromName)
/* Return size of chromosome. */
{
struct sqlConnection *conn;
int size;
char query[256];

conn = hAllocConn();
sprintf(query, "select size from chromInfo where chrom = '%s'", chromName);
size = sqlQuickNum(conn, query);
hFreeConn(&conn);
return size;
}

void hNibForChrom(char *chromName, char retNibName[512])
/* Get .nib file associated with chromosome. */
{
struct sqlConnection *conn;
char query[256];
conn = hAllocConn();
sprintf(query, "select fileName from chromInfo where chrom = '%s'", chromName);
if (sqlQuickQuery(conn, query, retNibName, 512) == NULL)
    errAbort("Sequence %s isn't in database", chromName);
hFreeConn(&conn);
}

struct hash *hCtgPosHash()
/* Return hash of ctgPos from current database keyed by contig name. */
{
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
struct hash *hash = newHash(10);
struct ctgPos *ctg;

conn = hAllocConn();
sr = sqlGetResult(conn, "select * from ctgPos");
while ((row = sqlNextRow(sr)) != NULL)
    {
    ctg = ctgPosLoad(row);
    hashAdd(hash, ctg->contig, ctg);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return hash;
}

struct dnaSeq *hDnaFromSeq(char *seqName, int start, int end, enum dnaCase dnaCase)
/* Fetch DNA */
{
char fileName[512];
struct dnaSeq *seq;

hNibForChrom(seqName, fileName);
seq = nibLoadPart(fileName, start, end-start);
if (dnaCase == dnaUpper)
    touppers(seq->dna);
return seq;
}

struct dnaSeq *hLoadChrom(char *chromName)
/* Fetch entire chromosome into memory. */
{
int size = hChromSize(chromName);
return hDnaFromSeq(chromName, 0, size, dnaLower);
}

struct slName *hAllChromNames()
/* Get list of all chromosome names. */
{
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
struct slName *list = NULL, *el;
char **row;

sr = sqlGetResult(conn, "select chrom from chromInfo");
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = newSlName(row[0]);
    slAddHead(&list, el);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&list);
return list;
}

struct largeSeqFile
/* Manages our large external sequence files.  Typically there will
 * be around four of these.  This basically caches the file handle
 * so don't have to keep opening and closing them. */
{
    struct largeSeqFile *next;  /* Next in list. */
    char *path;                 /* Path name for file. */
    HGID id;                    /* Id in extFile table. */
    int fd;                     /* File handle. */
    };

static struct largeSeqFile *largeFileList;  /* List of open large files. */

struct largeSeqFile *largeFileHandle(HGID extId)
/* Return handle to large external file. */
{
struct largeSeqFile *lsf;

/* Search for it on existing list and return it if found. */
for (lsf = largeFileList; lsf != NULL; lsf = lsf->next)
    {
    if (lsf->id == extId)
        return lsf;
    }

/* Open file and put it on list. */
    {
    struct largeSeqFile *lsf;
    struct sqlConnection *conn = hgAllocConn();
    char query[256];
    struct sqlResult *sr;
    char **row;
    long size;
    char *path;

    /* Query database to find full path name and size file should be. */
    sprintf(query, "select path,size from extFile where id=%u", extId);
    sr = sqlGetResult(conn,query);
    if ((row = sqlNextRow(sr)) == NULL)
        errAbort("Database inconsistency - no external file with id %lu", extId);

    /* Save info on list. Check that file size is what we think it should be. */
    AllocVar(lsf);
    lsf->path = path = cloneString(row[0]);
    size = sqlUnsigned(row[1]);
    if (fileSize(path) != size)
        errAbort("External file %s has changed, need to resync database", path);
    lsf->id = extId;
    if ((lsf->fd = open(path, O_RDONLY)) < 0)
        errAbort("Couldn't open external file %s", path);
    slAddHead(&largeFileList, lsf);
    sqlFreeResult(&sr);
    hgFreeConn(&conn);
    return lsf;
    }
}

void *readOpenFileSection(int fd, long offset, size_t size, char *fileName)
/* Allocate a buffer big enough to hold a section of a file,
 * and read that section into it. */
{
void *buf;
buf = needMem(size+1);
if (lseek(fd, offset, SEEK_SET) < 0)
        errAbort("Couldn't seek to %ld in %s", offset, fileName);
if (read(fd, buf, size) < size)
        errAbort("Couldn't read %u bytes at %ld in %s", size, offset, fileName);
return buf;
}


void hRnaSeqAndId(char *acc, struct dnaSeq **retSeq, HGID *retId)
/* Return sequence for RNA and it's database ID. */
{
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
char query[256];
int fd;
HGID extId;
size_t size;
long offset;
char *buf;
struct dnaSeq *seq;
struct largeSeqFile *lsf;

sprintf(query,
   "select id,extFile,file_offset,file_size from seq where seq.acc = '%s'",
   acc);
sr = sqlMustGetResult(conn, query);
row = sqlNextRow(sr);
if (row == NULL)
    errAbort("No sequence for %s in database", acc);
*retId = sqlUnsigned(row[0]);
extId = sqlUnsigned(row[1]);
offset = sqlUnsigned(row[2]);
size = sqlUnsigned(row[3]);
lsf = largeFileHandle(extId);
buf = readOpenFileSection(lsf->fd, offset, size, lsf->path);
*retSeq = seq = faFromMemText(buf);
sqlFreeResult(&sr);
hFreeConn(&conn);
}


struct dnaSeq *hExtSeq(char *acc)
/* Return sequence for external sequence. */
{
struct dnaSeq *seq;
HGID id;
hRnaSeqAndId(acc, &seq, &id);
return seq;
}

struct dnaSeq *hRnaSeq(char *acc)
/* Return sequence for RNA. */
{
return hExtSeq(acc);
}

static char *hFreezeDbConversion(char *database, char *freeze)
/* Find freeze given database or vice versa.  Pass in NULL
 * for parameter that is unknown and it will be returned
 * as a result.  This result can be freeMem'd when done. */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr;
char **row;
char *ret = NULL;
struct dyString *dy = newDyString(128);

if (database != NULL)
    dyStringPrintf(dy, "select description from dbDb where name = '%s'", database);
else if (freeze != NULL)
    dyStringPrintf(dy, "select name from dbDb where description = '%s'", freeze);
else
    internalErr();
sr = sqlGetResult(conn, dy->string);
if ((row = sqlNextRow(sr)) != NULL)
    ret = cloneString(row[0]);
sqlFreeResult(&sr);
hDisconnectCentral(&conn);
freeDyString(&dy);
return ret;
}


char *hFreezeFromDb(char *database)
/* return the freeze for the database version. 
   For example: "hg6" returns "Dec 12, 2000". If database
   not recognized returns NULL */
{
return hFreezeDbConversion(database, NULL);
}

char *hDbFromFreeze(char *freeze)
/* Return database version from freeze name. */
{
return hFreezeDbConversion(NULL, freeze);
}

struct dbDb *hDbDbList()
/* Return list of databases that are actually online. 
 * The list includes the name, description, and where to
 * find the nib-formatted DNA files. Free this with dbDbFree. */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr;
char **row;
struct dbDb *dbList = NULL, *db;
struct hash *hash = sqlHashOfDatabases();

sr = sqlGetResult(conn, "select * from dbDb");
while ((row = sqlNextRow(sr)) != NULL)
    {
    db = dbDbLoad(row);
    if (hashLookup(hash, db->name))
        {
	slAddHead(&dbList, db);
	}
    else
        dbDbFree(&db);
    }
sqlFreeResult(&sr);
hashFree(&hash);
hDisconnectCentral(&conn);
slReverse(&dbList);
return dbList;
}

struct slName *hDbList()
/* List of all database versions that are online (database
 * names only).  See also hDbDbList. */
{
struct slName *nList = NULL, *n;
struct dbDb *dbList, *db;

dbList = hDbDbList();
for (db = dbList; db != NULL; db = db->next)
    {
    n = newSlName(db->name);
    slAddTail(&nList, n);
    }
dbDbFree(&dbList);
return nList;
}


static boolean fitFields(struct hash *hash, char *chrom, char *start, char *end,
	char retChrom[32], char retStart[32], char retEnd[32])
/* Return TRUE if chrom/start/end are in hash.  
 * If so copy them to retChrom, retStart, retEnd. 
 * Helper routine for findChromStartEndFields below. */
{
if (hashLookup(hash, chrom) && hashLookup(hash, start) && hashLookup(hash, end))
    {
    strcpy(retChrom, chrom);
    strcpy(retStart, start);
    strcpy(retEnd, end);
    return TRUE;
    }
else
    return FALSE;
}

boolean hFindFieldsAndBin(char *table, 
	char retChrom[32], char retStart[32], char retEnd[32],
	boolean *retBinned)
/* Given a table return the fields for selecting chromosome, start, 
 * and whether it's binned . */
{
char query[256];
struct sqlResult *sr;
char **row;
struct hash *hash = newHash(5);
struct sqlConnection *conn = hAllocConn();
boolean gotIt = TRUE, binned = FALSE;

/* Read table description into hash. */
sprintf(query, "describe %s", table);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameString(row[0], "bin"))
        binned = TRUE;
    hashAdd(hash, row[0], NULL);
    }
sqlFreeResult(&sr);

/* Look for bed-style names. */
if (fitFields(hash, "chrom", "chromStart", "chromEnd", retChrom, retStart, retEnd))
    ;
/* Look for psl-style names. */
else if (fitFields(hash, "tName", "tStart", "tEnd", retChrom, retStart, retEnd))
    ;
/* Look for gene prediction names. */
else if (fitFields(hash, "chrom", "txStart", "txEnd", retChrom, retStart, retEnd))
    ;
/* Look for repeatMasker names. */
else if (fitFields(hash, "genoName", "genoStart", "genoEnd", retChrom, retStart, retEnd))
    ;
else if (startsWith("chr", table) && endsWith(table, "_gl") && hashLookup(hash, "start") && hashLookup(hash, "end"))
    {
    strcpy(retChrom, "");
    strcpy(retStart, "start");
    strcpy(retEnd, "end");
    }
else
    gotIt = FALSE;
freeHash(&hash);
hFreeConn(&conn);
*retBinned = binned;
return gotIt;
}

boolean hFindChromStartEndFields(char *table, 
	char retChrom[32], char retStart[32], char retEnd[32])
/* Given a table return the fields for selecting chromosome, start, and end. */
{
int isBinned;
return hFindFieldsAndBin(table, retChrom, retStart, retEnd, &isBinned);
}


struct hTableInfo
/* Some info to track table. */
    {
    struct hTableInfo *next;	/* Next in list. */
    char *rootName;		/* Name without chrN_. */
    boolean isPos;		/* True if table is positional. */
    boolean isSplit;		/* True if table is split. */
    boolean hasBin;		/* True if table starts with field. */
    char chromField[32];		/* Name of chromosome field. */
    char startField[32];		/* Name of chromosome start field. */
    char endField[32];		/* Name of chromosome end field. */
    };

static struct hTableInfo *hFindTableInfo(char *chrom, char *rootName)
/* Find table information.  Return NULL if no table. */
{
static struct hash *hash;
struct hTableInfo *hti;
char fullName[64];
boolean isSplit = FALSE;

if (chrom == NULL)
    chrom = "chr22";
if (hash == NULL)
    hash = newHash(7);
if ((hti = hashFindVal(hash, rootName)) == NULL)
    {
    sprintf(fullName, "%s_%s", chrom, rootName);
    if (hTableExists(fullName))
	isSplit = TRUE;
    else
        {
	strcpy(fullName, rootName);
	if (!hTableExists(fullName))
	    return NULL;
	}
    AllocVar(hti);
    hashAddSaveName(hash, rootName, hti, &hti->rootName);
    hti->isSplit = isSplit;
    hti->isPos = hFindFieldsAndBin(fullName, hti->chromField,
        hti->startField, hti->endField, &hti->hasBin);
    }
return hti;
}


boolean hFindSplitTable(char *chrom, char *rootName, 
	char retTableBuf[64], boolean *hasBin)
/* Find name of table that may or may not be split across chromosomes. 
 * Return FALSE if table doesn't exist.  */
{
struct hTableInfo *hti = hFindTableInfo(chrom, rootName);
if (hti == NULL)
    return FALSE;
if (retTableBuf != NULL)
    {
    if (hti->isSplit)
	sprintf(retTableBuf, "%s_%s", chrom, rootName);
    else
	strcpy(retTableBuf, rootName);
    }
if (hasBin != NULL)
    *hasBin = hti->hasBin;
return TRUE;
}

boolean hIsPrivateHost()
/* Return TRUE if this is running on private web-server. */
{
static boolean gotIt = FALSE;
static boolean private = FALSE;
if (!gotIt)
    {
    char *t = getenv("HTTP_HOST");
    if (t != NULL && startsWith("genome-test", t))
        private = TRUE;
    gotIt = TRUE;
    }
return private;
}


int hOffsetPastBin(char *chrom, char *table)
/* Return offset into a row of table that skips past bin
 * field if any. */
{
struct hTableInfo *hti = hFindTableInfo(chrom, table);
if (hti == NULL)
    return 0;
return hti->hasBin;
}

/* Stuff to handle binning - which helps us restrict our
 * attention to the parts of database that contain info
 * about a particular window on a chromosome. This scheme
 * will work without modification for chromosome sizes up
 * to half a gigaBase.  The finest sized bin is 128k (1<<17).
 * The next coarsest is 8x as big (1<<3).  There's a hierarchy
 * of bins with the chromosome itself being the final bin.
 * Features are put in the finest bin they'll fit in. */

static int binOffsets[] = {512+64+8+1, 64+8+1, 8+1, 1, 0};
#define binFirstShift 17
#define binNextShift 3


int hBinLevels()
/* Return number of levels to bins. */
{
return ArraySize(binOffsets);
}

int hBinFirstShift()
/* Return amount to shift a number to get to finest bin. */
{
return binFirstShift;
}

int hBinNextShift()
/* Return amount to shift a numbe to get to next coarser bin. */
{
return binNextShift;
}

int hFindBin(int start, int end)
/* Given start,end in chromosome coordinates assign it
 * a bin.   There's a bin for each 128k segment, for each
 * 1M segment, for each 8M segment, for each 64M segment,
 * and for each chromosome (which is assumed to be less than
 * 512M.)  A range goes into the smallest bin it will fit in. */
{
int startBin = start, endBin = end-1, i;
startBin >>= binFirstShift;
endBin >>= binFirstShift;
for (i=0; i<ArraySize(binOffsets); ++i)
    {
    if (startBin == endBin)
        return binOffsets[i] + startBin;
    startBin >>= binNextShift;
    endBin >>= binNextShift;
    }
errAbort("start %d, end %d out of range in findBin (max is 512M)", start, end);
return 0;
}

void hAddBinToQuery(int start, int end, struct dyString *query)
/* Add clause that will restrict to relevant bins to query. */
{
int startBin = (start>>binFirstShift), endBin = ((end-1)>>binFirstShift);
int i;

dyStringAppend(query, "(");
for (i=0; i<ArraySize(binOffsets); ++i)
    {
    if (i != 0)
        dyStringAppend(query, " or ");
    if (startBin == endBin)
        dyStringPrintf(query, "bin=%u", startBin + binOffsets[i]);
    else
        dyStringPrintf(query, "bin>=%u and bin<=%u", 
		startBin + binOffsets[i], endBin + binOffsets[i]);
    startBin >>= 3;
    endBin >>= 3;
    }
dyStringAppend(query, ")");
dyStringAppend(query, " and ");
}

static struct sqlResult *hExtendedRangeQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	int start, int end, char *extraWhere, int *retRowOffset, boolean order)
/* Construct and make a query to tables that may be split and/or
 * binned. */
{
struct hTableInfo *hti = hFindTableInfo(chrom, rootTable);
struct sqlResult *sr = NULL;
struct dyString *query = newDyString(1024);
char fullTable[64], *table = NULL;
int rowOffset = 0;

if (hti == NULL)
    {
    warn("table %s doesn't exist", rootTable);
    }
else
    {
    dyStringAppend(query, "select * from ");
    if (hti->isSplit)
	{
	char fullTable[64];
	sprintf(fullTable, "%s_%s", chrom, rootTable);
	if (!sqlTableExists(conn, fullTable))
	     warn("%s doesn't exist", fullTable);
	else
	    {
	    table = fullTable;
	    dyStringPrintf(query, "%s where ", table);
	    }
	}
    else
        {
	table = rootTable;
	dyStringPrintf(query, "%s where %s='%s' and ", 
	    table, hti->chromField, chrom);
	}
    }
if (table != NULL)
    {
    if (hti->hasBin)
        {
	hAddBinToQuery(start, end, query);
	rowOffset = 1;
	}
    dyStringPrintf(query, "%s<%u and %s>%u", 
    	hti->startField, end, hti->endField, start);
    if (extraWhere)
        dyStringPrintf(query, " and %s", extraWhere);
    if (order)
        dyStringPrintf(query, " order by %s", hti->startField);
    sr = sqlGetResult(conn, query->string);
    }
freeDyString(&query);
*retRowOffset = rowOffset;
return sr;
}

struct sqlResult *hRangeQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	int start, int end, char *extraWhere, int *retRowOffset)
/* Construct and make a query to tables that may be split and/or
 * binned. */
{
return hExtendedRangeQuery(conn, rootTable, chrom, start, end, 
	extraWhere, retRowOffset, FALSE);
}

struct sqlResult *hOrderedRangeQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	int start, int end, char *extraWhere, int *retRowOffset)
/* Construct and make a query to tables that may be split and/or
 * binned. Forces return values to be sorted by chromosome start. */
{
return hExtendedRangeQuery(conn, rootTable, chrom, start, end, 
	extraWhere, retRowOffset, TRUE);
}


struct sqlResult *hChromQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	char *extraWhere, int *retRowOffset)
/* Construct and make a query across whole chromosome to tables 
 * that may be split and/or
 * binned. */
{
struct hTableInfo *hti = hFindTableInfo(chrom, rootTable);
struct sqlResult *sr = NULL;
struct dyString *query = newDyString(1024);
char fullTable[64], *table = NULL;
int rowOffset = 0;

if (hti == NULL)
    {
    warn("table %s doesn't exist", rootTable);
    }
else
    {
    rowOffset = hti->hasBin;
    if (hti->isSplit)
        dyStringPrintf(query, "select * from %s_%s", chrom, rootTable);
    else
        dyStringPrintf(query, "select * from %s where %s='%s'", rootTable,
		hti->chromField, chrom);
    sr = sqlGetResult(conn, query->string);
    }
freeDyString(&query);
*retRowOffset = rowOffset;
return sr;
}

boolean hTrackOnChrom(struct trackDb *tdb, char *chrom)
/* Return TRUE if track exists on this chromosome. */
{
boolean chromOk = TRUE;
char splitTable[64];
if (tdb->restrictCount > 0)
    chromOk =  (stringArrayIx(chrom, tdb->restrictList, tdb->restrictCount) >= 0);
return (chromOk && 
	hFindSplitTable(chrom, tdb->tableName, splitTable, NULL) &&
	!sameString(splitTable, "mrna")	 /* Long ago we reused this name badly. */
	);
}

struct trackDb *hTrackDb(char *chrom)
/* Load tracks associated with current chromosome (which may
 * be NULL */
{
struct sqlConnection *conn = hAllocConn();
struct trackDb *tdbList = NULL, *tdb;
boolean privateToo = hIsPrivateHost();
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, "select * from trackDb");
while ((row = sqlNextRow(sr)) != NULL)
    {
    boolean keeper = FALSE;
    tdb = trackDbLoad(row);
    if (!tdb->private || privateToo)
	{
	if (hTrackOnChrom(tdb, chrom))
	    {
	    slAddHead(&tdbList, tdb);
	    keeper = TRUE;
	    }
	}
    if (!keeper)
       trackDbFree(&tdb);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&tdbList);
return tdbList;
}


boolean hgParseChromRange(char *spec, char **retChromName, 
	int *retWinStart, int *retWinEnd)
/* Parse something of form chrom:start-end into pieces. */
{
char *chrom, *start, *end;
char buf[256];
int iStart, iEnd;

strncpy(buf, spec, 256);
chrom = buf;
start = strchr(chrom, ':');

if (start == NULL)
    {
    /* If just chromosome name cover all of it. */
    if ((chrom = hgOfficialChromName(chrom)) == NULL)
	return FALSE;
    else
       {
       chrom;
       iStart = 0;
       iEnd = hChromSize(chrom);
       }
    }
else 
    {
    *start++ = 0;
    end = strchr(start, '-');
    if (end == NULL)
	return FALSE;
    else
    *end++ = 0;
    chrom = trimSpaces(chrom);
    start = trimSpaces(start);
    end = trimSpaces(end);
    if (!isdigit(start[0]))
	return FALSE;
    if (!isdigit(end[0]))
	return FALSE;
    if ((chrom = hgOfficialChromName(chrom)) == NULL)
	return FALSE;
    iStart = atoi(start)-1;
    iEnd = atoi(end);
    }
if (retChromName != NULL)
    *retChromName = chrom;
if (retWinStart != NULL)
    *retWinStart = iStart;
if (retWinEnd != NULL)
    *retWinEnd = iEnd;
return TRUE;
}

boolean hgIsChromRange(char *spec)
/* Returns TRUE if spec is chrom:N-M for some human
 * chromosome chrom and some N and M. */
{
return hgParseChromRange(spec, NULL, NULL, NULL);
}

boolean hgParseContigRange(char *spec, char **retChromName, 
	int *retWinStart, int *retWinEnd)
/* Parse something of form contig:start-end into pieces. */
{
char *contig, *start,*end;
char buf[256];
char contigName[256];
int iStart, iEnd;
size_t colinspot;
char *chrom;
int contigstart,contigend;
int spot;

strncpy(buf, spec, 256);
contig = buf;
start = strchr(contig, ':');

if (startsWith("ctg",contig) == FALSE)
    return FALSE;

if (start == NULL)
    return FALSE;
else 
    {
    spot = strcspn(contig,":");
    strncpy(contigName,contig,spot);
    contigName[spot] = '\0';
    findContigPos(contigName,&chrom,&contigstart,&contigend);
    *start++ = 0;
    end = strchr(start, '-');
    if (end == NULL)
	return FALSE;
    else
    *end++ = 0;
    contig = trimSpaces(contig);
    start = trimSpaces(start);
    end = trimSpaces(end);
    if (!isdigit(start[0]))
	return FALSE;
    if (!isdigit(end[0]))
	return FALSE;
/*    if ((contig = hgOfficialChromName(contig)) == NULL)
      return FALSE; */
    iStart = atoi(start)-1;
    iEnd = atoi(end);
    }
if (retChromName != NULL)
    *retChromName = chrom;
if (retWinStart != NULL)
    *retWinStart = contigstart + iStart;
if (retWinEnd != NULL)
    *retWinEnd = contigstart + iEnd;
return TRUE; 
}

boolean hgIsContigRange(char *spec)
/* Returns TRUE if spec is chrom:N-M for some human
 * chromosome chrom and some N and M. */
{
return hgParseContigRange(spec, NULL, NULL, NULL);
}  

struct trackDb *hTrackInfo(struct sqlConnection *conn, char *trackName)
/* Look up track in database. */
{
char query[256];
struct sqlResult *sr;
char **row;
struct trackDb *tdb;

sprintf(query, "select * from trackDb where tableName = '%s'", trackName);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) == NULL)
    errAbort("Track %s not found", trackName);
tdb = trackDbLoad(row);
sqlFreeResult(&sr);
return tdb;
}

struct dbDb *hGetBlatIndexedDatabases()
/* Get list of databases for which there is a BLAT index. 
 * Dispose of this with dbDbFreeList. */
{
struct hash *hash=newHash(5);
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr;
char **row;
struct dbDb *dbList = NULL, *db;

/* Get hash of active blat servers. */
sr = sqlGetResult(conn, "select db from blatServers");
while ((row = sqlNextRow(sr)) != NULL)
    hashAdd(hash, row[0], NULL);
sqlFreeResult(&sr);

/* Scan through dbDb table, keeping ones that are indexed. */
sr = sqlGetResult(conn, "select * from dbDb");
while ((row = sqlNextRow(sr)) != NULL)
    {
    db = dbDbLoad(row);
    if (hashLookup(hash, db->name))
        {
	slAddHead(&dbList, db);
	}
    else
        dbDbFree(&db);
    }
sqlFreeResult(&sr);
hDisconnectCentral(&conn);
hashFree(&hash);
slReverse(&dbList);
return dbList;
}

boolean hIsBlatIndexedDatabase(char *db)
/* Return TRUE if have a BLAT server on sequence corresponding 
 * to give database. */
{
struct sqlConnection *conn = hConnectCentral();
boolean gotIx;
char query[256];

sprintf(query, "select name from dbDb where name = '%s'", db);
gotIx = sqlExists(conn, query);
hDisconnectCentral(&conn);
return gotIx;
}
