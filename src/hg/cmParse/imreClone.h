/* imreClone.h was originally generated by the autoSql program, which also 
 * generated imreClone.c and imreClone.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef IMRECLONE_H
#define IMRECLONE_H

struct imreClone
/* A single line in a tpf file */
    {
    struct imreClone *next;  /* Next in singly linked list. */
    char *accession;	/* Genbank Accession */
    char *cloneName;	/* Name of clone, often RP11-something */
    char *imreContig;	/* Name in TFP contig */
    char *origContig;	/* Contig in premerged map */
    char *source;	/* Source of positioning: TPF or MAP */
    };

void imreCloneStaticLoad(char **row, struct imreClone *ret);
/* Load a row from imreClone table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct imreClone *imreCloneLoad(char **row);
/* Load a imreClone from row fetched with select * from imreClone
 * from database.  Dispose of this with imreCloneFree(). */

struct imreClone *imreCloneLoadAll(char *fileName);
/* Load all imreClone from a tab-separated file.
 * Dispose of this with imreCloneFreeList(). */

struct imreClone *imreCloneCommaIn(char **pS, struct imreClone *ret);
/* Create a imreClone out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new imreClone */

void imreCloneFree(struct imreClone **pEl);
/* Free a single dynamically allocated imreClone such as created
 * with imreCloneLoad(). */

void imreCloneFreeList(struct imreClone **pList);
/* Free a list of dynamically allocated imreClone's */

void imreCloneOutput(struct imreClone *el, FILE *f, char sep, char lastSep);
/* Print out imreClone.  Separate fields with sep. Follow last field with lastSep. */

#define imreCloneTabOut(el,f) imreCloneOutput(el,f,'\t','\n');
/* Print out imreClone as a line in a tab-separated file. */

#define imreCloneCommaOut(el,f) imreCloneOutput(el,f,',',',');
/* Print out imreClone as a comma separated list including final comma. */

#endif /* IMRECLONE_H */

