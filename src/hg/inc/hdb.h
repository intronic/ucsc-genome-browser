/* hdb - human genome browser database. */

#ifndef HDB_H
#define HDB_H

#ifndef DNAUTIL_H
#include "dnautil.h"
#endif

#ifndef DNASEQ_H
#include "dnaseq.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

#ifndef SUBTEXT_H
#include "subText.h"
#endif

#ifndef JKSQL_H
#include "jksql.h"
#endif 

#ifndef TRACKDB_H
#include "trackDb.h"
#endif

#ifndef HGRELATE_H
#include "hgRelate.h"
#endif

#ifndef BED_H
#include "bed.h"
#endif

struct blatServerTable
/* Information about a BLAT server. */
{
    char *db;		/* Database name. */
    char *genome;	/* Genome name. */
    boolean isTrans;	/* Is tranlated to protein? */
    char *host;		/* Name of machine hosting server. */
    char *port;		/* Port that hosts server. */
    char *nibDir;	/* Directory of sequence files. */
};

struct hTableInfo
/* Some info on a track table, extracted from its field names. */
    {
    struct hTableInfo *next;	/* Next in list. */
    char *rootName;		/* Name without chrN_. */
    boolean isPos;		/* True if table is positional. */
    boolean isSplit;		/* True if table is split. */
    boolean hasBin;		/* True if table starts with field. */
    char chromField[32];	/* Name of chromosome field. */
    char startField[32];	/* Name of chromosome start field. */
    char endField[32];		/* Name of chromosome end field. */
    char nameField[32];		/* Name of item name field. */
    char scoreField[32];	/* Name of score field. */
    char strandField[32];	/* Name of strand field. */
    char cdsStartField[32];	/* Name of cds(thick)Start field. */
    char cdsEndField[32];	/* Name of cds(thick)End field. */
    char countField[32];	/* Name of exon(block)Count field. */
    char startsField[32];	/* Name of exon(block)Starts field. */
    char endsSizesField[32];	/* Name of exon(block)Ends(Sizes) field. */
    boolean hasCDS;		/* True if it has cdsStart,cdsEnd fields. */
    boolean hasBlocks;		/* True if it has count,starts,endsSizes. */
    char *type;			/* A guess at the trackDb type for this. */
    };

void hDefaultConnect();
/* read the default settings from the config file */

char *hTrackDbName();
/* return the name of the track database from the config file. Freez when done */

char *hPdbFromGdb(char *genomeDb);
/* return the name of the proteome database given the genome database name */

void hSetDbConnect(char* host, char *db, char *user, char *password);
/* set the connection information for the database */

void hSetDbConnect2(char* host, char *db, char *user, char *password);
/* set the connection information for the database */

boolean hDbExists(char *database);
/* Function to check if this is a valid db name */

void hSetDb(char *dbName);
/* Set the database name. */

void hSetDb2(char *dbName);
/* Set the database name. */

char *hDefaultDb();
/* Return the default db if all else fails */

char *hDefaultChrom();
/* Return the first chrom in chromInfo from the current db. */

char *hGetDb();
/* Return the current database name. */

char *hGetDb2();
/* Return the secondary database name. */

char *hGetDbHost();
/* Return the current database host. */

char *hGetDbName();
/* Return the current database name. */

char *hGetDbUser();
/* Return the current database user. */

char *hGetDbPassword();
/* Return the current database password. */

struct sqlConnection *hAllocConn();
/* Get free connection if possible. If not allocate a new one. */

struct sqlConnection *hAllocConn2();
/* Get free connection if possible. If not allocate a new one. */

struct sqlConnection *hAllocConnDb(char *db);
/* Get free connection for a database if possible. If not allocate a new one. */

void hFreeConn(struct sqlConnection **pConn);
/* Put back connection for reuse. */

void hFreeConn2(struct sqlConnection **pConn);
/* Put back secondary db connection for reuse. */

struct sqlConnection *hConnectCentral();
/* Connect to central database where user info and other info
 * not specific to a particular genome lives.  Free this up
 * with hDisconnectCentral(). */

void hDisconnectCentral(struct sqlConnection **pConn);
/* Put back connection for reuse. */

char *hgOfficialChromName(char *name);
/* Returns "cannonical" name of chromosome or NULL
 * if not a chromosome. */

boolean hgNearOk(char *database);
/* Return TRUE if ok to put up familyBrowser (hgNear) 
 * on this database. */

boolean hTableExists(char *table);
/* Return TRUE if a table exists in database. */

boolean hTableExists2(char *table);
/* Return TRUE if a table exists in secondary database. */

boolean hTableExistsDb(char *db, char *table);
/* Return TRUE if a table exists in db. */

