/* test.c was originally generated by the autoSql program, which also 
 * generated test.h and test.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "test.h"

struct pt *ptCommaIn(char **pS, struct pt *ret)
/* Create a pt out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pt */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->x = sqlSignedComma(&s);
ret->y = sqlSignedComma(&s);
*pS = s;
return ret;
}

void ptOutput(struct pt *el, FILE *f, char sep, char lastSep) 
/* Print out pt.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%d", el->x, sep);
fputc(sep,f);
fprintf(f, "%d", el->y, lastSep);
fputc(lastSep,f);
}

struct point *pointCommaIn(char **pS, struct point *ret)
/* Create a point out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new point */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
sqlFixedStringComma(&s, ret->acc, sizeof(ret->acc));
ret->x = sqlSignedComma(&s);
ret->y = sqlSignedComma(&s);
ret->z = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
ptCommaIn(&s, &ret->pt);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void pointFree(struct point **pEl)
/* Free a single dynamically allocated point such as created
 * with pointLoad(). */
{
struct point *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void pointFreeList(struct point **pList)
/* Free a list of dynamically allocated point's */
{
struct point *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pointFree(&el);
    }
*pList = NULL;
}

void pointOutput(struct point *el, FILE *f, char sep, char lastSep) 
/* Print out point.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->acc, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->x, sep);
fputc(sep,f);
fprintf(f, "%d", el->y, sep);
fputc(sep,f);
fprintf(f, "%d", el->z, sep);
fputc(sep,f);
if (sep == ',') fputc('{',f);
ptCommaOut(&el->pt,f);
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

struct polygon *polygonLoad(char **row)
/* Load a polygon from row fetched with select * from polygon
 * from database.  Dispose of this with polygonFree(). */
{
struct polygon *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->pointCount = sqlSigned(row[1]);
ret->id = sqlUnsigned(row[0]);
s = row[2];
for (i=0; i<ret->pointCount; ++i)
    {
    s = sqlEatChar(s, '{');
    slSafeAddHead(&ret->points, pointCommaIn(&s, NULL));
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
slReverse(&ret->points);
s = row[3];
AllocArray(ret->persp, ret->pointCount);
for (i=0; i<ret->pointCount; ++i)
    {
    s = sqlEatChar(s, '{');
    ptCommaIn(&s, &ret->persp[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
return ret;
}

struct polygon *polygonCommaIn(char **pS, struct polygon *ret)
/* Create a polygon out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polygon */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
ret->pointCount = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
for (i=0; i<ret->pointCount; ++i)
    {
    s = sqlEatChar(s, '{');
    slSafeAddHead(&ret->points, pointCommaIn(&s,NULL));
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
slReverse(&ret->points);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->persp, ret->pointCount);
for (i=0; i<ret->pointCount; ++i)
    {
    s = sqlEatChar(s, '{');
    ptCommaIn(&s, &ret->persp[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void polygonFree(struct polygon **pEl)
/* Free a single dynamically allocated polygon such as created
 * with polygonLoad(). */
{
struct polygon *el;

if ((el = *pEl) == NULL) return;
pointFreeList(&el->points);
freeMem(el->persp);
freez(pEl);
}

void polygonFreeList(struct polygon **pList)
/* Free a list of dynamically allocated polygon's */
{
struct polygon *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    polygonFree(&el);
    }
*pList = NULL;
}

void polygonOutput(struct polygon *el, FILE *f, char sep, char lastSep) 
/* Print out polygon.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->id, sep);
fputc(sep,f);
fprintf(f, "%d", el->pointCount, sep);
fputc(sep,f);
/* Loading point list. */
    {
    struct point *it = el->points;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->pointCount; ++i)
        {
        fputc('{',f);
        pointCommaOut(it,f);
        it = it->next;
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(sep,f);
/* Loading pt list. */
    {
    struct pt *it = el->persp;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->pointCount; ++i)
        {
        fputc('{',f);
        ptCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(lastSep,f);
}

struct polyhedron *polyhedronLoad(char **row)
/* Load a polyhedron from row fetched with select * from polyhedron
 * from database.  Dispose of this with polyhedronFree(). */
{
struct polyhedron *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->polygonCount = sqlSigned(row[2]);
ret->id = sqlUnsigned(row[0]);
s = cloneString(row[1]);
sqlStringArray(s, ret->names, 2);
s = row[3];
for (i=0; i<ret->polygonCount; ++i)
    {
    s = sqlEatChar(s, '{');
    slSafeAddHead(&ret->polygons, polygonCommaIn(&s, NULL));
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
slReverse(&ret->polygons);
s = row[4];
for (i=0; i<2; ++i)
    {
    s = sqlEatChar(s, '{');
    ptCommaIn(&s, &ret->screenBox[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
return ret;
}

struct polyhedron *polyhedronCommaIn(char **pS, struct polyhedron *ret)
/* Create a polyhedron out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polyhedron */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
for (i=0; i<2; ++i)
    {
    ret->names[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->polygonCount = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
for (i=0; i<ret->polygonCount; ++i)
    {
    s = sqlEatChar(s, '{');
    slSafeAddHead(&ret->polygons, polygonCommaIn(&s,NULL));
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
slReverse(&ret->polygons);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
for (i=0; i<2; ++i)
    {
    s = sqlEatChar(s, '{');
    ptCommaIn(&s, &ret->screenBox[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void polyhedronFree(struct polyhedron **pEl)
/* Free a single dynamically allocated polyhedron such as created
 * with polyhedronLoad(). */
{
struct polyhedron *el;

if ((el = *pEl) == NULL) return;
freeMem(el->names[0]);
polygonFreeList(&el->polygons);
freez(pEl);
}

void polyhedronFreeList(struct polyhedron **pList)
/* Free a list of dynamically allocated polyhedron's */
{
struct polyhedron *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    polyhedronFree(&el);
    }
*pList = NULL;
}

void polyhedronOutput(struct polyhedron *el, FILE *f, char sep, char lastSep) 
/* Print out polyhedron.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->id, sep);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<2; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->names[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%d", el->polygonCount, sep);
fputc(sep,f);
/* Loading polygon list. */
    {
    struct polygon *it = el->polygons;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->polygonCount; ++i)
        {
        fputc('{',f);
        polygonCommaOut(it,f);
        it = it->next;
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(sep,f);
/* Loading pt list. */
    {
    struct pt *it = el->screenBox;
    if (sep == ',') fputc('{',f);
    for (i=0; i<2; ++i)
        {
        fputc('{',f);
        ptCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(lastSep,f);
}

struct twoPoint *twoPointCommaIn(char **pS, struct twoPoint *ret)
/* Create a twoPoint out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new twoPoint */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
sqlFixedStringComma(&s, ret->name, sizeof(ret->name));
s = sqlEatChar(s, '{');
ptCommaIn(&s, &ret->a);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
ptCommaIn(&s, &ret->b);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
for (i=0; i<2; ++i)
    {
    s = sqlEatChar(s, '{');
    ptCommaIn(&s, &ret->points[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void twoPointOutput(struct twoPoint *el, FILE *f, char sep, char lastSep) 
/* Print out twoPoint.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
ptCommaOut(&el->a,f);
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
ptCommaOut(&el->b,f);
if (sep == ',') fputc('}',f);
fputc(sep,f);
/* Loading pt list. */
    {
    struct pt *it = el->points;
    if (sep == ',') fputc('{',f);
    for (i=0; i<2; ++i)
        {
        fputc('{',f);
        ptCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
fputc(lastSep,f);
}

