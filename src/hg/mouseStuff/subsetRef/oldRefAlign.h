/* oldRefAlign.h was originally generated by the autoSql program, which also 
 * generated oldRefAlign.c and oldRefAlign.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef OLDREFALIGN_H
#define OLDREFALIGN_H

struct refAlign
/* Contains a region of a reference alignment */
    {
    struct refAlign *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    unsigned score;	/* Score from 0-1000 */
    unsigned matches;	/* Number of bases that match */
    unsigned misMatches;	/* Number of bases that don't match */
    unsigned aNumInsert;	/* Number of inserts in aligned seq */
    int aBaseInsert;	/* Number of bases inserted in query */
    unsigned hNumInsert;	/* Number of inserts in human */
    int hBaseInsert;	/* Number of bases inserted in human */
    char *humanSeq;	/* Human sequence, contains - for aligned seq inserts */
    char *alignSeq;	/* Aligned sequence, contains - for human seq inserts */
    };

void refAlignStaticLoad(char **row, struct refAlign *ret);
/* Load a row from refAlign table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct refAlign *refAlignLoad(char **row);
/* Load a refAlign from row fetched with select * from refAlign
 * from database.  Dispose of this with refAlignFree(). */

struct refAlign *refAlignLoadAll(char *fileName);
/* Load all refAlign from a tab-separated file.
 * Dispose of this with refAlignFreeList(). */

struct refAlign *refAlignCommaIn(char **pS, struct refAlign *ret);
/* Create a refAlign out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new refAlign */

void refAlignFree(struct refAlign **pEl);
/* Free a single dynamically allocated refAlign such as created
 * with refAlignLoad(). */

void refAlignFreeList(struct refAlign **pList);
/* Free a list of dynamically allocated refAlign's */

void refAlignOutput(struct refAlign *el, FILE *f, char sep, char lastSep);
/* Print out refAlign.  Separate fields with sep. Follow last field with lastSep. */

#define refAlignTabOut(el,f) refAlignOutput(el,f,'\t','\n');
/* Print out refAlign as a line in a tab-separated file. */

#define refAlignCommaOut(el,f) refAlignOutput(el,f,',',',');
/* Print out refAlign as a comma separated list including final comma. */

#endif /* OLDREFALIGN_H */

