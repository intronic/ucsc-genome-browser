/* sgdDescription.h was originally generated by the autoSql program, which also 
 * generated sgdDescription.c and sgdDescription.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef SGDDESCRIPTION_H
#define SGDDESCRIPTION_H

#define SGDDESCRIPTION_NUM_COLS 3

struct sgdDescription
/* Description of SGD Genes and Other Features */
    {
    struct sgdDescription *next;  /* Next in singly linked list. */
    char *name;	/* Name in sgdGene or sgdOther table */
    char *type;	/* Type of feature from gff3 file */
    char *description;	/* Description of feature */
    };

void sgdDescriptionStaticLoad(char **row, struct sgdDescription *ret);
/* Load a row from sgdDescription table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct sgdDescription *sgdDescriptionLoad(char **row);
/* Load a sgdDescription from row fetched with select * from sgdDescription
 * from database.  Dispose of this with sgdDescriptionFree(). */

struct sgdDescription *sgdDescriptionLoadAll(char *fileName);
/* Load all sgdDescription from whitespace-separated file.
 * Dispose of this with sgdDescriptionFreeList(). */

struct sgdDescription *sgdDescriptionLoadAllByChar(char *fileName, char chopper);
/* Load all sgdDescription from chopper separated file.
 * Dispose of this with sgdDescriptionFreeList(). */

#define sgdDescriptionLoadAllByTab(a) sgdDescriptionLoadAllByChar(a, '\t');
/* Load all sgdDescription from tab separated file.
 * Dispose of this with sgdDescriptionFreeList(). */

struct sgdDescription *sgdDescriptionCommaIn(char **pS, struct sgdDescription *ret);
/* Create a sgdDescription out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new sgdDescription */

void sgdDescriptionFree(struct sgdDescription **pEl);
/* Free a single dynamically allocated sgdDescription such as created
 * with sgdDescriptionLoad(). */

void sgdDescriptionFreeList(struct sgdDescription **pList);
/* Free a list of dynamically allocated sgdDescription's */

void sgdDescriptionOutput(struct sgdDescription *el, FILE *f, char sep, char lastSep);
/* Print out sgdDescription.  Separate fields with sep. Follow last field with lastSep. */

#define sgdDescriptionTabOut(el,f) sgdDescriptionOutput(el,f,'\t','\n');
/* Print out sgdDescription as a line in a tab-separated file. */

#define sgdDescriptionCommaOut(el,f) sgdDescriptionOutput(el,f,',',',');
/* Print out sgdDescription as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SGDDESCRIPTION_H */

