/* ggMrnaAli.h was originally generated by the autoSql program, which also 
 * generated ggMrnaAli.c and ggMrnaAli.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef GGMRNAALI_H
#define GGMRNAALI_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#ifndef PSL_H
#include "psl.h"
#endif

#ifndef DNASEQ_H
#include "dnaseq.h"
#endif

struct ggMrnaBlock
/*  A single block of an mRNA alignment. */
    {
    struct ggMrnaBlock *next;  /* Next in singly linked list. */
    int qStart;	/* Start of block in query */
    int qEnd;	/* End of block in query */
    int tStart;	/* Start of block in target */
    int tEnd;	/* End of block in target */
    };

void ggMrnaBlockStaticLoad(char **row, struct ggMrnaBlock *ret);
/* Load a row from ggMrnaBlock table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct ggMrnaBlock *ggMrnaBlockLoad(char **row);
/* Load a ggMrnaBlock from row fetched with select * from ggMrnaBlock
 * from database.  Dispose of this with ggMrnaBlockFree(). */

struct ggMrnaBlock *ggMrnaBlockLoadAll(char *fileName);
/* Load all ggMrnaBlock from a tab-separated file.
 * Dispose of this with ggMrnaBlockFreeList(). */

struct ggMrnaBlock *ggMrnaBlockLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all ggMrnaBlock from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with ggMrnaBlockFreeList(). */

void ggMrnaBlockSaveToDb(struct sqlConnection *conn, struct ggMrnaBlock *el, char *tableName, int updateSize);
/* Save ggMrnaBlock as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use ggMrnaBlockSaveToDbEscaped() */

void ggMrnaBlockSaveToDbEscaped(struct sqlConnection *conn, struct ggMrnaBlock *el, char *tableName, int updateSize);
/* Save ggMrnaBlock as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than ggMrnaBlockSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct ggMrnaBlock *ggMrnaBlockCommaIn(char **pS, struct ggMrnaBlock *ret);
/* Create a ggMrnaBlock out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ggMrnaBlock */

void ggMrnaBlockFree(struct ggMrnaBlock **pEl);
/* Free a single dynamically allocated ggMrnaBlock such as created
 * with ggMrnaBlockLoad(). */

void ggMrnaBlockFreeList(struct ggMrnaBlock **pList);
/* Free a list of dynamically allocated ggMrnaBlock's */

void ggMrnaBlockOutput(struct ggMrnaBlock *el, FILE *f, char sep, char lastSep);
/* Print out ggMrnaBlock.  Separate fields with sep. Follow last field with lastSep. */

#define ggMrnaBlockTabOut(el,f) ggMrnaBlockOutput(el,f,'\t','\n');
/* Print out ggMrnaBlock as a line in a tab-separated file. */

#define ggMrnaBlockCommaOut(el,f) ggMrnaBlockOutput(el,f,',',',');
/* Print out ggMrnaBlock as a comma separated list including final comma. */

struct ggMrnaAli
/*  An mRNA alignment, little richer data format than psl  */
    {
    struct ggMrnaAli *next;  /* Next in singly linked list. */
    char *tName;	/* target that this alignment it to  */
    int tStart;	/* start in target sequence  */
    int tEnd;	/* end  in target sequence  */
    char strand[4];	/* + or - depending on which strand alignment is to  */
    char *qName;	/* name (accession) of mRNA  */
    int qStart;	/* start of alignment in query  */
    int qEnd;	/* end of alignment in query  */
    unsigned baseCount;	/* number of bases in query  */
    short orientation;	/* 1 or -1 orientation query appears to be in given biological evidence (i.e. splice site)  0 indicates no evidence. This could disagree with strand if est submitted rc'd for example  */
    int hasIntrons;	/* TRUE if intron present, FALSE otherwise */
    short milliScore;	/* Score 0-1000  */
    short blockCount;	/* Number of blocks.  */
    struct ggMrnaBlock *blocks;	/* Dynamically allocated array.  */
    };