boolean hColExists(char *table, char *column);
/* Return TRUE if a column exists in table. */

boolean hColExistsDb(char *db, char *table, char *column);
/* Return TRUE if a column exists in table in db. */


void hParseTableName(char *table, char trackName[128], char chrom[32]);
/* Parse an actual table name like "chr17_random_blastzWhatever" into 
 * the track name (blastzWhatever) and chrom (chr17_random). */

int hdbChromSize(char *db, char *chromName);
/* Get chromosome size from given database . */

int hChromSize(char *chromName);
/* Return size of chromosome. */

int hChromSize2(char *chromName);
/* Return size of chromosome from secondary database. */

struct dnaSeq *hChromSeq(char *chrom, int start, int end);
/* Return lower case DNA from chromosome. */

struct dnaSeq *hSeqForBed(struct bed *bed);
/* Get the sequence associated with a particular bed concatenated together. */

boolean hChromBand(char *chrom, int pos, char retBand[64]);
/* Fill in text string that says what band pos is on. 
 * Return FALSE if not on any band, or table missing. */

boolean hChromBandConn(struct sqlConnection *conn, 
	char *chrom, int pos, char retBand[64]);
/* Fill in text string that says what band pos is on. 
 * Return FALSE if not on any band, or table missing. */

boolean hChromBand(char *chrom, int pos, char retBand[64]);
/* Fill in text string that says what band pos is on. 
 * Return FALSE if not on any band, or table missing. */

boolean hScaffoldPos(char *chrom, int start, int end,
                        char **retScaffold, int *retStart, int *retEnd);
/* Return the scaffold, and start end coordinates on a scaffold, for
 * a chromosome range.  If the range extends past end of a scaffold,
 * it is truncated to the scaffold end.
 * Return FALSE if unable to convert */

char table[64];
struct dnaSeq *hDnaFromSeq(char *seqName, 
	int start, int end, enum dnaCase dnaCase);
/* Fetch DNA in a variety of cases.  */

struct dnaSeq *hLoadChrom(char *chromName);
/* Fetch entire chromosome into memory. */

void hNibForChrom(char *chromName, char retNibName[512]);
/* Get .nib file associated with chromosome. */

struct slName *hAllChromNames();
/* Get list of all chromosomes. */

struct slName *hAllChromNamesDb(char *db);
/* Get list of all chromosomes in database. */

char *hExtFileName(char *extFileTable, unsigned extFileId);
/* Get external file name from table and ID.  Typically
 * extFile table will be 'extFile' or 'gbExtFile'
 * Abort if the id is not in the table or if the file
 * fails size check.  Please freeMem the result when you 
 * are done with it. */

int hRnaSeqAndIdx(char *acc, struct dnaSeq **retSeq, HGID *retId, char *gbdate, struct sqlConnection *conn);
/* Return sequence for RNA, it's database ID, and optionally genbank 
 * modification date. Return -1 if not found. */

char* hGetSeqAndId(struct sqlConnection *conn, char *acc, HGID *retId);
/* Return sequence as a fasta record in a string and it's database ID, or 
 * NULL if not found. */

struct dnaSeq *hExtSeq(char *acc);
/* Return sequence for external seq. */

struct dnaSeq *hRnaSeq(char *acc);
/* Return sequence for RNA. */

aaSeq *hPepSeq(char *acc);
/* Return sequence for a peptide. */

boolean hGenBankHaveSeq(char *acc, char *compatTable);
/* Get a GenBank or RefSeq mRNA or EST sequence or NULL if it doesn't exist.
 * This handles compatibility between pre-incremental genbank databases where
 * refSeq sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */

struct dnaSeq *hGenBankGetMrna(char *acc, char *compatTable);
/* Get a GenBank or RefSeq mRNA or EST sequence or NULL if it doesn't exist.
 * This handles compatibility between pre-incremental genbank databases where
 * refSeq sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */

aaSeq *hGenBankGetPep(char *acc, char *compatTable);
/* Get a RefSeq peptide sequence or NULL if it doesn't exist.  This handles
 * compatibility between pre-incremental genbank databases where refSeq
 * sequences were stored in tables and the newer scheme that keeps all
 * sequences in external files.  If compatTable is not NULL and the table
 * exists, it is used to obtain the sequence.  Otherwise the seq and gbSeq
 * tables are checked.
 */

struct bed *hGetBedRange(char *table, char *chrom, int chromStart,
			 int chromEnd, char *sqlConstraints);
/* Return a bed list of all items (that match sqlConstraints, if nonNULL) 
   in the given range in table. */

struct bed *hGetBedRangeDb(char *db, char *table, char *chrom, int chromStart,
			   int chromEnd, char *sqlConstraints);
