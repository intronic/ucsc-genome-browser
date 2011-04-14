/* hdb - human genome browser database. */
#include "common.h"
#include "obscure.h"
#include "hash.h"
#include "portable.h"
#include "linefile.h"
#include "binRange.h"
#include "jksql.h"
#include "dnautil.h"
#include "dnaseq.h"
#include "nib.h"
#include "hdb.h"
#include "hgRelate.h"
#include "fa.h"
#include "hgConfig.h"
#include "trackDb.h"
#include "hCommon.h"
#include "dbDb.h"
#include "blatServers.h"
#include "bed.h"
#include "defaultDb.h"
#include "scoredRef.h"
#include "maf.h"
#include "ra.h"
#include "liftOver.h"
#include "liftOverChain.h"
#include "grp.h"
#include "twoBit.h"
#include "ra.h"
#include "genbank.h"
#include "chromInfo.h"
#ifndef GBROWSE
#include "axtInfo.h"
#include "ctgPos.h"
#include "hubConnect.h"
#include "customTrack.h"
#include "hgFind.h"
#endif /* GBROWSE */
#include "hui.h"

static char const rcsid[] = "$Id: hdb.c,v 1.433 2010/05/21 16:39:57 angie Exp $";

#ifdef LOWELAB
#define DEFAULT_PROTEINS "proteins060115"
#define DEFAULT_GENOME "Pyrobaculum aerophilum"
#else
#define DEFAULT_PROTEINS "proteins"
#define DEFAULT_GENOME "Human"
#endif


static struct sqlConnCache *hdbCc = NULL;  /* cache for primary database connection */
static struct sqlConnCache *centralCc = NULL;
static char *centralDb = NULL;
static struct sqlConnCache *centralArchiveCc = NULL;
static char *centralArchiveDb = NULL;
static struct sqlConnCache *cartCc = NULL;  /* cache for cart; normally same as centralCc */
static char *cartDb = NULL;
static char *hdbTrackDb = NULL;

/* cached list of tables in databases.  This keeps a hash of databases to
 * hashes of track/table name to slName list of actual table names, which
 * might be split.  Since individual tables can be mapped to different
 * profiles, and this information is only available when processing trackDb,
 * another table is kept to determine if these need to be checked.
 */
static struct hash *tableList = NULL; // db to track to tables
static struct hash *tableListProfChecked = NULL;  // profile:db that have been check

static struct chromInfo *lookupChromInfo(char *db, char *chrom)
/* Query db.chromInfo for the first entry matching chrom. */
{
struct chromInfo *ci = NULL;
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr = NULL;
char **row = NULL;
char query[256];
safef(query, sizeof(query), "select * from chromInfo where chrom = '%s'",
      chrom);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    {
    ci = chromInfoLoad(row);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return ci;
}

struct chromInfo *hGetChromInfo(char *db, char *chrom)
/* Get chromInfo for named chromosome (case-insens.) from db.
 * Return NULL if no such chrom. */
/* Cache results, but build up the hash incrementally instead of in one slurp
 * from chromInfo because that takes a *long* time for scaffold-based dbs and
 * is usually not necessary. */
{
static struct hash *dbToInfo = NULL;
struct hash *infoHash = NULL;
struct hashEl *dHel = NULL;
struct chromInfo *ci = NULL;
char upcName[HDB_MAX_CHROM_STRING];
safef(upcName, sizeof(upcName), "%s", chrom);
touppers(upcName);

if (dbToInfo == NULL)
    dbToInfo = hashNew(0);
dHel = hashLookup(dbToInfo, db);
if (dHel == NULL)
    {
    infoHash = hashNew(0);
    hashAdd(dbToInfo, db, infoHash);
    }
else
    infoHash = (struct hash *)(dHel->val);
ci = (struct chromInfo *)hashFindVal(infoHash, upcName);
if (ci == NULL)
    {
    ci = lookupChromInfo(db, chrom);
    hashAdd(infoHash, upcName, ci);
    }
return ci;
}

static struct chromInfo *mustGetChromInfo(char *db, char *chrom)
/* Get chromInfo for named chrom from primary database or
 * die trying. */
{
struct chromInfo *ci = hGetChromInfo(db, chrom);
if (ci == NULL)
    errAbort("Couldn't find chromosome/scaffold %s in database", chrom);
return ci;
}

char *hgOfficialChromName(char *db, char *name)
/* Returns "canonical" name of chromosome or NULL
 * if not a chromosome. (Case-insensitive search w/sameWord()) */
{
struct chromInfo *ci = NULL;
char buf[HDB_MAX_CHROM_STRING];
strncpy(buf, name, HDB_MAX_CHROM_STRING);
buf[HDB_MAX_CHROM_STRING-1] = 0;
ci = hGetChromInfo(db, buf);
if (ci != NULL)
    return cloneString(ci->chrom);
else
    return NULL;
}

boolean hgIsOfficialChromName(char *db, char *name)
/* Determine if name is exact (case-sensitive) match with
 * a chromosome in the given assembly */
{
char *chrom;
return ((chrom = hgOfficialChromName(db, name)) != NULL &&
	sameString(name, chrom));
}


int hGetMinIndexLength(char *db)
/* get the minimum index size for the given database that won't smoosh
 * together chromNames. */
{
static boolean minLen = 0;
if (minLen <= 0)
    {
    struct slName *nameList = hAllChromNames(db);
    struct slName *name, *last;
    int len = 4;
    slSort(&nameList, slNameCmp);
    last = nameList;
    if (last != NULL)
        {
	for (name = nameList->next; name != NULL; name = name->next)
	    {
	    while (strncmp(name->name, last->name, len) == 0)
	        ++len;
	    last = name;
	    }
	}
    slFreeList(&nameList);
    minLen = len;
    }
return minLen;
}

static char *hTrackDbPath()
/* return the comma-separated list of the track database from the config
 * file. Freez when done */
{
if(hdbTrackDb == NULL)
    hdbTrackDb = cfgOptionEnv("HGDB_TRACKDB", "db.trackDb");
if(hdbTrackDb == NULL)
    errAbort("Please set the db.trackDb field in the hg.conf config file.");
return cloneString(hdbTrackDb);
}

struct slName *hTrackDbList()
/* Return list of trackDb tables from the config file.  Free list when done. */
{
char *name = hTrackDbPath();
struct slName *list = slNameListFromComma(name);
freez(&name);
return list;
}

void hSetTrackDbName(char *trackDbName)
/* Override the hg.conf db.trackDb setting. */
{
hdbTrackDb = cloneString(trackDbName);
}

struct slName *hTrackTablesOfType(struct sqlConnection *conn, char *type)
/* get list of tables in trackDb list with type */
{
struct slName *tdbList = hTrackDbList();
struct hash *nameHash = newHash(4);

for(; tdbList; tdbList = tdbList->next)
    {
    if (hTableExists(sqlGetDatabase(conn), tdbList->name))
        {
        char query[1024];
        safef(query, sizeof query,
            "select tableName from %s where type like '%s'", tdbList->name, type);

        struct sqlResult *sr = sqlGetResult(conn, query);
        char **row;
        while ((row = sqlNextRow(sr)) != NULL)
            hashStore(nameHash, row[0]);
        sqlFreeResult(&sr);
        }
    }

struct slName *list = NULL;
struct hashCookie cook = hashFirst(nameHash);
struct hashEl *hel;

while ((hel = hashNext(&cook)) != NULL)
    {
    if (sqlTableExists(conn, hel->name))
	{
	struct slName *name = newSlName(hel->name);
	slAddHead(&list, name);
	}
    }

freeHash(&nameHash);
return list;
}

boolean hArchiveDbExists(char *database)
/*
  Function to check if this is a valid db name in the dbDbArch table
  of archived databases.
*/
{
struct sqlConnection *conn = hConnectCentral();
char buf[128];
char query[256];
boolean res = FALSE;
safef(query, sizeof(query), "select name from dbDbArch where name = '%s'",
      database);
res = (sqlQuickQuery(conn, query, buf, sizeof(buf)) != NULL);
hDisconnectCentral(&conn);
return res;
}

boolean hDbExists(char *database)
/*
  Function to check if this is a valid db name
*/
{
static struct hash *dbsChecked = NULL;

if (dbsChecked)
    {
    struct hashEl *hel = hashLookup(dbsChecked, database);
    if (hel != NULL)
	return ptToInt(hel->val);
    }
else
    dbsChecked = newHash(0);

struct sqlConnection *conn = hConnectCentral();
char buf[128];
char query[256];
char *escaped = sqlEscapeString(database);
safef(query, sizeof(query), "select name from dbDb where name = '%s'", escaped);
freez(&escaped);
boolean res = (sqlQuickQuery(conn, query, buf, sizeof(buf)) != NULL);
if (res)
    {
    // this is done instead of sqlDatabaseExists() since it uses the cache,
    // which will recycle free connections for new databases
    struct sqlConnection *conn2 = hAllocConnMaybe(database);
    res = (conn2 != NULL);
    hFreeConn(&conn2);
    }
hDisconnectCentral(&conn);
hashAddInt(dbsChecked, database, res);
return res;
}

boolean hDbIsActive(char *database)
/* Function to check if this is a valid and active db name */
{
static struct hash *dbsChecked = NULL;

if (dbsChecked)
    {
    void *hashDb = hashFindVal(dbsChecked, database);
    if (hashDb)
	return(hashIntVal(dbsChecked, database));
    }
else
    dbsChecked = newHash(0);

struct sqlConnection *conn = hConnectCentral();
char buf[128];
char query[256];
boolean res = FALSE;
safef(query, sizeof(query),
      "select name from dbDb where name = '%s' and active = 1", database);
res = (sqlQuickQuery(conn, query, buf, sizeof(buf)) != NULL);
hDisconnectCentral(&conn);
hashAddInt(dbsChecked, database, res);
return res;
}

char *hDefaultDbForGenome(char *genome)
/* Purpose: Return the default database matching the Genome.
 * param Genome - The Genome for which we are trying to get the
 *    default database.
 * return - The default database name for this Genome
 * Free the returned database name. */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr = NULL;
char **row;
struct defaultDb *db = NULL;
char query [256];
char *result = NULL;

if (NULL == genome)
    {
    genome = cfgOptionDefault("defaultGenome", DEFAULT_GENOME);
    }

/* Get proper default from defaultDb table */
safef(query, sizeof(query), "select * from defaultDb where genome = '%s'",
      genome);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    {
    db = defaultDbLoad(row);
    }
if (db == NULL)
    {
    /* Can't find any of specified ones ?  Then use the first
     *	This is for the product browser which may have none of
     *	the usual UCSC genomes, but it needs to be able to function.
     */
    safef(query, sizeof(query), "%s", "select * from defaultDb");
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
	{
	db = defaultDbLoad(row);
	}
    if (db == NULL)
	errAbort("Can't find genome \"%s\" in central database table defaultDb.\n", genome);
    }

sqlFreeResult(&sr);
hDisconnectCentral(&conn);
AllocArray(result, strlen(db->name) + 1);
strcpy(result, db->name);
defaultDbFree(&db);
return result;
}

char *hDefaultGenomeForClade(char *clade)
/* Return highest relative priority genome for clade. */
{
struct sqlConnection *conn = hConnectCentral();
char query[512];
char *genome = NULL;
/* Get the top-priority genome *with an active database* so if genomeClade
 * gets pushed from hgwdev to hgwbeta/RR with genomes whose dbs haven't been
 * pushed yet, they'll be ignored. */
safef(query, sizeof(query),
      "select genomeClade.genome from genomeClade,dbDb "
      "where genomeClade.clade = '%s' and genomeClade.genome = dbDb.genome "
      "and dbDb.active = 1 "
      "order by genomeClade.priority limit 1",
      clade);
genome = sqlQuickString(conn, query);
hDisconnectCentral(&conn);
return genome;
}

char *hDbForSciName(char *sciName)
/* Get default db for scientific name */
{
char *db = NULL;
char query[256];
struct sqlConnection *centralConn = hConnectCentral();

safef(query, sizeof(query),
    "select defaultDb.name from dbDb,defaultDb "
    "where dbDb.scientificName='%s' "
    "and dbDb.name = defaultDb.name ", sciName);
db = sqlQuickString(centralConn, query);
hDisconnectCentral(&centralConn);

return db;
}

char *hDbForTaxon(struct sqlConnection *conn, int taxon)
/* Get database associated with NCBI taxon number if any. */
{
char *db = NULL;
char *binomial = NULL;
char query[256];

/* Figure out scientific name. */
if (taxon != 0)
    {
    safef(query, sizeof(query),
	"select binomial from uniProt.taxon where id=%d", taxon);
    binomial = sqlQuickString(conn, query);
    }
/* Get default database for that organism. */
if (binomial != NULL)
    {
    struct sqlConnection *centralConn = hConnectCentral();
    safef(query, sizeof(query),
        "select defaultDb.name from dbDb,defaultDb "
	"where dbDb.scientificName='%s' "
	"and dbDb.name not like 'zoo%%' "
	"and dbDb.name = defaultDb.name ", binomial);
    db = sqlQuickString(centralConn, query);
    hDisconnectCentral(&centralConn);
    }
freeMem(binomial);
return db;
}

char *hDefaultDb()
/* Return the default db if all else fails */
{
char *genome = cfgOptionDefault("defaultGenome", DEFAULT_GENOME);
return hDefaultDbForGenome(genome);
}

char *hDefaultChrom(char *db)
/* Return some sequence named in chromInfo from the given db, or NULL if db
 * has no chromInfo. */
{
static struct hash *hash = NULL;
struct hashEl *hel = NULL;

if (hash == NULL)
    hash = hashNew(0);
hel = hashStore(hash, db);
if (hel->val == NULL)
    {
    struct sqlConnection *conn = hAllocConn(db);
    if (sqlTableExists(conn, "chromInfo"))
	hel->val = sqlQuickString(conn, "select chrom from chromInfo limit 1");
    hFreeConn(&conn);
    }
return hel->val;
}

int hChromCount(char *db)
/* Return the number of chromosomes (scaffolds etc.) in the given db. */
{
struct sqlConnection *conn = hAllocConn(db);
int count = sqlQuickNum(conn, "select count(*) from chromInfo");
hFreeConn(&conn);
return count;
}

struct sqlConnection *hAllocConn(char *db)
/* Get free connection if possible. If not allocate a new one. */
{
if (hdbCc == NULL)
    hdbCc = sqlConnCacheNew();
return sqlConnCacheAlloc(hdbCc, db);
}

struct sqlConnection *hAllocConnMaybe(char *db)
/* Get free connection if possible. If not allocate a new one. Return
 * NULL if db doesn't exist or can't be connected to. */
{
if (hdbCc == NULL)
    hdbCc = sqlConnCacheNew();
return sqlConnCacheMayAlloc(hdbCc, db);
}

struct sqlConnection *hAllocConnProfile(char *profileName, char *db)
/* Get free connection, specifying a profile and/or a database. If none
 * is available, allocate a new one. */
{
if (hdbCc == NULL)
    hdbCc = sqlConnCacheNew();
return sqlConnCacheProfileAlloc(hdbCc, profileName, db);
}

struct sqlConnection *hAllocConnProfileMaybe(char *profileName, char *db)
/* Get free connection, specifying a profile and/or a database. If none is
 * available, allocate a new one.  Return NULL if database doesn't exist. */
{
if (hdbCc == NULL)
    hdbCc = sqlConnCacheNew();
return sqlConnCacheProfileAllocMaybe(hdbCc, profileName, db);
}

char *getTrackProfileName(struct trackDb *tdb)
/* get profile is associated with a track, return it, otherwise NULL */
{
// FIXME: locicalDb is an old name used by the cancer browser
char *p =  trackDbSetting(tdb, "dbProfile");
if (p == NULL)
    p = trackDbSetting(tdb, "logicalDb");
return p;
}

struct sqlConnection *hAllocConnTrack(char *db, struct trackDb *tdb)
/* Get free connection for accessing tables associated with the specified
 * track and database. If none is available, allocate a new one. */
{
return hAllocConnProfile(getTrackProfileName(tdb), db);
}

static struct sqlConnection *hAllocConnProfileTblInternal(char *db, char *spec, char **tableRet, bool abortOnError)
/* Allocate a connection to db, spec can either be in the form `table' or
 * `profile:table'.  If it contains profile, connect via that profile.  Also
 * returns pointer to table in spec string. */
{
char buf[512], *profile = NULL;
char *sep = strchr(spec, ':');
if (sep != NULL)
    {
    *tableRet = sep+1;
    safecpy(buf, sizeof(buf), spec);
    buf[(sep-spec)] = '\0';
    profile = buf;
    }
else
    *tableRet = spec;
if (abortOnError)
    return hAllocConnProfile(profile, db);
else
    return hAllocConnProfileMaybe(profile, db);
}

struct sqlConnection *hAllocConnProfileTbl(char *db, char *spec, char **tableRet)
/* Allocate a connection to db, spec can either be in the form `table' or
 * `profile:table'.  If it contains profile, connect via that profile.  Also
 * returns pointer to table in spec string. */
{
return hAllocConnProfileTblInternal(db, spec, tableRet, TRUE);
}

struct sqlConnection *hAllocConnProfileTblMaybe(char *db, char *spec, char **tableRet)
/* Allocate a connection to db, spec can either be in the form `table' or
 * `profile:table'.  If it contains profile, connect via that profile.  Also
 * returns pointer to table in spec string. Return NULL if database doesn't exist */
{
return hAllocConnProfileTblInternal(db, spec, tableRet, FALSE);
}

struct sqlConnection *hAllocConnDbTbl(char *spec, char **tableRet, char *defaultDb)
/* Allocate a connection to db and table, spec is in form `db.table'; if
 * defaultDb is not NULL, 'table' can also be used.  Also returns pointer to
 * table in spec string. */
{
char buf[512], *db = NULL;;
char *sep = strchr(spec, '.');
if (sep == NULL)
    {
    if (defaultDb == NULL)
        errAbort("no defaultDb, expected db.table, got %s", spec);
    db = defaultDb;
    *tableRet = spec;
    }
else
    {
    *tableRet = sep+1;
    safecpy(buf, sizeof(buf), spec);
    buf[(sep-spec)] = '\0';
    db = buf;
    }
return hAllocConn(db);
}

void hFreeConn(struct sqlConnection **pConn)
/* Put back connection for reuse. */
{
if (*pConn != NULL)  // don't use hdbCc if never allocated
    sqlConnCacheDealloc(hdbCc, pConn);
}

static void hCentralMkCache()
/* create the central database cache, trying to connect to the
 * database and failing over if needed */
{
char *centralProfile = "central";
centralDb = cfgOption2(centralProfile, "db");
centralCc = sqlConnCacheNewProfile(centralProfile);
sqlSetParanoid(TRUE);
struct sqlConnection *conn = sqlConnCacheMayAlloc(centralCc, centralDb);
if ((conn == NULL) || !cartTablesOk(conn))
    {
    fprintf(stderr, "ASH: hConnectCentral failed over to backupcentral!  "
            "pid=%ld\n", (long)getpid());
    sqlConnCacheDealloc(centralCc, &conn);
    sqlConnCacheFree(&centralCc);
    centralProfile = "backupcentral";
    centralDb = cfgOption2(centralProfile, "database");
    centralCc = sqlConnCacheNewProfile(centralProfile);
    conn = sqlConnCacheAlloc(centralCc, centralDb);
    if (!cartTablesOk(conn))
        errAbort("Cannot access cart tables in central (nor backupcentral) "
                 "database.  Please check central and backupcentral "
                 "settings in the hg.conf file and the databases they "
                 "specify.");
    }
/* now that a profile has been determined, make sure this database goes
 * through that profile */
sqlProfileAddDb(centralProfile, centralDb);
sqlConnCacheDealloc(centralCc, &conn);
sqlSetParanoid(FALSE);
}

struct sqlConnection *hConnectCentral()
/* Connect to central database where user info and other info
 * not specific to a particular genome lives.  Free this up
 * with hDisconnectCentral(). */
{
if (centralCc == NULL)
    hCentralMkCache();
return sqlConnCacheAlloc(centralCc, centralDb);
}