struct ggMrnaAli *ggMrnaAliLoad(char **row);
/* Load a ggMrnaAli from row fetched with select * from ggMrnaAli
 * from database.  Dispose of this with ggMrnaAliFree(). */

struct ggMrnaAli *ggMrnaAliLoadAll(char *fileName);
/* Load all ggMrnaAli from a tab-separated file.
 * Dispose of this with ggMrnaAliFreeList(). */

struct ggMrnaAli *ggMrnaAliLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all ggMrnaAli from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with ggMrnaAliFreeList(). */

void ggMrnaAliSaveToDb(struct sqlConnection *conn, struct ggMrnaAli *el, char *tableName, int updateSize);
/* Save ggMrnaAli as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use ggMrnaAliSaveToDbEscaped() */

void ggMrnaAliSaveToDbEscaped(struct sqlConnection *conn, struct ggMrnaAli *el, char *tableName, int updateSize);
/* Save ggMrnaAli as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than ggMrnaAliSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct ggMrnaAli *ggMrnaAliCommaIn(char **pS, struct ggMrnaAli *ret);
/* Create a ggMrnaAli out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ggMrnaAli */

void ggMrnaAliFree(struct ggMrnaAli **pEl);
/* Free a single dynamically allocated ggMrnaAli such as created
 * with ggMrnaAliLoad(). */

void ggMrnaAliFreeList(struct ggMrnaAli **pList);
/* Free a list of dynamically allocated ggMrnaAli's */

void ggMrnaAliOutput(struct ggMrnaAli *el, FILE *f, char sep, char lastSep);
/* Print out ggMrnaAli.  Separate fields with sep. Follow last field with lastSep. */

#define ggMrnaAliTabOut(el,f) ggMrnaAliOutput(el,f,'\t','\n');
/* Print out ggMrnaAli as a line in a tab-separated file. */

#define ggMrnaAliCommaOut(el,f) ggMrnaAliOutput(el,f,',',',');
/* Print out ggMrnaAli as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct maRef 
/* Holds a reference to a ma */
    {
    struct maRef *next;  /* Next in list. */
    struct ggMrnaAli *ma;   /* Cdna alignment info. */
    };

struct ggMrnaInput
/* This holds the input for the core clustering algorithm. */
{
    char *tName;               /* name of target sequence, i.e. chrom */
    unsigned int tStart, tEnd;  /* coordinates that tSeq cooresponds to */
    char strand[3];             /* + or -, should always be plus for chromsome */
    struct dnaSeq *genoSeq;	/* target sequences. */
    struct ggMrnaAli *maList;     /* List of alignments. */
};

struct ggMrnaAli *pslToGgMrnaAli(struct psl *psl, char *chrom, unsigned int chromStart,
				 unsigned int chromEnd, struct dnaSeq *genoSeq);
/* Convert from psl format of alignment to ma format. chrom, chromStart, and chromEnd
   correspond to where genoseq comes from */

struct ggMrnaAli *pslListToGgMrnaAliList(struct psl *pslList, char *chrom, unsigned int chromStart, 
					 unsigned int chromEnd, struct dnaSeq *genoSeq, int maxGap);
/* create a ggMrnaAli list from a psl list merging gaps below size maxGap */

boolean ggMrnaAliMergeBlocks(struct ggMrnaAli *ma, int maxGap);
/* merge blocks that look to be separated by small amounts of
   sequencing noise only. */

int cmpGgMrnaAliTargetStart(const void *va, const void *vb);
/* Compare two ggMrnaAli based on their  strand, tEnd. */

void ggMrnaAliBedOut(struct ggMrnaAli *ma, FILE *f);
/* write out the target blocks in simple bed format, one bed per block */

void ggMrnaAliBed12Out(struct ggMrnaAli *ma, FILE *f);
/* Write out the target blocks as a linked feature bed format. */

struct ggMrnaInput *ggMrnaInputFromAlignments(struct ggMrnaAli *maList, struct dnaSeq *genoSeq);
/* wrap a ggMrnaInput around some alignments */

#endif /* GGMRNAALI_H */

