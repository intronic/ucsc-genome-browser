/* mdb.h was originally generated by the autoSql program, which also
 * generated mdb.c and mdb.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MDB_H
#define MDB_H

#include "jksql.h"
#define MDB_NUM_COLS 4

struct mdb
/* This contains metadata for a table, file or other predeclared object type. */
    {
    struct mdb *next;  /* Next in singly linked list. */
    char *obj;	/* Object name or ID. */
    char *var;	/* Metadata variable name. */
    char *varType;	/* txt | binary */
    char *val;	/* Metadata value. */
    };

void mdbStaticLoad(char **row, struct mdb *ret);
/* Load a row from mdb table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct mdb *mdbLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all mdb from table that satisfy the query given.
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something =
 * anotherTable.something'.
 * Dispose of this with mdbFreeList(). */

void mdbSaveToDb(struct sqlConnection *conn, struct mdb *el, char *tableName, int updateSize);
/* Save mdb as a row to the table specified by tableName.
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include"
 * If worried about this use mdbSaveToDbEscaped() */

void mdbSaveToDbEscaped(struct sqlConnection *conn, struct mdb *el, char *tableName, int updateSize);
/* Save mdb as a row to the table specified by tableName.
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically
 * escapes all simple strings (not arrays of string) but may be slower than mdbSaveToDb().
 * For example automatically copies and converts:
 * "autosql's features include" --> "autosql\'s features include"
 * before inserting into database. */

struct mdb *mdbLoad(char **row);
/* Load a mdb from row fetched with select * from mdb
 * from database.  Dispose of this with mdbFree(). */

struct mdb *mdbLoadAll(char *fileName);
/* Load all mdb from whitespace-separated file.
 * Dispose of this with mdbFreeList(). */

struct mdb *mdbLoadAllByChar(char *fileName, char chopper);
/* Load all mdb from chopper separated file.
 * Dispose of this with mdbFreeList(). */

#define mdbLoadAllByTab(a) mdbLoadAllByChar(a, '\t');
/* Load all mdb from tab separated file.
 * Dispose of this with mdbFreeList(). */

struct mdb *mdbCommaIn(char **pS, struct mdb *ret);
/* Create a mdb out of a comma separated string.
 * This will fill in ret if non-null, otherwise will
 * return a new mdb */

void mdbFree(struct mdb **pEl);
/* Free a single dynamically allocated mdb such as created
 * with mdbLoad(). */

void mdbFreeList(struct mdb **pList);
/* Free a list of dynamically allocated mdb's */

void mdbOutput(struct mdb *el, FILE *f, char sep, char lastSep);
/* Print out mdb.  Separate fields with sep. Follow last field with lastSep. */

#define mdbTabOut(el,f) mdbOutput(el,f,'\t','\n');
/* Print out mdb as a line in a tab-separated file. */

#define mdbCommaOut(el,f) mdbOutput(el,f,',',',');
/* Print out mdb as a comma separated list including final comma. */