void hDisconnectCentral(struct sqlConnection **pConn)
/* Put back connection for reuse. */
{
if (*pConn != NULL)
    sqlConnCacheDealloc(centralCc, pConn);
}

static void hArchiveCentralMkCache()
/* create the archive central database cache, trying to connect to the
 * database and failing over if needed */
{
char *archiveProfile = "archivecentral";
centralArchiveDb = cfgOption2(archiveProfile, "db");
centralArchiveCc = sqlConnCacheNewProfile(archiveProfile);
struct sqlConnection *conn = sqlConnCacheMayAlloc(centralArchiveCc, centralArchiveDb);
if (conn == NULL)
    {
    sqlConnCacheDealloc(centralArchiveCc, &conn);
    sqlConnCacheFree(&centralArchiveCc);
    archiveProfile = "archivebackup";
    centralDb = cfgOption2(archiveProfile, "database");
    centralCc = sqlConnCacheNewProfile(archiveProfile);
    conn = sqlConnCacheAlloc(centralCc, centralDb);
    }
/* now that a profile has been determined, make sure this database goes
 * through that profile */
sqlProfileAddDb(archiveProfile, centralArchiveDb);
sqlConnCacheDealloc(centralCc, &conn);
}

struct sqlConnection *hConnectArchiveCentral()
/* Connect to central database for archives.
 * Free this up with hDisconnectCentralArchive(). */
{
if (centralArchiveCc == NULL)
    hArchiveCentralMkCache();
return sqlConnCacheAlloc(centralArchiveCc, centralArchiveDb);
}

void hDisconnectArchiveCentral(struct sqlConnection **pConn)
/* Put back connection for reuse. */
{
if (*pConn != NULL)
    sqlConnCacheDealloc(centralArchiveCc, pConn);
}

static void hCartMkCache()
/* Create the cart connection cache.  Defaults to the central connection
 * unless cart.db or cart.host are configured. */
{
if ((cfgOption("cart.db") != NULL) || (cfgOption("cart.host") != NULL)
    || (cfgOption("cart.user") != NULL) || (cfgOption("cart.password") != NULL))
    {
    /* use explict cart options */
    cartDb = cfgOption("cart.db");
    if ((cartDb == NULL) || (cfgOption("cart.host") == NULL)
        || (cfgOption("cart.user") == NULL) || (cfgOption("cart.password") == NULL))
        errAbort("Must specify either all or none of the cart options in the hg.conf file.");
    cartCc = sqlConnCacheNewProfile("cart");
    /* make sure this database goes through that profile */
    sqlProfileAddDb("cart", cartDb);
    }
else
    {
    /* use centralCc */
    if (centralCc == NULL)
        hCentralMkCache();
    cartCc = centralCc;
    cartDb = centralDb;
    }
}

struct sqlConnection *hConnectCart()
/* Connect to cart database.  Defaults to the central connection
 * unless cart.db or cart.host are configured. Free this
 * up with hDisconnectCart(). */
{
if (cartCc == NULL)
    hCartMkCache();
return sqlConnCacheAlloc(cartCc, cartDb);
}

void hDisconnectCart(struct sqlConnection **pConn)
/* Put back connection for reuse. */
{
if (*pConn != NULL)
    sqlConnCacheDealloc(cartCc, pConn);
}

boolean hCanHaveSplitTables(char *db)
/* Return TRUE if split tables are allowed in database. */
{
struct sqlConnection *conn = hAllocConn(db);
int count = sqlTableSizeIfExists(conn, "chromInfo");
hFreeConn(&conn);
return (count >= 0 && count <= HDB_MAX_SEQS_FOR_SPLIT);
}

static void tableListHashAdd(struct hash *dbTblHash, char *profile, char *db)
/* Add to a hash that maps a track/table name (unsplit) to an slName list
 * of actual table names (possibly split) -- we can compute this once and
 * cache it to save a lot of querying if we will check existence of
 * lots of tables. */
{
struct sqlConnection *conn = hAllocConnProfile(profile, db);
struct slName *allTables =  sqlListTables(conn);

if (!sameString(CUSTOM_TRASH,db) && hCanHaveSplitTables(db))
    {
    /* Consolidate split tables into one list per track: */
    struct slName *tbl = NULL, *nextTbl = NULL;
    for (tbl = allTables;  tbl != NULL;  tbl = nextTbl)
	{
	struct hashEl *tHel = NULL;
	char trackName[HDB_MAX_TABLE_STRING];
	char chrom[HDB_MAX_CHROM_STRING];
	nextTbl = tbl->next;
	tbl->next = NULL;
	hParseTableName(db, tbl->name, trackName, chrom);
	tHel = hashLookup(dbTblHash, trackName);
	if (tHel == NULL)
	    hashAdd(dbTblHash, trackName, tbl);
	else if (! sameString(tbl->name, trackName))
	    slAddHead(&(tHel->val), tbl);
	}
    }
else
    {
    /* Just hash all table names: */
    struct slName *tbl = NULL, *nextTbl = NULL;
    for (tbl = allTables;  tbl != NULL;  tbl = nextTbl)
	{
	nextTbl = tbl->next;
	tbl->next = NULL;
	hashAdd(dbTblHash, tbl->name, tbl);
	}
    }
hFreeConn(&conn);
}

static struct hash *tableListGetDbHash(char *db)
/* Retrieve (or build if necessary) the cached hash of split-consolidated
 * tables for db. */
{
struct hashEl *dbHel = NULL;
if (tableList == NULL)
    tableList = hashNew(0);
dbHel = hashLookup(tableList, db);
if (dbHel == NULL)
    {
    struct hash *dbTblHash = hashNew(14);
    dbHel = hashAdd(tableList, db, dbTblHash);

    // fill in from default profile for database
    tableListHashAdd(dbTblHash, NULL, db);
    }
return dbHel->val;
}

static void tableListProcessTblProfile(char *profile, char *db)
/* Process a profile associated with a table in the database.  This
 * adds the profile if its not already been done. */
{
assert(profile != NULL);
if (tableListProfChecked == NULL)
    tableListProfChecked = hashNew(0);
char key[512];
safef(key, sizeof(key), "%s:%s", profile, db);
if (hashLookup(tableListProfChecked, key) == NULL)
    {
    // first time for this profile/db
    tableListHashAdd(tableListGetDbHash(db), profile, db);
    }
}

boolean hTableExists(char *db, char *table)
/* Return TRUE if a table exists in db. */
{
struct hash *hash = tableListGetDbHash(db);
struct slName *tableNames = NULL, *tbl = NULL;
char trackName[HDB_MAX_TABLE_STRING];
char chrom[HDB_MAX_CHROM_STRING];
if (hashFindVal(hash, "chromInfo"))
    hParseTableName(db, table, trackName, chrom);
else
    safef(trackName, sizeof(trackName), "%s", table);
tableNames = (struct slName *)hashFindVal(hash, trackName);
for (tbl = tableNames;  tbl != NULL;  tbl = tbl->next)
    {
    if (sameString(table, tbl->name))
	return TRUE;
    }
return FALSE;
}

char *hTableForTrack(char *db, char *trackName)
/* Return a table for a track in db. Returns one of the split
 * tables, or main table if not split.
 * In addition, caches all the tracks and tables in db the first
 * time it is called (could be many thousand tables). */
{
struct hash *hash = tableListGetDbHash(db);
struct slName *tableNames = NULL;
tableNames = (struct slName *)hashFindVal(hash, trackName);
if (tableNames != NULL)
    return tableNames->name;
return NULL;
}

boolean hTableOrSplitExists(char *db, char *track)
/* Return TRUE if track table (or split table) exists in db. */
{
if (!hDbExists(db))
    return FALSE;
struct hash *hash = tableListGetDbHash(db);
return (hashLookup(hash, track) != NULL);
}

void hParseTableName(char *db, char *table, char trackName[HDB_MAX_TABLE_STRING],
		     char chrom[HDB_MAX_CHROM_STRING])
/* Parse an actual table name like "chr17_random_blastzWhatever" into
 * the track name (blastzWhatever) and chrom (chr17_random). */
/* Note: for the sake of speed, this does not consult chromInfo
 * because that would be extremely slow for scaffold-based dbs.
 * Instead this makes some assumptions about chromosome names and split
 * table names in databases that support split tables, and just parses text.
 * When chromosome/table name conventions change, this will need an update! */
{
/*
 * WARNING: The behavior of the below code to handle split tables is very
 * fragile.  Other parts of the code rely on truncating the `chromosome' name
 * parsed out of the table name instead of generating an error.  It goes
 * something like this:
 *
 * Code somewhere wants to check for a split table existing.  If the table is not split,
 * but contains an underscore, such as
 *     encodeUcsdChipHeLaH3H4dmH3K4_p0
 * results it in calling this function with the default chromosome name pre-pended:
 *     chr9_encodeUcsdChipHeLaH3H4dmH3K4_p0
 * this is then split at the last underscore and the chrom name truncated:
 *    chrom: chr9_encodeUcsdC
 *    table: p0
 * from here, it most likely decides that the table doesn't exist an life is
 * fine.  Attempts to generate an error when the chrom name part of the table
 * name is truncated causes breakage of various parts of the genome browser.
 *
 * The rules as to when a table name can have an underscore in it that is not
 * part of a split table name are very complex.
 */

/* It might not be a split table; provide defaults: */
safecpy(trackName, HDB_MAX_TABLE_STRING, table);
safecpy(chrom, HDB_MAX_CHROM_STRING, hDefaultChrom(db));
if (startsWith("chr", table) || startsWith("Group", table))
    {
    char *ptr = strrchr(table, '_');
    if (ptr != NULL)
        {
        int chromLen = min(HDB_MAX_CHROM_STRING-1, (ptr - table));
        strncpy(chrom, table, chromLen);
        chrom[chromLen] = '\0';
        safecpy(trackName, HDB_MAX_TABLE_STRING, ptr+1);
	}
    }
}

int hChromSize(char *db, char *chromName)
/* Return size of chromosome. */
{
struct chromInfo *ci = mustGetChromInfo(db, chromName);
return ci->size;
}

void hNibForChrom(char *db, char *chromName, char retNibName[HDB_MAX_PATH_STRING])
/* Get .nib file associated with chromosome. */
{
if (hDbIsActive(db))
    {
    struct chromInfo *ci = mustGetChromInfo(db, chromName);
    safef(retNibName, HDB_MAX_PATH_STRING, "%s", ci->fileName);
    }
else
    {
    char *nibPath = hDbDbNibPath(db);

    if (nibPath == NULL)
	errAbort("nibPath is NULL for database '%s'", db);

    safef(retNibName, HDB_MAX_PATH_STRING, "%s/%s.2bit",
	nibPath, db);
    if (!fileExists(retNibName))
	{
	/* if 2bit file isn't there, try up one directory */
	safef(retNibName, HDB_MAX_PATH_STRING, "%s/../%s.2bit",
	    hDbDbNibPath(db), db);
	if (!fileExists(retNibName))
	    {
	    /* still no 2bit, let's just try to find a nib */
	    safef(retNibName, HDB_MAX_PATH_STRING, "%s/%s.nib",
		hDbDbNibPath(db), chromName);
	    }
	}
    }
}

static struct dnaSeq *fetchTwoBitSeq(char *fileName, char *seqName, int start, int end)
/* fetch a sequence from a 2bit, caching open of the file */
{
static struct twoBitFile *tbf = NULL;  // cache of open file
if ((tbf == NULL) || !sameString(fileName, tbf->fileName))
    {
    twoBitClose(&tbf);
    tbf = twoBitOpen(fileName);
    }
struct dnaSeq *seq = twoBitReadSeqFrag(tbf, seqName, start, end);
return seq;
}

struct dnaSeq *hFetchSeqMixed(char *fileName, char *seqName, int start, int end)
/* Fetch mixed case sequence. */
{
if (twoBitIsFile(fileName))
    return fetchTwoBitSeq(fileName, seqName, start, end);
else
    return nibLoadPartMasked(NIB_MASK_MIXED, fileName, start, end-start);
}

struct dnaSeq *hFetchSeq(char *fileName, char *seqName, int start, int end)
/* Fetch sequence from file.  If it is a .2bit file then fetch the named sequence.
   If it is .nib then just ignore seqName. */
{
if (twoBitIsFile(fileName))
    {
    struct dnaSeq *seq = fetchTwoBitSeq(fileName, seqName, start, end);
    tolowers(seq->dna);
    return seq;
    }
return nibLoadPart(fileName, start, end-start);
}

struct dnaSeq *hChromSeqMixed(char *db, char *chrom, int start, int end)
/* Return mixed case (repeats in lower case) DNA from chromosome. */
{
char fileName[HDB_MAX_PATH_STRING];
hNibForChrom(db, chrom, fileName);
return hFetchSeqMixed(fileName, chrom, start, end);
}

struct dnaSeq *hChromSeq(char *db, char *chrom, int start, int end)
/* Return lower case DNA from chromosome. */
{
char fileName[HDB_MAX_PATH_STRING];
hNibForChrom(db, chrom, fileName);
return hFetchSeq(fileName, chrom, start, end);
}

struct dnaSeq *hSeqForBed(char *db, struct bed *bed)
/* Get the sequence associated with a particular bed concatenated together. */
{
char fileName[HDB_MAX_PATH_STRING];
struct dnaSeq *block = NULL;
struct dnaSeq *bedSeq = NULL;
int i = 0 ;
assert(bed);
/* Handle very simple beds and beds with blocks. */
if(bed->blockCount == 0)
    {
    bedSeq = hChromSeq(db, bed->chrom, bed->chromStart, bed->chromEnd);
    freez(&bedSeq->name);
    bedSeq->name = cloneString(bed->name);
    }
else
    {
    int offSet = bed->chromStart;
    struct dyString *currentSeq = newDyString(2048);
    hNibForChrom(db, bed->chrom, fileName);
    for(i=0; i<bed->blockCount; i++)
	{
	block = hFetchSeq(fileName, bed->chrom,
			  offSet+bed->chromStarts[i], offSet+bed->chromStarts[i]+bed->blockSizes[i]);
	dyStringAppendN(currentSeq, block->dna, block->size);
	dnaSeqFree(&block);
	}
    AllocVar(bedSeq);
    bedSeq->name = cloneString(bed->name);
    bedSeq->dna = cloneString(currentSeq->string);
    bedSeq->size = strlen(bedSeq->dna);
    dyStringFree(&currentSeq);
    }
if(bed->strand[0] == '-')
    reverseComplement(bedSeq->dna, bedSeq->size);
return bedSeq;
}

boolean hChromBandConn(struct sqlConnection *conn,
	char *chrom, int pos, char retBand[HDB_MAX_BAND_STRING])
/* Return text string that says what band pos is on.
 * Return FALSE if not on any band, or table missing. */
{
char query[256];
char buf[HDB_MAX_BAND_STRING];
char *s;
boolean ok = TRUE;
boolean isDmel = startsWith("dm", sqlGetDatabase(conn));

safef(query, sizeof(query),
	"select name from cytoBand where chrom = '%s' and chromStart <= %d and chromEnd > %d",
	chrom, pos, pos);
buf[0] = 0;
s = sqlQuickQuery(conn, query, buf, sizeof(buf));
if (s == NULL)
   {
   s = "";
   ok = FALSE;
   }
safef(retBand, HDB_MAX_BAND_STRING, "%s%s",
      (isDmel ? "" : skipChr(chrom)), buf);
return ok;
}

boolean hChromBand(char *db, char *chrom, int pos, char retBand[HDB_MAX_BAND_STRING])
/* Return text string that says what band pos is on.
 * Return FALSE if not on any band, or table missing. */
{
if (!hTableExists(db, "cytoBand"))
    return FALSE;
else
    {
    struct sqlConnection *conn = hAllocConn(db);
    boolean ok = hChromBandConn(conn, chrom, pos, retBand);
    hFreeConn(&conn);
    return ok;
    }
}

boolean hScaffoldPos(char *db, char *chrom, int start, int end,
                     char **retScaffold, int *retStart, int *retEnd)
/* Return the scaffold, and start end coordinates on a scaffold, for
 * a chromosome range.  If the range extends past end of a scaffold,
 * it is truncated to the scaffold end.
 * Return FALSE if unable to convert */
{
int ret = FALSE;
char table[HDB_MAX_TABLE_STRING];
safef(table, sizeof(table), "%s_gold", chrom);
if (!hTableExists(db, table))
    return FALSE;
else
    {
    char query[256];
    struct sqlConnection *conn = hAllocConn(db);
    struct sqlResult *sr;
    char **row;
    int chromStart, chromEnd;
    int scaffoldStart, scaffoldEnd;
    safef(query, sizeof(query),
	"SELECT frag, chromStart, chromEnd FROM %s WHERE chromStart <= %d ORDER BY chromStart DESC LIMIT 1", table, start);
    sr = sqlGetResult(conn, query);
    if (sr != NULL)
        {
        row = sqlNextRow(sr);
        if (row != NULL)
            {
            chromStart = sqlUnsigned(row[1]);
            chromEnd = sqlUnsigned(row[2]);

            scaffoldStart = start - chromStart;
            if (retStart != NULL)
                *retStart = scaffoldStart;

            if (end > chromEnd)
                end = chromEnd;
            scaffoldEnd = end - chromStart;
            if (retEnd != NULL)
                *retEnd = scaffoldEnd;

            if (scaffoldStart < scaffoldEnd)
                {
                /* check for "reversed" endpoints -- e.g.
                 * if printing scaffold itself */
                if (retScaffold != NULL)
                    *retScaffold = cloneString(row[0]);
                 ret = TRUE;
                }
            }
        sqlFreeResult(&sr);
        }
    hFreeConn(&conn);
    return ret;
    }
}

struct dnaSeq *hDnaFromSeq(char *db, char *seqName, int start, int end, enum dnaCase dnaCase)
/* Fetch DNA */
{
struct dnaSeq *seq;
if (dnaCase == dnaMixed)
    seq = hChromSeqMixed(db, seqName, start, end);
else
    {
    seq = hChromSeq(db, seqName, start, end);
	if (dnaCase == dnaUpper)
	  touppers(seq->dna);
	}
return seq;
}

struct dnaSeq *hLoadChrom(char *db, char *chromName)
/* Fetch entire chromosome into memory. */
{
int size = hChromSize(db, chromName);
return hDnaFromSeq(db, chromName, 0, size, dnaLower);
}

struct slName *hAllChromNames(char *db)
/* Get list of all chromosome names in database. */
{
struct slName *list = NULL;
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, "select chrom from chromInfo");
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct slName *el = slNameNew(row[0]);
    slAddHead(&list, el);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return list;
}

char *hExtFileNameC(struct sqlConnection *conn, char *extFileTable, unsigned extFileId)
/* Get external file name from table and ID.  Typically
 * extFile table will be 'extFile' or 'gbExtFile'
 * Abort if the id is not in the table or if the file
 * fails size check.  Please freeMem the result when you
 * are done with it. (requires conn passed in) */
{
char query[256];
struct sqlResult *sr;
char **row;
long long dbSize, diskSize;
char *path;

safef(query, sizeof(query),
	"select path,size from %s where id = %u", extFileTable, extFileId);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) == NULL)
    {
    errAbort("Database inconsistency table '%s.%s' no ext file with id %u",
             sqlGetDatabase(conn), extFileTable, extFileId);
    }
path = cloneString(row[0]);
dbSize = sqlLongLong(row[1]);
diskSize = fileSize(path);
if (dbSize != diskSize)
    {
    errAbort("External file %s cannot be opened or has wrong size.  Old size %lld, new size %lld, error %s",
   	path, dbSize, diskSize, strerror(errno));
    }
sqlFreeResult(&sr);
return path;
}


char *hExtFileName(char *db, char *extFileTable, unsigned extFileId)
/* Get external file name from table and ID.  Typically
 * extFile table will be 'extFile' or 'gbExtFile'
 * Abort if the id is not in the table or if the file
 * fails size check.  Please freeMem the result when you
 * are done with it. */
{
struct sqlConnection *conn = hAllocConn(db);
char *path=hExtFileNameC(conn,extFileTable,extFileId);
hFreeConn(&conn);
return path;
}





