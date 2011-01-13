/* wgEncodeGencodeRefSeq.h was originally generated by the autoSql program, which also 
 * generated wgEncodeGencodeRefSeq.c and wgEncodeGencodeRefSeq.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef WGENCODEGENCODEREFSEQ_H
#define WGENCODEGENCODEREFSEQ_H

#define WGENCODEGENCODEREFSEQ_NUM_COLS 3

struct wgEncodeGencodeRefSeq
/* Gencode metadata table of associated RefSeq mRNA or non-coding RNA and peptide accessions */
    {
    struct wgEncodeGencodeRefSeq *next;  /* Next in singly linked list. */
    char *transcriptId;	/* Transcript ID for Gencode gene */
    char *refSeqRnaId;	/* RefSeq mRNA or non-coding RNA ID */
    char *refSeqPepId;	/* RefSeq peptide ID for coding transcripts */
    };

void wgEncodeGencodeRefSeqStaticLoad(char **row, struct wgEncodeGencodeRefSeq *ret);
/* Load a row from wgEncodeGencodeRefSeq table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wgEncodeGencodeRefSeq *wgEncodeGencodeRefSeqLoad(char **row);
/* Load a wgEncodeGencodeRefSeq from row fetched with select * from wgEncodeGencodeRefSeq
 * from database.  Dispose of this with wgEncodeGencodeRefSeqFree(). */

struct wgEncodeGencodeRefSeq *wgEncodeGencodeRefSeqLoadAll(char *fileName);
/* Load all wgEncodeGencodeRefSeq from whitespace-separated file.
 * Dispose of this with wgEncodeGencodeRefSeqFreeList(). */

struct wgEncodeGencodeRefSeq *wgEncodeGencodeRefSeqLoadAllByChar(char *fileName, char chopper);
/* Load all wgEncodeGencodeRefSeq from chopper separated file.
 * Dispose of this with wgEncodeGencodeRefSeqFreeList(). */

#define wgEncodeGencodeRefSeqLoadAllByTab(a) wgEncodeGencodeRefSeqLoadAllByChar(a, '\t');
/* Load all wgEncodeGencodeRefSeq from tab separated file.
 * Dispose of this with wgEncodeGencodeRefSeqFreeList(). */

struct wgEncodeGencodeRefSeq *wgEncodeGencodeRefSeqCommaIn(char **pS, struct wgEncodeGencodeRefSeq *ret);
/* Create a wgEncodeGencodeRefSeq out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wgEncodeGencodeRefSeq */

void wgEncodeGencodeRefSeqFree(struct wgEncodeGencodeRefSeq **pEl);
/* Free a single dynamically allocated wgEncodeGencodeRefSeq such as created
 * with wgEncodeGencodeRefSeqLoad(). */

void wgEncodeGencodeRefSeqFreeList(struct wgEncodeGencodeRefSeq **pList);
/* Free a list of dynamically allocated wgEncodeGencodeRefSeq's */

void wgEncodeGencodeRefSeqOutput(struct wgEncodeGencodeRefSeq *el, FILE *f, char sep, char lastSep);
/* Print out wgEncodeGencodeRefSeq.  Separate fields with sep. Follow last field with lastSep. */

#define wgEncodeGencodeRefSeqTabOut(el,f) wgEncodeGencodeRefSeqOutput(el,f,'\t','\n');
/* Print out wgEncodeGencodeRefSeq as a line in a tab-separated file. */

#define wgEncodeGencodeRefSeqCommaOut(el,f) wgEncodeGencodeRefSeqOutput(el,f,',',',');
/* Print out wgEncodeGencodeRefSeq as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* WGENCODEGENCODEREFSEQ_H */

