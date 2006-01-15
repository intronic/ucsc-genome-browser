/* snpTmp.h was originally generated by the autoSql program, which also 
 * generated snpTmp.c and snpTmp.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef SNPTMP_H
#define SNPTMP_H

#define SNPTMP_NUM_COLS 7

struct snpTmp
/* Polymorphism data subset used during processing */
    {
    struct snpTmp *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    char strand[2];	/* Which DNA strand contains the observed alleles */
    char *refNCBI;	/* Reference genomic from dbSNP */
    char *locType;	/* range, exact, between */
    };

void snpTmpStaticLoad(char **row, struct snpTmp *ret);
/* Load a row from snpTmp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snpTmp *snpTmpLoad(char **row);
/* Load a snpTmp from row fetched with select * from snpTmp
 * from database.  Dispose of this with snpTmpFree(). */

struct snpTmp *snpTmpLoadAll(char *fileName);
/* Load all snpTmp from whitespace-separated file.
 * Dispose of this with snpTmpFreeList(). */

struct snpTmp *snpTmpLoadAllByChar(char *fileName, char chopper);
/* Load all snpTmp from chopper separated file.
 * Dispose of this with snpTmpFreeList(). */

#define snpTmpLoadAllByTab(a) snpTmpLoadAllByChar(a, '\t');
/* Load all snpTmp from tab separated file.
 * Dispose of this with snpTmpFreeList(). */

struct snpTmp *snpTmpCommaIn(char **pS, struct snpTmp *ret);
/* Create a snpTmp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snpTmp */

void snpTmpFree(struct snpTmp **pEl);
/* Free a single dynamically allocated snpTmp such as created
 * with snpTmpLoad(). */

void snpTmpFreeList(struct snpTmp **pList);
/* Free a list of dynamically allocated snpTmp's */

void snpTmpOutput(struct snpTmp *el, FILE *f, char sep, char lastSep);
/* Print out snpTmp.  Separate fields with sep. Follow last field with lastSep. */

#define snpTmpTabOut(el,f) snpTmpOutput(el,f,'\t','\n');
/* Print out snpTmp as a line in a tab-separated file. */

#define snpTmpCommaOut(el,f) snpTmpOutput(el,f,',',',');
/* Print out snpTmp as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SNPTMP_H */

void snpTmpTableCreate(struct sqlConnection *conn, char *chromName);
/* Create a chrN_snpTmp table. */

int snpTmpCmp(const void *va, const void *vb);

