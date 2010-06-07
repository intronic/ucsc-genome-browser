/* tfbsConsFactors.h was originally generated by the autoSql program, which also 
 * generated tfbsConsFactors.c and tfbsConsFactors.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef TFBSCONSFACTORS_H
#define TFBSCONSFACTORS_H

#define TFBSCONSFACTORS_NUM_COLS 5

struct tfbsConsFactors
/* tfbsConsFactors Data */
    {
    struct tfbsConsFactors *next;  /* Next in singly linked list. */
    char *name;	/* Name of item */
    char *ac;	/* gene-regulation.com AC */
    char species[7];	/* common name, scientific name */
    char factor[65];	/* factor  */
    char id[26];	/* id */
    };

void tfbsConsFactorsStaticLoad(char **row, struct tfbsConsFactors *ret);
/* Load a row from tfbsConsFactors table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct tfbsConsFactors *tfbsConsFactorsLoad(char **row);
/* Load a tfbsConsFactors from row fetched with select * from tfbsConsFactors
 * from database.  Dispose of this with tfbsConsFactorsFree(). */

struct tfbsConsFactors *tfbsConsFactorsLoadAll(char *fileName);
/* Load all tfbsConsFactors from whitespace-separated file.
 * Dispose of this with tfbsConsFactorsFreeList(). */

struct tfbsConsFactors *tfbsConsFactorsLoadAllByChar(char *fileName, char chopper);
/* Load all tfbsConsFactors from chopper separated file.
 * Dispose of this with tfbsConsFactorsFreeList(). */

#define tfbsConsFactorsLoadAllByTab(a) tfbsConsFactorsLoadAllByChar(a, '\t');
/* Load all tfbsConsFactors from tab separated file.
 * Dispose of this with tfbsConsFactorsFreeList(). */

struct tfbsConsFactors *tfbsConsFactorsCommaIn(char **pS, struct tfbsConsFactors *ret);
/* Create a tfbsConsFactors out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new tfbsConsFactors */

void tfbsConsFactorsFree(struct tfbsConsFactors **pEl);
/* Free a single dynamically allocated tfbsConsFactors such as created
 * with tfbsConsFactorsLoad(). */

void tfbsConsFactorsFreeList(struct tfbsConsFactors **pList);
/* Free a list of dynamically allocated tfbsConsFactors's */

void tfbsConsFactorsOutput(struct tfbsConsFactors *el, FILE *f, char sep, char lastSep);
/* Print out tfbsConsFactors.  Separate fields with sep. Follow last field with lastSep. */

#define tfbsConsFactorsTabOut(el,f) tfbsConsFactorsOutput(el,f,'\t','\n');
/* Print out tfbsConsFactors as a line in a tab-separated file. */

#define tfbsConsFactorsCommaOut(el,f) tfbsConsFactorsOutput(el,f,',',',');
/* Print out tfbsConsFactors as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* TFBSCONSFACTORS_H */