/* Constants for selecting seq/extFile or gbSeq/gbExtFile */
#define SEQ_TBL_SET   1
#define GBSEQ_TBL_SET 2

struct largeSeqFile
/* Manages our large external sequence files.  Typically there will
 * be around four of these.  This basically caches the file handle
 * so don't have to keep opening and closing them. */
{
    struct largeSeqFile *next;  /* Next in list. */
    char *path;                 /* Path name for file. */
    char *extTable;             /* external file table */
    char *db;                   /* database this is associated with */
    HGID id;                    /* Id in extFile table. */
    int fd;                     /* File handle. */
    };

static struct largeSeqFile *largeFileList;  /* List of open large files. */


static struct largeSeqFile *largeFileHandle(struct sqlConnection *conn, HGID extId, char *extTable)
/* Return handle to large external file. */
{
struct largeSeqFile *lsf;
char *db = sqlGetDatabase(conn);

/* Search for it on existing list and return it if found. */
for (lsf = largeFileList; lsf != NULL; lsf = lsf->next)
    {
    if ((lsf->id == extId) && sameString(lsf->db, db) && sameString(lsf->extTable, extTable))
        return lsf;
    }

/* Open file and put it on list. */
    {
    struct largeSeqFile *lsf;
    AllocVar(lsf);
    lsf->path = hExtFileNameC(conn, extTable, extId);
    lsf->extTable = cloneString(extTable);
    lsf->db = cloneString(db);
    lsf->id = extId;
    if ((lsf->fd = open(lsf->path, O_RDONLY)) < 0)
        errAbort("Couldn't open external file %s", lsf->path);
    slAddHead(&largeFileList, lsf);
    return lsf;
    }
}

static void *readOpenFileSection(int fd, off_t offset, size_t size, char *fileName, char *acc)
/* Allocate a buffer big enough to hold a section of a file,
 * and read that section into it. */
{
void *buf;
buf = needMem(size+1);
if (lseek(fd, offset, SEEK_SET) < 0)
    errnoAbort("Couldn't read %s: error seeking to %lld in %s", acc, (long long)offset, fileName);
if (read(fd, buf, size) < size)
    errnoAbort("Couldn't read %s: error reading %lld bytes at %lld in %s", acc, (long long)size, (long long)offset, fileName);
return buf;
}

static char *dbTblParse(char *defaultDb, char *tbl, char **tblRet,
                        char *buf, int bufSize)
/* Check if tbl contains db, if so, then split it and the database and table,
 * returning the database. If a db isn't encoded, return defaultDb and tbl unchanged, or abort if
 * defaultDb is NULL.  buf must be big enough to hold the database. */
{
char *dot = strchr(tbl, '.');
if (dot == NULL)
    {
    if (defaultDb == NULL)
        errAbort("no default database and no database specified with table %s", tbl);
    *tblRet = tbl;
    return defaultDb;
    }
else
    {
    safencpy(buf, bufSize, tbl, (dot-tbl));
    *tblRet = dot+1;
    return buf;
    }
}

static bioSeq *seqGet(char *db, char *acc, boolean isDna, char *seqTbl, char *extFileTbl)
/* Return sequence from the specified seq and extFile tables.   The
 * seqTbl/extFileTbl arguments may include the database, in which case they
 * override what is in db (which could even be NULL). NULL if not
 * found. */
{
/* look up sequence */
char dbBuf[64];
char *seqDb = dbTblParse(db, seqTbl, &seqTbl, dbBuf, sizeof(dbBuf));
struct sqlConnection *conn = hAllocConn(seqDb);
char query[256];
safef(query, sizeof(query),
      "select extFile,file_offset,file_size from %s where acc = '%s'",
      seqTbl, acc);
struct sqlResult *sr = sqlMustGetResult(conn, query);
char **row = sqlNextRow(sr);
if (row == NULL)
    {
    sqlFreeResult(&sr);
    hFreeConn(&conn);
    return NULL;
    }
HGID extId = sqlUnsigned(row[0]);
off_t offset = sqlLongLong(row[1]);
size_t size = sqlUnsigned(row[2]);
sqlFreeResult(&sr);
hFreeConn(&conn);

/* look up extFile */
char *extDb = dbTblParse(db, extFileTbl, &extFileTbl, dbBuf, sizeof(dbBuf));
conn = hAllocConn(extDb);
struct largeSeqFile *lsf = largeFileHandle(conn, extId, extFileTbl);
char *buf = readOpenFileSection(lsf->fd, offset, size, lsf->path, acc);
hFreeConn(&conn);
return faSeqFromMemText(buf, isDna);
}

static bioSeq *seqMustGet(char *db, char *acc, boolean isDna, char *seqTbl, char *extFileTbl)
/* Return sequence from the specified seq and extFile tables.  The
 * seqTbl/extFileTbl arguments may include the database, in which case they
 * override what is in db (which could even be NULL).  Return Abort if not
 * found. */
{
bioSeq *seq = seqGet(db, acc, isDna, seqTbl, extFileTbl);
if (seq == NULL)
    errAbort("can't find \"%s\" in seq table %s.%s", acc, db, seqTbl);
return seq;
}

struct dnaSeq *hDnaSeqGet(char *db, char *acc, char *seqTbl, char *extFileTbl)
/* Get a cDNA or DNA sequence from the specified seq and extFile tables.  The
 * seqTbl/extFileTbl arguments may include the database, in which case they
 * override what is in db (which could even be NULL). Return NULL if not
 * found. */
{
return seqGet(db, acc, TRUE, seqTbl, extFileTbl);
}

struct dnaSeq *hDnaSeqMustGet(char *db, char *acc, char *seqTbl, char *extFileTbl)
/* Get a cDNA or DNA sequence from the specified seq and extFile tables.  The
 * seqTbl/extFileTbl arguments may include the database, in which case they
 * override what is in db (which could even be NULL).
 * Abort if not found. */
{
return seqMustGet(db, acc, TRUE, seqTbl, extFileTbl);
}

aaSeq *hPepSeqGet(char *db, char *acc, char *seqTbl, char *extFileTbl)
/* Get a peptide sequence from the specified seq and extFile tables.   The
 * seqTbl/extFileTbl arguments may include the database, in which case they
 * override what is in db (which could even be NULL). Return
 * NULL if not found. */
{
return seqGet(db, acc, FALSE, seqTbl, extFileTbl);
}

aaSeq *hPepSeqMustGet(char *db, char *acc, char *seqTbl, char *extFileTbl)
/* Get a peptide sequence from the specified seq and extFile tables.  The
 * seqTbl/extFileTbl arguments may include the database, in which case they
 * override what is in db (which could even be NULL). Abort if
 * not found. */
{
return seqMustGet(db, acc, FALSE, seqTbl, extFileTbl);
}

static boolean querySeqInfo(struct sqlConnection *conn, char *acc, char *seqTbl, char *extFileFld,
                            HGID *retId, HGID *retExtId, size_t *retSize, off_t *retOffset)
/* lookup information in the seq or gbSeq table */
{
boolean gotIt = FALSE;
if (hTableExists(sqlGetDatabase(conn), seqTbl))
    {
    char query[256];
    safef(query, sizeof(query),
       "select id, %s, file_offset, file_size from %s where acc = '%s'",
          extFileFld, seqTbl, acc);
    struct sqlResult *sr = sqlMustGetResult(conn, query);
    char **row = sqlNextRow(sr);
    if (row != NULL)
        {
        if (retId != NULL)
            *retId = sqlUnsigned(row[0]);
        if (retExtId != NULL)
            *retExtId = sqlUnsigned(row[1]);
        if (retOffset != NULL)
            *retOffset = sqlLongLong(row[2]);
        if (retSize != NULL)
            *retSize = sqlUnsigned(row[3]);
        gotIt = TRUE;
        }
    sqlFreeResult(&sr);
    }
return gotIt;
}

static char* getSeqAndId(struct sqlConnection *conn, char *acc, HGID *retId)
/* Return sequence as a fasta record in a string and it's database ID, or
 * NULL if not found. Optionally get genbank modification date. */
{
HGID extId;
size_t size;
off_t offset;
char *extTable = NULL;
/* try gbExtFile table first, as it tends to be  more performance sensitive */
if (querySeqInfo(conn, acc, "gbSeq", "gbExtFile", retId, &extId, &size, &offset))
    extTable = "gbExtFile";
else if (querySeqInfo(conn, acc, "seq", "extFile", retId, &extId, &size, &offset))
    extTable = "extFile";
else
    return NULL;
struct largeSeqFile *lsf = largeFileHandle(conn, extId, extTable);
char *buf = readOpenFileSection(lsf->fd, offset, size, lsf->path, acc);
return buf;
}

static char* mustGetSeqAndId(struct sqlConnection *conn, char *acc,
                             HGID *retId)
/* Return sequence as a fasta record in a string and it's database ID,
 * abort if not found */
{
char *buf= getSeqAndId(conn, acc, retId);
if (buf == NULL)
    errAbort("No sequence for %s in seq or gbSeq tables", acc);
return buf;
}

char* hGetSeqAndId(struct sqlConnection *conn, char *acc, HGID *retId)
/* Return sequence as a fasta record in a string and it's database ID, or
 * NULL if not found. */
{
return getSeqAndId(conn, acc, retId);
}

int hRnaSeqAndIdx(char *acc, struct dnaSeq **retSeq, HGID *retId, struct sqlConnection *conn)
/* Return sequence for RNA and it's database ID. Return -1 if not found. */
{
char *buf = getSeqAndId(conn, acc, retId);
if (buf == NULL)
    return -1;
*retSeq = faFromMemText(buf);
return 0;
}

void hRnaSeqAndId(char *db, char *acc, struct dnaSeq **retSeq, HGID *retId)
/* Return sequence for RNA and it's database ID. */
{
struct sqlConnection *conn = hAllocConn(db);
char *buf = mustGetSeqAndId(conn, acc, retId);
*retSeq = faFromMemText(buf);
hFreeConn(&conn);
}

struct dnaSeq *hExtSeq(char *db, char *acc)
/* Return sequence for external sequence. */
{
struct dnaSeq *seq;
HGID id;
hRnaSeqAndId(db, acc, &seq, &id);
return seq;
}

struct dnaSeq *hExtSeqPart(char *db, char *acc, int start, int end)
/* Return part of external sequence. */
{
struct dnaSeq *seq = hExtSeq(db, acc);
//FIXME: freeing this won't free up the entire DNA seq
if (end > seq->size)
    errAbort("Can't extract partial seq: acc=%s, end=%d, size=%d",
                acc, end, seq->size);
return newDnaSeq(seq->dna + start, end - start, acc);
}

struct dnaSeq *hRnaSeq(char *db, char *acc)
/* Return sequence for RNA. */
{
return hExtSeq(db, acc);
}

aaSeq *hPepSeq(char *db, char *acc)
/* Return sequence for a peptide. */
{
struct sqlConnection *conn = hAllocConn(db);
char *buf = mustGetSeqAndId(conn, acc, NULL);
hFreeConn(&conn);
return faSeqFromMemText(buf, FALSE);
}

static boolean checkIfInTable(struct sqlConnection *conn, char *acc,
                              char *column, char *table)
/* check if a a sequences exists in a table */
{
boolean inTable = FALSE;
char query[256];
struct sqlResult *sr;
char **row;
safef(query, sizeof(query), "select 0 from %s where %s = \"%s\"",
      table, column, acc);
sr = sqlGetResult(conn, query);
inTable = ((row = sqlNextRow(sr)) != NULL);
sqlFreeResult(&sr);
return inTable;
}

boolean hGenBankHaveSeq(char *db, char *acc, char *compatTable)
/* Get a GenBank or RefSeq mRNA or EST sequence or NULL if it doesn't exist.
 * This handles compatibility between pre-incremental genbank databases where
 * refSeq sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */
{
struct sqlConnection *conn = hAllocConn(db);
boolean haveSeq = FALSE;

/* Check compatTable if we have it, otherwise check seq and gbSeq */
if ((compatTable != NULL) && hTableExists(db, compatTable))
    {
    haveSeq = checkIfInTable(conn, acc, "name", compatTable);
    }
else
    {
    if (hTableExists(db, "gbSeq"))
        haveSeq = checkIfInTable(conn, acc, "acc", "gbSeq");
    if ((!haveSeq) && hTableExists(db, "seq"))
        haveSeq = checkIfInTable(conn, acc, "acc", "seq");
    }

hFreeConn(&conn);
return haveSeq;
}

static struct dnaSeq *loadSeqFromTable(struct sqlConnection *conn,
                                       char *acc, char *table)
/* load a sequence from table. */
{
struct dnaSeq *seq = NULL;
struct sqlResult *sr;
char **row;
char query[256];

safef(query, sizeof(query), "select name,seq from %s where name = '%s'",
      table, acc);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    seq = newDnaSeq(cloneString(row[1]), strlen(row[1]), row[0]);

sqlFreeResult(&sr);
return seq;
}



struct dnaSeq *hGenBankGetMrnaC(struct sqlConnection *conn, char *acc, char *compatTable)
/* Get a GenBank or RefSeq mRNA or EST sequence or NULL if it doesn't exist.
 * This handles compatibility between pre-incremental genbank databases where
 * refSeq sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */
{
struct dnaSeq *seq = NULL;

/* If we have the compat table, get the sequence from there, otherwise from
 * seq or gbSeq. */
if ((compatTable != NULL) && hTableExists(sqlGetDatabase(conn), compatTable))
    {
    seq = loadSeqFromTable(conn, acc, compatTable);
    }
else
    {
    char *buf = getSeqAndId(conn, acc, NULL);
    if (buf != NULL)
        seq = faFromMemText(buf);
    }

return seq;
}


struct dnaSeq *hGenBankGetMrna(char *db, char *acc, char *compatTable)
/* Get a GenBank or RefSeq mRNA or EST sequence or NULL if it doesn't exist.
 * This handles compatibility between pre-incremental genbank databases where
 * refSeq sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */
{
struct sqlConnection *conn = hAllocConn(db);
struct dnaSeq *seq = hGenBankGetMrnaC(conn, acc, compatTable);
hFreeConn(&conn);
return seq;
}



aaSeq *hGenBankGetPepC(struct sqlConnection *conn, char *acc, char *compatTable)
/* Get a RefSeq peptide sequence or NULL if it doesn't exist.  This handles
 * compatibility between pre-incremental genbank databases where refSeq
 * sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */
{
aaSeq *seq = NULL;


/* If we have the compat table, get the sequence from there, otherwise from
 * gbSeq. */
if ((compatTable != NULL) && hTableExists(sqlGetDatabase(conn), compatTable))
    {
    seq = loadSeqFromTable(conn, acc, compatTable);
    }
else
    {
    char *buf = getSeqAndId(conn, acc, NULL);
    if (buf != NULL)
        seq = faSeqFromMemText(buf, FALSE);
    }
return seq;
}


aaSeq *hGenBankGetPep(char *db, char *acc, char *compatTable)
/* Get a RefSeq peptide sequence or NULL if it doesn't exist.  This handles
 * compatibility between pre-incremental genbank databases where refSeq
 * sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */
{
struct sqlConnection *conn = hAllocConn(db);
aaSeq *seq = hGenBankGetPepC(conn, acc, compatTable);
hFreeConn(&conn);
return seq;
}

char *hGenBankGetDesc(char *db, char *acc, boolean native)
/* Get a description for a genbank or refseq mRNA. If native is TRUE, an
 * attempt is made to get a more compact description that doesn't include
 * species name. Acc may optionally include the version.  NULL is returned if
 * a description isn't available.  Free string when done. */
{
struct sqlConnection *conn = hAllocConn(db);
char *desc =  NULL;
char accId[GENBANK_ACC_BUFSZ], query[256];

genbankDropVer(accId, acc);

if (native && genbankIsRefSeqAcc(accId))
    {
    safef(query, sizeof(query), "select product from refLink where mrnaAcc = \"%s\"", accId);
    desc = sqlQuickString(conn, query);
    }

if (desc == NULL)
    {
    safef(query, sizeof(query), "select description.name from description,gbCdnaInfo "
          "where gbCdnaInfo.acc = \"%s\" "
          "and gbCdnaInfo.description = description.id", accId);
    desc = sqlQuickString(conn, query);
    }
hFreeConn(&conn);
return desc;
}

struct bed *hGetCtBedRange(char *db, char *browserDb, char *table, char *chrom, int chromStart,
			   int chromEnd, char *sqlConstraints)
