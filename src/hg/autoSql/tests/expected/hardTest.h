/* output/hardTest.h was originally generated by the autoSql program, which also 
 * generated output/hardTest.c and output/hardTest.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef OUTPUT/HARDTEST_H
#define OUTPUT/HARDTEST_H

#define POINT_NUM_COLS 3

struct point
/* Three dimensional point */
    {
    struct point *next;  /* Next in singly linked list. */
    int x;	/* x coor */
    int y;	/* y coor */
    int z;	/* z coor */
    };

struct point *pointCommaIn(char **pS, struct point *ret);
/* Create a point out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new point */

void pointFree(struct point **pEl);
/* Free a single dynamically allocated point such as created
 * with pointLoad(). */

void pointFreeList(struct point **pList);
/* Free a list of dynamically allocated point's */

void pointOutput(struct point *el, FILE *f, char sep, char lastSep);
/* Print out point.  Separate fields with sep. Follow last field with lastSep. */

#define pointTabOut(el,f) pointOutput(el,f,'\t','\n');
/* Print out point as a line in a tab-separated file. */

#define pointCommaOut(el,f) pointOutput(el,f,',',',');
/* Print out point as a comma separated list including final comma. */

#define AUTOTEST_NUM_COLS 16

struct autoTest
/* Just a test table */
    {
    struct autoTest *next;  /* Next in singly linked list. */
    unsigned id;	/* Unique ID */
    char shortName[13];	/* 12 character or less name */
    char *longName;	/* full name */
    char *aliases[3];	/* three nick-names */
    struct point *threeD;	/* Three dimensional coordinate */
    int ptCount;	/* number of points */
    short *pts;	/* point list */
    int difCount;	/* number of difs */
    unsigned char *difs;	/* dif list */
    int xy[2];	/* 2d coordinate */
    int valCount;	/* value count */
    char **vals;	/* list of values */
    double dblVal;	/* double value */
    float fltVal;	/* float value */
    double *dblArray;	/* double array */
    float *fltArray;	/* float array */
    };

struct autoTest *autoTestLoad(char **row);
/* Load a autoTest from row fetched with select * from autoTest
 * from database.  Dispose of this with autoTestFree(). */

struct autoTest *autoTestLoadAll(char *fileName);
/* Load all autoTest from whitespace-separated file.
 * Dispose of this with autoTestFreeList(). */

struct autoTest *autoTestLoadAllByChar(char *fileName, char chopper);
/* Load all autoTest from chopper separated file.
 * Dispose of this with autoTestFreeList(). */

#define autoTestLoadAllByTab(a) autoTestLoadAllByChar(a, '\t');
/* Load all autoTest from tab separated file.
 * Dispose of this with autoTestFreeList(). */

struct autoTest *autoTestCommaIn(char **pS, struct autoTest *ret);
/* Create a autoTest out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new autoTest */

void autoTestFree(struct autoTest **pEl);
/* Free a single dynamically allocated autoTest such as created
 * with autoTestLoad(). */

void autoTestFreeList(struct autoTest **pList);
/* Free a list of dynamically allocated autoTest's */

void autoTestOutput(struct autoTest *el, FILE *f, char sep, char lastSep);
/* Print out autoTest.  Separate fields with sep. Follow last field with lastSep. */

#define autoTestTabOut(el,f) autoTestOutput(el,f,'\t','\n');
/* Print out autoTest as a line in a tab-separated file. */

#define autoTestCommaOut(el,f) autoTestOutput(el,f,',',',');
/* Print out autoTest as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* OUTPUT/HARDTEST_H */

