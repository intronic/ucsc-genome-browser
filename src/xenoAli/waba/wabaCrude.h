/* wabaCrude.h was originally generated by the autoSql program, which also 
 * generated wabaCrude.c and wabaCrude.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef WABACRUDE_H
#define WABACRUDE_H

struct wabaCrude
/* Info on a crude alignment */
    {
    struct wabaCrude *next;  /* Next in singly linked list. */
    unsigned score;	/* score 0 to about 6000 */
    char *qFile;	/* query sequence file */
    char *qSeq;	/* query sequence within file */
    unsigned qStart;	/* query start position */
    unsigned qEnd;	/* query end position */
    signed char strand;	/* strand of alignment: +1 or -1 */
    char *tFile;	/* target sequence file */
    char *tSeq;	/* target sequence within file */
    unsigned tStart;	/* target start position */
    unsigned tEnd;	/* target end position */
    };

void wabaCrudeStaticLoad(char **row, struct wabaCrude *ret);
/* Load a row from wabaCrude table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wabaCrude *wabaCrudeLoad(char **row);
/* Load a wabaCrude from row fetched with select * from wabaCrude
 * from database.  Dispose of this with wabaCrudeFree(). */

struct wabaCrude *wabaCrudeCommaIn(char **pS, struct wabaCrude *ret);
/* Create a wabaCrude out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wabaCrude */

void wabaCrudeFree(struct wabaCrude **pEl);
/* Free a single dynamically allocated wabaCrude such as created
 * with wabaCrudeLoad(). */

void wabaCrudeFreeList(struct wabaCrude **pList);
/* Free a list of dynamically allocated wabaCrude's */

void wabaCrudeOutput(struct wabaCrude *el, FILE *f, char sep, char lastSep);
/* Print out wabaCrude.  Separate fields with sep. Follow last field with lastSep. */

#define wabaCrudeTabOut(el,f) wabaCrudeOutput(el,f,'\t','\n');
/* Print out wabaCrude as a line in a tab-separated file. */

#define wabaCrudeCommaOut(el,f) wabaCrudeOutput(el,f,',',',');
/* Print out wabaCrude as a comma separated list including final comma. */

#endif /* WABACRUDE_H */

