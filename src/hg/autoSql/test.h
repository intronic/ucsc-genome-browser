/* test.h was originally generated by the autoSql program, which also 
 * generated test.c and test.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef TEST_H
#define TEST_H

struct pt
/* Two dimensional point */
    {
    int x;	/* x coor */
    int y;	/* y coor */
    };

struct pt *ptCommaIn(char **pS, struct pt *ret);
/* Create a pt out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pt */

void ptOutput(struct pt *el, FILE *f, char sep, char lastSep);
/* Print out pt.  Separate fields with sep. Follow last field with lastSep. */

#define ptTabOut(el,f) ptOutput(el,f,'\t','\n');
/* Print out pt as a line in a tab-separated file. */

#define ptCommaOut(el,f) ptOutput(el,f,',',',');
/* Print out pt as a comma separated list including final comma. */

struct point
/* Three dimensional point */
    {
    struct point *next;  /* Next in singly linked list. */
    char acc[13];	/* GenBank Accession sequence */
    int x;	/* x coor */
    int y;	/* y coor */
    int z;	/* z coor */
    struct pt pt;	/* Transformed point. */
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

struct polygon
/* A face */
    {
    struct polygon *next;  /* Next in singly linked list. */
    unsigned id;	/* Unique ID */
    int pointCount;	/* point count */
    struct point *points;	/* Points list */
    struct pt *persp;	/* Points after perspective transformation */
    };

struct polygon *polygonLoad(char **row);
/* Load a polygon from row fetched with select * from polygon
 * from database.  Dispose of this with polygonFree(). */

struct polygon *polygonLoadAll(char *fileName);
/* Load all polygon from a tab-separated file.
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

struct polyhedron
/* A 3-d object */
    {
    struct polyhedron *next;  /* Next in singly linked list. */
    unsigned id;	/* Unique ID */
    char *names[2];	/* Name of this figure */
    int polygonCount;	/* Polygon count */
    struct polygon *polygons;	/* Polygons */
    struct pt screenBox[2];	/* Bounding box in screen coordinates */
    };

struct polyhedron *polyhedronLoad(char **row);
/* Load a polyhedron from row fetched with select * from polyhedron
 * from database.  Dispose of this with polyhedronFree(). */

struct polyhedron *polyhedronLoadAll(char *fileName);
/* Load all polyhedron from a tab-separated file.
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

struct twoPoint
/* Two points back to back */
    {
    char name[13];	/* name of points */
    struct pt a;	/* point a */
    struct pt b;	/* point b */
    struct pt points[2];	/* points as array */
    };

struct twoPoint *twoPointCommaIn(char **pS, struct twoPoint *ret);
/* Create a twoPoint out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new twoPoint */

void twoPointOutput(struct twoPoint *el, FILE *f, char sep, char lastSep);
/* Print out twoPoint.  Separate fields with sep. Follow last field with lastSep. */

#define twoPointTabOut(el,f) twoPointOutput(el,f,'\t','\n');
/* Print out twoPoint as a line in a tab-separated file. */

#define twoPointCommaOut(el,f) twoPointOutput(el,f,',',',');
/* Print out twoPoint as a comma separated list including final comma. */

struct stringArray
/* An array of strings */
    {
    struct stringArray *next;  /* Next in singly linked list. */
    short numNames;	/* Number of names */
    char **names;	/* Array of names */
    };

struct stringArray *stringArrayLoad(char **row);
/* Load a stringArray from row fetched with select * from stringArray
 * from database.  Dispose of this with stringArrayFree(). */

struct stringArray *stringArrayLoadAll(char *fileName);
/* Load all stringArray from a tab-separated file.
 * Dispose of this with stringArrayFreeList(). */

struct stringArray *stringArrayCommaIn(char **pS, struct stringArray *ret);
/* Create a stringArray out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stringArray */

void stringArrayFree(struct stringArray **pEl);
/* Free a single dynamically allocated stringArray such as created
 * with stringArrayLoad(). */

void stringArrayFreeList(struct stringArray **pList);
/* Free a list of dynamically allocated stringArray's */

void stringArrayOutput(struct stringArray *el, FILE *f, char sep, char lastSep);
/* Print out stringArray.  Separate fields with sep. Follow last field with lastSep. */

#define stringArrayTabOut(el,f) stringArrayOutput(el,f,'\t','\n');
/* Print out stringArray as a line in a tab-separated file. */

#define stringArrayCommaOut(el,f) stringArrayOutput(el,f,',',',');
/* Print out stringArray as a comma separated list including final comma. */

#endif /* TEST_H */

