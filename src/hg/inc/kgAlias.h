/* kgAlias.h was originally generated by the autoSql program, which also 
 * generated kgAlias.c and kgAlias.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef KGALIAS_H
#define KGALIAS_H

#define KGALIAS_NUM_COLS 2

struct kgAlias
/* Link together a Known Gene ID and a gene alias */
    {
    struct kgAlias *next;  /* Next in singly linked list. */
    char *kgID;	/* Known Gene ID */
    char *alias;	/* a gene alias */
    };

void kgAliasStaticLoad(char **row, struct kgAlias *ret);
/* Load a row from kgAlias table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct kgAlias *kgAliasLoad(char **row);
/* Load a kgAlias from row fetched with select * from kgAlias
 * from database.  Dispose of this with kgAliasFree(). */

struct kgAlias *kgAliasLoadAll(char *fileName);
/* Load all kgAlias from whitespace-separated file.
 * Dispose of this with kgAliasFreeList(). */

struct kgAlias *kgAliasLoadAllByChar(char *fileName, char chopper);
/* Load all kgAlias from chopper separated file.
 * Dispose of this with kgAliasFreeList(). */

#define kgAliasLoadAllByTab(a) kgAliasLoadAllByChar(a, '\t');
/* Load all kgAlias from tab separated file.
 * Dispose of this with kgAliasFreeList(). */

struct kgAlias *kgAliasCommaIn(char **pS, struct kgAlias *ret);
/* Create a kgAlias out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new kgAlias */

void kgAliasFree(struct kgAlias **pEl);
/* Free a single dynamically allocated kgAlias such as created
 * with kgAliasLoad(). */

void kgAliasFreeList(struct kgAlias **pList);
/* Free a list of dynamically allocated kgAlias's */

void kgAliasOutput(struct kgAlias *el, FILE *f, char sep, char lastSep);
/* Print out kgAlias.  Separate fields with sep. Follow last field with lastSep. */

#define kgAliasTabOut(el,f) kgAliasOutput(el,f,'\t','\n');
/* Print out kgAlias as a line in a tab-separated file. */

#define kgAliasCommaOut(el,f) kgAliasOutput(el,f,',',',');
/* Print out kgAlias as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* KGALIAS_H */