/* Return a bed list of all items (that match sqlConstraints, if nonNULL)
 * in the given range in table.  If chromEnd is 0, omit the range (whole chrom).
 * WARNING: this does not use the bin column and maybe slower than you would like. */
{
struct dyString *query = newDyString(512);
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr;
struct hTableInfo *hti;
struct bed *bedList=NULL, *bedItem;
char **row;
char parsedChrom[HDB_MAX_CHROM_STRING];
char rootName[256];
char fullTableName[256];
char rangeStr[32];
int count;
boolean canDoUTR, canDoIntrons;
boolean useSqlConstraints = sqlConstraints != NULL && sqlConstraints[0] != 0;
char tStrand = '?', qStrand = '?';
int i;
boolean gotWhere = FALSE;

/* Caller can give us either a full table name or root table name. */
if (browserDb)
    hParseTableName(browserDb, table, rootName, parsedChrom);
else
    hParseTableName(db, table, rootName, parsedChrom);
hti = hFindTableInfo(db, chrom, rootName);
if (hti == NULL)
    errAbort("Could not find table info for table %s (%s)",
	     rootName, table);
if (hti->isSplit)
    safef(fullTableName, sizeof(fullTableName), "%s_%s", chrom, rootName);
else
    safef(fullTableName, sizeof(fullTableName), "%s", rootName);
canDoUTR = hti->hasCDS;
canDoIntrons = hti->hasBlocks;

dyStringClear(query);
// row[0], row[1] -> start, end
dyStringPrintf(query, "SELECT %s,%s", hti->startField, hti->endField);
// row[2] -> name or placeholder
if (hti->nameField[0] != 0)
    dyStringPrintf(query, ",%s", hti->nameField);
else
    dyStringPrintf(query, ",%s", hti->startField);  // keep the same #fields!
// row[3] -> score or placeholder
if (hti->scoreField[0] != 0)
    dyStringPrintf(query, ",%s", hti->scoreField);
else
    dyStringPrintf(query, ",%s", hti->startField);  // keep the same #fields!
// row[4] -> strand or placeholder
if (hti->strandField[0] != 0)
    dyStringPrintf(query, ",%s", hti->strandField);
else
    dyStringPrintf(query, ",%s", hti->startField);  // keep the same #fields!
// row[5], row[6] -> cdsStart, cdsEnd or placeholders
if (hti->cdsStartField[0] != 0)
    dyStringPrintf(query, ",%s,%s", hti->cdsStartField, hti->cdsEndField);
else
    dyStringPrintf(query, ",%s,%s", hti->startField, hti->startField);  // keep the same #fields!
// row[7], row[8], row[9] -> count, starts, ends/sizes or empty.
if (hti->startsField[0] != 0)
    dyStringPrintf(query, ",%s,%s,%s", hti->countField, hti->startsField,
		   hti->endsSizesField);
else
    dyStringPrintf(query, ",%s,%s,%s", hti->startField, hti->startField,
		   hti->startField);  // keep same #fields!
// row[10] -> tSize for PSL '-' strand coord-swizzling only:
if (sameString("tStarts", hti->startsField))
    dyStringAppend(query, ",tSize");
else
    dyStringPrintf(query, ",%s", hti->startField);  // keep the same #fields!
dyStringPrintf(query, " FROM %s", fullTableName);
if (chromEnd != 0)
    {
    dyStringPrintf(query, " WHERE %s < %d AND %s > %d",
		   hti->startField, chromEnd, hti->endField, chromStart);
    gotWhere = TRUE;
    }
if (hti->chromField[0] != 0)
    {
    dyStringPrintf(query, " %s %s = '%s'",
		   (gotWhere ? "AND" : "WHERE"), hti->chromField, chrom);
    gotWhere = TRUE;
    }
if (useSqlConstraints)
    {
    dyStringPrintf(query, " %s %s",
		   (gotWhere ? "AND" : "WHERE"), sqlConstraints);
    gotWhere = TRUE;
    }

sr = sqlGetResult(conn, query->string);

while ((row = sqlNextRow(sr)) != NULL)
    {
    AllocVar(bedItem);
    bedItem->chrom      = cloneString(chrom);
    bedItem->chromStart = atoi(row[0]);
    bedItem->chromEnd   = atoi(row[1]);
    if (hti->nameField[0] != 0)
	bedItem->name   = cloneString(row[2]);
    else
	{
	snprintf(rangeStr, sizeof(rangeStr), "%s:%d-%d", chrom,
		 bedItem->chromStart+1,  bedItem->chromEnd);
	bedItem->name   = cloneString(rangeStr);
	}
    if (hti->scoreField[0] != 0)
	bedItem->score  = atoi(row[3]);
    else
	bedItem->score  = 0;
    if (hti->strandField[0] != 0)
	if (sameString("tStarts", hti->startsField))
	    {
	    // psl: use XOR of qStrand,tStrand if both are given.
	    qStrand = row[4][0];
	    tStrand = row[4][1];
	    if ((tStrand != '+') && (tStrand != '-'))
		bedItem->strand[0] = qStrand;
	    else if ((qStrand == '-' && tStrand == '+') ||
		     (qStrand == '+' && tStrand == '-'))
		strncpy(bedItem->strand, "-", 2);
	    else
		strncpy(bedItem->strand, "+", 2);
	    }
	else
	    strncpy(bedItem->strand, row[4], 2);
    else
	strcpy(bedItem->strand, ".");
    if (canDoUTR)
	{
	bedItem->thickStart = atoi(row[5]);
	bedItem->thickEnd   = atoi(row[6]);
	/* thickStart, thickEnd fields are sometimes used for other-organism
	   coords (e.g. synteny100000, syntenyBuild30).  So if they look
	   completely wrong, fake them out to start/end.  */
	if (bedItem->thickStart < bedItem->chromStart)
	    bedItem->thickStart = bedItem->chromStart;
	else if (bedItem->thickStart > bedItem->chromEnd)
	    bedItem->thickStart = bedItem->chromStart;
	if (bedItem->thickEnd < bedItem->chromStart)
	    bedItem->thickEnd = bedItem->chromEnd;
	else if (bedItem->thickEnd > bedItem->chromEnd)
	    bedItem->thickEnd = bedItem->chromEnd;
	}
    else
	{
	bedItem->thickStart = bedItem->chromStart;
	bedItem->thickEnd   = bedItem->chromEnd;
	}
    if (canDoIntrons)
	{
	bedItem->blockCount = atoi(row[7]);
	sqlSignedDynamicArray(row[8], &bedItem->chromStarts, &count);
	if (count != bedItem->blockCount)
	    errAbort("Data error: block count (%d) must be the same as the number of block starts (%d) for table %s item %s %s:%d-%d",
		     bedItem->blockCount, count, table,
		     bedItem->name, bedItem->chrom,
		     bedItem->chromStart, bedItem->chromEnd);
	sqlSignedDynamicArray(row[9], &bedItem->blockSizes, &count);
	if (count != bedItem->blockCount)
	    errAbort("Data error: block count (%d) must be the same as the number of block ends/sizes (%d) for table %s item %s %s:%d-%d",
		     bedItem->blockCount, count, table,
		     bedItem->name, bedItem->chrom,
		     bedItem->chromStart, bedItem->chromEnd);
	if (sameString("exonEnds", hti->endsSizesField))
	    {
	    // genePred: translate ends to sizes
	    for (i=0;  i < bedItem->blockCount;  i++)
		{
		bedItem->blockSizes[i] -= bedItem->chromStarts[i];
		}
	    }
	if (sameString("tStarts", hti->startsField)) // psls
	    {
	    if (tStrand == '-')
		{
		int tSize = atoi(row[10]);
		// if protein then blockSizes are in protein space
		if (bedItem->chromStart ==
			tSize - (3*bedItem->blockSizes[bedItem->blockCount - 1]  +
			bedItem->chromStarts[bedItem->blockCount - 1]))
		    {
		    for (i=0; i<bedItem->blockCount; ++i)
			bedItem->blockSizes[i] *= 3;
		    }

		// psl: if target strand is '-', flip the coords.
		// (this is the target part of pslRc from src/lib/psl.c)
		for (i=0; i<bedItem->blockCount; ++i)
		    {
		    bedItem->chromStarts[i] = tSize - (bedItem->chromStarts[i] +
						       bedItem->blockSizes[i]);
		    }
		reverseInts(bedItem->chromStarts, bedItem->blockCount);
		reverseInts(bedItem->blockSizes, bedItem->blockCount);
		}
	    else
		{
		// if protein then blockSizes are in protein space
		if (bedItem->chromEnd ==
			3*bedItem->blockSizes[bedItem->blockCount - 1]  +
			bedItem->chromStarts[bedItem->blockCount - 1])
		    {
		    for (i=0; i<bedItem->blockCount; ++i)
			bedItem->blockSizes[i] *= 3;
		    }
		}
	if (bedItem->chromStart != bedItem->chromStarts[0])
	    errAbort("Data error: start (%d) must be the same as first block start (%d) for table %s item %s %s:%d-%d",
		     bedItem->chromStart, bedItem->chromStarts[0],
		     table, bedItem->name,
		     bedItem->chrom, bedItem->chromStart, bedItem->chromEnd);
	    }
	if (! (sameString("chromStarts", hti->startsField) ||
	       sameString("blockStarts", hti->startsField)) )
	    {
	    // non-bed: translate absolute starts to relative starts
	    for (i=0;  i < bedItem->blockCount;  i++)
		{
		bedItem->chromStarts[i] -= bedItem->chromStart;
		}
	    }
	}
    else
	{
	bedItem->blockCount  = 0;
	bedItem->chromStarts = NULL;
	bedItem->blockSizes  = NULL;
	}
    slAddHead(&bedList, bedItem);
    }

dyStringFree(&query);
slReverse(&bedList);
hFreeConn(&conn);
return(bedList);
}

struct bed *hGetBedRange(char *db, char *table, char *chrom, int chromStart,
			   int chromEnd, char *sqlConstraints)
/* Return a bed list of all items (that match sqlConstraints, if nonNULL)
 * in the given range in table.  If chromEnd is 0, omit the range (whole chrom).
 * WARNING: this does not use the bin column and maybe slower than you would like. */
{
return hGetCtBedRange(db, NULL, table, chrom, chromStart, chromEnd, sqlConstraints);
}

int hGetBedRangeCount(char *db, char *table, char *chrom, int chromStart,
			 int chromEnd, char *sqlConstraints)
/* Return a count of all the items (that match sqlConstraints, if nonNULL)
 * in the given range in table.  If chromEnd is 0, omit the range (whole chrom).
 * WARNING: this does not use the bin column and maybe slower than you would like.
 * C.f. hGetBedRange() but returns only the result of SELECT COUNT(*) FROM ...  */
{
struct dyString *query = newDyString(512);
struct sqlConnection *conn = hAllocConn(db);
struct hTableInfo *hti;
char parsedChrom[HDB_MAX_CHROM_STRING];
char rootName[256];
char fullTableName[256];
int count;
boolean useSqlConstraints = sqlConstraints != NULL && sqlConstraints[0] != 0;
boolean gotWhere = FALSE;

/* Caller can give us either a full table name or root table name. */
hParseTableName(db, table, rootName, parsedChrom);
hti = hFindTableInfo(db, chrom, rootName);
if (hti == NULL)
    errAbort("Could not find table info for table %s (%s)",
	     rootName, table);
if (hti->isSplit)
    safef(fullTableName, sizeof(fullTableName), "%s_%s", chrom, rootName);
else
    safef(fullTableName, sizeof(fullTableName), "%s", rootName);

dyStringClear(query);
dyStringPrintf(query, "SELECT count(*) FROM %s", fullTableName);
if (chromEnd != 0)
    {
    dyStringPrintf(query, " WHERE %s < %d AND %s > %d",
		   hti->startField, chromEnd, hti->endField, chromStart);
    gotWhere = TRUE;
    }
if (hti->chromField[0] != 0)
    {
    dyStringPrintf(query, " %s %s = '%s'",
		   (gotWhere ? "AND" : "WHERE"), hti->chromField, chrom);
    gotWhere = TRUE;
    }
if (useSqlConstraints)
    {
    dyStringPrintf(query, " %s %s",
		   (gotWhere ? "AND" : "WHERE"), sqlConstraints);
    gotWhere = TRUE;
    }

count = sqlQuickNum(conn, query->string);

dyStringFree(&query);
hFreeConn(&conn);
return count;
}

struct bed *hGetFullBed(char *db, char *table)
/* Return a genome-wide bed list of the table. */
/* WARNING: This isn't designed for CGI use. It's a looped call to */
/* hGetBedRange() which has its own warning. */
{
struct slName *chromList = hChromList(db);
struct slName *chrom;
struct bed *list = NULL;
for (chrom = chromList; chrom != NULL; chrom = chrom->next)
    {
    struct bed *bedList = hGetBedRange(db, table, chrom->name, 0, 0, NULL);
    list = slCat(list, bedList);
    }
slFreeList(&chromList);
return list;
}

char *hPdbFromGdb(char *genomeDb)
/* Find proteome database name given genome database name */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr;
char **row;
char *ret = NULL;
struct dyString *dy = newDyString(128);

if (sqlTableExists(conn, "gdbPdb"))
    {
    if (genomeDb != NULL)
	dyStringPrintf(dy, "select proteomeDb from gdbPdb where genomeDb = '%s';", genomeDb);
    else
	internalErr();
    sr = sqlGetResult(conn, dy->string);
    if ((row = sqlNextRow(sr)) != NULL)
	{
	ret = cloneString(row[0]);
	}
    else
	{
	// if a corresponding protein DB is not found, get the default one from the gdbPdb table
        sqlFreeResult(&sr);
    	sr = sqlGetResult(conn,  "select proteomeDb from gdbPdb where genomeDb = 'default';");
    	if ((row = sqlNextRow(sr)) != NULL)
	    {
	    ret = cloneString(row[0]);
	    }
	else
	    {
	    errAbort("No protein database defined for %s.", genomeDb);
	    }
	}

    sqlFreeResult(&sr);
    }
hDisconnectCentral(&conn);
freeDyString(&dy);
return(ret);
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

boolean hgNearOk(char *database)
/* Return TRUE if ok to put up familyBrowser (hgNear)
 * on this database. */
{
struct sqlConnection *conn = hConnectCentral();
char query[256];
boolean ok;
safef(query, sizeof(query),
	"select hgNearOk from dbDb where name = '%s'", database);
ok = sqlQuickNum(conn, query);
hDisconnectCentral(&conn);
return ok;
}

boolean hgPbOk(char *database)
/* Return TRUE if ok to put up Proteome Browser (pbTracks)
 * on this database. */
{
struct sqlConnection *conn = hConnectCentral();
char query[256];
char **row;
struct sqlResult *sr = NULL;
boolean ok;
boolean dbDbHasPbOk;

dbDbHasPbOk = FALSE;
safef(query, sizeof(query), "describe dbDb");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameWord(row[0], "hgPbOk"))
        {
        dbDbHasPbOk = TRUE;
        }
    }
sqlFreeResult(&sr);
if (!dbDbHasPbOk) return(FALSE);

safef(query, sizeof(query),
        "select hgPbOk from dbDb where name = '%s'", database);
ok = sqlQuickNum(conn, query);
hDisconnectCentral(&conn);
return ok;
}

boolean hgPcrOk(char *database)
/* Return TRUE if ok to put up hgPcr on this database. */
{
struct sqlConnection *conn = hConnectCentral();
char query[256];
boolean ok;
safef(query, sizeof(query),
	"select canPcr from blatServers where db = '%s' and isTrans=0", database);
ok = sqlQuickNum(conn, query);
hDisconnectCentral(&conn);
return ok;
}

char *hArchiveOrCentralDbDbOptionalField(char *database, char *field, boolean archive)
/* Look up field in dbDb table keyed by database,
 * Return NULL if database doesn't exist.
 * Free this string when you are done. Look in
 * either the regular or the archive dbDb table for .
 * The name for this function may be a little silly. */
{
struct sqlConnection *conn = hConnectCentral();
char buf[128];
char query[256];
char dbDbTable[128];
char *res = NULL;

if (archive)
    safef(dbDbTable, sizeof(dbDbTable), "dbDbArch");
else
    safef(dbDbTable, sizeof(dbDbTable), "dbDb");

safef(query, sizeof(query), "select %s from %s where name = '%s'",
      field, dbDbTable, database);
if (sqlQuickQuery(conn, query, buf, sizeof(buf)) != NULL)
    res = cloneString(buf);

hDisconnectCentral(&conn);
return res;
}

char *hArchiveDbDbOptionalField(char *database, char *field)
/* Wrapper for hArchiveOrCentralDbDbOptionalField to
 * look up in the archive database. */
{
return hArchiveOrCentralDbDbOptionalField(database, field, TRUE);
}

char *hDbDbOptionalField(char *database, char *field)
/* Wrapper for hArchiveOrCentralDbDbOptionalField to
 * look up in the regular central database. */
{
return hArchiveOrCentralDbDbOptionalField(database, field, FALSE);
}

char *hDbDbField(char *database, char *field)
/* Look up field in dbDb table keyed by database.
 * Free this string when you are done. */
{
char *res = hDbDbOptionalField(database, field);
if (res == NULL)
    errAbort("Can't find %s for %s", field, database);
return res;
}

char *hDefaultPos(char *database)
/* Return default chromosome position for the
  organism associated with database.   use freeMem on
 * this when done. */
{
return hDbDbField(database, "defaultPos");
}

char *hOrganism(char *database)
/* Return organism associated with database.   use freeMem on
 * this when done. */
{
if (sameString(database, "rep"))    /* bypass dbDb if repeat */
    return cloneString("Repeat");
return hDbDbOptionalField(database, "organism");
}

char *hArchiveOrganism(char *database)
/* Return organism name from the archive database.  E.g. "hg12". */
{
char *organism = hOrganism(database);
if (!organism)
    organism = hArchiveDbDbOptionalField(database, "organism");
return organism;
}

char *hGenomeOrArchive(char *database)
/* Return genome name associated from the regular or the archive database. */
{
char *genome = hGenome(database);
if (!genome)
    genome = hArchiveDbDbOptionalField(database, "genome");
return genome;
}

char *hDbDbNibPath(char *database)
/* return nibPath from dbDb for database */
{
return hDbDbOptionalField(database, "nibPath");
}

char *hGenome(char *database)
/* Return genome associated with database.
 * use freeMem on this when done. */
{
return hDbDbOptionalField(database, "genome");
}

char *hScientificName(char *database)
/* Return scientific name for organism represented by this database */
/* Return NULL if unknown database */
/* NOTE: must free returned string after use */
{
return hDbDbOptionalField(database, "scientificName");
}

char *hHtmlPath(char *database)
/* Return /gbdb path name to html description for this database */
/* Return NULL if unknown database */
/* NOTE: must free returned string after use */
{
return hDbDbOptionalField(database, "htmlPath");
}

char *hFreezeDate(char *database)
/* Return freeze date of database. Use freeMem when done. */
{
return hDbDbField(database, "description");
}

char *hFreezeDateOpt(char *database)
/* Return freeze date of database or NULL if unknown database
 *  Use freeMem when done. */
{
return hDbDbOptionalField(database, "description");
}

int hOrganismID(char *database)
/* Get organism ID from relational organism table */
/* Return 0 if not found. */
{
char query[256];
struct sqlConnection *conn = hAllocConn(database);
int ret;

safef(query, sizeof(query), "select id from organism where name = '%s'",
				    hScientificName(database));
ret = sqlQuickNum(conn, query);
hFreeConn(&conn);
return ret;
}

static boolean hGotCladeConn(struct sqlConnection *conn)
/* Return TRUE if central db contains clade info tables. */
{
return (sqlTableExists(conn, "clade") && sqlTableExists(conn, "genomeClade"));
}

boolean hGotClade()
/* Return TRUE if central db contains clade info tables. */
{
struct sqlConnection *conn = hConnectCentral();
boolean gotClade = hGotCladeConn(conn);
hDisconnectCentral(&conn);
return gotClade;
}

char *hClade(char *genome)
/* If central database has clade tables, return the clade for the
 * given genome; otherwise return NULL. */
{
struct sqlConnection *conn = hConnectCentral();
if (hGotCladeConn(conn))
    {
    char query[512];
    char *clade;
    safef(query, sizeof(query),
	  "select clade from genomeClade where genome = '%s'", genome);
    clade = sqlQuickString(conn, query);
    hDisconnectCentral(&conn);
    if (clade == NULL)
	{
	warn("Warning: central database genomeClade doesn't contain "
	     "genome \"%s\"", genome);
	return cloneString("other");
	}
    else
	return clade;
    }
else
    {
    hDisconnectCentral(&conn);
    return NULL;
    }
}

struct dbDb *hDbDb(char *database)
/* Return dbDb entry for a database */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr;
char **row;
struct dbDb *db = NULL;

struct dyString *ds = dyStringNew(0);
dyStringPrintf(ds, "select * from dbDb where name='%s'", database);
sr = sqlGetResult(conn, ds->string);
if ((row = sqlNextRow(sr)) != NULL)
    db = dbDbLoad(row);
sqlFreeResult(&sr);
hDisconnectCentral(&conn);
dyStringFree(&ds);
return db;
}

