/* metaTbl.h was originally generated by the autoSql program, which also
 * generated metaTbl.c and metaTbl.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef METATBL_H
#define METATBL_H

#include "jksql.h"
#define METATBL_NUM_COLS 5

struct metaTbl
/* This contains metadata for a table, file or other predeclared object type. */
    {
    struct metaTbl *next;   // Next in singly linked list.
    char *objName;          // Object name or ID.
    char *objType;          // table | file
    char *var;              // Metadata variable name.
    char *varType;          // txt | binary
    char *val;              // Metadata value.
    };

void metaTblStaticLoad(char **row, struct metaTbl *ret);
/* Load a row from metaTbl table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct metaTbl *metaTblLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all metaTbl from table that satisfy the query given.
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something =
 * anotherTable.something'.
 * Dispose of this with metaTblFreeList(). */

void metaTblSaveToDb(struct sqlConnection *conn, struct metaTbl *el, char *tableName, int updateSize);
/* Save metaTbl as a row to the table specified by tableName.
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include"
 * If worried about this use metaTblSaveToDbEscaped() */

void metaTblSaveToDbEscaped(struct sqlConnection *conn, struct metaTbl *el, char *tableName, int updateSize);
/* Save metaTbl as a row to the table specified by tableName.
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically
 * escapes all simple strings (not arrays of string) but may be slower than metaTblSaveToDb().
 * For example automatically copies and converts:
 * "autosql's features include" --> "autosql\'s features include"
 * before inserting into database. */

struct metaTbl *metaTblLoad(char **row);
/* Load a metaTbl from row fetched with select * from metaTbl
 * from database.  Dispose of this with metaTblFree(). */

struct metaTbl *metaTblLoadAll(char *fileName);
/* Load all metaTbl from whitespace-separated file.
 * Dispose of this with metaTblFreeList(). */

struct metaTbl *metaTblLoadAllByChar(char *fileName, char chopper);
/* Load all metaTbl from chopper separated file.
 * Dispose of this with metaTblFreeList(). */

#define metaTblLoadAllByTab(a) metaTblLoadAllByChar(a, '\t');
/* Load all metaTbl from tab separated file.
 * Dispose of this with metaTblFreeList(). */

struct metaTbl *metaTblCommaIn(char **pS, struct metaTbl *ret);
/* Create a metaTbl out of a comma separated string.
 * This will fill in ret if non-null, otherwise will
 * return a new metaTbl */

void metaTblFree(struct metaTbl **pEl);
/* Free a single dynamically allocated metaTbl such as created
 * with metaTblLoad(). */

void metaTblFreeList(struct metaTbl **pList);
/* Free a list of dynamically allocated metaTbl's */

void metaTblOutput(struct metaTbl *el, FILE *f, char sep, char lastSep);
/* Print out metaTbl.  Separate fields with sep. Follow last field with lastSep. */

#define metaTblTabOut(el,f) metaTblOutput(el,f,'\t','\n');
/* Print out metaTbl as a line in a tab-separated file. */

