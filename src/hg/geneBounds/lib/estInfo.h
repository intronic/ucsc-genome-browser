/* estInfo.h was originally generated by the autoSql program, which also 
 * generated estInfo.c and estInfo.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ESTINFO_H
#define ESTINFO_H

struct estInfo
/* Extra information on ESTs - calculated by polyInfo program */
    {
    struct estInfo *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Accession of EST */
    short intronOrientation;	/* Orientation of introns with respect to EST */
    short sizePolyA;	/* Number of trailing A's */
    short revSizePolyA;	/* Number of trailing A's on reverse strand */
    short signalPos;	/* Position of polyA signal or 0 if no signal */
    short revSignalPos;	/* PolyA signal position on reverse strand if any */
    };

void estInfoStaticLoad(char **row, struct estInfo *ret);
/* Load a row from estInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct estInfo *estInfoLoad(char **row);
/* Load a estInfo from row fetched with select * from estInfo
 * from database.  Dispose of this with estInfoFree(). */

struct estInfo *estInfoLoadAll(char *fileName);
/* Load all estInfo from a tab-separated file.
 * Dispose of this with estInfoFreeList(). */

struct estInfo *estInfoLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all estInfo from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with estInfoFreeList(). */

struct estInfo *estInfoCommaIn(char **pS, struct estInfo *ret);
/* Create a estInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new estInfo */

void estInfoFree(struct estInfo **pEl);
/* Free a single dynamically allocated estInfo such as created
 * with estInfoLoad(). */

void estInfoFreeList(struct estInfo **pList);
/* Free a list of dynamically allocated estInfo's */

void estInfoOutput(struct estInfo *el, FILE *f, char sep, char lastSep);
/* Print out estInfo.  Separate fields with sep. Follow last field with lastSep. */

#define estInfoTabOut(el,f) estInfoOutput(el,f,'\t','\n');
/* Print out estInfo as a line in a tab-separated file. */

#define estInfoCommaOut(el,f) estInfoOutput(el,f,',',',');
/* Print out estInfo as a comma separated list including final comma. */

#endif /* ESTINFO_H */