struct dbDb *hDbDbListMaybeCheck(boolean doCheck)
/* Return list of databases in dbDb.  If doCheck, check database existence.
 * The list includes the name, description, and where to
 * find the nib-formatted DNA files. Free this with dbDbFree. */
{
struct sqlConnection *conn = hConnectCentral();
struct sqlResult *sr;
char **row;
struct dbDb *dbList = NULL, *db;
struct hash *hash = sqlHashOfDatabases();

sr = sqlGetResult(conn, "select * from dbDb order by orderKey,name desc");
while ((row = sqlNextRow(sr)) != NULL)
    {
    db = dbDbLoad(row);
    if (!doCheck || hashLookup(hash, db->name))
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

struct dbDb *hDbDbList()
/* Return list of databases that are actually online.
 * The list includes the name, description, and where to
 * find the nib-formatted DNA files. Free this with dbDbFree. */
{
return hDbDbListMaybeCheck(TRUE);
}

static struct dbDb *hDbDbListDeadOrAlive()
/* Return list of databases named in dbDb whether they exist or not.
 * The list includes the name, description, and where to
 * find the nib-formatted DNA files. Free this with dbDbFree. */
{
return hDbDbListMaybeCheck(FALSE);
}

struct dbDb *archiveDbDbLoad(char **row)
/* Load a archive dbDb from row fetched with select * from dbDb
         from database.  Dispose of this with dbDbFree().
  NOTE: this table schema is now detached from the
  main production dbDb, so we are not using the autoSql functions */
{
    struct dbDb *ret;

    AllocVar(ret);
    ret->name = cloneString(row[0]);
    ret->description = cloneString(row[1]);
    ret->nibPath = cloneString(row[2]);
    ret->organism = cloneString(row[3]);
    ret->defaultPos = cloneString(row[4]);
    ret->active = sqlSigned(row[5]);
    ret->orderKey = sqlSigned(row[6]);
    ret->genome = cloneString(row[7]);
    ret->scientificName = cloneString(row[8]);
    ret->htmlPath = cloneString(row[9]);
    ret->hgNearOk = sqlSigned(row[10]);
    return ret;
}

struct dbDb *hArchiveDbDbList()
/* Return list of databases in archive central dbDb.
 * Free this with dbDbFree. */
{
struct sqlConnection *conn;
struct sqlResult *sr;
char **row;
struct dbDb *dbList = NULL, *db;
char *assembly;
char *next;

conn = hConnectCentral();

if (conn)
    {
    /* NOTE: archive orderKey convention is opposite of production server! */
    sr = sqlGetResult(conn, "select * from dbDbArch order by orderKey desc,name desc");
    while ((row = sqlNextRow(sr)) != NULL)
        {
        db = archiveDbDbLoad(row);
        /* strip organism out of assembly description if it's there
         * (true in hg6-hg11 entries) */
        next = assembly = cloneString(db->description);
        if (sameString(nextWord(&next), db->genome))
            {
            freez(&db->description);
            db->description = cloneString(next);
            }
        freez(&assembly);
        slAddHead(&dbList, db);
        }
    sqlFreeResult(&sr);

    hDisconnectCentral(&conn);
    slReverse(&dbList);
    }
return dbList;
}

int hDbDbCmpOrderKey(const void *va, const void *vb)
/* Compare to sort based on order key */
{
const struct dbDb *a = *((struct dbDb **)va);
const struct dbDb *b = *((struct dbDb **)vb);

if (b->orderKey > a->orderKey) return -1;
else if (b->orderKey < a->orderKey) return 1;
else return 0;
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

char *hPreviousAssembly(char *database)
/* Return previous assembly for the genome associated with database, or NULL.
 * Must free returned string */

{
struct dbDb *dbList = NULL, *db, *prevDb;
char *prev = NULL;

/* NOTE: relies on this list being ordered descendingly */
dbList = hDbDbList();
for (db = dbList; db != NULL; db = db->next)
    {
    if (sameString(db->name, database))
        {
        prevDb = db->next;
        if (prevDb)
            prev = cloneString(prevDb->name);
        break;
        }
    }
if (dbList)
    dbDbFreeList(&dbList);
return prev;
}


static boolean fitField(struct hash *hash, char *fieldName,
	char retField[HDB_MAX_FIELD_STRING])
/* Return TRUE if fieldName is in hash.
 * If so copy it to retField.
 * Helper routine for findMoreFields below. */
{
if (hashLookup(hash, fieldName))
    {
    strcpy(retField, fieldName);
    return TRUE;
    }
else
    {
    retField[0] = 0;
    return FALSE;
    }
}

static boolean fitFields(struct hash *hash, char *chrom, char *start, char *end,
	char retChrom[HDB_MAX_FIELD_STRING], char retStart[HDB_MAX_FIELD_STRING], char retEnd[HDB_MAX_FIELD_STRING])
/* Return TRUE if chrom/start/end are in hash.
 * If so copy them to retChrom, retStart, retEnd.
 * Helper routine for findChromStartEndFields below. */
{
if (!fitField(hash, chrom, retChrom))
    return FALSE;
if (!fitField(hash, start, retStart))
    return FALSE;
if (!fitField(hash, end, retEnd))
    return FALSE;
return TRUE;
}

boolean hIsBinned(char *db, char *table)
/* Return TRUE if a table is binned. */
{
char query[256];
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr;
char **row;
boolean binned = FALSE;

/* Read table description into hash. */
safef(query, sizeof(query), "describe %s", table);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameString(row[0], "bin"))
        binned = TRUE;
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return binned;
}

int hFieldIndex(char *db, char *table, char *field)
/* Return index of field in table or -1 if it doesn't exist. */
{
struct sqlConnection *conn = hAllocConn(db);
int result = sqlFieldIndex(conn, table, field);
hFreeConn(&conn);
return result;
}

boolean hHasField(char *db, char *table, char *field)
/* Return TRUE if table has field */
{
return hFieldIndex(db, table, field) >= 0;
}

boolean hFieldHasIndex(char *db, char *table, char *field)
/* Return TRUE if a SQL index exists for table.field. */
{
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr = NULL;
char **row = NULL;
boolean gotIndex = FALSE;
char query[512];

safef(query, sizeof(query), "show index from %s", table);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameString(row[4], field))
	{
	gotIndex = TRUE;
	break;
	}
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
return(gotIndex);
}

static boolean hFindBed12FieldsAndBinWithConn(struct sqlConnection *conn, char *table,
	char retChrom[HDB_MAX_FIELD_STRING],
	char retStart[HDB_MAX_FIELD_STRING],
	char retEnd[HDB_MAX_FIELD_STRING],
	char retName[HDB_MAX_FIELD_STRING],
	char retScore[HDB_MAX_FIELD_STRING],
	char retStrand[HDB_MAX_FIELD_STRING],
        char retCdsStart[HDB_MAX_FIELD_STRING],
	char retCdsEnd[HDB_MAX_FIELD_STRING],
	char retCount[HDB_MAX_FIELD_STRING],
	char retStarts[HDB_MAX_FIELD_STRING],
	char retEndsSizes[HDB_MAX_FIELD_STRING],
        char retSpan[HDB_MAX_FIELD_STRING], boolean *retBinned)
/* Given a table return the fields corresponding to all the bed 12
 * fields, if they exist.  Fields that don't exist in the given table
 * will be set to "". */
{
char query[256];
struct sqlResult *sr;
char **row;
struct hash *hash = newHash(5);
boolean gotIt = TRUE, binned = FALSE;
char *db;

db = sqlGetDatabase(conn);

/* Set field names to empty strings */
retChrom[0] = 0;
retStart[0] = 0;
retEnd[0] = 0;
retName[0] = 0;
retScore[0] = 0;
retStrand[0] = 0;
retCdsStart[0] = 0;
retCdsEnd[0] = 0;
retCount[0] = 0;
retStarts[0] = 0;
retEndsSizes[0] = 0;
retSpan[0] = 0;

/* Read table description into hash. */
safef(query, sizeof(query), "describe %s", table);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameString(row[0], "bin"))
        binned = TRUE;
    hashAdd(hash, row[0], NULL);
    }
sqlFreeResult(&sr);

/* Look for bed-style or linkedFeatures names. */
if (fitFields(hash, "chrom", "chromStart", "chromEnd", retChrom, retStart, retEnd))
    {
    if (!fitField(hash, "name", retName))
	if (!fitField(hash, "acc", retName))
	    if (!fitField(hash, "frag", retName))
		if (!fitField(hash, "contig", retName))
		    fitField(hash, "sequence", retName); // so that tagAlign can masquerade as BED
    fitField(hash, "score", retScore);
    fitField(hash, "strand", retStrand);
    fitField(hash, "thickStart", retCdsStart);
    fitField(hash, "thickEnd", retCdsEnd);
    if (!fitField(hash, "blockCount", retCount))
	fitField(hash, "lfCount", retCount);
    if (!fitField(hash, "chromStarts", retStarts))
        {
	if (!fitField(hash, "blockStarts", retStarts))
            fitField(hash, "lfStarts", retStarts);
        }
    if (!fitField(hash, "blockSizes", retEndsSizes))
	fitField(hash, "lfSizes", retEndsSizes);
    fitField(hash, "span", retSpan);
    }
/* Look for names of psl and psl-like (chain, chainLink, net, altGraphX,
   some older types). */
else if (fitFields(hash, "tName", "tStart", "tEnd", retChrom, retStart, retEnd))
    {
    if (!fitField(hash, "qName", retName))
	{
	if (!fitField(hash, "name", retName))
	    fitField(hash, "chainId", retName);
	}
    fitField(hash, "strand", retStrand);
    fitField(hash, "blockCount", retCount);
    fitField(hash, "tStarts", retStarts);
    fitField(hash, "blockSizes", retEndsSizes);
    }
/* Look for gene prediction names. */
else if (fitFields(hash, "chrom", "txStart", "txEnd", retChrom, retStart, retEnd))
    {
    if (!fitField(hash, "geneName", retName))  // tweak for refFlat type
	fitField(hash, "name", retName);
    fitField(hash, "score", retScore);      // some variants might have it...
    fitField(hash, "strand", retStrand);
    fitField(hash, "cdsStart", retCdsStart);
    fitField(hash, "cdsEnd", retCdsEnd);
    fitField(hash, "exonCount", retCount);
    fitField(hash, "exonStarts", retStarts);
    fitField(hash, "exonEnds", retEndsSizes);
    }
/* Look for repeatMasker names. */
else if (fitFields(hash, "genoName", "genoStart", "genoEnd", retChrom, retStart, retEnd))
    {
    fitField(hash, "repName", retName);
    fitField(hash, "swScore", retScore);
    fitField(hash, "strand", retStrand);
    }
else if (fitField(hash, "chrom", retChrom) &&
	 fitField(hash, "chromStart", retStart))
    {
    }
else if (startsWith("chr", table) && endsWith(table, "_gl") && hashLookup(hash, "start") && hashLookup(hash, "end"))
    {
    strcpy(retChrom, "");
    strcpy(retStart, "start");
    strcpy(retEnd, "end");
    fitField(hash, "frag", retName);
    fitField(hash, "strand", retStrand);
    }
else
    {
    if (hashLookup(hash, "acc"))
         strcpy(retName, "acc");
    else if (hashLookup(hash, "id"))
         strcpy(retName, "id");
    else if (hashLookup(hash, "name"))
         strcpy(retName, "name");
    gotIt = FALSE;
    }
freeHash(&hash);
*retBinned = binned;
return gotIt;
}

boolean hFindFieldsAndBinWithConn(struct sqlConnection *conn, char *db, char *table,
	char retChrom[HDB_MAX_FIELD_STRING],
	char retStart[HDB_MAX_FIELD_STRING], char retEnd[HDB_MAX_FIELD_STRING],
	boolean *retBinned)
/* Given a table return the fields for selecting chromosome, start, end,
 * and whether it's binned . */
{
char retName[HDB_MAX_FIELD_STRING];
char retScore[HDB_MAX_FIELD_STRING];
char retStrand[HDB_MAX_FIELD_STRING];
char retCdsStart[HDB_MAX_FIELD_STRING];
char retCdsEnd[HDB_MAX_FIELD_STRING];
char retCount[HDB_MAX_FIELD_STRING];
char retStarts[HDB_MAX_FIELD_STRING];
char retEndsSizes[HDB_MAX_FIELD_STRING];
char retSpan[HDB_MAX_FIELD_STRING];
return hFindBed12FieldsAndBinWithConn(conn, table,
			      retChrom, retStart, retEnd,
			      retName, retScore, retStrand,
			      retCdsStart, retCdsEnd,
			      retCount, retStarts, retEndsSizes,
			      retSpan, retBinned);
}

boolean hFindChromStartEndFieldsWithConn(struct sqlConnection *conn, char *db, char *table,
	char retChrom[HDB_MAX_FIELD_STRING],
	char retStart[HDB_MAX_FIELD_STRING], char retEnd[HDB_MAX_FIELD_STRING])
/* Given a table return the fields for selecting chromosome, start, and end. */
{
char retName[HDB_MAX_FIELD_STRING];
char retScore[HDB_MAX_FIELD_STRING];
char retStrand[HDB_MAX_FIELD_STRING];
char retCdsStart[HDB_MAX_FIELD_STRING];
char retCdsEnd[HDB_MAX_FIELD_STRING];
char retCount[HDB_MAX_FIELD_STRING];
char retStarts[HDB_MAX_FIELD_STRING];
char retEndsSizes[HDB_MAX_FIELD_STRING];
char retSpan[HDB_MAX_FIELD_STRING];
boolean isBinned;
return hFindBed12FieldsAndBinWithConn(conn, table,
			      retChrom, retStart, retEnd,
			      retName, retScore, retStrand,
			      retCdsStart, retCdsEnd,
			      retCount, retStarts, retEndsSizes,
			      retSpan, &isBinned);
}

boolean hFindBed12FieldsAndBin(char *db, char *table,
	char retChrom[HDB_MAX_FIELD_STRING],
	char retStart[HDB_MAX_FIELD_STRING],
	char retEnd[HDB_MAX_FIELD_STRING],
	char retName[HDB_MAX_FIELD_STRING],
	char retScore[HDB_MAX_FIELD_STRING],
	char retStrand[HDB_MAX_FIELD_STRING],
        char retCdsStart[HDB_MAX_FIELD_STRING],
	char retCdsEnd[HDB_MAX_FIELD_STRING],
	char retCount[HDB_MAX_FIELD_STRING],
	char retStarts[HDB_MAX_FIELD_STRING],
	char retEndsSizes[HDB_MAX_FIELD_STRING],
        char retSpan[HDB_MAX_FIELD_STRING], boolean *retBinned)
/* Given a table return the fields corresponding to all the bed 12
 * fields, if they exist.  Fields that don't exist in the given table
 * will be set to "". */
{
struct sqlConnection *conn = NULL;
boolean gotIt;

if (! hTableExists(db, table))
    return FALSE;
conn = hAllocConn(db);
gotIt = hFindBed12FieldsAndBinWithConn(conn, table,
        retChrom,
        retStart,
        retEnd,
        retName,
        retScore,
        retStrand,
        retCdsStart,
        retCdsEnd,
        retCount,
        retStarts,
        retEndsSizes,
        retSpan, retBinned);
hFreeConn(&conn);
return(gotIt);
}

boolean hFindFieldsAndBin(char *db, char *table,
	char retChrom[HDB_MAX_FIELD_STRING],
	char retStart[HDB_MAX_FIELD_STRING], char retEnd[HDB_MAX_FIELD_STRING],
	boolean *retBinned)
/* Given a table return the fields for selecting chromosome, start, end,
 * and whether it's binned . */
{
char retName[HDB_MAX_FIELD_STRING];
char retScore[HDB_MAX_FIELD_STRING];
char retStrand[HDB_MAX_FIELD_STRING];
char retCdsStart[HDB_MAX_FIELD_STRING];
char retCdsEnd[HDB_MAX_FIELD_STRING];
char retCount[HDB_MAX_FIELD_STRING];
char retStarts[HDB_MAX_FIELD_STRING];
char retEndsSizes[HDB_MAX_FIELD_STRING];
char retSpan[HDB_MAX_FIELD_STRING];
return hFindBed12FieldsAndBin(db, table,
			      retChrom, retStart, retEnd,
			      retName, retScore, retStrand,
			      retCdsStart, retCdsEnd,
			      retCount, retStarts, retEndsSizes,
			      retSpan, retBinned);
}

boolean hFindChromStartEndFields(char *db, char *table,
	char retChrom[HDB_MAX_FIELD_STRING],
	char retStart[HDB_MAX_FIELD_STRING], char retEnd[HDB_MAX_FIELD_STRING])
/* Given a table return the fields for selecting chromosome, start, and end. */
{
char retName[HDB_MAX_FIELD_STRING];
char retScore[HDB_MAX_FIELD_STRING];
char retStrand[HDB_MAX_FIELD_STRING];
char retCdsStart[HDB_MAX_FIELD_STRING];
char retCdsEnd[HDB_MAX_FIELD_STRING];
char retCount[HDB_MAX_FIELD_STRING];
char retStarts[HDB_MAX_FIELD_STRING];
char retEndsSizes[HDB_MAX_FIELD_STRING];
char retSpan[HDB_MAX_FIELD_STRING];
boolean isBinned;
return hFindBed12FieldsAndBin(db, table,
			      retChrom, retStart, retEnd,
			      retName, retScore, retStrand,
			      retCdsStart, retCdsEnd,
			      retCount, retStarts, retEndsSizes,
			      retSpan, &isBinned);
}

struct hTableInfo *hFindTableInfo(char *db, char *chrom, char *rootName)
/* Find table information.  Return NULL if no db.  */
{
struct sqlConnection *conn;
struct hTableInfo *hti;

conn = hAllocConn(db);
if (conn == NULL)
    {
    return(NULL);
    }
else
    {
    hti = hFindTableInfoWithConn(conn, chrom, rootName);
    }

hFreeConn(&conn);
return hti;
}

struct hTableInfo *hFindTableInfoWithConn(struct sqlConnection *conn, char *chrom, char *rootName)
/* Find table information, with conn as part of input parameters.  Return NULL if no table.  */
{
static struct hash *dbHash = NULL;	/* Values are hashes of tables. */
struct hash *hash;
struct hTableInfo *hti;
char fullName[HDB_MAX_TABLE_STRING];
boolean isSplit = FALSE;
char *db;

if (conn == NULL) return FALSE;

db = sqlGetDatabase(conn);

if (chrom == NULL)
    chrom = hDefaultChrom(db);
if (dbHash == NULL)
    dbHash = newHash(8);
hash = hashFindVal(dbHash, db);
if (hash == NULL)
    {
    hash = newHash(8);
    hashAdd(dbHash, db, hash);
    }
if ((hti = hashFindVal(hash, rootName)) == NULL)
    {
    if ((sameString(rootName, "mrna") && sqlTableExists(conn, "all_mrna")) ||
	(sameString(rootName, "est") && sqlTableExists(conn, "all_est")))
	{
	safef(fullName, sizeof(fullName), "all_%s", rootName);
	rootName = fullName;
	}
    else
	{
	if (chrom != NULL)
	    {
	    safef(fullName, sizeof(fullName), "%s_%s", chrom, rootName);
	    if (sqlTableExists(conn, fullName))
		isSplit = TRUE;
	    }
	if (!isSplit)
	    {
	    safecpy(fullName, sizeof(fullName), rootName);
	    if (!sqlTableExists(conn, fullName))
		return NULL;
	    }
	}
    AllocVar(hti);
    hashAddSaveName(hash, rootName, hti, &hti->rootName);
    hti->isSplit = isSplit;
    hti->isPos = hFindBed12FieldsAndBinWithConn(conn, fullName,
	hti->chromField, hti->startField, hti->endField,
	hti->nameField, hti->scoreField, hti->strandField,
	hti->cdsStartField, hti->cdsEndField,
	hti->countField, hti->startsField, hti->endsSizesField,
	hti->spanField, &hti->hasBin);
    hti->hasCDS = (hti->cdsStartField[0] != 0);
    hti->hasBlocks = (hti->startsField[0] != 0);
    if (hti->isPos)
	{
	if (sameString(hti->startsField, "exonStarts"))
	    hti->type = cloneString("genePred");
	else if (sameString(hti->startsField, "chromStarts") ||
		 sameString(hti->startsField, "blockStarts"))
	    hti->type = cloneString("bed 12");
	else if (sameString(hti->startsField, "lfStarts"))
	    hti->type = cloneString("linkedFeatures");
	else if (sameString(hti->startsField, "tStarts"))
	    hti->type = cloneString("psl");
	else if (hti->cdsStartField[0] != 0)
	    hti->type = cloneString("bed 8");
	else if (hti->strandField[0] !=0  &&  hti->chromField[0] == 0)
	    hti->type = cloneString("gl");
	else if (hti->strandField[0] !=0)
	    hti->type = cloneString("bed 6");
	else if (hti->spanField[0] !=0)
	    hti->type = cloneString("wiggle");
	else if (hti->nameField[0] !=0)
	    hti->type = cloneString("bed 4");
	else if (hti->endField[0] != 0)
	    hti->type = cloneString("bed 3");
	else
	    {
	    hti->type = cloneString("chromGraph");
	    safef(hti->endField, sizeof(hti->endField), "%s+1",
	    	hti->startField);
	    }
	}
    else
	hti->type = NULL;
    }
return hti;
}

int hTableInfoBedFieldCount(struct hTableInfo *hti)
/* Return number of BED fields needed to save hti. */
{
if (hti->hasBlocks)
    return 12;
else if (hti->hasCDS)
    return 8;
else if (hti->strandField[0] != 0)
    return 6;
else if (hti->scoreField[0] != 0)
    return 5;
else if (hti->nameField[0] != 0)
    return 4;
else
    return 3;
}



boolean hFindSplitTable(char *db, char *chrom, char *rootName,
	char retTableBuf[HDB_MAX_TABLE_STRING], boolean *hasBin)
/* Find name of table in a given database that may or may not
 * be split across chromosomes. Return FALSE if table doesn't exist.  */
{
struct hTableInfo *hti = hFindTableInfo(db, chrom, rootName);
if (hti == NULL)
    return FALSE;
if (retTableBuf != NULL)
    {
    if (chrom == NULL)
	chrom = hDefaultChrom(db);
    if (hti->isSplit)
	safef(retTableBuf, HDB_MAX_TABLE_STRING, "%s_%s", chrom, rootName);
    else
	safef(retTableBuf, HDB_MAX_TABLE_STRING, "%s", rootName);
    }
if (hasBin != NULL)
    *hasBin = hti->hasBin;
return TRUE;
}

