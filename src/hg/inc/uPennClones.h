/* uPennClones.h was originally generated by the autoSql program, which also 
 * generated uPennClones.c and uPennClones.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef UPENNCLONES_H
#define UPENNCLONES_H

struct uPennClones
/* Clones positioned on the assembly by U Penn (V. Cheung) */
    {
    struct uPennClones *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome or 'unknown' */
    int chromStart;	/* Start position in chrom - negative 1 if unpositioned */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Name of Clone */
    unsigned score;	/* Score - always 1000 */
    char strand[2];	/* + or - */
    char *accT7;	/* Accession number for T7 BAC end sequence */
    unsigned startT7;	/* Start position in chrom for T7 end sequence */
    unsigned endT7;	/* End position in chrom for T7 end sequence */
    char strandT7[2];	/* + or - */
    char *accSP6;	/* Accession number for SP6 BAC end sequence */
    unsigned startSP6;	/* Start position in chrom for SP6 end sequence */
    unsigned endSP6;	/* End position in chrom for SP6 end sequence */
    char strandSP6[2];	/* + or - */
    char *stsMarker;	/* Name of STS marker found in clone */
    unsigned stsStart;	/* Start position in chrom for STS marker */
    unsigned stsEnd;	/* End position in chrom for STS marker */
    };

void uPennClonesStaticLoad(char **row, struct uPennClones *ret);
/* Load a row from uPennClones table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct uPennClones *uPennClonesLoad(char **row);
/* Load a uPennClones from row fetched with select * from uPennClones
 * from database.  Dispose of this with uPennClonesFree(). */

struct uPennClones *uPennClonesLoadAll(char *fileName);
/* Load all uPennClones from a tab-separated file.
 * Dispose of this with uPennClonesFreeList(). */

struct uPennClones *uPennClonesLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all uPennClones from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with uPennClonesFreeList(). */

struct uPennClones *uPennClonesCommaIn(char **pS, struct uPennClones *ret);
/* Create a uPennClones out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new uPennClones */

void uPennClonesFree(struct uPennClones **pEl);
/* Free a single dynamically allocated uPennClones such as created
 * with uPennClonesLoad(). */

void uPennClonesFreeList(struct uPennClones **pList);
/* Free a list of dynamically allocated uPennClones's */

void uPennClonesOutput(struct uPennClones *el, FILE *f, char sep, char lastSep);
/* Print out uPennClones.  Separate fields with sep. Follow last field with lastSep. */

#define uPennClonesTabOut(el,f) uPennClonesOutput(el,f,'\t','\n');
/* Print out uPennClones as a line in a tab-separated file. */

#define uPennClonesCommaOut(el,f) uPennClonesOutput(el,f,',',',');
/* Print out uPennClones as a comma separated list including final comma. */

#endif /* UPENNCLONES_H */

