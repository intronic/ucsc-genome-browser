/* ccdsInfo.h was originally generated by the autoSql program, which also 
 * generated ccdsInfo.c and ccdsInfo.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef CCDSINFO_H
#define CCDSINFO_H

#define CCDSINFO_NUM_COLS 4

enum ccdsInfoSrcDb
/* source database */
{
    ccdsInfoNull = 0,  /* used for no src db */
    ccdsInfoNcbi = 1,
    ccdsInfoEnsembl = 2,
    ccdsInfoVega = 3
};

struct ccdsInfo
/* Consensus CDS information, links CCDS ids to NCBI and Hinxton accessions */
    {
    struct ccdsInfo *next;  /* Next in singly linked list. */
    char ccds[13];	/* CCDS id */
    enum ccdsInfoSrcDb srcDb;	/* source database: N=NCBI, H=Hinxton */
    char mrnaAcc[19];	/* mRNA accession (NCBI or Hinxton) */
    char protAcc[19];	/* protein accession (NCBI or Hinxton) */
    };

void ccdsInfoStaticLoad(char **row, struct ccdsInfo *ret);
/* Load a row from ccdsInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct ccdsInfo *ccdsInfoLoad(char **row);
/* Load a ccdsInfo from row fetched with select * from ccdsInfo
 * from database.  Dispose of this with ccdsInfoFree(). */

struct ccdsInfo *ccdsInfoLoadAll(char *fileName);
/* Load all ccdsInfo from whitespace-separated file.
 * Dispose of this with ccdsInfoFreeList(). */

struct ccdsInfo *ccdsInfoLoadAllByChar(char *fileName, char chopper);
/* Load all ccdsInfo from chopper separated file.
 * Dispose of this with ccdsInfoFreeList(). */

#define ccdsInfoLoadAllByTab(a) ccdsInfoLoadAllByChar(a, '\t');
/* Load all ccdsInfo from tab separated file.
 * Dispose of this with ccdsInfoFreeList(). */

struct ccdsInfo *ccdsInfoCommaIn(char **pS, struct ccdsInfo *ret);
/* Create a ccdsInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new ccdsInfo */

void ccdsInfoFree(struct ccdsInfo **pEl);
/* Free a single dynamically allocated ccdsInfo such as created
 * with ccdsInfoLoad(). */

void ccdsInfoFreeList(struct ccdsInfo **pList);
/* Free a list of dynamically allocated ccdsInfo's */

void ccdsInfoOutput(struct ccdsInfo *el, FILE *f, char sep, char lastSep);
/* Print out ccdsInfo.  Separate fields with sep. Follow last field with lastSep. */

#define ccdsInfoTabOut(el,f) ccdsInfoOutput(el,f,'\t','\n');
/* Print out ccdsInfo as a line in a tab-separated file. */

#define ccdsInfoCommaOut(el,f) ccdsInfoOutput(el,f,',',',');
/* Print out ccdsInfo as a comma separated list including final comma. */

void ccdsInfoMRnaSort(struct ccdsInfo **ccdsInfos);
/* Sort list by mrnaAcc */

struct ccdsInfo *ccdsInfoSelectByCcds(struct sqlConnection *conn, char *ccdsId,
                                      enum ccdsInfoSrcDb srcDb);
/* Obtain list of ccdsInfo object for the specified id and srcDb.  If srcDb is
 * ccdsInfoNull, return all.  Return NULL if ccdsId it's not valid */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CCDSINFO_H */

