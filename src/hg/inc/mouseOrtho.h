/* mouseOrtho.h was originally generated by the autoSql program, which also 
 * generated mouseOrtho.c and mouseOrtho.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef MOUSEORTHO_H
#define MOUSEORTHO_H

struct mouseOrtho
/* Human Mouse Orthologs */
    {
    struct mouseOrtho *next;  /* Next in singly linked list. */
    char *chrom;	/* Human Chrom */
    unsigned chromStart;	/* Start on Human */
    unsigned chromEnd;	/* End on Human */
    char *name;	/* Mouse Gene Prediction */
    unsigned score;	/* Human mouse aligment score using Blat */
    char *strand;	/* Strand on Human */
    unsigned thickStart;	/* Start on Human */
    unsigned thickEnd;	/* End on Human */
    };

void mouseOrthoStaticLoad(char **row, struct mouseOrtho *ret);
/* Load a row from mouseOrtho table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct mouseOrtho *mouseOrthoLoad(char **row);
/* Load a mouseOrtho from row fetched with select * from mouseOrtho
 * from database.  Dispose of this with mouseOrthoFree(). */

struct mouseOrtho *mouseOrthoLoadAll(char *fileName);
/* Load all mouseOrtho from a tab-separated file.
 * Dispose of this with mouseOrthoFreeList(). */

struct mouseOrtho *mouseOrthoLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all mouseOrtho from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with mouseOrthoFreeList(). */

struct mouseOrtho *mouseOrthoCommaIn(char **pS, struct mouseOrtho *ret);
/* Create a mouseOrtho out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mouseOrtho */

void mouseOrthoFree(struct mouseOrtho **pEl);
/* Free a single dynamically allocated mouseOrtho such as created
 * with mouseOrthoLoad(). */

void mouseOrthoFreeList(struct mouseOrtho **pList);
/* Free a list of dynamically allocated mouseOrtho's */

void mouseOrthoOutput(struct mouseOrtho *el, FILE *f, char sep, char lastSep);
/* Print out mouseOrtho.  Separate fields with sep. Follow last field with lastSep. */

#define mouseOrthoTabOut(el,f) mouseOrthoOutput(el,f,'\t','\n');
/* Print out mouseOrtho as a line in a tab-separated file. */

#define mouseOrthoCommaOut(el,f) mouseOrthoOutput(el,f,',',',');
/* Print out mouseOrtho as a comma separated list including final comma. */

#endif /* MOUSEORTHO_H */

