/* hgdpGeo.h was originally generated by the autoSql program, which also 
 * generated hgdpGeo.c and hgdpGeo.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HGDPGEO_H
#define HGDPGEO_H

#define HGDPGEO_NUM_COLS 7
#define HGDPGEO_POP_COUNT 53

struct hgdpGeo
/* Human Genome Diversity Project population allele frequencies (bed 4+) */
    {
    struct hgdpGeo *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* SNP ID (dbSNP reference SNP rsNNNNN ID) */
    char ancestralAllele;	/* Ancestral allele on forward strand of reference genome assembly */
    char derivedAllele;	/* Derived allele on forward strand of reference genome assembly */
    float popFreqs[HGDPGEO_POP_COUNT];	/* For each population in alphabetical order, the proportion of the population carrying the ancestral allele. */
    };

void hgdpGeoStaticLoad(char **row, struct hgdpGeo *ret);
/* Load a row from hgdpGeo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hgdpGeo *hgdpGeoLoad(char **row);
/* Load a hgdpGeo from row fetched with select * from hgdpGeo
 * from database.  Dispose of this with hgdpGeoFree(). */

struct hgdpGeo *hgdpGeoLoadAll(char *fileName);
/* Load all hgdpGeo from whitespace-separated file.
 * Dispose of this with hgdpGeoFreeList(). */

struct hgdpGeo *hgdpGeoLoadAllByChar(char *fileName, char chopper);
/* Load all hgdpGeo from chopper separated file.
 * Dispose of this with hgdpGeoFreeList(). */

#define hgdpGeoLoadAllByTab(a) hgdpGeoLoadAllByChar(a, '\t');
/* Load all hgdpGeo from tab separated file.
 * Dispose of this with hgdpGeoFreeList(). */

struct hgdpGeo *hgdpGeoCommaIn(char **pS, struct hgdpGeo *ret);
/* Create a hgdpGeo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hgdpGeo */

void hgdpGeoFree(struct hgdpGeo **pEl);
/* Free a single dynamically allocated hgdpGeo such as created
 * with hgdpGeoLoad(). */

void hgdpGeoFreeList(struct hgdpGeo **pList);
/* Free a list of dynamically allocated hgdpGeo's */

void hgdpGeoOutput(struct hgdpGeo *el, FILE *f, char sep, char lastSep);
/* Print out hgdpGeo.  Separate fields with sep. Follow last field with lastSep. */

#define hgdpGeoTabOut(el,f) hgdpGeoOutput(el,f,'\t','\n');
/* Print out hgdpGeo as a line in a tab-separated file. */

#define hgdpGeoCommaOut(el,f) hgdpGeoOutput(el,f,',',',');
/* Print out hgdpGeo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */


#endif /* HGDPGEO_H */

