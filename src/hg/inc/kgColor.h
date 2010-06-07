/* kgColor.h was originally generated by the autoSql program, which also 
 * generated kgColor.c and kgColor.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef KGCOLOR_H
#define KGCOLOR_H

#define KGCOLOR_NUM_COLS 4

struct kgColor
/* What color to draw known gene in? */
    {
    struct kgColor *next;  /* Next in singly linked list. */
    char *kgID;	/* Known gene ID */
    unsigned char r;	/* Red component 0-255 */
    unsigned char g;	/* Green component 0-255 */
    unsigned char b;	/* Blue component 0-255 */
    };

void kgColorStaticLoad(char **row, struct kgColor *ret);
/* Load a row from kgColor table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct kgColor *kgColorLoad(char **row);
/* Load a kgColor from row fetched with select * from kgColor
 * from database.  Dispose of this with kgColorFree(). */

struct kgColor *kgColorLoadAll(char *fileName);
/* Load all kgColor from whitespace-separated file.
 * Dispose of this with kgColorFreeList(). */

struct kgColor *kgColorLoadAllByChar(char *fileName, char chopper);
/* Load all kgColor from chopper separated file.
 * Dispose of this with kgColorFreeList(). */

#define kgColorLoadAllByTab(a) kgColorLoadAllByChar(a, '\t');
/* Load all kgColor from tab separated file.
 * Dispose of this with kgColorFreeList(). */

struct kgColor *kgColorCommaIn(char **pS, struct kgColor *ret);
/* Create a kgColor out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new kgColor */

void kgColorFree(struct kgColor **pEl);
/* Free a single dynamically allocated kgColor such as created
 * with kgColorLoad(). */

void kgColorFreeList(struct kgColor **pList);
/* Free a list of dynamically allocated kgColor's */

void kgColorOutput(struct kgColor *el, FILE *f, char sep, char lastSep);
/* Print out kgColor.  Separate fields with sep. Follow last field with lastSep. */

#define kgColorTabOut(el,f) kgColorOutput(el,f,'\t','\n');
/* Print out kgColor as a line in a tab-separated file. */

#define kgColorCommaOut(el,f) kgColorOutput(el,f,',',',');
/* Print out kgColor as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* KGCOLOR_H */