struct slName *hSplitTableNames(char *db, char *rootName)
/* Return a list of all split tables for rootName, or of just rootName if not
 * split, or NULL if no such tables exist. */
{
struct hash *hash = NULL;
struct hashEl *hel = NULL;

hash = tableListGetDbHash(db);
hel = hashLookup(hash, rootName);
if (hel == NULL)
    return NULL;
else
    return slNameCloneList((struct slName *)(hel->val));
}

boolean hHostHasPrefix(char *prefix)
/* Return TRUE if this is running on web-server with host name prefix */
{
if (prefix == NULL)
    return FALSE;

char *httpHost = getenv("HTTP_HOST");
if (httpHost == NULL)
    // make sure this works when CGIs are run from the command line.
    httpHost = getenv("HOST");

if (httpHost == NULL)
    return FALSE;

return startsWith(prefix, httpHost);
}

boolean hIsPrivateHost()
/* Return TRUE if this is running on private (development) web-server.
 * This was originally genome-test as well as hgwdev, however genome-test
 * may be repurposed to direct users to the preview site instead of development site. */
{
return hHostHasPrefix("hgwdev") || hHostHasPrefix("genome-test");  // FIXME: If genome-test
}

boolean hIsBetaHost()
/* Return TRUE if this is running on beta (QA) web-server.
 * Use sparingly as behavior on beta should be as close to RR as possible. */
{
return hHostHasPrefix("hgwbeta");
}

boolean hIsPreviewHost()
/* Return TRUE if this is running on preview web-server.  The preview
 * server is a mirror of the development server provided for public
 * early access. */
{
if (cfgOption("test.preview"))
    return TRUE;
return hHostHasPrefix("genome-preview");
}

char *hBrowserName()
/* Return browser name based on host name */
{
return (hIsPreviewHost() ? "Preview Genome Browser" :
        (hIsPrivateHost() ? "TEST Genome Browser" : "Genome Browser"));
}

int hOffsetPastBin(char *db, char *chrom, char *table)
/* Return offset into a row of table that skips past bin
 * field if any. */
{
struct hTableInfo *hti = hFindTableInfo(db, chrom, table);
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

int hFindBin(int start, int end)
/* Given start,end in chromosome coordinates assign it
 * a bin.   There's a bin for each 128k segment, for each
 * 1M segment, for each 8M segment, for each 64M segment,
 * and for each chromosome (which is assumed to be less than
 * 512M.)  A range goes into the smallest bin it will fit in. */
{
return binFromRange(start, end);
}

static void hAddBinToQueryStandard(char *binField, int start, int end,
	struct dyString *query, boolean selfContained)
/* Add clause that will restrict to relevant bins to query. */
{
int bFirstShift = binFirstShift(), bNextShift = binNextShift();
int startBin = (start>>bFirstShift), endBin = ((end-1)>>bFirstShift);
int i, levels = binLevels();

if (selfContained)
    dyStringAppend(query, "(");
for (i=0; i<levels; ++i)
    {
    int offset = binOffset(i);
    if (i != 0)
        dyStringAppend(query, " or ");
    if (startBin == endBin)
        dyStringPrintf(query, "%s=%u", binField, startBin + offset);
    else
        dyStringPrintf(query, "%s>=%u and %s<=%u",
		binField, startBin + offset, binField, endBin + offset);
    startBin >>= bNextShift;
    endBin >>= bNextShift;
    }
if (selfContained)
    {
    dyStringPrintf(query, " or %s=%u )", binField, _binOffsetOldToExtended);
    dyStringAppend(query, " and ");
    }
}

static void hAddBinToQueryExtended(char *binField, int start, int end,
	struct dyString *query)
/* Add clause that will restrict to relevant bins to query. */
{
int bFirstShift = binFirstShift(), bNextShift = binNextShift();
int startBin = (start>>bFirstShift), endBin = ((end-1)>>bFirstShift);
int i, levels = binLevelsExtended();

dyStringAppend(query, "(");

if (start < BINRANGE_MAXEND_512M)
    {
    hAddBinToQueryStandard(binField, start, BINRANGE_MAXEND_512M, query, FALSE);
    dyStringAppend(query, " or ");
    }

for (i=0; i<levels; ++i)
    {
    int offset = binOffsetExtended(i);
    if (i != 0)
	dyStringAppend(query, " or ");
    if (startBin == endBin)
        dyStringPrintf(query, "%s=%u", binField, startBin + offset);
    else
        dyStringPrintf(query, "%s>=%u and %s<=%u",
		binField, startBin + offset, binField, endBin + offset);
    startBin >>= bNextShift;
    endBin >>= bNextShift;
    }
dyStringAppend(query, ")");
dyStringAppend(query, " and ");
}

void hAddBinToQueryGeneral(char *binField, int start, int end,
	struct dyString *query)
/* Add clause that will restrict to relevant bins to query. */
{
if (end <= BINRANGE_MAXEND_512M)
    hAddBinToQueryStandard(binField, start, end, query, TRUE);
else
    hAddBinToQueryExtended(binField, start, end, query);
}

void hAddBinToQuery(int start, int end, struct dyString *query)
/* Add clause that will restrict to relevant bins to query. */
{
hAddBinToQueryGeneral("bin", start, end, query);
}

struct sqlResult *hExtendedRangeQuery(
	struct sqlConnection *conn,  /* Open SQL connection. */
	char *rootTable, 	     /* Table (not including any chrN_) */
	char *chrom, int start, int end,  /* Range. */
	char *extraWhere,            /* Extra things to add to where clause. */
	boolean order, 	   /* If true order by start position (can be slow). */
	char *fields,      /* If non-NULL comma separated field list. */
	int *retRowOffset) /* Returns offset past bin field. */
/* Range query with lots of options. */
{
char *db = sqlGetDatabase(conn);
/* call hFindTableInfoWithConn() to support tracks may from different hosts */
struct hTableInfo *hti = hFindTableInfoWithConn(conn, chrom, rootTable);
struct sqlResult *sr = NULL;
struct dyString *query = newDyString(1024);
char *table = NULL;
int rowOffset = 0;

if (fields == NULL) fields = "*";
if (hti == NULL)
    {
    warn("table %s doesn't exist or hFindTableInfoDb failed", rootTable);
    }
else
    {
    dyStringPrintf(query, "select %s from ", fields);
    if (hti->isSplit)
	{
	char fullTable[HDB_MAX_TABLE_STRING];
	safef(fullTable, sizeof(fullTable), "%s_%s", chrom, rootTable);
	if (!hTableExists(db, fullTable))
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
        {
        /* allow more flexible additions to where clause */
        if (!startsWith("order", extraWhere) &&
            !startsWith("limit", extraWhere))
                dyStringAppend(query, " and ");
        dyStringPrintf(query, " %s", extraWhere);
        }
    if (order)
        dyStringPrintf(query, " order by %s", hti->startField);
    sr = sqlGetResult(conn, query->string);
    }
freeDyString(&query);
if (retRowOffset != NULL)
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
	extraWhere, FALSE, NULL, retRowOffset);
}

struct sqlResult *hOrderedRangeQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	int start, int end, char *extraWhere, int *retRowOffset)
/* Construct and make a query to tables that may be split and/or
 * binned. Forces return values to be sorted by chromosome start. */
{
return hExtendedRangeQuery(conn, rootTable, chrom, start, end,
	extraWhere, TRUE, NULL, retRowOffset);
}

struct sqlResult *hExtendedChromQuery(
	struct sqlConnection *conn,  /* Open SQL connection. */
	char *rootTable, 	     /* Table (not including any chrN_) */
	char *chrom,  		     /* Chromosome. */
	char *extraWhere,            /* Extra things to add to where clause. */
	boolean order, 	   /* If true order by start position (can be slow). */
	char *fields,      /* If non-NULL comma separated field list. */
	int *retRowOffset) /* Returns offset past bin field. */
/* Chromosome query fields for tables that may be split and/or binned,
 * with lots of options. */
{
char *db = sqlGetDatabase(conn);
struct hTableInfo *hti = hFindTableInfo(db, chrom, rootTable);
struct sqlResult *sr = NULL;
struct dyString *query = newDyString(1024);
int rowOffset = 0;

if (fields == NULL) fields = "*";
if (hti == NULL)
    {
    warn("table %s doesn't exist", rootTable);
    }
else
    {
    rowOffset = hti->hasBin;
    if (hti->isSplit)
	{
        dyStringPrintf(query, "select %s from %s_%s", fields, chrom, rootTable);
	if (extraWhere != NULL)
	    dyStringPrintf(query, " where %s", extraWhere);
	}
    else
	{
        dyStringPrintf(query, "select %s from %s where %s='%s'",
		fields, rootTable, hti->chromField, chrom);
	if (extraWhere != NULL)
	    dyStringPrintf(query, " and (%s)", extraWhere);
	}
    if (order)
        dyStringPrintf(query, " order by %s", hti->startField);
    sr = sqlGetResult(conn, query->string);
    }
freeDyString(&query);
if (retRowOffset != NULL)
    *retRowOffset = rowOffset;
return sr;
}

struct sqlResult *hChromQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	char *extraWhere, int *retRowOffset)
/* Construct and make a query across whole chromosome to tables
 * that may be split and/or * binned. */
{
return hExtendedChromQuery(conn, rootTable, chrom, extraWhere,
	FALSE, NULL, retRowOffset);
}

boolean hTrackOnChrom(struct trackDb *tdb, char *chrom)
/* Return TRUE if track exists on this chromosome. */
{
boolean chromOk = TRUE;
if (tdb->restrictCount > 0 && chrom != NULL)
    chromOk =  (stringArrayIx(chrom, tdb->restrictList, tdb->restrictCount)) >= 0;
return chromOk;
}

static boolean loadOneTrackDb(char *db, char *where, char *tblSpec,
                              struct trackDb **tdbList, struct hash *loaded)
/* Load a trackDb table, including handling profiles:tbl. Returns
 * TRUE if table exists */
{
char *tbl;
boolean exists;
// when using dbProfiles and a list of trackDb entries, it's possible that the
// database doesn't exist in one of servers.
struct sqlConnection *conn = hAllocConnProfileTblMaybe(db, tblSpec, &tbl);
if ((exists = ((conn != NULL) && sqlTableExists(conn, tbl))))
    {
    struct trackDb *oneTable = trackDbLoadWhere(conn, tbl, where), *oneRow;
    while ((oneRow = slPopHead(&oneTable)) != NULL)
        {
        if (!hashLookup(loaded, oneRow->track))
            {
            hashAdd(loaded, oneRow->track, NULL);
            slAddHead(tdbList, oneRow);
            // record for use in check available tracks
            char *profileName = getTrackProfileName(oneRow);
            if (profileName != NULL)
                tableListProcessTblProfile(profileName, db);
	    }
        else
	    {
            trackDbFree(&oneRow);
	    }
        }
    }

hFreeConn(&conn);
return exists;
}

static struct trackDb *loadTrackDb(char *db, char *where)
/* Load each trackDb table.  Will put supertracks in parent field of given tracks but
 * these are still in track list. */
{
struct trackDb *tdbList = NULL;
struct slName *tableList = hTrackDbList(), *one;
boolean foundOne = FALSE;
struct hash *loaded = hashNew(0);
for (one = tableList; one != NULL; one = one->next)
    {
    if (loadOneTrackDb(db, where, one->name, &tdbList, loaded))
        foundOne = TRUE;
    }
if (!foundOne)
    errAbort("can not find any trackDb tables for %s, check db.trackDb specification in hg.conf",
             db);
slNameFreeList(&tableList);
hashFree(&loaded);

/* fill in supertrack fields, if any in settings */
trackDbSuperMarkup(tdbList);
trackDbAddTableField(tdbList);
return tdbList;
}

static boolean trackDataAccessible(char *database, struct trackDb *tdb)
/* Return TRUE if data accessible - meaning either it has a bigDataUrl, or the
 * table exists. */
{
return trackDbSetting(tdb, "bigDataUrl") != NULL || hTableForTrack(database, tdb->table) != NULL;
}

static void addTrackIfDataAccessible(char *database, struct trackDb *tdb,
	       boolean privateHost, struct trackDb **tdbRetList)
/* check if a trackDb entry should be included in display, and if so
 * add it to the list, otherwise free it */
{
if ((!tdb->private || privateHost) && trackDataAccessible(database, tdb))
    slAddHead(tdbRetList, tdb);
else if (sameWord(tdb->type,"downloadsOnly"))
    {
    if (sameString(tdb->table,tdb->track))
        tdb->table = NULL;
    slAddHead(tdbRetList, tdb);
    }
else
    trackDbFree(&tdb);
}

#ifdef UNUSED
static void inheritFieldsFromParents(struct trackDb *tdb, struct trackDb *parent)
/* Inherit undefined fields (outside of settings) from parent. */
{
if (tdb->shortLabel == NULL)
    tdb->shortLabel = cloneString(parent->shortLabel);
if (tdb->type == NULL)
    tdb->type = cloneString(parent->type);
if (tdb->longLabel == NULL)
    tdb->longLabel = cloneString(parent->longLabel);
if (tdb->restrictList == NULL)
    tdb->restrictList = parent->restrictList;
if (tdb->url == NULL)
    tdb->url = cloneString(parent->url);
if (tdb->html == NULL)
    tdb->html = parent->html;
if (tdb->grp == NULL)
    tdb->grp = cloneString(parent->grp);
}
#endif /* UNUSED */

static void inheritFromSuper(struct trackDb *subtrackTdb,
			    struct trackDb *compositeTdb)
/* Do the sort of inheritence a supertrack expects.  It's very similar in
 * effect to the inheritance of subtracks in effect if not in method.  Rather
 * than relying on trackDbSetting to chase missing settings from parents, this
 * on copies in the settings from parent to child settingsHash.  I suspect that
 * this is no longer needed, but am leaving it in until such time as the
 * supertrack system as a whole is factored out -jk. */
{
assert(subtrackTdb->parent == compositeTdb);
subtrackTdb->parentName = compositeTdb->track;
if (!trackDbSettingClosestToHome(subtrackTdb, "noInherit"))
    {
    if (isEmpty(subtrackTdb->type))
        subtrackTdb->type = cloneString(compositeTdb->type);
    subtrackTdb->grp = cloneString(compositeTdb->grp);

    /* inherit items in parent's settings hash that aren't
     * overriden in subtrack */
    trackDbHashSettings(subtrackTdb);
    trackDbHashSettings(compositeTdb);
    struct hashEl *hel;
    struct hashCookie hc = hashFirst(compositeTdb->settingsHash);
    while ((hel = hashNext(&hc)) != NULL)
	{
	if (!hashLookup(subtrackTdb->settingsHash, hel->name))
	    hashAdd(subtrackTdb->settingsHash, hel->name, hel->val);
	}
    }
}

static void rInheritFields(struct trackDb *tdbList)
/* Go through list inheriting fields from parent if possible, and invoking self on children. */
{
struct trackDb *tdb;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    if (tdb->parent != NULL)
        {
	if (tdbIsSuperTrack(tdb->parent))
	    /* Do supertrack-specific inheritance. */
	    inheritFromSuper(tdb, tdb->parent);
	}
    rInheritFields(tdb->subtracks);
    }
}

static void trackDbContainerMarkup(struct trackDb *parent, struct trackDb *tdbList)
/* Set up things so that the COMPOSITE_NODE and related macros work on tdbList. */
{
if (parent != NULL)
    {
    if (trackDbLocalSetting(parent, "compositeTrack"))
        tdbMarkAsComposite(parent);
    if (trackDbLocalSetting(parent, "container"))
        tdbMarkAsMultiTrack(parent);
    }
struct trackDb *tdb;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    if (parent != NULL)
        {
        if (tdbIsMultiTrack(parent))
            tdbMarkAsMultiTrackChild(tdb);

        if (tdbIsComposite(parent) || (parent->parent != NULL && tdbIsComposite(parent->parent)))
            {
            if (tdb->subtracks == NULL)
                tdbMarkAsCompositeChild(tdb);
            else
               tdbMarkAsCompositeView(tdb);
            }
        }
    trackDbContainerMarkup(tdb, tdb->subtracks);
    }
}


static struct trackDb *pruneEmpties(struct trackDb *tdbList, char *db,
	boolean privateHost, int level)
/* Remove tracks without data.  For parent tracks data in any child is sufficient to keep
 * them alive. */
{
struct trackDb *newList = NULL, *tdb, *next;
for (tdb = tdbList; tdb != NULL; tdb = next)
    {
    next = tdb->next;
    if (tdb->subtracks != NULL)
	{
	tdb->subtracks = pruneEmpties(tdb->subtracks, db, privateHost, level+1);
	}
    if (tdb->subtracks != NULL)
        {
	slAddHead(&newList, tdb);
	}
    else
        {
        addTrackIfDataAccessible(db, tdb, privateHost, &newList);
	}
    }
slReverse(&newList);
return newList;
}

static struct trackDb *rFindTrack(int level, struct trackDb *tdbList, char *track)
/* Look for named track in list or children. */
{
struct trackDb *tdb;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    if (sameString(track, tdb->track))
        return tdb;
    struct trackDb *matchingChild = rFindTrack(level+1, tdb->subtracks, track);
    if (matchingChild != NULL)
        return matchingChild;
    }

/* Look "to the sky" in parents of root generation as well. */
if (level == 0)
    {
    for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
	{
	struct trackDb *p = tdb->parent;
	if (p != NULL && sameString(track, p->track))
	    return p;
	}
    }

return NULL;
}

#ifdef DEBUG
static void dumpFlagStatus(struct trackDb *tdbList, char *tableName, char *label)
/* Look for tdbList for track matching tableName.  Print out info on it starting with label. */
{
struct trackDb *tdb = rFindTrack(0, tdbList, tableName);
if (tdb == NULL)
     printf("%s: nil<BR>\n", label);
else
     printf("%s: treeNodeType %d, composite? %d, supertrack ? %d<BR>\n", label, tdb->treeNodeType, COMPOSITE_NODE(tdb->treeNodeType), SUPERTRACK_NODE(tdb->treeNodeType));
}
#endif /* DEBUG */

static void addChildRefsToParents(struct trackDb *tdbList)
/* Go through tdbList and set up the ->children field in parents with references
 * to their children. */
{
struct trackDb *tdb;

/* Insert a little paranoid check here to make sure this doesn't get called twice. */
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    if (tdb->children !=  NULL)
        internalErr();
        
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    struct trackDb *parent = tdb->parent;
    if (parent != NULL)
        refAdd(&parent->children, tdb);
    }
}

struct trackDb *trackDbPolishAfterLinkup(struct trackDb *tdbList, char *db)
/* Do various massaging that can only be done after parent/child
 * relationships are established. */
{
tdbList = pruneEmpties(tdbList, db, hIsPrivateHost() || hIsPreviewHost(), 0);
addChildRefsToParents(tdbList);
trackDbContainerMarkup(NULL, tdbList);
rInheritFields(tdbList);
slSort(&tdbList, trackDbCmp);
return tdbList;
}

struct trackDb *hTrackDb(char *db)
/* Load tracks associated with current db.
 * Supertracks are loaded as a trackDb, but are not in the returned list,
 * but are accessible via the parent pointers of the member tracks.  Also,
 * the supertrack trackDb subtrack fields are not set here (would be
 * incompatible with the returned list)
 * Returns list sorted by priority
 *	NOTE: this result is cached, do not free it !
 */
{
// static char *existingDb = NULL;
// static struct trackDb *tdbList = NULL;
struct trackDb *tdbList = NULL;
//if (differentStringNullOk(existingDb, db))
//    {
    tdbList = loadTrackDb(db, NULL);
    tdbList = trackDbLinkUpGenerations(tdbList);
    tdbList = trackDbPolishAfterLinkup(tdbList, db);
//    freeMem(existingDb);
//    existingDb = cloneString(db);
//    }
return tdbList;
}

static struct trackDb *loadAndLookupTrackDb(struct sqlConnection *conn,
					    char *where)
/* Load trackDb object(s). Nothing done for composite tracks here. */
{
return loadTrackDb(sqlGetDatabase(conn), where);
}

static struct trackDb *loadTrackDbForTrack(struct sqlConnection *conn,
					   char *track)
