/* bed8Attrs.h was originally generated by the autoSql program, which also 
 * generated bed8Attrs.c and bed8Attrs.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef BED8ATTRS_H
#define BED8ATTRS_H

#define BED8ATTRS_NUM_COLS 11

struct bed8Attrs
/* Items with thin (outer) and/or thick (inner) regions and an arbitrary set of attributes */
    {
    struct bed8Attrs *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 (might not be applicable) */
    char strand[2];	/* +, - or . for unknown */
    unsigned thickStart;	/* Start of where display should be thick */
    unsigned thickEnd;	/* End of where display should be thick */
    int attrCount;	/* Number of attributes */
    char **attrTags;	/* Attribute tags/keys */
    char **attrVals;	/* Attribute values */
    };

struct bed8Attrs *bed8AttrsLoad(char **row);
/* Load a bed8Attrs from row fetched with select * from bed8Attrs
 * from database.  Dispose of this with bed8AttrsFree(). */

struct bed8Attrs *bed8AttrsLoadAll(char *fileName);
/* Load all bed8Attrs from whitespace-separated file.
 * Dispose of this with bed8AttrsFreeList(). */

struct bed8Attrs *bed8AttrsLoadAllByChar(char *fileName, char chopper);
/* Load all bed8Attrs from chopper separated file.
 * Dispose of this with bed8AttrsFreeList(). */

#define bed8AttrsLoadAllByTab(a) bed8AttrsLoadAllByChar(a, '\t');
/* Load all bed8Attrs from tab separated file.
 * Dispose of this with bed8AttrsFreeList(). */

struct bed8Attrs *bed8AttrsCommaIn(char **pS, struct bed8Attrs *ret);
/* Create a bed8Attrs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed8Attrs */

void bed8AttrsFree(struct bed8Attrs **pEl);
/* Free a single dynamically allocated bed8Attrs such as created
 * with bed8AttrsLoad(). */

void bed8AttrsFreeList(struct bed8Attrs **pList);
/* Free a list of dynamically allocated bed8Attrs's */

void bed8AttrsOutput(struct bed8Attrs *el, FILE *f, char sep, char lastSep);
/* Print out bed8Attrs.  Separate fields with sep. Follow last field with lastSep. */

#define bed8AttrsTabOut(el,f) bed8AttrsOutput(el,f,'\t','\n');
/* Print out bed8Attrs as a line in a tab-separated file. */

#define bed8AttrsCommaOut(el,f) bed8AttrsOutput(el,f,',',',');
/* Print out bed8Attrs as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* BED8ATTRS_H */

