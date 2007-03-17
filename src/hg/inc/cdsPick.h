/* cdsPick.h was originally generated by the autoSql program, which also 
 * generated cdsPick.c and cdsPick.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CDSPICK_H
#define CDSPICK_H

#define CDSPICK_NUM_COLS 11

struct cdsPick
/* Information on CDS that we picked from competing cdsEvidence */
    {
    struct cdsPick *next;  /* Next in singly linked list. */
    char *name;	/* Name of transcript */
    int start;	/* CDS start within transcript, zero based */
    int end;	/* CDS end, non-inclusive */
    char *source;	/* Source of best evidence */
    double score;	/* Higher is better. */
    unsigned char startComplete;	/* Starts with ATG? */
    unsigned char endComplete;	/* Ends with stop codon? */
    char *swissProt;	/* Matching swissProt if available. */
    char *uniProt;	/* Matching uniProt if available. */
    char *refProt;	/* RefSeq protein if available. */
    char *refSeq;	/* RefSeq transcript if available. */
    };

void cdsPickStaticLoad(char **row, struct cdsPick *ret);
/* Load a row from cdsPick table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct cdsPick *cdsPickLoad(char **row);
/* Load a cdsPick from row fetched with select * from cdsPick
 * from database.  Dispose of this with cdsPickFree(). */

struct cdsPick *cdsPickLoadAll(char *fileName);
/* Load all cdsPick from whitespace-separated file.
 * Dispose of this with cdsPickFreeList(). */

struct cdsPick *cdsPickLoadAllByChar(char *fileName, char chopper);
/* Load all cdsPick from chopper separated file.
 * Dispose of this with cdsPickFreeList(). */

#define cdsPickLoadAllByTab(a) cdsPickLoadAllByChar(a, '\t');
/* Load all cdsPick from tab separated file.
 * Dispose of this with cdsPickFreeList(). */

struct cdsPick *cdsPickCommaIn(char **pS, struct cdsPick *ret);
/* Create a cdsPick out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new cdsPick */

void cdsPickFree(struct cdsPick **pEl);
/* Free a single dynamically allocated cdsPick such as created
 * with cdsPickLoad(). */

void cdsPickFreeList(struct cdsPick **pList);
/* Free a list of dynamically allocated cdsPick's */

void cdsPickOutput(struct cdsPick *el, FILE *f, char sep, char lastSep);
/* Print out cdsPick.  Separate fields with sep. Follow last field with lastSep. */

#define cdsPickTabOut(el,f) cdsPickOutput(el,f,'\t','\n');
/* Print out cdsPick as a line in a tab-separated file. */

#define cdsPickCommaOut(el,f) cdsPickOutput(el,f,',',',');
/* Print out cdsPick as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CDSPICK_H */

