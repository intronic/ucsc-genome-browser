/* hgFindSpec.h was originally generated by the autoSql program, which also 
 * generated hgFindSpec.c and hgFindSpec.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HGFINDSPEC_H
#define HGFINDSPEC_H

#define HGFINDSPEC_NUM_COLS 12

struct hgFindSpec
/* This defines a search to be performed by hgFind. */
    {
    struct hgFindSpec *next;  /* Next in singly linked list. */
    char *searchName;	/* Unique name for this search.  Defaults to searchTable if not specified in .ra. */
    char *searchTable;	/* (Non-unique!) Table to be searched.  (Like trackDb.tableName: if split, omit chr*_ prefix.) */
    char *searchMethod;	/* Type of search (exact, prefix, fuzzy). */
    char *searchType;	/* Type of search (bed, genePred, knownGene etc). */
    unsigned char shortCircuit;	/* If nonzero, and there is a result from this search, jump to the result instead of performing other searches. */
    char *termRegex;	/* Regular expression (see man 7 regex) to eval on search term: if it matches, perform search query. */
    char *query;	/* sprintf format string for SQL query on a given table and value. */
    char *xrefTable;	/* If search is xref, perform xrefQuery on search term, then query with that result. */
    char *xrefQuery;	/* sprintf format string for SQL query on a given (xref) table and value. */
    float searchPriority;	/* 0-1000 - relative order/importance of this search.  0 is top. */
    char *searchDescription;	/* Description of table/search (default: trackDb.{longLabel,tableName}) */
    char *searchSettings;	/* Name/value pairs for searchType-specific stuff. */
    struct hash *settingsHash;  /* Hash for settings. Not saved in database.
                                 * Don't use directly, rely on 
				 * hgFindSpecSetting to access. */
    };

void hgFindSpecStaticLoad(char **row, struct hgFindSpec *ret);
/* Load a row from hgFindSpec table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgFindSpec *hgFindSpecLoad(char **row);
/* Load a hgFindSpec from row fetched with select * from hgFindSpec
 * from database.  Dispose of this with hgFindSpecFree(). */

struct hgFindSpec *hgFindSpecLoadAll(char *fileName);
/* Load all hgFindSpec from whitespace-separated file.
 * Dispose of this with hgFindSpecFreeList(). */

struct hgFindSpec *hgFindSpecLoadAllByChar(char *fileName, char chopper);
/* Load all hgFindSpec from chopper separated file.
 * Dispose of this with hgFindSpecFreeList(). */

#define hgFindSpecLoadAllByTab(a) hgFindSpecLoadAllByChar(a, '\t');
/* Load all hgFindSpec from tab separated file.
 * Dispose of this with hgFindSpecFreeList(). */

struct hgFindSpec *hgFindSpecCommaIn(char **pS, struct hgFindSpec *ret);
/* Create a hgFindSpec out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgFindSpec */

void hgFindSpecFree(struct hgFindSpec **pEl);
/* Free a single dynamically allocated hgFindSpec such as created
 * with hgFindSpecLoad(). */

void hgFindSpecFreeList(struct hgFindSpec **pList);
/* Free a list of dynamically allocated hgFindSpec's */

void hgFindSpecOutput(struct hgFindSpec *el, FILE *f, char sep, char lastSep);
/* Print out hgFindSpec.  Separate fields with sep. Follow last field with lastSep. */

#define hgFindSpecTabOut(el,f) hgFindSpecOutput(el,f,'\t','\n');
/* Print out hgFindSpec as a line in a tab-separated file. */

#define hgFindSpecCommaOut(el,f) hgFindSpecOutput(el,f,',',',');
/* Print out hgFindSpec as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

boolean matchRegex(char *name, char *exp);
/* Return TRUE if name matches the regular expression pattern
 * (case insensitive). */

int hgFindSpecCmp(const void *va, const void *vb);
/* Compare to sort based on searchPriority. */

struct hgFindSpec *hgFindSpecFromRa(char *raFile);
/* Load track info from ra file into list. */

char *hgFindSpecSetting(struct hgFindSpec *hfs, char *name);
/* Return setting string or NULL if none exists. */

char *hgFindSpecRequiredSetting(struct hgFindSpec *hfs, char *name);
/* Return setting string or squawk and die. */

char *hgFindSpecSettingOrDefault(struct hgFindSpec *hfs, char *name,
				 char *defaultVal);
/* Return setting string, or defaultVal if none exists */

struct hgFindSpec *hgFindSpecGetSpecs(boolean shortCircuit);
/* Load all short-circuit (or not) search specs from the current db, sorted by 
 * searchPriority. */

void hgFindSpecGetAllSpecs(struct hgFindSpec **retShortCircuitList,
			   struct hgFindSpec **retAdditiveList);
/* Load all search specs from the current db, separated according to 
 * shortCircuit and sorted by searchPriority. */

#endif /* HGFINDSPEC_H */

