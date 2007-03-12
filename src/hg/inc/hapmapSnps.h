/* hapmapSnps.h was originally generated by the autoSql program, which also 
 * generated hapmapSnps.c and hapmapSnps.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HAPMAPSNPS_H
#define HAPMAPSNPS_H

#define HAPMAPSNPS_NUM_COLS 12

struct hapmapSnps
/* HapMap genotype summary */
    {
    struct hapmapSnps *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom (0 based) */
    unsigned chromEnd;	/* End position in chrom (1 based) */
    char *name;	/* Reference SNP identifier from dbSnp */
    unsigned score;	/* Not used */
    char strand[2];	/* Which genomic strand contains the observed alleles */
    char *observed;	/* Observed string from genotype file */
    char allele1[2];	/* This allele has been observed */
    unsigned homoCount1;	/* Count of individuals who are homozygous for allele1 */
    char *allele2;	/* This allele may not have been observed */
    unsigned homoCount2;	/* Count of individuals who are homozygous for allele2 */
    unsigned heteroCount;	/* Count of individuals who are heterozygous */
    };

void hapmapSnpsStaticLoad(char **row, struct hapmapSnps *ret);
/* Load a row from hapmapSnps table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hapmapSnps *hapmapSnpsLoad(char **row);
/* Load a hapmapSnps from row fetched with select * from hapmapSnps
 * from database.  Dispose of this with hapmapSnpsFree(). */

struct hapmapSnps *hapmapSnpsLoadAll(char *fileName);
/* Load all hapmapSnps from whitespace-separated file.
 * Dispose of this with hapmapSnpsFreeList(). */

struct hapmapSnps *hapmapSnpsLoadAllByChar(char *fileName, char chopper);
/* Load all hapmapSnps from chopper separated file.
 * Dispose of this with hapmapSnpsFreeList(). */

#define hapmapSnpsLoadAllByTab(a) hapmapSnpsLoadAllByChar(a, '\t');
/* Load all hapmapSnps from tab separated file.
 * Dispose of this with hapmapSnpsFreeList(). */

struct hapmapSnps *hapmapSnpsCommaIn(char **pS, struct hapmapSnps *ret);
/* Create a hapmapSnps out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hapmapSnps */

void hapmapSnpsFree(struct hapmapSnps **pEl);
/* Free a single dynamically allocated hapmapSnps such as created
 * with hapmapSnpsLoad(). */

void hapmapSnpsFreeList(struct hapmapSnps **pList);
/* Free a list of dynamically allocated hapmapSnps's */

void hapmapSnpsOutput(struct hapmapSnps *el, FILE *f, char sep, char lastSep);
/* Print out hapmapSnps.  Separate fields with sep. Follow last field with lastSep. */

#define hapmapSnpsTabOut(el,f) hapmapSnpsOutput(el,f,'\t','\n');
/* Print out hapmapSnps as a line in a tab-separated file. */

#define hapmapSnpsCommaOut(el,f) hapmapSnpsOutput(el,f,',',',');
/* Print out hapmapSnps as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */
/* items for trackUi options and filters */

#define HAP_POP_MIXED "hapmapSnps.isMixed"
#define HAP_POP_COUNT "hapmapSnps.popCount"
#define HAP_TYPE "hapmapSnps.polyType"
#define HAP_MIN_FREQ "hapmapSnps.minorAlleleFreqMinimum"
#define HAP_MAX_FREQ "hapmapSnps.minorAlleleFreqMaximum"
#define HAP_MIN_HET "hapmapSnps.hetMinimum"
#define HAP_MAX_HET "hapmapSnps.hetMaximum"
#define HAP_MONO "hapmapSnps.monomorphic"
#define HAP_CHIMP "hapmapSnps.chimp"
#define HAP_CHIMP_QUAL "hapmapSnps.chimpQualScore"
#define HAP_MACAQUE "hapmapSnps.macaque"
#define HAP_MACAQUE_QUAL "hapmapSnps.macaqueQualScore"

#define HAP_POP_MIXED_DEFAULT "any"
#define HAP_POP_COUNT_DEFAULT "any"
#define HAP_TYPE_DEFAULT      "any"

#define HAP_MIN_FREQ_DEFAULT  "0.0"
#define HAP_MAX_FREQ_DEFAULT  "0.5"
#define HAP_MIN_HET_DEFAULT   "0.0"
#define HAP_MAX_HET_DEFAULT   "0.5"

#define HAP_MONO_DEFAULT      "any"

#define HAP_CHIMP_DEFAULT     "any"
#define HAP_CHIMP_QUAL_DEFAULT "0"
#define HAP_MACAQUE_DEFAULT   "any"
#define HAP_MACAQUE_QUAL_DEFAULT "0"

#endif /* HAPMAPSNPS_H */


