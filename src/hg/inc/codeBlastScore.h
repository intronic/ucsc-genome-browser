/* codeBlastScore.h was originally generated by the autoSql program, which also 
 * generated codeBlastScore.c and codeBlastScore.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CODEBLASTSCORE_H
#define CODEBLASTSCORE_H

#define CODEBLASTSCORE_NUM_COLS 13

struct codeBlastScore
/* Table storing the codes and positional info for blast runs. */
    {
    struct codeBlastScore *next;  /* Next in singly linked list. */
    char *qName;	/* Name of item */
    char code[2];	/* Code */
    double evalue;	/* evalue */
    unsigned GI;	/* GI Number */
    float PI;	/* Percent Identity */
    unsigned length;	/* Alignment length */
    unsigned gap;	/* gap length */
    unsigned score;	/* score from evalue */
    unsigned seqstart;	/* Where alignment begins */
    unsigned seqend;	/* Where alignment ends */
    char species[256];	/* Code */
    char product[256];	/* Code */
    char name[256];	/* Code */
    };

void codeBlastScoreStaticLoad(char **row, struct codeBlastScore *ret);
/* Load a row from codeBlastScore table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct codeBlastScore *codeBlastScoreLoad(char **row);
/* Load a codeBlastScore from row fetched with select * from codeBlastScore
 * from database.  Dispose of this with codeBlastScoreFree(). */

struct codeBlastScore *codeBlastScoreLoadAll(char *fileName);
/* Load all codeBlastScore from whitespace-separated file.
 * Dispose of this with codeBlastScoreFreeList(). */

struct codeBlastScore *codeBlastScoreLoadAllByChar(char *fileName, char chopper);
/* Load all codeBlastScore from chopper separated file.
 * Dispose of this with codeBlastScoreFreeList(). */

#define codeBlastScoreLoadAllByTab(a) codeBlastScoreLoadAllByChar(a, '\t');
/* Load all codeBlastScore from tab separated file.
 * Dispose of this with codeBlastScoreFreeList(). */

struct codeBlastScore *codeBlastScoreCommaIn(char **pS, struct codeBlastScore *ret);
/* Create a codeBlastScore out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new codeBlastScore */

void codeBlastScoreFree(struct codeBlastScore **pEl);
/* Free a single dynamically allocated codeBlastScore such as created
 * with codeBlastScoreLoad(). */

void codeBlastScoreFreeList(struct codeBlastScore **pList);
/* Free a list of dynamically allocated codeBlastScore's */

void codeBlastScoreOutput(struct codeBlastScore *el, FILE *f, char sep, char lastSep);
/* Print out codeBlastScore.  Separate fields with sep. Follow last field with lastSep. */

#define codeBlastScoreTabOut(el,f) codeBlastScoreOutput(el,f,'\t','\n');
/* Print out codeBlastScore as a line in a tab-separated file. */

#define codeBlastScoreCommaOut(el,f) codeBlastScoreOutput(el,f,',',',');
/* Print out codeBlastScore as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CODEBLASTSCORE_H */

