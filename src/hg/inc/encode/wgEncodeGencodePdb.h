/* wgEncodeGencodePdb.h was originally generated by the autoSql program, which also 
 * generated wgEncodeGencodePdb.c and wgEncodeGencodePdb.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef WGENCODEGENCODEPDB_H
#define WGENCODEGENCODEPDB_H

#define WGENCODEGENCODEPDB_NUM_COLS 2

struct wgEncodeGencodePdb
/* GENCODE transcript to Protein Data Bank (PDB) protein structure mapping */
    {
    struct wgEncodeGencodePdb *next;  /* Next in singly linked list. */
    char *transcriptId;	/* GENCODE transcript identifier */
    char *pdbId;	/* Protein Data Bank (PDB) identifier */
    };

void wgEncodeGencodePdbStaticLoad(char **row, struct wgEncodeGencodePdb *ret);
/* Load a row from wgEncodeGencodePdb table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wgEncodeGencodePdb *wgEncodeGencodePdbLoad(char **row);
/* Load a wgEncodeGencodePdb from row fetched with select * from wgEncodeGencodePdb
 * from database.  Dispose of this with wgEncodeGencodePdbFree(). */

struct wgEncodeGencodePdb *wgEncodeGencodePdbLoadAll(char *fileName);
/* Load all wgEncodeGencodePdb from whitespace-separated file.
 * Dispose of this with wgEncodeGencodePdbFreeList(). */

struct wgEncodeGencodePdb *wgEncodeGencodePdbLoadAllByChar(char *fileName, char chopper);
/* Load all wgEncodeGencodePdb from chopper separated file.
 * Dispose of this with wgEncodeGencodePdbFreeList(). */

#define wgEncodeGencodePdbLoadAllByTab(a) wgEncodeGencodePdbLoadAllByChar(a, '\t');
/* Load all wgEncodeGencodePdb from tab separated file.
 * Dispose of this with wgEncodeGencodePdbFreeList(). */

struct wgEncodeGencodePdb *wgEncodeGencodePdbCommaIn(char **pS, struct wgEncodeGencodePdb *ret);
/* Create a wgEncodeGencodePdb out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wgEncodeGencodePdb */

void wgEncodeGencodePdbFree(struct wgEncodeGencodePdb **pEl);
/* Free a single dynamically allocated wgEncodeGencodePdb such as created
 * with wgEncodeGencodePdbLoad(). */

void wgEncodeGencodePdbFreeList(struct wgEncodeGencodePdb **pList);
/* Free a list of dynamically allocated wgEncodeGencodePdb's */

void wgEncodeGencodePdbOutput(struct wgEncodeGencodePdb *el, FILE *f, char sep, char lastSep);
/* Print out wgEncodeGencodePdb.  Separate fields with sep. Follow last field with lastSep. */

#define wgEncodeGencodePdbTabOut(el,f) wgEncodeGencodePdbOutput(el,f,'\t','\n');
/* Print out wgEncodeGencodePdb as a line in a tab-separated file. */

#define wgEncodeGencodePdbCommaOut(el,f) wgEncodeGencodePdbOutput(el,f,',',',');
/* Print out wgEncodeGencodePdb as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* WGENCODEGENCODEPDB_H */

