/* softberryHom.h was originally generated by the autoSql program, which also 
 * generated softberryHom.c and softberryHom.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef SOFTBERRYHOM_H
#define SOFTBERRYHOM_H

struct softberryHom
/* Protein homologies behind Softberry genes */
    {
    struct softberryHom *next;  /* Next in singly linked list. */
    char *name;	/* Softberry gene name */
    char *giString;	/* String with Genbank gi and accession */
    char *description;	/* Freeform (except for no tabs) description */
    };

void softberryHomStaticLoad(char **row, struct softberryHom *ret);
/* Load a row from softberryHom table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct softberryHom *softberryHomLoad(char **row);
/* Load a softberryHom from row fetched with select * from softberryHom
 * from database.  Dispose of this with softberryHomFree(). */

struct softberryHom *softberryHomCommaIn(char **pS, struct softberryHom *ret);
/* Create a softberryHom out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new softberryHom */

void softberryHomFree(struct softberryHom **pEl);
/* Free a single dynamically allocated softberryHom such as created
 * with softberryHomLoad(). */

void softberryHomFreeList(struct softberryHom **pList);
/* Free a list of dynamically allocated softberryHom's */

void softberryHomOutput(struct softberryHom *el, FILE *f, char sep, char lastSep);
/* Print out softberryHom.  Separate fields with sep. Follow last field with lastSep. */

#define softberryHomTabOut(el,f) softberryHomOutput(el,f,'\t','\n');
/* Print out softberryHom as a line in a tab-separated file. */

#define softberryHomCommaOut(el,f) softberryHomOutput(el,f,',',',');
/* Print out softberryHom as a comma separated list including final comma. */

#endif /* SOFTBERRYHOM_H */