void mdbJsonOutput(struct mdb *el, FILE *f);
/* Print out mdb in JSON format. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#include "trackDb.h"

#define MDB_DEFAULT_NAME "metaDb"

// The mdb holds metadata primarily for tables.
//   Many types of objects could be supported, though currently files are the only other type.
// It is easy to imagine using the current mdb to support hierarchical trees of metadata.
// For example a composite type object called "myComposte" could have metadata that is valid for
// all tables that have the var=composite val=myComposte metadata defined.
//
// There are 2 ways to look at the metadata: By Obj: obj->[var=val] and By Var: var->[val->[obj]].
// By Obj: an object has many var/val pairs but only one val for each unique var.  Querying by
//         object creates a single (2 level) one to many structure.
// By Var: a variable has many possible values and each value may be defined for more than one object.
//         Therefore, querying by var results in a (3 level) one to many to many structure.

enum mdbVarType
// metadata Variavble are only certain declared types
    {
    vtTxt     =0,  // Txt is default
    vtBinary  =1,  // Could support binary blobs
    vtUnknown =99  // Not determined.
    };

struct mdbVar
// The metadata var=val construct. This is contained by mdbObj
    {
    struct mdbVar* next;     // Next in singly linked list of variables
    char *var;               // Metadata variable name.
    enum mdbVarType varType; // txt | binary
    char *val;               // Metadata value.
    };

struct mdbObj
// The standard container of a single object's metadata.
// Also: when searching metadata obj->var->val this is the top struct.
    {
    struct mdbObj* next;     // Next in singly linked list of objects
    char *obj;               // Object name or ID
    boolean deleteThis;      // Used when loading formatted file which may contain delete requests
    struct mdbVar* vars;     // if NOT NULL: list of variables belonging to this object
    struct hash* varHash;    // if NOT NULL: variables are also hashed! (var str to mdbVar struct)
    };

struct mdbLeafObj
// When searching var->val->obj this is the bottom-level obj struct.
    {
    struct mdbLeafObj* next; // Next in singly linked list of variables
    char *obj;               // Object name or ID
    };

struct mdbLimbVal
// When searching var->val->obj this is the mid-level val->obj struct.
    {
    struct mdbLimbVal* next; // Next in singly linked list of variables
    char *val;               // Metadata value.
    struct mdbLeafObj* objs; // if NOT NULL: list of Objects which have this variable
    struct hash* objHash;    // if NOT NULL: hash of objects  (val str to leafObj struct)
    };

struct mdbByVar
// When searching metadata var->val->object this is the top struct
    {
    struct mdbByVar* next;   // Next in singly linked list of variables
    char *var;               // Metadata variable name.
    enum mdbVarType varType; // txt | binary
    boolean notEqual;        // For querying only
    struct mdbLimbVal* vals; // list of values associated with this var
    struct hash* valHash;    // if NOT NULL: hash of vals  (val str to limbVal struct)
    };

// -------------- Enum to Strings --------------
enum mdbVarType mdbVarTypeStringToEnum(char *varType);
// Convert metadata varType string to enum

char *mdbVarTypeEnumToString(enum mdbVarType varType);
// Convert metadata varType enum string

// ------ Parsing lines ------
struct mdbObj *metadataLineParse(char *line);
/* Parses a single formatted metadata line into mdbObj for updates or queries. */

struct mdbByVar *mdbByVarsLineParse(char *line);
/* Parses a line of "var1=val1 var2=val2 into a mdbByVar object for queries. */


// ------ Loading from args, hashes ------
struct mdbObj *mdbObjCreate(char *obj,char *var, char *varType,char *val);
/* Creates a singular mdbObj query object based on obj and all other optional params. */

struct mdbByVar *mdbByVarCreate(char *var, char *varType,char *val);
/* Creates a singular var=val pair struct for metadata queries. */

struct mdbObj *mdbObjsLoadFromHashes(struct hash *objsHash);
// Load all mdbObjs from a file containing metadata formatted lines


// ------ Loading from files ------
struct mdbObj *mdbObjsLoadFromFormattedFile(char *fileName,boolean *validated);
// Load all mdbObjs from a file containing metadata formatted lines
// If requested, will determine if a magic number at the end of the file matches contents

struct mdbObj *mdbObjsLoadFromRAFile(char *fileName,boolean *validated);
// Load all mdbObjs from a file containing RA formatted 'metaObjects'
// If requested, will determine if a magic number at the end of the file matches contents



// ------ Table name and creation ------
void mdbReCreate(struct sqlConnection *conn,char *tblName,boolean testOnly);
// Creates ore Recreates the named mdb.

char*mdbTableName(struct sqlConnection *conn,boolean mySandBox);
// returns the mdb table name or NULL if conn supplied but the table doesn't exist


// -------------- Updating the DB --------------
int mdbObjsSetToDb(struct sqlConnection *conn,char *tableName,struct mdbObj *mdbObjs,boolean replace,boolean testOnly);
// Adds or updates metadata obj/var pairs into the named table.  Returns total rows affected

int mdbObjsLoadToDb(struct sqlConnection *conn,char *tableName,struct mdbObj *mdbObjs,boolean testOnly);
// Adds mdb Objs with minimal error checking

// ------------------ Querys -------------------
struct mdbObj *mdbObjQuery(struct sqlConnection *conn,char *table,struct mdbObj *mdbObj);
// Query the metadata table by obj and optional vars and vals in mdbObj struct.  If mdbObj is NULL query all.
// Returns new mdbObj struct fully populated and sorted in obj,var order.
#define mdbObjsQueryAll(conn,table) mdbObjQuery((conn),(table),NULL)

struct mdbObj *mdbObjQueryByObj(struct sqlConnection *conn,char *table,char *obj,char *var);
// Query a single metadata object and optional var from a table (default mdb).

struct mdbByVar *mdbByVarsQuery(struct sqlConnection *conn,char *table,struct mdbByVar *mdbByVars);
// Query the metadata table by one or more var=val pairs to find the distinct set of objs that satisfy ANY conditions.
// Returns new mdbByVar struct fully populated and sorted in var,val,obj order.
#define mdbByVarsQueryAll(conn,table) mdbByVarsQuery((conn),(table),NULL)

