/* ccdsAccessions.h was originally generated by the autoSql program, which also 
 * generated ccdsAccessions.c and ccdsAccessions.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CCDSACCESSIONS_H
#define CCDSACCESSIONS_H

#define CCDSACCESSIONS_NUM_COLS 18

struct exonCoords
/* coordinate of an exon (hand addition) */
{
    unsigned start;
    unsigned end;
};


struct ccdsAccessions
/* parses NcbiHinxtonAllAccessions.txt from NCBI */
    {
    struct ccdsAccessions *next;  /* Next in singly linked list. */
    int tax_id;	/* taxon id */
    char *chromosome;	/* chromosome accession.version */
    int gene_id;	/* gene id */
    char *group_id;	/* group id.version */
    char *ccds;	/* ccds1.1 */
    char *group_ccds_status;	/* status of CCDS */
    char *ncbi_mrna;	/* acc.version */
    char *ncbi_prot;	/* acc.version */
    char *hinxton_mrna;	/* ensemble id */
    char *hinxton_prot;	/* ensemble id */
    char *ncbi_mrna_non;	/* when non-ccds */
    char *ncbi_prot_non;	/* when non-ccds */
    char *hinxton_mrna_non;	/* when non-ccds */
    char *hinxton_prot_non;	/* when non-ccds */
    char cds_strand[2];	/* + or - */
    int cds_from;	/* in 0-based chromosome coordinates  */
    int cds_to;	/* in 0-based chromosome coordinates */
    char *cds_loc;	/* comma separated list of from-to, in 0-based chromosome coordinates, for all exons */

    /* fields not contained in the file and built externally. */
    int ncbiAccCnt;    /* accessions split into lists (stored as a single memory block) */
    char **ncbiMRnaLst;
    char **ncbiProtLst;
    int hinxtonAccCnt;
    char **hinxtonMRnaLst;
    char **hinxtonProtLst;
    int numExons;       /* exons split into a list, not converted to open-ended */
    struct exonCoords *exons;
    };

void ccdsAccessionsStaticLoad(char **row, struct ccdsAccessions *ret);
/* Load a row from ccdsAccessions table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct ccdsAccessions *ccdsAccessionsLoad(char **row);
/* Load a ccdsAccessions from row fetched with select * from ccdsAccessions
 * from database.  Dispose of this with ccdsAccessionsFree(). */

struct ccdsAccessions *ccdsAccessionsLoadAll(char *fileName);
/* Load all ccdsAccessions from whitespace-separated file.
 * Dispose of this with ccdsAccessionsFreeList(). */

struct ccdsAccessions *ccdsAccessionsLoadAllByChar(char *fileName, char chopper);
/* Load all ccdsAccessions from chopper separated file.
 * Dispose of this with ccdsAccessionsFreeList(). */

#define ccdsAccessionsLoadAllByTab(a) ccdsAccessionsLoadAllByChar(a, '\t');
/* Load all ccdsAccessions from tab separated file.
 * Dispose of this with ccdsAccessionsFreeList(). */

struct ccdsAccessions *ccdsAccessionsCommaIn(char **pS, struct ccdsAccessions *ret);
/* Create a ccdsAccessions out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ccdsAccessions */

void ccdsAccessionsFree(struct ccdsAccessions **pEl);
/* Free a single dynamically allocated ccdsAccessions such as created
 * with ccdsAccessionsLoad(). */

void ccdsAccessionsFreeList(struct ccdsAccessions **pList);
/* Free a list of dynamically allocated ccdsAccessions's */

void ccdsAccessionsOutput(struct ccdsAccessions *el, FILE *f, char sep, char lastSep);
/* Print out ccdsAccessions.  Separate fields with sep. Follow last field with lastSep. */

#define ccdsAccessionsTabOut(el,f) ccdsAccessionsOutput(el,f,'\t','\n');
/* Print out ccdsAccessions as a line in a tab-separated file. */

#define ccdsAccessionsCommaOut(el,f) ccdsAccessionsOutput(el,f,',',',');
/* Print out ccdsAccessions as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CCDSACCESSIONS_H */