/* Return a bed list of all items (that match sqlConstraints, if nonNULL) 
   in the given range in table. */

struct hash *hCtgPosHash();
/* Return hash of ctgPos from current database keyed by contig name. */

char *hFreezeFromDb(char *database);
/* return the freeze for the database version. 
   For example: "hg6" returns "Dec 12, 2000". If database
   not recognized returns NULL */

char *hDbFromFreeze(char *freeze);
/* Return database version from freeze name. */

struct slName *hDbList();
/* List of all database versions that are online (database
 * names only).  See also hDbDbList. */

struct dbDb *hDbDbList();
/* Return list of databases that are actually online. 
 * The list includes the name, description, and where to
 * find the nib-formatted DNA files. Free this with dbDbFree. */

boolean hIsPrivateHost();
/* Return TRUE if this is running on private web-server. */

boolean hIsMgscHost();
/* Return TRUE if this is running on web server only
 * accessible to Mouse Genome Sequencing Consortium. */

boolean hTrackOnChrom(struct trackDb *tdb, char *chrom);
/* Return TRUE if track exists on this chromosome. */

struct trackDb *hTrackDb(char *chrom);
/* Load tracks associated with current chromosome (which may
 * be NULL */

void hFindDefaultChrom(char *db, char defaultChrom[64]);
/* Find chromosome to use if none specified. */

struct hTableInfo *hFindTableInfo(char *chrom, char *rootName);
/* Find table information.  Return NULL if no table. */

struct hTableInfo *hFindTableInfoDb(char *db, char *chrom, char *rootName);
/* Find table information in specified db.  Return NULL if no table. */

boolean hFindChromStartEndFields(char *table, 
	char retChrom[32], char retStart[32], char retEnd[32]);
/* Given a table return the fields for selecting chromosome, start, and end. */

boolean hFindChromStartEndFieldsDb(char *db, char *table, 
	char retChrom[32], char retStart[32], char retEnd[32]);
/* Given a table return the fields for selecting chromosome, start, and end. */

boolean hFindBed12Fields(char *table, 
	char retChrom[32], char retStart[32], char retEnd[32],
	char retName[32], char retScore[32], char retStrand[32],
        char retCdsStart[32], char retCdsEnd[32],
	char retCount[32], char retStarts[32], char retEndsSizes[32]);
/* Given a table return the fields corresponding to all the bed 12 
 * fields, if they exist.  Fields that don't exist in the given table 
 * will be set to "". */

boolean hIsBinned(char *table);
/* Return TRUE if a table is binned. */

int hFieldIndex(char *table, char *field)
/* Return index of field in table or -1 if it doesn't exist. */;

boolean hHasField(char *table, char *field);
/* Return TRUE if table has field */

boolean hFindFieldsAndBin(char *table, 
	char retChrom[32], char retStart[32], char retEnd[32],
	boolean *retBinned);
/* Given a table return the fields for selecting chromosome, start, end,
 * and whether it's binned . */

boolean hFindSplitTable(char *chrom, char *rootName, 
	char retTableBuf[64], boolean *hasBin);
/* Find name of table that may or may not be split across chromosomes. 
 * Return FALSE if table doesn't exist.  */

boolean hFindSplitTableDb(char *db, char *chrom, char *rootName, 
	char retTableBuf[64], boolean *hasBin);
/* Find name of table in a given database that may or may not 
 * be split across chromosomes. Return FALSE if table doesn't exist.  */

boolean hTrackExists(char *trackName);
/* Return TRUE if track exists. */

int hBinLevels();
/* Return number of levels to bins. */

int hBinFirstShift();
/* Return amount to shift a number to get to finest bin. */

int hBinNextShift();
/* Return amount to shift a numbe to get to next coarser bin. */

int hFindBin(int start, int end);
/* Given start,end in chromosome coordinates assign it
 * a bin.   There's a bin for each 128k segment, for each
 * 1M segment, for each 8M segment, for each 64M segment,
 * and for each chromosome (which is assumed to be less than
 * 512M.)  A range goes into the smallest bin it will fit in. */

void hAddBinToQueryGeneral(char *binField, int start, int end, struct dyString *query);
/* Add clause that will restrict to relevant bins to query. allow bin field name to be specified */

void hAddBinToQuery(int start, int end, struct dyString *query);
/* Add clause that will restrict to relevant bins to query. */

struct sqlResult *hRangeQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	int start, int end, char *extraWhere, int *retRowOffset);
/* Construct and make a query to tables that may be split and/or
 * binned. */

struct sqlResult *hOrderedRangeQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	int start, int end, char *extraWhere, int *retRowOffset);
/* Construct and make a query to tables that may be split and/or
 * binned. Forces return values to be sorted by chromosome start. */

