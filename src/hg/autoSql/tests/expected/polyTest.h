/* polyTest.h was originally generated by the autoSql program, which also 
 * generated polyTest.c and polyTest.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef POLYTEST_H
#define POLYTEST_H

#define POINT_NUM_COLS 4

struct point
/* Three dimensional point */
    {
    struct point *next;  /* Next in singly linked list. */
    char acc[13];	/* GenBank Accession sequence */
    int x;	/* x coor */
    int y;	/* y coor */
    int z;	/* z coor */
    };

struct point *pointLoad(char **row);
/* Load a point from row fetched with select * from point
 * from database.  Dispose of this with pointFree(). */

struct point *pointLoadAll(char *fileName);
/* Load all point from whitespace-separated file.
 * Dispose of this with pointFreeList(). */

struct point *pointLoadAllByChar(char *fileName, char chopper);
/* Load all point from chopper separated file.
 * Dispose of this with pointFreeList(). */

#define pointLoadAllByTab(a) pointLoadAllByChar(a, '\t');
/* Load all point from tab separated file.
 * Dispose of this with pointFreeList(). */

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

#define POLYGON_NUM_COLS 3

struct polygon
/* A face */
    {
    struct polygon *next;  /* Next in singly linked list. */
    unsigned id;	/* Unique ID */
    int pointCount;	/* point count */
    struct point *points;	/* Points list */
    };

struct polygon *polygonLoad(char **row);
/* Load a polygon from row fetched with select * from polygon
 * from database.  Dispose of this with polygonFree(). */

struct polygon *polygonLoadAll(char *fileName);
/* Load all polygon from whitespace-separated file.
 * Dispose of this with polygonFreeList(). */

struct polygon *polygonLoadAllByChar(char *fileName, char chopper);
/* Load all polygon from chopper separated file.
 * Dispose of this with polygonFreeList(). */

#define polygonLoadAllByTab(a) polygonLoadAllByChar(a, '\t');
/* Load all polygon from tab separated file.
 * Dispose of this with polygonFreeList(). */

struct polygon *polygonCommaIn(char **pS, struct polygon *ret);
/* Create a polygon out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polygon */

void polygonFree(struct polygon **pEl);
/* Free a single dynamically allocated polygon such as created
 * with polygonLoad(). */

void polygonFreeList(struct polygon **pList);
/* Free a list of dynamically allocated polygon's */

void polygonOutput(struct polygon *el, FILE *f, char sep, char lastSep);
/* Print out polygon.  Separate fields with sep. Follow last field with lastSep. */

#define polygonTabOut(el,f) polygonOutput(el,f,'\t','\n');
/* Print out polygon as a line in a tab-separated file. */

#define polygonCommaOut(el,f) polygonOutput(el,f,',',',');
/* Print out polygon as a comma separated list including final comma. */

#define POLYHEDRON_NUM_COLS 4

struct polyhedron
/* A 3-d object */
    {
    struct polyhedron *next;  /* Next in singly linked list. */
    unsigned id;	/* Unique ID */
    char *names[2];	/* Name of this figure */
    int polygonCount;	/* Polygon count */
    struct polygon *polygons;	/* Polygons */
    };

struct polyhedron *polyhedronLoad(char **row);
/* Load a polyhedron from row fetched with select * from polyhedron
 * from database.  Dispose of this with polyhedronFree(). */

struct polyhedron *polyhedronLoadAll(char *fileName);
/* Load all polyhedron from whitespace-separated file.
 * Dispose of this with polyhedronFreeList(). */

struct polyhedron *polyhedronLoadAllByChar(char *fileName, char chopper);
/* Load all polyhedron from chopper separated file.
 * Dispose of this with polyhedronFreeList(). */

#define polyhedronLoadAllByTab(a) polyhedronLoadAllByChar(a, '\t');
/* Load all polyhedron from tab separated file.
 * Dispose of this with polyhedronFreeList(). */

struct polyhedron *polyhedronCommaIn(char **pS, struct polyhedron *ret);
/* Create a polyhedron out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polyhedron */

void polyhedronFree(struct polyhedron **pEl);
/* Free a single dynamically allocated polyhedron such as created
 * with polyhedronLoad(). */

void polyhedronFreeList(struct polyhedron **pList);
/* Free a list of dynamically allocated polyhedron's */

void polyhedronOutput(struct polyhedron *el, FILE *f, char sep, char lastSep);
/* Print out polyhedron.  Separate fields with sep. Follow last field with lastSep. */

#define polyhedronTabOut(el,f) polyhedronOutput(el,f,'\t','\n');
/* Print out polyhedron as a line in a tab-separated file. */

#define polyhedronCommaOut(el,f) polyhedronOutput(el,f,',',',');
/* Print out polyhedron as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* POLYTEST_H */