/* Load trackDb object for a track. this is common code for two external
 * functions. Handle composite tracks and subtrack inheritance here.
 */
{
struct trackDb *trackTdb = NULL;
char where[256];

safef(where, sizeof(where), "tableName = '%s'", track);
trackTdb = loadAndLookupTrackDb(conn, where);
if (!trackTdb)
    return NULL;
return trackTdb;
}

struct trackDb *tdbForTrack(char *db, char *track,struct trackDb **tdbList)
/* Load trackDb object for a track. If track is composite, its subtracks
 * will also be loaded and inheritance will be handled; if track is a
 * subtrack then inheritance will be handled.  (Unless a subtrack has
 * "noInherit on"...) This will die if the current database does not have
 * a trackDb, but will return NULL if track is not found.
 * MAY pass in prepopulated trackDb list, or may receive the trackDb list as an inout. */
{
/* Get track list .*/
struct trackDb *theTdbs = NULL;
if (tdbList == NULL || *tdbList == NULL)
    {
#ifdef NOTNOW   /* this is handled by the routines that call us, removed
                 * from here because we don't have a cart down here */
    if (isHubTrack(track))
        {
	struct hash *hash = hashNew(0);
	theTdbs = hubConnectAddHubForTrackAndFindTdb(db, track, tdbList, hash);
	}
    else
#endif
	{
	theTdbs = hTrackDb(db);
	if (tdbList != NULL)
	    *tdbList = theTdbs;
	}
    }
else
    theTdbs = *tdbList;
return rFindTrack(0, theTdbs, track);
}

struct trackDb *hTrackDbForTrackAndAncestors(char *db, char *track)
/* Load trackDb object for a track. If need be grab its ancestors too.
 * This does not load children. hTrackDbForTrack will handle children, and
 * is actually faster if being called on lots of tracks.  This function
 * though is faster on one or two tracks. */
{
struct sqlConnection *conn = hAllocConn(db);
struct trackDb *tdb = loadTrackDbForTrack(conn, track);
struct trackDb *ancestor = tdb;
for (;;)
    {
    /* Get name of previous generation if any handling both
     * composite and supertrack ancestor tags. */
    char *parentTrack = NULL;
    char *parent = trackDbLocalSetting(ancestor, "parent");
    if (parent != NULL)
	parentTrack = cloneFirstWord(parent);
    if (parentTrack == NULL)
        {
	char *super = trackDbLocalSetting(ancestor, "superTrack");
	if (super != NULL && !startsWith("on", super))
	    parentTrack = cloneFirstWord(super);
	}

    /* If no parent we're done. */
    if (parentTrack == NULL)
        break;

    ancestor->parent = loadTrackDbForTrack(conn, parentTrack);
    if (ancestor->parent == NULL)
        break;

    if (!tdbIsSuper(ancestor->parent)) // supers link to children differently
        ancestor->parent->subtracks = ancestor;
    else
        ancestor->parent->children = slRefNew(ancestor);
    ancestor = ancestor->parent;
    }
if (tdbIsSuper(ancestor))
    ancestor = ancestor->children->val;
trackDbContainerMarkup(NULL, ancestor);
rInheritFields(ancestor);
hFreeConn(&conn);
return tdb;
}

struct trackDb *hCompositeTrackDbForSubtrack(char *db, struct trackDb *sTdb)
/* Given a trackDb that may be for a subtrack of a composite track,
 * return the trackDb for the composite track if we can find it, else NULL.
 * Note: if the composite trackDb is found and returned, then its subtracks
 * member will contain a newly allocated tdb like sTdb (but not ==). */
{
struct trackDb *cTdb = NULL;
if (sTdb != NULL)
    {
    char *subTrackSetting = cloneString(trackDbLocalSetting(sTdb, "parent"));
    if (subTrackSetting != NULL)
	{
	char *compositeName = firstWordInLine(subTrackSetting);
	cTdb = hTrackDbForTrack(db, compositeName);
	freez(&subTrackSetting);
	}
    }
return cTdb;
}

boolean hgParseChromRange(char *db, char *spec, char **retChromName,
	int *retWinStart, int *retWinEnd)
/* Parse something of form chrom:start-end into pieces.
 * if db != NULL then check with chromInfo for names */
{
boolean haveDb = (db != NULL);
char *chrom, *start, *end;
char buf[256];
int iStart, iEnd;

safecpy(buf, sizeof(buf), spec);
stripChar(buf, ',');
chrom = buf;
start = strchr(chrom, ':');

if (start == NULL)
    {
    /* If just chromosome name cover all of it. */
    if (!haveDb || ((chrom = hgOfficialChromName(db, chrom)) == NULL))
	return FALSE;
    else
       {
       iStart = 0;
       iEnd = hChromSize(db, chrom);
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
    if (haveDb && ((chrom = hgOfficialChromName(db, chrom)) == NULL))
	return FALSE;
    iStart = atoi(start)-1;
    iEnd = atoi(end);
    }
if (retChromName != NULL)
    *retChromName = (haveDb)? chrom : cloneString(chrom);
if (retWinStart != NULL)
    *retWinStart = iStart;
if (retWinEnd != NULL)
    *retWinEnd = iEnd;
return TRUE;
}

boolean hgIsChromRange(char *db, char *spec)
/* Returns TRUE if spec is chrom:N-M for some human
 * chromosome chrom and some N and M. */
{
return hgParseChromRange(db, spec, NULL, NULL, NULL);
}

struct trackDb *hMaybeTrackInfo(struct sqlConnection *conn, char *trackName)
/* Load trackDb object for a track.  Actually the whole trackDb gets loaded
 * and just the one track returned. This is mostly just so parent and subtracks
 * fields can be correct. Don't die if conn has no trackDb table.  Return NULL
 * if trackName is not found. */
{
struct slName *tdb, *tdbs = hTrackDbList();
for (tdb = tdbs; tdb != NULL; tdb = tdb->next)
    {
    if (sqlTableExists(conn, tdb->name))
        {
        slFreeList(&tdbs);
        return loadTrackDbForTrack(conn, trackName);
        }
    }
slFreeList(&tdbs);
return NULL;
}

#ifdef UNUSED
struct trackDb *hTrackInfo(struct sqlConnection *conn, char *trackName)
/* Look up track in database, errAbort if it's not there. */
{
struct trackDb *tdb;

tdb = hMaybeTrackInfo(conn, trackName);
if (tdb == NULL)
    errAbort("Track %s not found", trackName);
return tdb;
}
#endif /* UNUSED */


boolean hTrackCanPack(char *db, char *trackName)
/* Return TRUE if this track can be packed. */
{
struct sqlConnection *conn = hAllocConn(db);
struct trackDb *tdb = hMaybeTrackInfo(conn, trackName);
boolean ret = FALSE;
if (tdb != NULL)
    {
    ret = tdb->canPack;
    trackDbFree(&tdb);
    }
hFreeConn(&conn);
return ret;
}

char *hTrackOpenVis(char *db, char *trackName)
/* Return "pack" if track is packable, otherwise "full". */
{
return hTrackCanPack(db, trackName) ? "pack" : "full";
}

char *hGetParent(char *db, char *subtrackName)
/* Given a subtrack table, find its parent */
{
struct sqlConnection *conn = hAllocConn(db);
struct trackDb *tdb = hMaybeTrackInfo(conn, subtrackName);
char *ret = NULL;
if (tdb != NULL)
    {
    ret = trackDbLocalSetting(tdb, "parent");
    trackDbFree(&tdb);
    }
hFreeConn(&conn);
return ret;
}

static struct hash *makeTrackSettingsHash(char *db)
/* Create  a hash of hashes with all track settings for database.
 * The returned hash is keyed by track.   The contained hashes
 * are keyed by tags and contain generic text values, corresponding
 * to the trackDb.ra settings for that track. Generally you want to
 * call the version that caches results below instead. */
{
struct hash *hash = hashNew(0);
struct slName *trackTable, *trackTableList = hTrackDbList();
struct sqlConnection *conn = hAllocConn(db);
for (trackTable = trackTableList; trackTable != NULL; trackTable = trackTable->next)
    {
    if (hTableExists(db, trackTable->name))
        {
	char query[512];
	safef(query, sizeof(query), "select tableName,settings from %s", trackTable->name);
	struct sqlResult *sr = sqlGetResult(conn, query);
	char **row;
	while ((row = sqlNextRow(sr)) != NULL)
	    {
	    struct hash *settings = trackDbSettingsFromString(row[1]);
	    hashAdd(hash, row[0], settings);
	    }
	sqlFreeResult(&sr);
	}
    }
slNameFreeList(&trackTableList);
hFreeConn(&conn);
return hash;
}

struct hash *hTdbGetTrackSettingsHash(char *db)
/* Get a hash of hashes with all track settings for database.
 * The returned hash is keyed by track.   The contained hashes
 * are keyed by tags and contain generic text values, corresponding
 * to the trackDb.ra settings for that track.  The result returned
 * is cached, and should not be altered. */
{
static struct hash *dbHash = NULL;
if (dbHash == NULL)
    dbHash = hashNew(0);
struct hash *hoh = hashFindVal(dbHash, db);
if (hoh == NULL)
    {
    hoh = makeTrackSettingsHash(db);
    hashAdd(dbHash, db, hoh);
    }
return hoh;
}

static struct hash *makeDbTableToTrackHash(char *db)
/* Create a hash based on trackDb tables in given database that
 * will give you a track name given a table name as a key. */
{
struct hash *hoh = hTdbGetTrackSettingsHash(db);
struct hash *tableToTrackHash = hashNew(0);
struct sqlConnection *conn = hAllocConn(db);
struct hashCookie cookie = hashFirst(hoh);
struct hashEl *hel;
while ((hel = hashNext(&cookie)) != NULL)
    {
    struct hash *settings = hel->val;
    char *track = hel->name;
    char *table = hashFindVal(settings, "table");
    if (table == NULL)
	{
	// backwards compatibility with older trackDb:
	if (sameString(track, "mrna"))
	    table = "all_mrna";
	else if (sameString(track, "est"))
	    table = "all_est";
	else
	    table = track;
	}
    hashAdd(tableToTrackHash, table, track);
    }
hFreeConn(&conn);
return tableToTrackHash;
}

char *hGetTrackForTable(char *db, char *table)
/* Given a table name, get first track associated with it. */
{
static struct hash *dbHash = NULL;
if (dbHash == NULL)
    dbHash = hashNew(0);
struct hash *tableToTrackHash = hashFindVal(dbHash, db);
if (tableToTrackHash == NULL)
    {
    tableToTrackHash = makeDbTableToTrackHash(db);
    hashAdd(dbHash, db, tableToTrackHash);
    }
return hashFindVal(tableToTrackHash, table);
}

char *hGetTableForTrack(char *db, char *track)
/* Given a track name, get table associated with it. */
{
struct hash *hoh = hTdbGetTrackSettingsHash(db);
struct hash *settings = hashFindVal(hoh, track);
if (settings == NULL)
    errAbort("Couldn't find settigns for track %s in hGetTableForTrack", track);
char *table = hashFindVal(settings, "table");
if (table == NULL)
    table = track;
return table;
}

static struct dbDb *hGetIndexedDbsMaybeClade(char *theDb)
/* Get list of active databases, in theDb's clade if theDb is not NULL.
 * Dispose of this with dbDbFreeList. */
{
char *theClade = theDb ? hClade(hGenome(theDb)) : NULL;
struct sqlConnection *conn = hConnectCentral(); // after hClade, since it access hgcentral too
struct sqlResult *sr = NULL;
char **row;
struct dbDb *dbList = NULL, *db;

/* Scan through dbDb table, loading into list */
if (theClade != NULL)
    {
    char query[1024];
    safef(query, sizeof(query),
	  "select dbDb.* from dbDb,genomeClade where dbDb.active = 1 and "
	  "dbDb.genome = genomeClade.genome and genomeClade.clade = \"%s\" "
	  "order by dbDb.orderKey,dbDb.name desc", theClade);
    sr = sqlGetResult(conn, query);
    }
    else
	sr = sqlGetResult(conn,
	   "select * from dbDb where active = 1 order by orderKey,name desc");
while ((row = sqlNextRow(sr)) != NULL)
    {
    db = dbDbLoad(row);
    slAddHead(&dbList, db);
    }
sqlFreeResult(&sr);
hDisconnectCentral(&conn);
slReverse(&dbList);
return dbList;
}

struct dbDb *hGetIndexedDatabases()
/* Get list of all active databases.
 * Dispose of this with dbDbFreeList. */
{
return hGetIndexedDbsMaybeClade(NULL);
}

struct dbDb *hGetIndexedDatabasesForClade(char *db)
/* Get list of active databases in db's clade.
 * Dispose of this with dbDbFreeList. */
{
return hGetIndexedDbsMaybeClade(db);
}

struct slName *hLiftOverFromDbs()
/* Return a list of names of the DBs in the
 * fromDb column of the liftOverChain.*/
{
struct slName *names = NULL;
struct liftOverChain *chainList = liftOverChainListFiltered(), *chain;
for (chain = chainList; chain != NULL; chain = chain->next)
    slNameStore(&names, chain->fromDb);
liftOverChainFreeList(&chainList);
return names;
}

struct slName *hLiftOverToDbs(char *fromDb)
/* Return a list of names of the DBs in the
 * toDb column of the liftOverChain.
 * If fromDb!=NULL, return only those with that
 * fromDb. */
{
struct slName *names = NULL;
struct liftOverChain *chainList = liftOverChainListForDbFiltered(fromDb), *chain;
for (chain = chainList; chain != NULL; chain = chain->next)
    slNameStore(&names, chain->toDb);
liftOverChainFreeList(&chainList);
return names;
}

struct slName *hLiftOverOrgs(boolean from, char *fromDb)
/* Just a function hLiftOverFromOrgs and
 * hLiftOverToOrgs call. */
{
struct slName *dbs = (from) ? hLiftOverFromDbs() : hLiftOverToDbs(fromDb);
struct slName *names = NULL, *org;
for (org = dbs; org != NULL; org = org->next)
    slNameStore(&names, hArchiveOrganism(org->name));
slReverse(&names);
slFreeList(&dbs);
return names;
}

struct slName *hLiftOverFromOrgs()
/* Return a list of names of organisms that
 * have databases in the fromDb column of
 * liftOverChain.*/
{
return hLiftOverOrgs(TRUE,NULL);
}

struct slName *hLiftOverToOrgs(char *fromDb)
/* Return a list of names of the organisms with
 * databases in the toDb column of the liftOverChain.
 * If fromDb!=NULL, return only those with that
 * fromDb. */
{
return hLiftOverOrgs(FALSE,fromDb);
}


struct hash *hGetDatabaseRank()
/* Get list of databases and make a hash of order rank
 * Dispose of this with hashFree. */
{
struct dbDb *allDbList = NULL, *regDb = NULL, *archDb = NULL, *dbDb;
struct hash *dbNameHash = newHash(3);
int rank = 0;

/* Get list of all current and archived databases */
regDb = hDbDbListDeadOrAlive();
archDb = hArchiveDbDbList();
allDbList = slCat(regDb, archDb);

/* Create a hash all dbs with rank number */
for (dbDb = allDbList; dbDb != NULL; dbDb = dbDb->next)
    {
    if (!hashFindVal(dbNameHash, dbDb->name))
	{
	if (dbDb->active)
    	    hashAddInt(dbNameHash, dbDb->name, ++rank);
	}
    }
hashAddInt(dbNameHash, "maxRank", rank);
dbDbFreeList(&allDbList);
return dbNameHash;
}

struct dbDb *hGetLiftOverFromDatabases()
/* Get list of databases for which there is at least one liftOver chain file
 * from this assembly to another.
 * Dispose of this with dbDbFreeList. */
{
struct dbDb *allDbList = NULL, *regDb = NULL, *archDb = NULL;
struct dbDb *liftOverDbList = NULL, *dbDb, *nextDbDb;
struct liftOverChain *chainList = NULL, *chain;
struct hash *hash = newHash(0), *dbNameHash = newHash(3);

/* Get list of all liftOver chains in central database */
chainList = liftOverChainListFiltered();

/* Create hash of databases having liftOver chains from this database */
for (chain = chainList; chain != NULL; chain = chain->next)
    {
    if (!hashFindVal(hash, chain->fromDb))
        hashAdd(hash, chain->fromDb, chain->fromDb);
    }

/* Get list of all current and archived databases */
regDb = hDbDbListDeadOrAlive();
archDb = hArchiveDbDbList();
allDbList = slCat(regDb, archDb);

/* Create a new dbDb list of all entries in the liftOver hash */
for (dbDb = allDbList; dbDb != NULL; dbDb = nextDbDb)
    {
    /* current dbDb entries */
    nextDbDb = dbDb->next;
    if (hashFindVal(hash, dbDb->name) && !hashFindVal(dbNameHash, dbDb->name))
	{
        slAddHead(&liftOverDbList, dbDb);
	hashAdd(dbNameHash, dbDb->name, dbDb->name);
	}
    else
        dbDbFree(&dbDb);
    }

hashFree(&hash);
hashFree(&dbNameHash);
liftOverChainFreeList(&chainList);

/* sort by orderKey so that assemblies always appear from most recent */
/* to the oldest assemblies in the dropdown menu for fromDbs */
slSort(&liftOverDbList, hDbDbCmpOrderKey);

return liftOverDbList;
}

struct dbDb *hGetLiftOverToDatabases(char *fromDb)
/* Get list of databases for which there are liftOver chain files
 * to convert from the fromDb assembly.
 * Dispose of this with dbDbFreeList. */
{
struct dbDb *allDbList = NULL, *liftOverDbList = NULL, *dbDb, *nextDbDb;
struct liftOverChain *chainList = NULL, *chain;
struct hash *hash = newHash(0);
struct hash *dbNameHash = newHash(3);

/* Get list of all liftOver chains in central database */
chainList = liftOverChainListForDbFiltered(fromDb);

/* Create hash of databases having liftOver chains from the fromDb */
for (chain = chainList; chain != NULL; chain = chain->next)
    if (sameString(chain->fromDb,fromDb))
	hashAdd(hash, chain->toDb, chain->toDb);

/* Get list of all current databases */
allDbList = slCat(hDbDbListDeadOrAlive(),hArchiveDbDbList());

/* Create a new dbDb list of all entries in the liftOver hash */
for (dbDb = allDbList; dbDb != NULL; dbDb = nextDbDb)
    {
    nextDbDb = dbDb->next;
    if (hashFindVal(hash, dbDb->name) && !hashFindVal(dbNameHash, dbDb->name))
	{
        slAddHead(&liftOverDbList, dbDb);
	/* to avoid duplicates in the returned list. */
	hashAdd(dbNameHash, dbDb->name, dbDb->name);
	}
    else
        dbDbFree(&dbDb);
    }
hashFree(&hash);
liftOverChainFreeList(&chainList);

/* sort by orderKey so that assemblies always appear from most recent */
/* to the oldest assemblies in the dropdown menu for toDbs */
slSort(&liftOverDbList, hDbDbCmpOrderKey);
return liftOverDbList;
}

#ifndef GBROWSE
struct dbDb *hGetAxtInfoDbs(char *db)
/* Get list of db's where we have axt files listed in axtInfo .
 * The db's with the same organism as current db go last.
 * Dispose of result with dbDbFreeList. */
{
struct dbDb *dbDbList = NULL, *dbDb;
struct hash *hash = hashNew(7); // 2^^7 entries = 128
struct slName *dbNames = NULL, *dbName;
struct dyString *query = newDyString(256);
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr = NULL;
char **row;
char *organism = hOrganism(db);
int count;

if (! hTableExists(db, "axtInfo"))
    {
    dyStringFree(&query);
    hashFree(&hash);
    hFreeConn(&conn);
    return NULL;
    }

/* "species" is a misnomer, we're really looking up database names. */
sr = sqlGetResult(conn, "select species from axtInfo");
while ((row = sqlNextRow(sr)) != NULL)
    {
    // uniquify database names and make sure the databases still exist
    if ((hashLookup(hash, row[0]) == NULL) && hDbExists(row[0]))
	{
	struct slName *sln = newSlName(cloneString(row[0]));
	slAddHead(&dbNames, sln);
	hashStoreName(hash, cloneString(row[0]));
	}
    }
sqlFreeResult(&sr);
hFreeConn(&conn);

/* Traverse the uniquified list of databases twice: first for db's with
 * a different organism, then for db's with this organism. */
conn = hConnectCentral();
dyStringClear(query);
dyStringAppend(query, "SELECT * from dbDb");
count = 0;
for (dbName = dbNames;  dbName != NULL;  dbName = dbName->next)
    {
    char *dbOrg = hOrganism(dbName->name);
    if (! sameString(dbOrg, organism))
	{
	count++;
	if (count == 1)
	    dyStringPrintf(query, " where active = 1 and (name = '%s'",
			   dbName->name);
	else
	    dyStringPrintf(query, " or name = '%s'", dbName->name);
	}
    }
dyStringPrintf(query, ") order by orderKey desc");
if (count > 0)
    {
    sr = sqlGetResult(conn, query->string);
    while ((row = sqlNextRow(sr)) != NULL)
	{
	dbDb = dbDbLoad(row);
	slAddHead(&dbDbList, dbDb);
	}
    sqlFreeResult(&sr);
    }
dyStringClear(query);
dyStringAppend(query, "SELECT * from dbDb");
count = 0;
for (dbName = dbNames;  dbName != NULL;  dbName = dbName->next)
    {
    char *dbOrg = hOrganism(dbName->name);
    if (sameString(dbOrg, organism))
	{
	count++;
	if (count == 1)
	    dyStringPrintf(query, " where active = 1 and (name = '%s'",
			   dbName->name);
	else
	    dyStringPrintf(query, " or name = '%s'", dbName->name);
	}
    }
dyStringPrintf(query, ") order by orderKey, name desc");
if (count > 0)
    {
    sr = sqlGetResult(conn, query->string);
    while ((row = sqlNextRow(sr)) != NULL)
	{
	dbDb = dbDbLoad(row);
	slAddHead(&dbDbList, dbDb);
	}
    sqlFreeResult(&sr);
    }
hDisconnectCentral(&conn);
slFreeList(&dbNames);
dyStringFree(&query);
hashFree(&hash);

slReverse(&dbDbList);
return(dbDbList);
}

struct axtInfo *hGetAxtAlignments(char *db, char *otherDb)
/* Get list of alignments where we have axt files listed in axtInfo .
 * Dispose of this with axtInfoFreeList. */
{
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr = NULL;
char **row;
struct axtInfo *aiList = NULL, *ai;
char query[256];

safef(query, sizeof(query),
      "select * from axtInfo where species = '%s' and chrom = '%s' "
      "order by sort",
      otherDb, hDefaultChrom(db));
/* Scan through axtInfo table, loading into list */
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    ai = axtInfoLoad(row);
    slAddHead(&aiList, ai);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&aiList);
return aiList;
}

