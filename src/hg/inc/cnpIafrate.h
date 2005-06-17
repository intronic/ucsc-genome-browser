/* cnpIafrate.h was originally generated by the autoSql program, which also 
 * generated cnpIafrate.c and cnpIafrate.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CNPIAFRATE_H
#define CNPIAFRATE_H

#define CNPIAFRATE_NUM_COLS 6

struct cnpIafrate
/* CNP data from Iafrate lab */
    {
    struct cnpIafrate *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    char *variationType;	/* {Gain},{Loss},{Gain and Loss} */
    float score;	/* Score */
    };

void cnpIafrateStaticLoad(char **row, struct cnpIafrate *ret);
/* Load a row from cnpIafrate table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct cnpIafrate *cnpIafrateLoad(char **row);
/* Load a cnpIafrate from row fetched with select * from cnpIafrate
 * from database.  Dispose of this with cnpIafrateFree(). */

struct cnpIafrate *cnpIafrateLoadAll(char *fileName);
/* Load all cnpIafrate from whitespace-separated file.
 * Dispose of this with cnpIafrateFreeList(). */

struct cnpIafrate *cnpIafrateLoadAllByChar(char *fileName, char chopper);
/* Load all cnpIafrate from chopper separated file.
 * Dispose of this with cnpIafrateFreeList(). */

#define cnpIafrateLoadAllByTab(a) cnpIafrateLoadAllByChar(a, '\t');
/* Load all cnpIafrate from tab separated file.
 * Dispose of this with cnpIafrateFreeList(). */

struct cnpIafrate *cnpIafrateCommaIn(char **pS, struct cnpIafrate *ret);
/* Create a cnpIafrate out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cnpIafrate */

void cnpIafrateFree(struct cnpIafrate **pEl);
/* Free a single dynamically allocated cnpIafrate such as created
 * with cnpIafrateLoad(). */

void cnpIafrateFreeList(struct cnpIafrate **pList);
/* Free a list of dynamically allocated cnpIafrate's */

void cnpIafrateOutput(struct cnpIafrate *el, FILE *f, char sep, char lastSep);
/* Print out cnpIafrate.  Separate fields with sep. Follow last field with lastSep. */

#define cnpIafrateTabOut(el,f) cnpIafrateOutput(el,f,'\t','\n');
/* Print out cnpIafrate as a line in a tab-separated file. */

#define cnpIafrateCommaOut(el,f) cnpIafrateOutput(el,f,',',',');
/* Print out cnpIafrate as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CNPIAFRATE_H */