#define metaTblCommaOut(el,f) metaTblOutput(el,f,',',',');
/* Print out metaTbl as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#include "trackDb.h"

#define METATBL_DEFAULT_NAME "metaTbl"

// The metaTbl holds metadata primarily for tables.
//   Many types of objects could be supported, though currently files are the only other type.
// It is easy to imagine using the current metaTbl to support hierarchical trees of metadata.
// For example a composite type object called "myComposte" could have metadata that is valid for
// all tables that have the var=composite val=myComposte metadata defined.
//
// There are 2 ways to look at the metadata: By Obj: obj->[var=val] and By Var: var->[val->[obj]].
// By Obj: an object has many var/val pairs but only one val for each unique var.  Querying by
//         object creates a single (2 level) one to many structure.
// By Var: a variable has many possible values and each value may be defined for more than one object.
//         Therefore, querying by var results in a (3 level) one to many to many structure.

enum metaObjType
// metadata Obects are only certain declared types
    {
    otUnknown=0,  // Unknown type
    otTable  =1,  // Table is default
    otFile   =2   // Files are also supported and used by ENCODE download only datasets.
    };

enum metaVarType
// metadata Variavble are only certain declared types
    {
    vtUnknown=0,  // Unknown type
    vtTxt    =1,  // Txt is default
    vtBinary =2   // Could support binary blobs
    };

struct metaVar
// The metadata var=val construct. This is contained by metaObj
    {
    struct metaVar* next;     // Next in singly linked list of variables
    char *var;                // Metadata variable name.
    enum metaVarType varType; // txt | binary
    char *val;                // Metadata value.
    };

struct metaObj
// The standard container of a single object's metadata.
// Also: when searching metadata obj->var->val this is the top struct.
    {
    struct metaObj* next;     // Next in singly linked list of objects
    char *obj;                // Object name or ID
    enum metaObjType objType; // table | file
    boolean deleteThis;       // Used when loading formatted file which may contain delete requests
    struct metaVar* vars;     // if NOT NULL: list of variables belonging to this object
    struct hash* varHash;     // if NOT NULL: variables are also hashed! (var str to metaVar struct)
    };

struct metaLeafObj
// When searching var->val->obj this is the bottom-level obj struct.
    {
    struct metaLeafObj* next; // Next in singly linked list of variables
    char *obj;                // Object name or ID
    enum metaObjType objType; // table | file
    };

struct metaLimbVal
// When searching var->val->obj this is the mid-level val->obj struct.
    {
    struct metaLimbVal* next; // Next in singly linked list of variables
    char *val;                // Metadata value.
    struct metaLeafObj* objs; // if NOT NULL: list of Objects which have this variable
    struct hash* objHash;     // if NOT NULL: hash of objects  (val str to leafObj struct)
    };

struct metaByVar
// When searching metadata var->val->object this is the top struct
    {
    struct metaByVar* next;   // Next in singly linked list of variables
    char *var;                // Metadata variable name.
    enum metaVarType varType; // txt | binary
    boolean notEqual;         // For querying only
    struct metaLimbVal* vals; // list of values associated with this var
    struct hash* valHash;     // if NOT NULL: hash of vals  (val str to limbVal struct)
    };

// -------------- Enum to Strings --------------
enum metaObjType metaObjTypeStringToEnum(char *objType);
// Convert metadata objType string to enum

char *metaObjTypeEnumToString(enum metaObjType objType);
// Convert metadata objType enum string

enum metaVarType metaVarTypeStringToEnum(char *varType);
// Convert metadata varType string to enum

char *metaVarTypeEnumToString(enum metaVarType varType);
// Convert metadata varType enum string

// ------ Parsing lines ------
struct metaObj *metadataLineParse(char *line);
/* Parses a single formatted metadata line into metaObj for updates or queries. */

struct metaByVar *metaByVarsLineParse(char *line);
/* Parses a line of "var1=val1 var2=val2 into a metaByVar object for queries. */


// ------ Loading from args, hashes ------
struct metaObj *metaObjCreate(char *obj,char *type,char *var, char *varType,char *val);
/* Creates a singular metaObj query object based on obj and all other optional params. */

struct metaByVar *metaByVarCreate(char *var, char *varType,char *val);
/* Creates a singular var=val pair struct for metadata queries. */

struct metaObj *metaObjsLoadFromHashes(struct hash *objsHash);
// Load all metaObjs from a file containing metadata formatted lines


// ------ Loading from files ------
struct metaObj *metaObjsLoadFromFormattedFile(char *fileName,boolean *validated);
// Load all metaObjs from a file containing metadata formatted lines
// If requested, will determine if a magic number at the end of the file matches contents

struct metaObj *metaObjsLoadFromRAFile(char *fileName,boolean *validated);
// Load all metaObjs from a file containing RA formatted 'metaObjects'
// If requested, will determine if a magic number at the end of the file matches contents



// ------ Table name and creation ------
void metaTblReCreate(struct sqlConnection *conn,char *tblName,boolean testOnly);
// Creates ore Recreates the named metaTbl.

char*metaTblName(struct sqlConnection *conn,boolean mySandBox);
// returns the metaTbl name or NULL if conn supplied but the table doesn't exist


// -------------- Updating the DB --------------
int metaObjsSetToDb(struct sqlConnection *conn,char *tableName,struct metaObj *metaObjs,boolean replace,boolean testOnly);
// Adds or updates metadata obj/var pairs into the named table.  Returns total rows affected