struct axtInfo *hGetAxtAlignmentsChrom(char *db, char *otherDb, char *chrom)
/* Get list of alignments where we have axt files listed in axtInfo for a specified chromosome .
 * Dispose of this with axtInfoFreeList. */
{
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr = NULL;
char **row;
struct axtInfo *aiList = NULL, *ai;
char query[256];

safef(query, sizeof(query),
      "select * from axtInfo where species = '%s' and chrom = '%s'",
      otherDb, chrom);
/* Scan through axtInfo table, loading into list */
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    ai = axtInfoLoad(row);
    slAddHead(&aiList, ai);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&aiList);
return aiList;
}
#endif /* GBROWSE */

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
sr = sqlGetResult(conn, "select * from dbDb order by orderKey,name desc");
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

safef(query, sizeof(query), "select db from blatServers where db = '%s'", db);
gotIx = sqlExists(conn, query);
hDisconnectCentral(&conn);
return gotIx;
}

struct blatServerTable *hFindBlatServer(char *db, boolean isTrans)
/* Return server for given database.  Db can either be
 * database name or description. Ponter returned is owned
 * by this function and shouldn't be modified */
{
static struct blatServerTable st;
struct sqlConnection *conn = hConnectCentral();
char query[256];
struct sqlResult *sr;
char **row;
char dbActualName[32];

/* If necessary convert database description to name. */
safef(query, sizeof(query), "select name from dbDb where name = '%s'", db);
if (!sqlExists(conn, query))
    {
    safef(query, sizeof(query),
	  "select name from dbDb where description = '%s'", db);
    if (sqlQuickQuery(conn, query, dbActualName, sizeof(dbActualName)) != NULL)
        db = dbActualName;
    }

/* Do a little join to get data to fit into the blatServerTable. */
safef(query, sizeof(query),
               "select dbDb.name,dbDb.description,blatServers.isTrans"
               ",blatServers.host,blatServers.port,dbDb.nibPath "
	       "from dbDb,blatServers where blatServers.isTrans = %d and "
	       "dbDb.name = '%s' and dbDb.name = blatServers.db",
	       isTrans, db);
sr = sqlGetResult(conn, query);
if ((row = sqlNextRow(sr)) == NULL)
    {
    errAbort("Can't find a server for %s database %s\n",
	    (isTrans ? "translated" : "DNA"), db);
    }
st.db = cloneString(row[0]);
st.genome = cloneString(row[1]);
st.isTrans = atoi(row[2]);
st.host = cloneString(row[3]);
st.port = cloneString(row[4]);
st.nibDir = cloneString(row[5]);
sqlFreeResult(&sr);
hDisconnectCentral(&conn);
return &st;
}

char *sqlGetField(char *db, char *tblName, char *fldName,
  	          char *condition)
/* Return a single field from the database, table name, field name, and a
   condition string */
{
struct sqlConnection *conn = hAllocConn(db);
char query[256];
struct sqlResult *sr;
char **row;
char *answer;

answer = NULL;
safef(query, sizeof(query), "select %s from %s.%s  where %s;",
      fldName, db, tblName, condition);
//printf("<br>%s\n", query); fflush(stdout);
sr  = sqlGetResult(conn, query);
row = sqlNextRow(sr);

if (row != NULL)
    {
    answer = cloneString(row[0]);
    }

sqlFreeResult(&sr);
hFreeConn(&conn);
return answer;
}

struct hash *hChromSizeHash(char *db)
/* Get hash of chromosome sizes for database.  Just hashFree it when done. */
{
struct sqlConnection *conn = sqlConnect(db);
struct sqlResult *sr;
char **row;
struct hash *hash = newHash(0);
sr = sqlGetResult(conn, "select chrom,size from chromInfo");
while ((row = sqlNextRow(sr)) != NULL)
    hashAddInt(hash, row[0], sqlUnsigned(row[1]));
sqlFreeResult(&sr);
sqlDisconnect(&conn);
return hash;
}

struct slName *hChromList(char *db)
/* Get the list of chrom names from the database's chromInfo table. */
{
struct sqlConnection *conn = hAllocConn(db);
struct slName *list = sqlQuickList(conn, "select chrom from chromInfo");
hFreeConn(&conn);
return list;
}

char *hgDirForOrg(char *org)
/* Make directory name from organism name - getting
 * rid of dots and spaces. */
{
org = cloneString(org);
stripChar(org, '.');
subChar(org, ' ', '_');
return org;
}

struct hash *hgReadRa(char *genome, char *database, char *rootDir,
	char *rootName, struct hash **retHashOfHash)
/* Read in ra in root, root/org, and root/org/database.
 * Returns a list of hashes, one for each ra record.  Optionally
 * if retHashOfHash is non-null it returns there a
 * a hash of hashes keyed by the name field in each
 * ra sub-hash. */
{
struct hash *hashOfHash = newHash(10);
char *org = hgDirForOrg(genome);
char fileName[HDB_MAX_PATH_STRING];
struct hashEl *helList, *hel;
struct hash *raList = NULL, *ra;

/* Create hash of hash. */
safef(fileName, sizeof(fileName), "%s/%s", rootDir, rootName);
raFoldIn(fileName, hashOfHash);
safef(fileName, sizeof(fileName), "%s/%s/%s", rootDir, org, rootName);
raFoldIn(fileName, hashOfHash);
safef(fileName, sizeof(fileName), "%s/%s/%s/%s", rootDir, org, database, rootName);
raFoldIn(fileName, hashOfHash);
freez(&org);

/* Create list. */
helList = hashElListHash(hashOfHash);
for (hel = helList; hel != NULL; hel = hel->next)
    {
    ra = hel->val;
    slAddHead(&raList, ra);
    hel->val = NULL;
    }
hashElFreeList(&helList);

if (retHashOfHash)
    *retHashOfHash = hashOfHash;
else
    hashFree(&hashOfHash);

return raList;
}

char *addCommasToPos(char *db, char *position)
/* add commas to the numbers in a position
 * returns pointer to static */
{
static char buffer[256];
int winStart, winEnd;
char *chromName;
char num1Buf[64], num2Buf[64]; /* big enough for 2^64 (and then some) */

if (position == NULL)
    return NULL;

buffer[sizeof(buffer) - 1] = 0;
if (!hgParseChromRange(NULL, position, &chromName, &winStart, &winEnd))
    strncpy(buffer, position, sizeof(buffer) - 1);
else
    {
    sprintLongWithCommas(num1Buf, winStart + 1);
    sprintLongWithCommas(num2Buf, winEnd);
    safef(buffer, sizeof(buffer) - 1, "%s:%s-%s",chromName, num1Buf,  num2Buf);
    }

return buffer;
}

static struct grp* loadGrps(char *db, char *confName, char *defaultTbl)
/* load all of the grp rows from a table.  The table name is first looked up
 * in hg.conf with confName. If not there, use defaultTbl.  If the table
 * doesn't exist, return NULL */
{
char query[128];
struct grp *grps = NULL;
char *tbl = cfgOption(confName);
struct slName *tables = NULL, *table;

if (tbl == NULL)
    tbl = defaultTbl;
tables = slNameListFromComma(tbl);
slReverse(&tables);

for (table = tables; table != NULL; table = table->next)
    {
    struct grp *oneTable = NULL;
    char *actualTableName = NULL;
    struct sqlConnection *conn = hAllocConnProfileTbl(db, table->name, &actualTableName);
    if (sqlTableExists(conn, actualTableName))
	{
	safef(query, sizeof(query), "select * from %s", actualTableName);
	oneTable = grpLoadByQuery(conn, query);
	}
    slUniqify(&oneTable, grpCmpName, grpFree);
    if (grps && oneTable)
	grpSuperimpose(&grps, &oneTable);
    else if (!grps)
	grps = oneTable;
    hFreeConn(&conn);
    }
return grps;
}

struct grp* hLoadGrps(char *db)
/* load the grp tables using the list configured in hg.conf, returning a list
 * sorted by priority. */
{
struct grp *grps = loadGrps(db, "db.grp", "grp");
slSort(&grps, grpCmpPriority);
return grps;
}

int chrStrippedCmp(char *chrA, char *chrB)
/*	compare chrom names after stripping chr, Scaffold_ or ps_ prefix
 *	database ci1 has the Scaffold_ prefix, cioSav1 has the ps_
 *	prefix, dp2 has an unusual ContigN_ContigN pattern
 *	all the rest are prefixed chr
 *	This can be used in sort compare functions to order the chroms
 *	by number  (the _random's come out conveniently after everything
 *	else)
 */
{
int dif;
int lenA = 0;
int lenB = 0;

if (startsWith("chr", chrA))
    chrA += strlen("chr");
else if (startsWith("Scaffold_",chrA))
    chrA += strlen("Scaffold_");
else if (startsWith("ps_",chrA))
    chrA += strlen("ps_");

if (startsWith("chr",chrB))
    chrB += strlen("chr");
else if (startsWith("Scaffold_",chrB))
    chrB += strlen("Scaffold_");
else if (startsWith("ps_",chrB))
    chrB += strlen("ps_");

lenA = strlen(chrA);
lenB = strlen(chrB);

dif = lenA - lenB;

if (dif == 0)
    dif = strcmp(chrA, chrB);

return dif;
}


int chrNameCmp(char *str1, char *str2)
/* Compare chromosome or linkage group names by number, then suffix.
 * str1 and str2 must match the regex
 * "(chr|Group)([0-9]+|[A-Za-z0-9]+)(_[A-Za-z0-9_]+)?". */
{
int num1 = 0, num2 = 0;
int match1 = 0, match2 = 0;
char suffix1[512], suffix2[512];

/* get past "chr" or "Group" prefix: */
if (startsWith("chr", str1))
    str1 += 3;
else if (startsWith("Group", str1))
    str1 += 5;
else
    return -1;
if (startsWith("chr", str2))
    str2 += 3;
else if (startsWith("Group", str2))
    str2 += 5;
else
    return 1;
/* If only one is numeric, that one goes first. */
/* If both are numeric, compare by number; if same number, look at suffix. */
/* Otherwise go alph. but put M and U/Un/Un_random at end. */
match1 = sscanf(str1, "%d%s", &num1, suffix1);
match2 = sscanf(str2, "%d%s", &num2, suffix2);
if (match1 && !match2)
    return -1;
else if (!match1 && match2)
    return 1;
else if (match1 && match2)
    {
    int diff = num1 - num2;
    if (diff != 0)
	return diff;
    /* same chrom number... got suffix? */
    if (match1 > 1 && match2 <= 1)
	return 1;
    else if (match1 <= 1 && match2 > 1)
	return -1;
    else if (match1 > 1 && match2 > 1)
	return strcmp(suffix1, suffix2);
    else
	/* This shouldn't happen (duplicate chrom name passed in) */
	return 0;
    }
else if (sameString(str1, "M") && !sameString(str2, "M"))
    return 1;
else if (!sameString(str1, "M") && sameString(str2, "M"))
    return -1;
else if (str1[0] == 'U' && str2[0] != 'U')
    return 1;
else if (str1[0] != 'U' && str2[0] == 'U')
    return -1;
else
    return strcmp(str1, str2);
}

int chrSlNameCmp(const void *el1, const void *el2)
/* Compare chromosome names by number, then suffix.  el1 and el2 must be
 * slName **s (as passed in by slSort) whose names match the regex
 * "chr([0-9]+|[A-Za-z0-9]+)(_[A-Za-z0-9_]+)?". */
{
struct slName *sln1 = *(struct slName **)el1;
struct slName *sln2 = *(struct slName **)el2;
return chrNameCmp(sln1->name, sln2->name);
}

int bedCmpExtendedChr(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart.  Use extended
 * chrom name comparison, that strip prefixes and does numeric compare */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
int dif;
dif = chrNameCmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}

int getTableSize(char *db, char *table)
/* Get count of rows in a table in the primary database */
{
struct sqlConnection *conn = hAllocConn(db);
int ct = sqlTableSize(conn, table);
hFreeConn(&conn);
return ct;
}

int compareDbs(char *dbA, char *dbB)
/* Compare two org# e.g. mm6 vs. mm16 or mm6 vs. hg17
 * Return > 0 if dbA > dbB, < 0 if less than, and 0 if equal */
{
char *dbAOrg = splitOffNonNumeric(dbA);
char *dbBOrg = splitOffNonNumeric(dbB);
int result = strcmp(dbAOrg,dbBOrg);
if (result == 0)
    {
    char *dbANum = splitOffNumber(dbA);
    char *dbBNum = splitOffNumber(dbB);
    result = sqlUnsigned(dbANum) - sqlUnsigned(dbBNum);
    freez(&dbANum);
    freez(&dbBNum);
    }
freez(&dbAOrg);
freez(&dbBOrg);
return result;
}

/* PLEASE NOTE: USE getPfamDomainList() FOR PFAM DOMAINS */
struct slName *getDomainList(struct sqlConnection *conn, char *ucscGeneId,
	char *domainDb)
/* Get list of accessions from external database associated with
 * protein domain entity.  The db parameter can be "Pfam", "Scop", etc. */
{
char query[255];
char lowerCaseName[255];

/* Capitalize first character for description table name */
safef(lowerCaseName, sizeof(lowerCaseName), "%s", domainDb);
lowerCaseName[0] = tolower(lowerCaseName[0]);

safef(query, sizeof(query),
    "select acc from ucsc%s u, %sDesc p"
    " where ucscId  = '%s' and u.domainName=p.name "
    , domainDb, lowerCaseName, ucscGeneId);
return sqlQuickList(conn, query);
}

struct slName *getPfamDomainList(struct sqlConnection *conn, char *ucscGeneId)
/* Get list of accessions from external database associated with
 * Pfam protein domain entity.  */
{
char query[255];
char lowerCaseName[255];

/* Capitalize first character for description table name */
safef(lowerCaseName, sizeof(lowerCaseName), "pfam");
lowerCaseName[0] = tolower(lowerCaseName[0]);

safef(query, sizeof(query),
    "select value from knownToPfam k, %sDesc p"
    " where name = '%s' and value=p.pfamAC "
    , lowerCaseName, ucscGeneId);
return sqlQuickList(conn, query);
}

boolean isUnknownChrom(char *dataBase, char *chromName)
/* Return true if chrom is one of our "unknown" chromomsomes (e.g. chrUn). */
{
return  endsWith(chromName, "_random") || startsWith("chrUn", chromName);
}

char *hGenbankModDate(char *acc, struct sqlConnection *conn)
/* Get string for genbank last modification date, or NULL if not found..
 * Free resulting string. */
{
char query[128];
safef(query, sizeof(query),
      "select moddate from gbCdnaInfo where (acc = '%s')", acc);
return sqlQuickString(conn, query);
}

struct trackDb *findTdbForTable(char *db,struct trackDb *parent,char *table, struct customTrack *(*ctLookupName)(char *table))
/* Find or creates the tdb for this table.  Might return NULL! (e.g. all tables)
 * References an externally defined function ctLookupName() which looks up a
 * custom track by name
 * If this is a custom track, pass in function ctLookupName(table) which looks up a
 * custom track by name, otherwise pass NULL
 */
{
if(isEmpty(table))
    return parent;
struct trackDb *tdb = NULL;
if (isCustomTrack(table))
    {
    if (!ctLookupName)
	errAbort("No custom track lookup function provided\n");
    struct customTrack *ct = (*ctLookupName)(table);
    if (ct != NULL)
        tdb = ct->tdb;
    }
else
    {
    char *track = hGetTrackForTable(db, table);
    if (track == NULL)
        return NULL;
    tdb = tdbFindOrCreate(db,parent,track);
    }
return tdb;
}

char *findTypeForTable(char *db,struct trackDb *parent,char *table, struct customTrack *(*ctLookupName)(char *table))
/* Finds the TrackType for this Table
 * if table has no parent trackDb pass NULL for parent
 * If this is a custom track, pass in function ctLookupName(table) which looks up a
 * custom track by name, otherwise pass NULL
 */
{
struct trackDb *tdb = findTdbForTable(db,parent,table, ctLookupName);
if(tdb)
    return tdb->type;
return (parent?parent->type:NULL);
}

boolean trackIsType(char *database, char *table, struct trackDb *parent, char *type, struct customTrack *(*ctLookupName)(char *table))
/* Return TRUE track is a specific type.  Type should be something like "bed" or
 * "bigBed" or "bigWig"
 * if table has no parent trackDb pass NULL for parent
 * If this is a custom track, pass in function ctLookupName(table) which looks up a
 * custom track by name, otherwise pass NULL
 */
{
char *tdbType = findTypeForTable(database, parent, table, ctLookupName);
return (tdbType && startsWithWord(type, tdbType));
}

boolean hIsBigBed(char *database, char *table, struct trackDb *parent, struct customTrack *(*ctLookupName)(char *table))
/* Return TRUE if table corresponds to a bigBed file.
 * if table has no parent trackDb pass NULL for parent
 * If this is a custom track, pass in function ctLookupName(table) which looks up a
 * custom track by name, otherwise pass NULL
 */
{
return trackIsType(database, table, parent, "bigBed", ctLookupName);
}

char *bbiNameFromSettingOrTable(struct trackDb *tdb, struct sqlConnection *conn, char *table)
/* Return file name from bigDataUrl or little table. */
{
char *fileName = cloneString(trackDbSetting(tdb, "bigDataUrl"));
if (fileName == NULL)
    {
    char query[256];
    safef(query, sizeof(query), "select fileName from %s", table);
    fileName = sqlQuickString(conn, query);
    if (fileName == NULL)
	errAbort("Missing fileName in %s table", table);
    }
return fileName;
}
