/* simpleTest.c was originally generated by the autoSql program, which also 
 * generated simpleTest.h and simpleTest.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "output/simpleTest.h"


struct point *pointLoad(char **row)
/* Load a point from row fetched with select * from point
 * from database.  Dispose of this with pointFree(). */
{
struct point *ret;

AllocVar(ret);
ret->x = sqlSigned(row[0]);
ret->y = sqlSigned(row[1]);
return ret;
}

struct point *pointLoadAll(char *fileName) 
/* Load all point from a whitespace-separated file.
 * Dispose of this with pointFreeList(). */
{
struct point *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = pointLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct point *pointLoadAllByChar(char *fileName, char chopper) 
/* Load all point from a chopper separated file.
 * Dispose of this with pointFreeList(). */
{
struct point *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pointLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct point *pointCommaIn(char **pS, struct point *ret)
/* Create a point out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new point */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->x = sqlSignedComma(&s);
ret->y = sqlSignedComma(&s);
*pS = s;
return ret;
}

void pointOutput(struct point *el, FILE *f, char sep, char lastSep) 
/* Print out point.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->x);
fputc(sep,f);
fprintf(f, "%d", el->y);
fputc(lastSep,f);
}

struct namedPoint *namedPointLoad(char **row)
/* Load a namedPoint from row fetched with select * from namedPoint
 * from database.  Dispose of this with namedPointFree(). */
{
struct namedPoint *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
{
char *s = row[1];
if(s != NULL && differentString(s, ""))
   pointCommaIn(&s, &ret->point);
}
return ret;
}

struct namedPoint *namedPointLoadAll(char *fileName) 
/* Load all namedPoint from a whitespace-separated file.
 * Dispose of this with namedPointFreeList(). */
{
struct namedPoint *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = namedPointLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct namedPoint *namedPointLoadAllByChar(char *fileName, char chopper) 
/* Load all namedPoint from a chopper separated file.
 * Dispose of this with namedPointFreeList(). */
{
struct namedPoint *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = namedPointLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct namedPoint *namedPointCommaIn(char **pS, struct namedPoint *ret)
/* Create a namedPoint out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new namedPoint */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
s = sqlEatChar(s, '{');
if(s[0] != '}')    pointCommaIn(&s, &ret->point);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void namedPointFreeInternals(struct namedPoint *array, int count)
/* Free internals of a simple type namedPoint (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct namedPoint *el = &array[i];
    freeMem(el->name);
    }
}

void namedPointOutput(struct namedPoint *el, FILE *f, char sep, char lastSep) 
/* Print out namedPoint.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
pointCommaOut(&el->point,f);
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

struct triangle *triangleLoad(char **row)
/* Load a triangle from row fetched with select * from triangle
 * from database.  Dispose of this with triangleFree(). */
{
struct triangle *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
{
int i;
char *s = row[1];
for (i=0; i<3; ++i)
    {
    s = sqlEatChar(s, '{');
    pointCommaIn(&s, &ret->points[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
}
return ret;
}

struct triangle *triangleLoadAll(char *fileName) 
/* Load all triangle from a whitespace-separated file.
 * Dispose of this with triangleFreeList(). */
{
struct triangle *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = triangleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct triangle *triangleLoadAllByChar(char *fileName, char chopper) 
/* Load all triangle from a chopper separated file.
 * Dispose of this with triangleFreeList(). */
{
struct triangle *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = triangleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct triangle *triangleCommaIn(char **pS, struct triangle *ret)
/* Create a triangle out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new triangle */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
for (i=0; i<3; ++i)
    {
    s = sqlEatChar(s, '{');
    if(s[0] != '}')        pointCommaIn(&s, &ret->points[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void triangleFreeInternals(struct triangle *array, int count)
/* Free internals of a simple type triangle (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct triangle *el = &array[i];
    freeMem(el->name);
    }
}

void triangleOutput(struct triangle *el, FILE *f, char sep, char lastSep) 
/* Print out triangle.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
/* Loading point list. */
    {
    struct point *it = el->points;
    if (sep == ',') fputc('{',f);
    for (i=0; i<3; ++i)
        {
        fputc('{',f);
        pointCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
}
fputc(lastSep,f);
}

struct polygon *polygonLoad(char **row)
/* Load a polygon from row fetched with select * from polygon
 * from database.  Dispose of this with polygonFree(). */
{
struct polygon *ret;

AllocVar(ret);
ret->vertexCount = sqlSigned(row[1]);
ret->name = cloneString(row[0]);
{
int i;
char *s = row[2];
AllocArray(ret->vertices, ret->vertexCount);
for (i=0; i<ret->vertexCount; ++i)
    {
    s = sqlEatChar(s, '{');
    pointCommaIn(&s, &ret->vertices[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
}
return ret;
}

struct polygon *polygonLoadAll(char *fileName) 
/* Load all polygon from a whitespace-separated file.
 * Dispose of this with polygonFreeList(). */
{
struct polygon *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = polygonLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct polygon *polygonLoadAllByChar(char *fileName, char chopper) 
/* Load all polygon from a chopper separated file.
 * Dispose of this with polygonFreeList(). */
{
struct polygon *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = polygonLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct polygon *polygonCommaIn(char **pS, struct polygon *ret)
/* Create a polygon out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polygon */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->vertexCount = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->vertices, ret->vertexCount);
for (i=0; i<ret->vertexCount; ++i)
    {
    s = sqlEatChar(s, '{');
    if(s[0] != '}')        pointCommaIn(&s, &ret->vertices[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void polygonFreeInternals(struct polygon *array, int count)
/* Free internals of a simple type polygon (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct polygon *el = &array[i];
    freeMem(el->name);
    freeMem(el->vertices);
    }
}

void polygonOutput(struct polygon *el, FILE *f, char sep, char lastSep) 
/* Print out polygon.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->vertexCount);
fputc(sep,f);
{
int i;
/* Loading point list. */
    {
    struct point *it = el->vertices;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->vertexCount; ++i)
        {
        fputc('{',f);
        pointCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
}
fputc(lastSep,f);
}

struct person *personLoad(char **row)
/* Load a person from row fetched with select * from person
 * from database.  Dispose of this with personFree(). */
{
struct person *ret;

AllocVar(ret);
ret->firstName = cloneString(row[0]);
ret->lastName = cloneString(row[1]);
ret->ssn = sqlLongLong(row[2]);
return ret;
}

struct person *personLoadAll(char *fileName) 
/* Load all person from a whitespace-separated file.
 * Dispose of this with personFreeList(). */
{
struct person *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = personLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct person *personLoadAllByChar(char *fileName, char chopper) 
/* Load all person from a chopper separated file.
 * Dispose of this with personFreeList(). */
{
struct person *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = personLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct person *personCommaIn(char **pS, struct person *ret)
/* Create a person out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new person */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->firstName = sqlStringComma(&s);
ret->lastName = sqlStringComma(&s);
ret->ssn = sqlLongLongComma(&s);
*pS = s;
return ret;
}

void personFreeInternals(struct person *array, int count)
/* Free internals of a simple type person (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct person *el = &array[i];
    freeMem(el->firstName);
    freeMem(el->lastName);
    }
}

void personOutput(struct person *el, FILE *f, char sep, char lastSep) 
/* Print out person.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->firstName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->lastName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%lld", el->ssn);
fputc(lastSep,f);
}

struct couple *coupleLoad(char **row)
/* Load a couple from row fetched with select * from couple
 * from database.  Dispose of this with coupleFree(). */
{
struct couple *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
{
int i;
char *s = row[1];
for (i=0; i<2; ++i)
    {
    s = sqlEatChar(s, '{');
    personCommaIn(&s, &ret->members[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
}
return ret;
}

struct couple *coupleLoadAll(char *fileName) 
/* Load all couple from a whitespace-separated file.
 * Dispose of this with coupleFreeList(). */
{
struct couple *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = coupleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct couple *coupleLoadAllByChar(char *fileName, char chopper) 
/* Load all couple from a chopper separated file.
 * Dispose of this with coupleFreeList(). */
{
struct couple *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = coupleLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct couple *coupleCommaIn(char **pS, struct couple *ret)
/* Create a couple out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new couple */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
{
int i;
s = sqlEatChar(s, '{');
for (i=0; i<2; ++i)
    {
    s = sqlEatChar(s, '{');
    if(s[0] != '}')        personCommaIn(&s, &ret->members[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void coupleFreeInternals(struct couple *array, int count)
/* Free internals of a simple type couple (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct couple *el = &array[i];
    freeMem(el->name);
    personFreeInternals(el->members, ArraySize(el->members));
    }
}

void coupleOutput(struct couple *el, FILE *f, char sep, char lastSep) 
/* Print out couple.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
{
int i;
/* Loading person list. */
    {
    struct person *it = el->members;
    if (sep == ',') fputc('{',f);
    for (i=0; i<2; ++i)
        {
        fputc('{',f);
        personCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
}
fputc(lastSep,f);
}

struct group *groupLoad(char **row)
/* Load a group from row fetched with select * from group
 * from database.  Dispose of this with groupFree(). */
{
struct group *ret;

AllocVar(ret);
ret->size = sqlSigned(row[1]);
ret->name = cloneString(row[0]);
{
int i;
char *s = row[2];
AllocArray(ret->members, ret->size);
for (i=0; i<ret->size; ++i)
    {
    s = sqlEatChar(s, '{');
    personCommaIn(&s, &ret->members[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
}
return ret;
}

struct group *groupLoadAll(char *fileName) 
/* Load all group from a whitespace-separated file.
 * Dispose of this with groupFreeList(). */
{
struct group *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = groupLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct group *groupLoadAllByChar(char *fileName, char chopper) 
/* Load all group from a chopper separated file.
 * Dispose of this with groupFreeList(). */
{
struct group *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = groupLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct group *groupCommaIn(char **pS, struct group *ret)
/* Create a group out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new group */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->size = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->members, ret->size);
for (i=0; i<ret->size; ++i)
    {
    s = sqlEatChar(s, '{');
    if(s[0] != '}')        personCommaIn(&s, &ret->members[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void groupFreeInternals(struct group *array, int count)
/* Free internals of a simple type group (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct group *el = &array[i];
    freeMem(el->name);
    personFreeInternals(el->members, el->size);
    freeMem(el->members);
    }
}

void groupOutput(struct group *el, FILE *f, char sep, char lastSep) 
/* Print out group.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->size);
fputc(sep,f);
{
int i;
/* Loading person list. */
    {
    struct person *it = el->members;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->size; ++i)
        {
        fputc('{',f);
        personCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
}
fputc(lastSep,f);
}

struct metaGroup *metaGroupLoad(char **row)
/* Load a metaGroup from row fetched with select * from metaGroup
 * from database.  Dispose of this with metaGroupFree(). */
{
struct metaGroup *ret;

AllocVar(ret);
ret->metaSize = sqlSigned(row[1]);
ret->name = cloneString(row[0]);
{
int i;
char *s = row[2];
AllocArray(ret->groups, ret->metaSize);
for (i=0; i<ret->metaSize; ++i)
    {
    s = sqlEatChar(s, '{');
    groupCommaIn(&s, &ret->groups[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
}
return ret;
}

struct metaGroup *metaGroupLoadAll(char *fileName) 
/* Load all metaGroup from a whitespace-separated file.
 * Dispose of this with metaGroupFreeList(). */
{
struct metaGroup *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = metaGroupLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct metaGroup *metaGroupLoadAllByChar(char *fileName, char chopper) 
/* Load all metaGroup from a chopper separated file.
 * Dispose of this with metaGroupFreeList(). */
{
struct metaGroup *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = metaGroupLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct metaGroup *metaGroupCommaIn(char **pS, struct metaGroup *ret)
/* Create a metaGroup out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new metaGroup */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->metaSize = sqlSignedComma(&s);
{
int i;
s = sqlEatChar(s, '{');
AllocArray(ret->groups, ret->metaSize);
for (i=0; i<ret->metaSize; ++i)
    {
    s = sqlEatChar(s, '{');
    if(s[0] != '}')        groupCommaIn(&s, &ret->groups[i]);
    s = sqlEatChar(s, '}');
    s = sqlEatChar(s, ',');
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
}
*pS = s;
return ret;
}

void metaGroupFreeInternals(struct metaGroup *array, int count)
/* Free internals of a simple type metaGroup (one not put on a list). */
{
int i;
for (i=0; i<count; ++i)
    {
    struct metaGroup *el = &array[i];
    freeMem(el->name);
    groupFreeInternals(el->groups, el->metaSize);
    freeMem(el->groups);
    }
}

void metaGroupOutput(struct metaGroup *el, FILE *f, char sep, char lastSep) 
/* Print out metaGroup.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->metaSize);
fputc(sep,f);
{
int i;
/* Loading group list. */
    {
    struct group *it = el->groups;
    if (sep == ',') fputc('{',f);
    for (i=0; i<el->metaSize; ++i)
        {
        fputc('{',f);
        groupCommaOut(&it[i],f);
        fputc('}',f);
        fputc(',',f);
        }
    if (sep == ',') fputc('}',f);
    }
}
fputc(lastSep,f);
}

void metaGroupLogoStaticLoad(char **row, struct metaGroupLogo *ret)
/* Load a row from metaGroupLogo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

{
char *s = row[0];
if(s != NULL && differentString(s, ""))
   polygonCommaIn(&s, &ret->logo);
}
{
char *s = row[1];
if(s != NULL && differentString(s, ""))
   metaGroupCommaIn(&s, &ret->conspiracy);
}
}

struct metaGroupLogo *metaGroupLogoLoad(char **row)
/* Load a metaGroupLogo from row fetched with select * from metaGroupLogo
 * from database.  Dispose of this with metaGroupLogoFree(). */
{
struct metaGroupLogo *ret;

AllocVar(ret);
{
char *s = row[0];
if(s != NULL && differentString(s, ""))
   polygonCommaIn(&s, &ret->logo);
}
{
char *s = row[1];
if(s != NULL && differentString(s, ""))
   metaGroupCommaIn(&s, &ret->conspiracy);
}
return ret;
}

struct metaGroupLogo *metaGroupLogoLoadAll(char *fileName) 
/* Load all metaGroupLogo from a whitespace-separated file.
 * Dispose of this with metaGroupLogoFreeList(). */
{
struct metaGroupLogo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = metaGroupLogoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct metaGroupLogo *metaGroupLogoLoadAllByChar(char *fileName, char chopper) 
/* Load all metaGroupLogo from a chopper separated file.
 * Dispose of this with metaGroupLogoFreeList(). */
{
struct metaGroupLogo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = metaGroupLogoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct metaGroupLogo *metaGroupLogoCommaIn(char **pS, struct metaGroupLogo *ret)
/* Create a metaGroupLogo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new metaGroupLogo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
s = sqlEatChar(s, '{');
if(s[0] != '}')    polygonCommaIn(&s, &ret->logo);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
if(s[0] != '}')    metaGroupCommaIn(&s, &ret->conspiracy);
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void metaGroupLogoFree(struct metaGroupLogo **pEl)
/* Free a single dynamically allocated metaGroupLogo such as created
 * with metaGroupLogoLoad(). */
{
struct metaGroupLogo *el;

if ((el = *pEl) == NULL) return;
polygonFreeInternals(&el->logo, 1);
metaGroupFreeInternals(&el->conspiracy, 1);
freez(pEl);
}

void metaGroupLogoFreeList(struct metaGroupLogo **pList)
/* Free a list of dynamically allocated metaGroupLogo's */
{
struct metaGroupLogo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    metaGroupLogoFree(&el);
    }
*pList = NULL;
}

void metaGroupLogoOutput(struct metaGroupLogo *el, FILE *f, char sep, char lastSep) 
/* Print out metaGroupLogo.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('{',f);
polygonCommaOut(&el->logo,f);
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
metaGroupCommaOut(&el->conspiracy,f);
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