// ------------------ Querys -------------------
struct metaObj *metaObjQuery(struct sqlConnection *conn,char *table,struct metaObj *metaObj);
// Query the metadata table by obj and optional vars and vals in metaObj struct.  If metaObj is NULL query all.
// Returns new metaObj struct fully populated and sorted in obj,var order.
#define metaObjsQueryAll(conn,table) metaObjQuery((conn),(table),NULL)

struct metaObj *metaObjQueryByObj(struct sqlConnection *conn,char *table,char *obj,char *var);
// Query a single metadata object and optional var from a table (default metaTbl).

struct metaByVar *metaByVarsQuery(struct sqlConnection *conn,char *table,struct metaByVar *metaByVars);
// Query the metadata table by one or more var=val pairs to find the distinct set of objs that satisfy ANY conditions.
// Returns new metaByVar struct fully populated and sorted in var,val,obj order.
#define metaByVarsQueryAll(conn,table) metaByVarsQuery((conn),(table),NULL)

struct metaByVar *metaByVarQueryByVar(struct sqlConnection *conn,char *table,char *varName,char *val);
// Query a single metadata variable and optional val from a table (default metaTbl) for searching val->obj.

struct metaObj *metaObjsQueryByVars(struct sqlConnection *conn,char *table,struct metaByVar *metaByVars);
// Query the metadata table by one or more var=val pairs to find the distinct set of objs that satisfy ALL conditions.
// Returns new metaObj struct fully populated and sorted in obj,var order.


// ----------- Printing and Counting -----------
void metaObjPrint(struct metaObj *metaObjs,boolean raStyle);
// prints objs and var=val pairs as formatted metadata lines or ra style

void metaByVarPrint(struct metaByVar *metaByVars,boolean raStyle);
// prints var=val pairs and objs that go with them single lines or ra style

int metaObjCount(struct metaObj *metaObjs, boolean objs);
// returns the count of vars belonging to this obj or objs;

int metaByVarCount(struct metaByVar *metaByVars,boolean vars, boolean vals);
// returns the count of objs belonging to this set of vars;


// ----------------- Utilities -----------------
char *metaObjFindValue(struct metaObj *metaObj, char *var);
// Finds the val associated with the var or retruns NULL

boolean metaObjContains(struct metaObj *metaObj, char *var, char *val);
// Returns TRUE if object contains var, val or both

boolean metaByVarContains(struct metaByVar *metaByVar, char *val, char *obj);
// Returns TRUE if var contains val, obj or both

void metaObjReorderVars(struct metaObj *metaObjs, char *vars,boolean back);
// Reorders vars list based upon list of vars "cell antibody treatment".  Send to front or back.

void metaObjRemoveVars(struct metaObj *metaObjs, char *vars);
// Prunes list of vars for an object, freeing the memory.  Doesn't touch DB.

void metaObjSwapVars(struct metaObj *metaObjs, char *vars,boolean deleteThis);
// Replaces objs' vars with var=vap pairs provided, preparing for DB update.

void metaObjTransformToUpdate(struct metaObj *metaObjs, char *var, char *varType,char *val,boolean deleteThis);
// Turns one or more metaObjs into the stucture needed to add/update or delete.

struct metaObj *metaObjClone(const struct metaObj *metaObj);
// Clones a single metaObj, including hash and maintining order


// --------------- Free at last ----------------
void metaObjsFree(struct metaObj **metaObjsPtr);
// Frees one or more metadata objects and any contained metaVars.  Will free any hashes as well.

void metaByVarsFree(struct metaByVar **metaByVarsPtr);
// Frees one or more metadata vars and any contained vals and objs.  Will free any hashes as well.


// ----------------- CGI specific routines for use with tdb -----------------
const struct metaObj *metadataForTable(char *db,struct trackDb *tdb,char *table);
// Returns the metadata for a table.  NEVER FREE THIS STRUCT!
// This is the main routine for CGIs to access metadata

const char *metadataFindValue(struct trackDb *tdb, char *var);
// Finds the val associated with the var or retruns NULL


#endif /* METATBL_H */