struct mdbByVar *mdbByVarQueryByVar(struct sqlConnection *conn,char *table,char *varName,char *val);
// Query a single metadata variable and optional val from a table (default mdb) for searching val->obj.

struct mdbObj *mdbObjsQueryByVars(struct sqlConnection *conn,char *table,struct mdbByVar *mdbByVars);
// Query the metadata table by one or more var=val pairs to find the distinct set of objs that satisfy ALL conditions.
// Returns new mdbObj struct fully populated and sorted in obj,var order.


// ----------- Printing and Counting -----------
void mdbObjPrint(struct mdbObj *mdbObjs,boolean raStyle);
// prints objs and var=val pairs as formatted metadata lines or ra style

void mdbObjPrintToFile(struct mdbObj *mdbObjs,boolean raStyle, char *file);
// prints (to file) objs and var=val pairs as formatted metadata lines or ra style

void mdbObjPrintToStream(struct mdbObj *mdbObjs,boolean raStyle, FILE *outF);
// prints (to stream) objs and var=val pairs as formatted metadata lines or ra style

char *mdbObjVarValPairsAsLine(struct mdbObj *mdbObj,boolean objTypeExclude);
// returns NULL or a line for a single mdbObj as "var1=val1; var2=val2 ...".  Must be freed.

void mdbByVarPrint(struct mdbByVar *mdbByVars,boolean raStyle);
// prints var=val pairs and objs that go with them single lines or ra style

int mdbObjCount(struct mdbObj *mdbObjs, boolean objs);
// returns the count of vars belonging to this obj or objs;

int mdbByVarCount(struct mdbByVar *mdbByVars,boolean vars, boolean vals);
// returns the count of objs belonging to this set of vars;

// ----------------- Utilities -----------------
struct mdbVar *mdbObjFind(struct mdbObj *mdbObj, char *var);
// Finds the val associated with the var or retruns NULL

char *mdbObjFindValue(struct mdbObj *mdbObj, char *var);
// Finds the val associated with the var or retruns NULL

boolean mdbObjContains(struct mdbObj *mdbObj, char *var, char *val);
// Returns TRUE if object contains var, val or both

boolean mdbObjsContainAtleastOne(struct mdbObj *mdbObjs, char *var);
// Returns TRUE if any object in set contains var

struct mdbObj *mdbObjsCommonVars(struct mdbObj *mdbObjs);
// Returns a new mdbObj with all vars that are contained in every obj passed in.
// Note that the returnd mdbObj has a meaningles obj name and vals.

boolean mdbByVarContains(struct mdbByVar *mdbByVar, char *val, char *obj);
// Returns TRUE if var contains val, obj or both

void mdbObjReorderVars(struct mdbObj *mdbObjs, char *vars,boolean back);
// Reorders vars list based upon list of vars "cell antibody treatment".  Send to front or back.

void mdbObjsSortOnVars(struct mdbObj **mdbObjs, char *vars);
// Sorts on var,val pairs vars lists: fwd case-sensitive.  Assumes all objs' vars are in identical order.
// Optionally give list of vars "cell antibody treatment" to sort on (bringing to front of vars lists).

void mdbObjsSortOnVarPairs(struct mdbObj **mdbObjs,struct slPair *varValPairs);
// Sorts on var,val pairs vars lists: fwd case-sensitive.  Assumes all objs' vars are in identical order.
// This method will use mdbObjsSortOnVars()

void mdbObjRemoveVars(struct mdbObj *mdbObjs, char *vars);
// Prunes list of vars for an object, freeing the memory.  Doesn't touch DB.

char *mdbRemoveCommonVar(struct mdbObj *mdbList, char *var);
// Removes var from set of mdbObjs but only if all that hav it have a commmon val
// Returns the val if removed, else NULL

void mdbObjSwapVars(struct mdbObj *mdbObjs, char *vars,boolean deleteThis);
// Replaces objs' vars with var=vap pairs provided, preparing for DB update.

struct mdbObj *mdbObjsFilter(struct mdbObj **pMdbObjs, char *var, char *val,boolean exclude);
// Filters mdb objects to only those that include/exclude vars.  Optionally checks val too.  Frees removed objects

struct mdbObj *mdbObjsFilterTablesOrFiles(struct mdbObj **pMdbObjs,boolean table, boolean files);
// Filters mdb objects to only those that have associated tables or files. Returns removed non-table/file objects
// Note: Since table/file objects overlap, there are 3 possibilites: tables, files, table && files

struct mdbObj *mdbObjIntersection(struct mdbObj **a, struct mdbObj *b);
// return duplicate objs from an intersection of two mdbObj lists.
// List b is untouched but pA will contain the resulting intersection