struct sqlResult *hChromQuery(struct sqlConnection *conn,
	char *rootTable, char *chrom,
	char *extraWhere, int *retRowOffset);
/* Construct and make a query across whole chromosome to tables 
 * that may be split and/or
 * binned. */

int hOffsetPastBin(char *chrom, char *table);
/* Return offset into a row of table that skips past bin
 * field if any. */

boolean hgParseChromRange(char *spec, char **retChromName, 
	int *retWinStart, int *retWinEnd);
/* Parse something of form chrom:start-end into pieces. */

boolean hgIsChromRange(char *spec);
/* Returns TRUE if spec is chrom:N-M for some human
 * chromosome chrom and some N and M. */

struct trackDb *hMaybeTrackInfo(struct sqlConnection *conn, char *trackName);
/* Look up track in database, return NULL if it's not there. */

struct trackDb *hTrackInfo(struct sqlConnection *conn, char *trackName);
/* Look up track in database, errAbort if it's not there. */

boolean hTrackCanPack(char *trackName);
/* Return TRUE if this track can be packed. */

char *hTrackOpenVis(char *trackName);
/* Return "pack" if track is packable, otherwise "full". */

struct dbDb *hGetIndexedDatabases();
/* Get list of databases for which there is a nib dir. 
 * Dispose of this with dbDbFreeList. */

struct dbDb *hGetAxtInfoDbs();
/* Get list of db's where we have axt files listed in axtInfo . 
 * The db's with the same organism as organism go last.
 * Dispose of this with dbDbFreeList. */

struct axtInfo *hGetAxtAlignments(char *db);
/* Get list of alignments where we have axt files listed in axtInfo . 
 * Dispose of this with axtInfoFreeList. */

struct dbDb *hGetBlatIndexedDatabases();
/* Get list of databases for which there is a BLAT index. 
 * Dispose of this with dbDbFreeList. */

boolean hIsBlatIndexedDatabase(char *db);
/* Return TRUE if have a BLAT server on sequence corresponding 
 * to give database. */

struct blatServerTable *hFindBlatServer(char *db, boolean isTrans);
/* return the blat server information corresponding to the database */


char *hDefaultPos(char *database);
/* param database - The database within which to look up this position.
   return - default chromosome position associated with database.
    Use freeMem on return value when done.
 */

char *hOrganism(char *database);
/* Return organism associated with database.   Use freeMem on
 * return value when done. */

int hOrganismID(char *database);
/* Get organism ID from relational organism table */
/* Return -1 if not found */

char *hScientificName(char *database);
/* Return scientific name for organism represented by this database */
/* Return NULL if unknown database */
/* NOTE: must free returned string after use */

char *hHtmlPath(char *database);
/* Return /gbdb path name to html description for this database */
/* Return NULL if unknown database */
/* NOTE: must free returned string after use */

char *hFreezeDate(char *database);
/* Return freeze date of database. Use freeMem when done. */

char *hGenome(char *database);
/* Return genome associated with database.   Use freeMem on
 * return value when done. */

void hAddDbSubVars(char *prefix, char *database, struct subText **pList);
/* Add substitution variables associated with database to list. */

void hLookupStringsInTdb(struct trackDb *tdb, char *database);
/* Lookup strings in track database. */

char *hDefaultDbForGenome(char *genome);
/*
Purpose: Return the default database matching the organism.

param organism - The organism for which we are trying to get the 
    default database.
return - The default database name for this organism
 */

char *sqlGetField(struct sqlConnection *connIn, 
   	          char *dbName, char *tblName, char *fldName, 
  	          char *condition);
/* Return a single field from the database, given database name, 
   table name, field name, and a condition string */

struct hash *hChromSizeHash(char *db);
/* Get hash of chromosome sizes for database.  Just hashFree it when done. */

struct mafAli *mafLoadInRegion(struct sqlConnection *conn, char *table,
	char *chrom, int start, int end);
/* Return list of alignments in region. */

struct mafAli *axtLoadAsMafInRegion(struct sqlConnection *conn, char *table,
	char *chrom, int start, int end, 
	char *tPrefix, char *qPrefix, int tSize,  struct hash *qSizeHash);
/* Return list of alignments in region from axt external file as a maf. */

char *hgDirForOrg(char *org);
/* Make directory name from organism name - getting
 * rid of dots and spaces. */

struct hash *hgReadRa(char *genome, char *database, char *rootDir, 
	char *rootName, struct hash **retHashOfHash);
/* Read in ra in root, root/org, and root/org/database. 
 * Returns a list of hashes, one for each ra record.  Optionally
 * if retHashOfHash is non-null it returns there a
 * a hash of hashes keyed by the name field in each
 * ra sub-hash. */


#endif /* HDB_H */
