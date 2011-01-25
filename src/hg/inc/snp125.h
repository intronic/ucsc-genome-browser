/* snp125.h was originally generated by the autoSql program, which also 
 * generated snp125.c and snp125.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef SNP125_H
#define SNP125_H

#include "memgfx.h"
#define SNP125_NUM_COLS 17

struct snp125
/* Polymorphism data from dbSnp database or genotyping arrays */
    {
    struct snp125 *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    unsigned score;	/* Not used */
    char *strand;	/* Which DNA strand contains the observed alleles */
    char *refNCBI;	/* Reference genomic from dbSNP */
    char *refUCSC;	/* Reference genomic from nib lookup */
    char *observed;	/* The sequences of the observed alleles from rs-fasta files */
    char *molType;	/* Sample type from exemplar ss */
    char *class;	/* The class of variant (simple, insertion, deletion, range, etc.) */
    char *valid;	/* The validation status of the SNP */
    float avHet;	/* The average heterozygosity from all observations */
    float avHetSE;	/* The Standard Error for the average heterozygosity */
    char *func;	/* The functional category of the SNP (coding-synon, coding-nonsynon, intron, etc.) */
    char *locType;	/* How the variant affects the reference sequence */
    unsigned weight;	/* The quality of the alignment */
    };

void snp125StaticLoad(char **row, struct snp125 *ret);
/* Load a row from snp125 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snp125 *snp125Load(char **row);
/* Load a snp125 from row fetched with select * from snp125
 * from database.  Dispose of this with snp125Free(). */

struct snp125 *snp125LoadAll(char *fileName);
/* Load all snp125 from whitespace-separated file.
 * Dispose of this with snp125FreeList(). */

struct snp125 *snp125LoadAllByChar(char *fileName, char chopper);
/* Load all snp125 from chopper separated file.
 * Dispose of this with snp125FreeList(). */

#define snp125LoadAllByTab(a) snp125LoadAllByChar(a, '\t');
/* Load all snp125 from tab separated file.
 * Dispose of this with snp125FreeList(). */

struct snp125 *snp125CommaIn(char **pS, struct snp125 *ret);
/* Create a snp125 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snp125 */

void snp125Free(struct snp125 **pEl);
/* Free a single dynamically allocated snp125 such as created
 * with snp125Load(). */

void snp125FreeList(struct snp125 **pList);
/* Free a list of dynamically allocated snp125's */

void snp125Output(struct snp125 *el, FILE *f, char sep, char lastSep);
/* Print out snp125.  Separate fields with sep. Follow last field with lastSep. */

#define snp125TabOut(el,f) snp125Output(el,f,'\t','\n');
/* Print out snp125 as a line in a tab-separated file. */

#define snp125CommaOut(el,f) snp125Output(el,f,',',',');
/* Print out snp125 as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */


void snp125TableCreate(struct sqlConnection *conn, char *tableName);
/* create a snp125 table */

int snp125Cmp(const void *va, const void *vb);



struct snp125Extended
/* Polymorphism data from dbSnp database or genotyping arrays */
    {
    struct snp125Extended *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    unsigned score;	/* Not used */
    char *strand;	/* Which DNA strand contains the observed alleles */
    char *refNCBI;	/* Reference genomic from dbSNP */
    char *refUCSC;	/* Reference genomic from nib lookup */
    char *observed;	/* The sequences of the observed alleles from rs-fasta files */
    char *molType;	/* Sample type from exemplar ss */
    char *class;	/* The class of variant (simple, insertion, deletion, range, etc.) */
    char *valid;	/* The validation status of the SNP */
    float avHet;	/* The average heterozygosity from all observations */
    float avHetSE;	/* The Standard Error for the average heterozygosity */
    char *func;	/* The functional category of the SNP (coding-synon, coding-nonsynon, intron, etc.) */
    char *locType;	/* How the variant affects the reference sequence */
    unsigned weight;	/* The quality of the alignment */
	/*  extra fields */
    char *nameExtra;    /* additional text to be drawn with name in image */
    Color color;       /* color for drawing in hgTracks */
    };

struct snp125Extended *snpExtendedLoad(char **row);
/* Load a snp125 from row fetched with select * from snp125 from
 * database.  Additional fields are for run-time drawing and
 * calculations */

int snpVersion(char *track);
/* If track starts with snpNNN where NNN is 125 or later, return the number;
 * otherwise return 0. */

#endif /* SNP125_H */

