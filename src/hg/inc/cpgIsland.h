/* cpgIsland.h was originally generated by the autoSql program, which also 
 * generated cpgIsland.c and cpgIsland.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef CPGISLAND_H
#define CPGISLAND_H

struct cpgIsland
/* Describes the CpG Islands */
    {
    struct cpgIsland *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome or FPC contig */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* CpG Island */
    unsigned length;	/* Island Length */
    unsigned cpgNum;	/* Number of CpGs in island */
    unsigned gcNum;	/* Number of C and G in island */
    float perCpg;	/* Percentage of island that is CpG */
    float perGc;	/* Percentage of island that is C or G */
    };

void cpgIslandStaticLoad(char **row, struct cpgIsland *ret);
/* Load a row from cpgIsland table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct cpgIsland *cpgIslandLoad(char **row);
/* Load a cpgIsland from row fetched with select * from cpgIsland
 * from database.  Dispose of this with cpgIslandFree(). */

struct cpgIsland *cpgIslandCommaIn(char **pS, struct cpgIsland *ret);
/* Create a cpgIsland out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cpgIsland */

void cpgIslandFree(struct cpgIsland **pEl);
/* Free a single dynamically allocated cpgIsland such as created
 * with cpgIslandLoad(). */

void cpgIslandFreeList(struct cpgIsland **pList);
/* Free a list of dynamically allocated cpgIsland's */

void cpgIslandOutput(struct cpgIsland *el, FILE *f, char sep, char lastSep);
/* Print out cpgIsland.  Separate fields with sep. Follow last field with lastSep. */

#define cpgIslandTabOut(el,f) cpgIslandOutput(el,f,'\t','\n');
/* Print out cpgIsland as a line in a tab-separated file. */

#define cpgIslandCommaOut(el,f) cpgIslandOutput(el,f,',',',');
/* Print out cpgIsland as a comma separated list including final comma. */

#endif /* CPGISLAND_H */

