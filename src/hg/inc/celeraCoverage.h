/* celeraCoverage.h was originally generated by the autoSql program, which also 
 * generated celeraCoverage.c and celeraCoverage.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CELERACOVERAGE_H
#define CELERACOVERAGE_H

struct celeraCoverage
/* Summary of large genomic Duplications from Celera Data */
    {
    struct celeraCoverage *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Source of Information */
    };

void celeraCoverageStaticLoad(char **row, struct celeraCoverage *ret);
/* Load a row from celeraCoverage table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct celeraCoverage *celeraCoverageLoad(char **row);
/* Load a celeraCoverage from row fetched with select * from celeraCoverage
 * from database.  Dispose of this with celeraCoverageFree(). */

struct celeraCoverage *celeraCoverageLoadAll(char *fileName);
/* Load all celeraCoverage from a tab-separated file.
 * Dispose of this with celeraCoverageFreeList(). */

struct celeraCoverage *celeraCoverageLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all celeraCoverage from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with celeraCoverageFreeList(). */

struct celeraCoverage *celeraCoverageCommaIn(char **pS, struct celeraCoverage *ret);
/* Create a celeraCoverage out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new celeraCoverage */

void celeraCoverageFree(struct celeraCoverage **pEl);
/* Free a single dynamically allocated celeraCoverage such as created
 * with celeraCoverageLoad(). */

void celeraCoverageFreeList(struct celeraCoverage **pList);
/* Free a list of dynamically allocated celeraCoverage's */

void celeraCoverageOutput(struct celeraCoverage *el, FILE *f, char sep, char lastSep);
/* Print out celeraCoverage.  Separate fields with sep. Follow last field with lastSep. */

#define celeraCoverageTabOut(el,f) celeraCoverageOutput(el,f,'\t','\n');
/* Print out celeraCoverage as a line in a tab-separated file. */

#define celeraCoverageCommaOut(el,f) celeraCoverageOutput(el,f,',',',');
/* Print out celeraCoverage as a comma separated list including final comma. */

#endif /* CELERACOVERAGE_H */

