/* polyASize.h was originally generated by the autoSql program, which also 
 * generated polyASize.c and polyASize.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef POLYASIZE_H
#define POLYASIZE_H

#define POLYASIZE_NUM_COLS 4
struct hash;

struct polyASize
/* used to read output of faPolyASizes */
    {
    struct polyASize *next;  /* Next in singly linked list. */
    char *id;	/* sequence id */
    unsigned seqSize;	/* sequence size */
    unsigned tailPolyASize;	/* size of poly-A at head (some noise allowed) */
    unsigned headPolyTSize;	/* size of poly-T at tail (some noise allowed) */
    };

void polyASizeStaticLoad(char **row, struct polyASize *ret);
/* Load a row from polyASize table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct polyASize *polyASizeLoad(char **row);
/* Load a polyASize from row fetched with select * from polyASize
 * from database.  Dispose of this with polyASizeFree(). */

struct polyASize *polyASizeLoadAll(char *fileName);
/* Load all polyASize from whitespace-separated file.
 * Dispose of this with polyASizeFreeList(). */

struct polyASize *polyASizeLoadAllByChar(char *fileName, char chopper);
/* Load all polyASize from chopper separated file.
 * Dispose of this with polyASizeFreeList(). */

#define polyASizeLoadAllByTab(a) polyASizeLoadAllByChar(a, '\t');
/* Load all polyASize from tab separated file.
 * Dispose of this with polyASizeFreeList(). */

struct polyASize *polyASizeCommaIn(char **pS, struct polyASize *ret);
/* Create a polyASize out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polyASize */

void polyASizeFree(struct polyASize **pEl);
/* Free a single dynamically allocated polyASize such as created
 * with polyASizeLoad(). */

void polyASizeFreeList(struct polyASize **pList);
/* Free a list of dynamically allocated polyASize's */

void polyASizeOutput(struct polyASize *el, FILE *f, char sep, char lastSep);
/* Print out polyASize.  Separate fields with sep. Follow last field with lastSep. */

#define polyASizeTabOut(el,f) polyASizeOutput(el,f,'\t','\n');
/* Print out polyASize as a line in a tab-separated file. */

#define polyASizeCommaOut(el,f) polyASizeOutput(el,f,',',',');
/* Print out polyASize as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct hash *polyASizeLoadHash(char *fileName);
/* load a tab-separated file of polyASize objects into a hash table.  Objects
 * are stored in the local mem of the hash, and thus freed when the hash is
 * freed. */


#endif /* POLYASIZE_H */

