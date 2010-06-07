/* traceInfo.h was originally generated by the autoSql program, which also 
 * generated traceInfo.c and traceInfo.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef TRACEINFO_H
#define TRACEINFO_H

struct traceInfo
/* subset of the sequencing trace ancillary information, see http://www.ncbi.nlm.nih.gov/Traces/TraceArchiveRFC.html */
    {
    struct traceInfo *next;  /* Next in singly linked list. */
    char *ti;	/* Trace identifier */
    char *templateId;	/* Name of the template */
    unsigned size;	/* Size of read */
    };

void traceInfoStaticLoad(char **row, struct traceInfo *ret);
/* Load a row from traceInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct traceInfo *traceInfoLoad(char **row);
/* Load a traceInfo from row fetched with select * from traceInfo
 * from database.  Dispose of this with traceInfoFree(). */

struct traceInfo *traceInfoLoadAll(char *fileName);
/* Load all traceInfo from a tab-separated file.
 * Dispose of this with traceInfoFreeList(). */

struct traceInfo *traceInfoCommaIn(char **pS, struct traceInfo *ret);
/* Create a traceInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new traceInfo */

void traceInfoFree(struct traceInfo **pEl);
/* Free a single dynamically allocated traceInfo such as created
 * with traceInfoLoad(). */

void traceInfoFreeList(struct traceInfo **pList);
/* Free a list of dynamically allocated traceInfo's */

void traceInfoOutput(struct traceInfo *el, FILE *f, char sep, char lastSep);
/* Print out traceInfo.  Separate fields with sep. Follow last field with lastSep. */

#define traceInfoTabOut(el,f) traceInfoOutput(el,f,'\t','\n');
/* Print out traceInfo as a line in a tab-separated file. */

#define traceInfoCommaOut(el,f) traceInfoOutput(el,f,',',',');
/* Print out traceInfo as a comma separated list including final comma. */

#endif /* TRACEINFO_H */