void mdbObjTransformToUpdate(struct mdbObj *mdbObjs, char *var, char *varType,char *val,boolean deleteThis);
// Turns one or more mdbObjs into the stucture needed to add/update or delete.

struct mdbObj *mdbObjClone(const struct mdbObj *mdbObj);
// Clones a single mdbObj, including hash and maintining order

struct slName *mdbObjToSlName(struct mdbObj *mdbObjs);
// Creates slNames list of mdbObjs->obj.  mdbObjs remains untouched

int mdbVarCmp(const void *va, const void *vb);
/* Compare to sort on label. */


// --------------- Free at last ----------------
void mdbObjsFree(struct mdbObj **mdbObjsPtr);
// Frees one or more metadata objects and any contained mdbVars.  Will free any hashes as well.

void mdbByVarsFree(struct mdbByVar **mdbByVarsPtr);
// Frees one or more metadata vars and any contained vals and objs.  Will free any hashes as well.


// ----------------- CGI specific routines for use with tdb -----------------
const struct mdbObj *metadataForTable(char *db,struct trackDb *tdb,char *table);
// Returns the metadata for a table.  NEVER FREE THIS STRUCT!
// This is the main routine for CGIs to access metadata

const char *metadataFindValue(struct trackDb *tdb, char *var);
// Finds the val associated with the var or retruns NULL


#define MDB_VAL_STD_TRUNCATION 64
struct mdbObj *mdbObjSearch(struct sqlConnection *conn, char *var, char *val, char *op, int limit);
// Search the metaDb table for objs by var and val.  Can restrict by op "is", "like", "in" and accept (non-zero) limited string size
// Search is via mysql, so it's case-insensitive.  Return is sorted on obj.

struct mdbObj *mdbObjRepeatedSearch(struct sqlConnection *conn,struct slPair *varValPairs,boolean tables,boolean files);
// Search the metaDb table for objs by var,val pairs.  Uses mdbCvSearchMethod() if available.
// This method will use mdbObjsQueryByVars()

struct slName *mdbObjNameSearch(struct sqlConnection *conn, char *var, char *val, char *op, int limit, boolean tables, boolean files);
// Search the metaDb table for objs by var and val.  Can restrict by op "is" or "like" and accept (non-zero) limited string size
// Search is via mysql, so it's case-insensitive.  Return is sorted on obj.

struct slName *mdbValSearch(struct sqlConnection *conn, char *var, int limit, boolean tables, boolean files);
// Search the metaDb table for vals by var.  Can impose (non-zero) limit on returned string size of val
// Search is via mysql, so it's case-insensitive.  Return is sorted on val.

struct slPair *mdbValLabelSearch(struct sqlConnection *conn, char *var, int limit, boolean tables, boolean files);
// Search the metaDb table for vals by var and returns controlled vocabulary (cv) label
// (if it exists) and val as a pair.  Can impose (non-zero) limit on returned string size of name.
// Return is case insensitive sorted on name (label or else val).

struct hash *mdbCvTermHash(char *term);
// returns a hash of hashes of a term which should be defined in cv.ra

struct hash *mdbCvTermTypeHash();
// returns a hash of hashes of mdb and controlled vocabulary (cv) term types
// Those terms should contain label,descrition,searchable,cvDefined,hidden

struct slPair *mdbCvWhiteList(boolean searchTracks, boolean cvLinks);
// returns the official mdb/controlled vocabulary terms that have been whitelisted for certain uses.

enum mdbCvSearchable
// metadata Variavble are only certain declared types
    {
    cvsNotSearchable        =0,  // Txt is default
    cvsSearchByMultiSelect  =1,  // Search by drop down multi-select of supplied list (NOT YET IMPLEMENTED)
    cvsSearchBySingleSelect =2,  // Search by drop down single-select of supplied list
    cvsSearchByFreeText     =3,  // Search by free text field (NOT YET IMPLEMENTED)
    cvsSearchByDateRange    =4,  // Search by discovered date range (NOT YET IMPLEMENTED)
    cvsSearchByIntegerRange =5   // Search by discovered integer range (NOT YET IMPLEMENTED)
    };

enum mdbCvSearchable mdbCvSearchMethod(char *term);
// returns whether the term is searchable // TODO: replace with mdbCvWhiteList() returning struct

const char *cvLabel(char *term);
// returns cv label if term found or else just term

int mdbObjsValidate(struct mdbObj *mdbObjs);
// Validates vars and vals against cv.ra.  Returns count of errors found

#endif /* MDB_H */

