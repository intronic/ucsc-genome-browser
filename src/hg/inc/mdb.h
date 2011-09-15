/* mdb.h was originally generated by the autoSql program, which also
 * generated mdb.c and mdb.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MDB_H
#define MDB_H

#include "jksql.h"
#define MDB_NUM_COLS 3

struct mdb
/* This contains metadata for a table, file or other predeclared object type. */
    {
    struct mdb *next;  /* Next in singly linked list. */
    char *obj;	/* Object name or ID. */
    char *var;	/* Metadata variable name. */
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

#include "cv.h"
#include "trackDb.h"

#define MDB_DEFAULT_NAME     "metaDb"

// The three mdb tuples
#define MDB_OBJ                 "obj"
#define MDB_VAR                 "var"
#define MDB_VAL                 "val"

// OBJECT TYPES
#define MDB_OBJ_TYPE            "objType"
#define MDB_OBJ_TYPE_TABLE      "table"
#define MDB_OBJ_TYPE_FILE       "file"
#define MDB_OBJ_TYPE_COMPOSITE  "composite"

// WELL KNOWN MDB VARS
#define MDB_VAR_PI              CV_TERM_GRANT
#define MDB_VAR_LAB             CV_TERM_LAB
#define MDB_VAR_COMPOSITE       MDB_OBJ_TYPE_COMPOSITE
#define MDB_VAR_ANTIBODY        CV_TERM_ANTIBODY
#define MDB_VAR_CELL            CV_TERM_CELL
#define MDB_VAR_DATATYPE        CV_TERM_DATA_TYPE
#define MDB_VAR_VIEW            CV_TERM_VIEW
#define MDB_VAR_CONTROL         CV_TERM_CONTROL
#define MDB_VAR_TABLENAME       "tableName"
#define MDB_VAR_FILENAME        "fileName"
#define MDB_VAR_MD5SUM          "md5sum"
#define MDB_VAR_FILEINDEX       "fileIndex"
#define MDB_VAR_DCC_ACCESSION   "dccAccession"
#define MDB_VAR_PROJECT         "project"
#define MDB_VAR_REPLICATE       "replicate"
#define MDB_VAR_LAB_VERSION     "labVersion"
#define MDB_VAR_SOFTWARE_VERSION "softwareVersion"
#define MDB_VAR_SUBMIT_VERSION  "submittedDataVersion"

// ENCODE Specific (at least for now)
#define MDB_VAL_ENCODE_PROJECT  "wgEncode"
#define MDB_VAR_ENCODE_SUBID    "subId"
#define MDB_VAR_ENCODE_EDVS     "expVars"
#define MDB_VAR_ENCODE_EXP_ID   "expId"
#define MDB_VAL_ENCODE_EDV_NONE "None"


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

struct mdbVar
// The metadata var=val construct. This is contained by mdbObj
    {
    struct mdbVar* next;     // Next in singly linked list of variables
    char *var;               // Metadata variable name.
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
    boolean notEqual;        // For querying only
    struct mdbLimbVal* vals; // list of values associated with this var
    struct hash* valHash;    // if NOT NULL: hash of vals  (val str to limbVal struct)
    };

// ------ Parsing lines ------
struct mdbObj *metadataLineParse(char *line);
/* Parses a single formatted metadata line into mdbObj for updates or queries. */

struct mdbByVar *mdbByVarsLineParse(char *line);
/* Parses a line of "var1=val1 var2=val2 into a mdbByVar object for queries. */


// ------ Loading from args, hashes ------
struct mdbObj *mdbObjCreate(char *obj,char *var, char *val);
/* Creates a singular mdbObj query object based on obj and all other optional params. */

struct mdbObj *mdbObjNew(char *obj,struct mdbVar *mdbVars);
// Returns a new mdbObj with whatever was passed in.
// An mdbObj requires and obj, so if one is not supplied it will be "[unknown]"

struct mdbByVar *mdbByVarCreate(char *var, char *val);
/* Creates a singular var=val pair struct for metadata queries. */

boolean mdbByVarAppend(struct mdbByVar *mdbByVars,char *var, char *val,boolean notEqual);
/* Adds a another var to a list of mdbByVar pairs to be used in metadata queries. */

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

struct mdbObj *mdbObjsQueryByVarValString(struct sqlConnection *conn,char *tableName,char *varVals);
// returns mdbObjs matching varVals in form of: [var1=val1 var2=val2a,val2b var3=v%3 var4="val 4" var5!=val5 var6=]
//   var2=val2a,val2b: matches asny of comma separated list
//   var3=v%3        : matches '%' and '?' wild cards.
//   var4="val 4"    : matches simple double quoted strings.
//   var5!=val5      : matches not equal.
//   var6=           : matches that var exists (same as var6=%). var6!= also works.
#define mdbObjsQueryByVarVals(conn,varVals) mdbObjsQueryByVarValString((conn),NULL,(varVals))

struct mdbObj *mdbObjsQueryByVarPairs(struct sqlConnection *conn,char *tableName,struct slPair *varValPairs);
// returns mdbObjs matching varValPairs provided.
//   The != logic of mdbObjsQueryByVarValString() is not possible, but other cases are supported:
//   as val may be NULL, a comma delimited list, double quoted string, containing wilds: % and ?
#define mdbObjsQueryByVarValPairs(conn,varValPairs) mdbObjsQueryByVarPairs((conn),NULL,(varValPairs))

struct mdbObj *mdbObjQueryCompositeObj(struct sqlConnection *conn,char *tableName,struct mdbObj *mdbObj);
// returns NULL or the composite mdbObj associated with the object passed in.
#define mdbObjQueryComposite(conn,mdbObj) mdbObjQueryCompositeObj((conn),NULL,(mdbObj))


// ----------- Printing and Counting -----------
void mdbObjPrint(struct mdbObj *mdbObjs,boolean raStyle);
// prints objs and var=val pairs as formatted metadata lines or ra style

void mdbObjPrintToFile(struct mdbObj *mdbObjs,boolean raStyle, char *file);
// prints (to file) objs and var=val pairs as formatted metadata lines or ra style

void mdbObjPrintToStream(struct mdbObj *mdbObjs,boolean raStyle, FILE *outF);
// prints (to stream) objs and var=val pairs as formatted metadata lines or ra style

int mdbObjPrintToTabFile(struct mdbObj *mdbObjs, char *file);
// prints all objs as tab delimited obj var val into file for SQL LOAD DATA.  Returns count.

void mdbObjPrintOrderedToStream(FILE *outF,struct mdbObj **mdbObjs,char *order, char *seperator, boolean header);
// prints mdbObjs as a table, but only the vars listed in comma delimited order.
// Examples of seperator: " " "\t\t" or "<TD>", in which case this is an HTML table.
// mdbObjs list will be reordered. Sort fails when vars are missing in objs.

char *mdbObjVarValPairsAsLine(struct mdbObj *mdbObj,boolean objTypeExclude,boolean cvLabels);
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

struct slName *mdbObjsFindAllVals(struct mdbObj *mdbObjs, char *var);
// Returns a list of all vals in mdbObjs for a requested var

boolean mdbObjContains(struct mdbObj *mdbObj, char *var, char *val);
// Returns TRUE if object contains var, val or both

boolean mdbObjsContainAltleastOneMatchingVar(struct mdbObj *mdbObjs, char *var, char *val);
// Returns TRUE if any object in set contains var
#define mdbObjsContainAtleastOne(mdbObjs, var) mdbObjsContainAltleastOneMatchingVar((mdbObjs),(var),NULL)

struct mdbObj *mdbObjsCommonVars(struct mdbObj *mdbObjs);
// Returns a new mdbObj with all vars that are contained in every obj passed in.
// Note that the returnd mdbObj has a meaningles obj name and vals.

boolean mdbByVarContains(struct mdbByVar *mdbByVar, char *val, char *obj);
// Returns TRUE if var contains val, obj or both

void mdbObjReorderVars(struct mdbObj *mdbObjs, char *vars,boolean back);
// Reorders vars list based upon list of vars "cell antibody treatment".  Send to front or back.

void mdbObjReorderByCv(struct mdbObj *mdbObjs, boolean includeHidden);
// Reorders vars list based upon cv.ra typeOfTerms priority

void mdbObjsSortOnVars(struct mdbObj **mdbObjs, char *vars);
// Sorts on var,val pairs vars lists: fwd case-sensitive.  Assumes all objs' vars are in identical order.
// Optionally give list of vars "cell antibody treatment" to sort on (bringing to front of vars lists).

void mdbObjsSortOnVarPairs(struct mdbObj **mdbObjs,struct slPair *varValPairs);
// Sorts on var,val pairs vars lists: fwd case-sensitive.  Assumes all objs' vars are in identical order.
// This method will use mdbObjsSortOnVars()

void mdbObjsSortOnCv(struct mdbObj **mdbObjs, boolean includeHidden);
// Puts obj->vars in order based upon cv.ra typeOfTerms priority,
//  then case-sensitively sorts all objs in list based upon that var order.

void mdbObjRemoveVars(struct mdbObj *mdbObjs, char *vars);
// Prunes list of vars for an object, freeing the memory.  Doesn't touch DB.

void mdbObjRemoveHiddenVars(struct mdbObj *mdbObjs);
// Prunes list of vars for mdb objs that have been declared as hidden in cv.ra typeOfTerms

boolean mdbObjsHasCommonVar(struct mdbObj *mdbList, char *var, boolean missingOk);
// Returns TRUE if all mbObjs passed in have the var with the same value

char *mdbRemoveCommonVar(struct mdbObj *mdbList, char *var);
// Removes var from set of mdbObjs but only if all that hav it have a commmon val
// Returns the val if removed, else NULL

boolean mdbObjSetVar(struct mdbObj *mdbObj, char *var,char *val);
// Sets the string value to a single var in an obj, preparing for DB update.
// returns TRUE if updated, FALSE if added

boolean mdbObjSetVarInt(struct mdbObj *mdbObj, char *var,int val);
// Sets an integer value to a single var in an obj, preparing for DB update.
// returns TRUE if updated, FALSE if added

void mdbObjSwapVars(struct mdbObj *mdbObjs, char *vars,boolean deleteThis);
// Replaces objs' vars with var=vap pairs provided, preparing for DB update.

struct mdbObj *mdbObjsFilter(struct mdbObj **pMdbObjs, char *var, char *val,boolean returnMatches);
// Filters mdb objects to only those that include/exclude vars.  Optionally checks val too.
// Returns matched or unmatched items objects as requested, maintaining sort order

struct mdbObj *mdbObjsFilterByVars(struct mdbObj **pMdbObjs,char *vars,boolean noneEqualsNotFound,boolean returnMatches);
// Filters mdb objects to only those that include/exclude var=val pairs (e.g. "var1=val1 var2 var3!=val3 var4=None").
// Supports != ("var!=" means var not found). Optionally supports var=None equal to var is not found
// Returns matched or unmatched items objects as requested.  Multiple passes means sort order is destroyed.

struct mdbObj *mdbObjsFilterTablesOrFiles(struct mdbObj **pMdbObjs,boolean table, boolean files);
// Filters mdb objects to only those that have associated tables or files. Returns removed non-table/file objects
// Note: Since table/file objects overlap, there are 3 possibilites: tables, files, table && files

struct mdbObj *mdbObjIntersection(struct mdbObj **a, struct mdbObj *b);
// return duplicate objs from an intersection of two mdbObj lists.
// List b is untouched but pA will contain the resulting intersection

void mdbObjTransformToUpdate(struct mdbObj *mdbObjs, char *var, char *val,boolean deleteThis);
// Turns one or more mdbObjs into the stucture needed to add/update or delete.

struct mdbObj *mdbObjClone(const struct mdbObj *mdbObj);
// Clones a single mdbObj, including hash and maintining order

struct slName *mdbObjToSlName(struct mdbObj *mdbObjs);
// Creates slNames list of mdbObjs->obj.  mdbObjs remains untouched

int mdbVarCmp(const void *va, const void *vb);
/* Compare to sort on label. */


// ----------------- Validation and specialty APIs -----------------

boolean mdbObjIsComposite(struct mdbObj *mdbObj);
// returns TRUE if this is a valid composite object

boolean mdbObjIsCompositeMember(struct mdbObj *mdbObj);
// returns TRUE if this is a valid member of a composite.  DOES not confirm that composite obj exists

int mdbObjsValidate(struct mdbObj *mdbObjs, boolean full);
// Validates vars and vals against cv.ra.  Returns count of errors found.
// Full considers vars not defined in cv as invalids

struct slName *mdbObjFindCompositeNamedEncodeEdvs(struct sqlConnection *conn,char *tableName,struct mdbObj *mdbObj);
// returns NULL or the Experiment Defining Variable names for this composite
#define mdbObjFindCompositeEncodeEdvs(conn,mdbObj) mdbObjFindCompositeNamedEncodeEdvs((conn),NULL,(mdbObj))

struct mdbVar *mdbObjFindEncodeEdvPairs(struct sqlConnection *conn,char *tableName,struct mdbObj *mdbObj,boolean includeNone);
// returns NULL or the Experiment Defining Variables and values for this composite member object
// If includeNone, then defined variables not found in obj will be included as {var}="None".
#define mdbObjFindEncodeEdvs(conn,mdbObj,includeNone) mdbObjFindEncodeEdvPairs((conn),NULL,(mdbObj),(includeNone))

struct mdbObj *mdbObjsEncodeExperimentify(struct sqlConnection *conn,char *db,char *tableName,char *expTable,
                      struct mdbObj **pMdbObjs,int warn,boolean createExpIfNecessary,boolean updateAccession);
// Organizes objects into experiments and validates experiment IDs.  Will add/update the ids in the structures.
// If warn=1, then prints to stdout all the experiments/obs with missing or wrong expIds;
//    warn=2, then print line for each obj with expId or warning.
// createExpIfNecessary means add expId to encodeExp table. updateAccession too if necessary.
// Returns a new set of mdbObjs that is what can (and should) be used to update the mdb via mdbObjsSetToDb().

boolean mdbObjIsEncode(struct mdbObj *mdbObj);
// Returns TRUE if MDB object is an ENCODE object (project=wgEncode)

boolean mdbObjInComposite(struct mdbObj *mdb, char *composite);
// Returns TRUE if metaDb object is in specified composite.
// If composite is NULL, always return true // FIXME: KATE Why return true if composite not defined???

// -- Requested by Kate? --
//struct encodeExp *encodeExps(char *composite,char *expTable);
//struct mdbObjs *mdbObjsForDefinedExpId(int expId);
// Returns the mdb objects belonging to a single encode experiment defined in the encodExp table


// --------------- Free at last ----------------
void mdbObjsFree(struct mdbObj **mdbObjsPtr);
// Frees one or more metadata objects and any contained mdbVars.  Will free any hashes as well.
#define mdbObjFree(mdbObjPtr) mdbObjsFree(mdbObjPtr)

void mdbVarsFree(struct mdbVar **mdbVarsPtr);
// Frees one or more metadata vars and any val as well
#define mdbVarFree(mdbVarPtr) mdbVarsFree(mdbVarPtr)

void mdbByVarsFree(struct mdbByVar **mdbByVarsPtr);
// Frees one or more metadata vars and any contained vals and objs.  Will free any hashes as well.
#define mdbByVarFree(mdbByVarPtr) mdbByVarsFree(mdbByVarPtr)


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

struct slName *mdbValSearch(struct sqlConnection *conn, char *var, int limit, boolean hasTableName, boolean hasFileName);
// Search the metaDb table for vals by var.  Can impose (non-zero) limit on returned string size of val
// Search is via mysql, so it's case-insensitive.  Return is sorted on val.
// Searchable vars are only for table or file objects.  Further restrict to vars associated with tableName, fileName or both.

struct slPair *mdbValLabelSearch(struct sqlConnection *conn, char *var, int limit, boolean tags, boolean hasTableName, boolean hasFileName);
// Search the metaDb table for vals by var and returns val (as pair->name) and controlled vocabulary (cv) label
// (if it exists) (as pair->val).  Can impose (non-zero) limit on returned string size of name.
// Searchable vars are only for table or file objects.  Further restrict to vars associated with tableName, fileName or both.
// Return is case insensitive sorted on label (cv label or else val). If requested, return cv tag instead of mdb val.
#define mdbPairVal(pair) (pair)->name
#define mdbPairLabel(pair) (pair)->val

struct slPair *mdbVarsSearchable(struct sqlConnection *conn, boolean hasTableName, boolean hasFileName);
// returns a white list of mdb vars that actually exist in the current DB.
// Searchable vars are only for table or file objects.  Further restrict to vars associated with tableName, fileName or both.

#endif /* MDB_H */
