/* wgEncodeGencodeTag.h was originally generated by the autoSql program, which also 
 * generated wgEncodeGencodeTag.c and wgEncodeGencodeTag.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef WGENCODEGENCODETAG_H
#define WGENCODEGENCODETAG_H

#define WGENCODEGENCODETAG_NUM_COLS 2

struct wgEncodeGencodeTag
/* Tags associated with GENCODE transcripts. */
    {
    struct wgEncodeGencodeTag *next;  /* Next in singly linked list. */
    char *transcriptId;	/* GENCODE transcript identifier */
    char *tag;	/* symbolic tag */
    };

void wgEncodeGencodeTagStaticLoad(char **row, struct wgEncodeGencodeTag *ret);
/* Load a row from wgEncodeGencodeTag table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wgEncodeGencodeTag *wgEncodeGencodeTagLoad(char **row);
/* Load a wgEncodeGencodeTag from row fetched with select * from wgEncodeGencodeTag
 * from database.  Dispose of this with wgEncodeGencodeTagFree(). */

struct wgEncodeGencodeTag *wgEncodeGencodeTagLoadAll(char *fileName);
/* Load all wgEncodeGencodeTag from whitespace-separated file.
 * Dispose of this with wgEncodeGencodeTagFreeList(). */

struct wgEncodeGencodeTag *wgEncodeGencodeTagLoadAllByChar(char *fileName, char chopper);
/* Load all wgEncodeGencodeTag from chopper separated file.
 * Dispose of this with wgEncodeGencodeTagFreeList(). */

#define wgEncodeGencodeTagLoadAllByTab(a) wgEncodeGencodeTagLoadAllByChar(a, '\t');
/* Load all wgEncodeGencodeTag from tab separated file.
 * Dispose of this with wgEncodeGencodeTagFreeList(). */

struct wgEncodeGencodeTag *wgEncodeGencodeTagCommaIn(char **pS, struct wgEncodeGencodeTag *ret);
/* Create a wgEncodeGencodeTag out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wgEncodeGencodeTag */

void wgEncodeGencodeTagFree(struct wgEncodeGencodeTag **pEl);
/* Free a single dynamically allocated wgEncodeGencodeTag such as created
 * with wgEncodeGencodeTagLoad(). */

void wgEncodeGencodeTagFreeList(struct wgEncodeGencodeTag **pList);
/* Free a list of dynamically allocated wgEncodeGencodeTag's */

void wgEncodeGencodeTagOutput(struct wgEncodeGencodeTag *el, FILE *f, char sep, char lastSep);
/* Print out wgEncodeGencodeTag.  Separate fields with sep. Follow last field with lastSep. */

#define wgEncodeGencodeTagTabOut(el,f) wgEncodeGencodeTagOutput(el,f,'\t','\n');
/* Print out wgEncodeGencodeTag as a line in a tab-separated file. */

#define wgEncodeGencodeTagCommaOut(el,f) wgEncodeGencodeTagOutput(el,f,',',',');
/* Print out wgEncodeGencodeTag as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* WGENCODEGENCODETAG_H */

