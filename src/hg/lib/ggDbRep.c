/*****************************************************************************
 * Copyright (C) 2000 Jim Kent.  This source code may be freely used         *
 * for personal, academic, and non-profit purposes.  Commercial use          *
 * permitted only by explicit agreement with Jim Kent (jim_kent@pacbell.net) *
 *****************************************************************************/
/* ggDbRep.c was originally generated by the autoSql program, which also 
 * generated ggDbRep.h and ggDbRep.sql.  This module links the database and the RAM 
 * representation of objects. */
#include "common.h"
#include "jksql.h"
#include "ggDbRep.h"

struct altGraph *altGraphLoad(char **row)
/* Load a altGraph from row fetched with select * from altGraph
 * from database.  Dispose of this with altGraphFree(). */
{
struct altGraph *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->vertexCount = sqlUnsigned(row[6]);
ret->edgeCount = sqlUnsigned(row[10]);
ret->mrnaRefCount = sqlSigned(row[13]);
ret->id = sqlUnsigned(row[0]);
ret->orientation = sqlSigned(row[1]);
ret->startBac = sqlUnsigned(row[2]);
ret->startPos = sqlUnsigned(row[3]);
ret->endBac = sqlUnsigned(row[4]);
ret->endPos = sqlUnsigned(row[5]);
sqlUbyteDynamicArray(row[7], &ret->vTypes, &sizeOne);
assert(sizeOne == ret->vertexCount);
sqlUnsignedDynamicArray(row[8], &ret->vBacs, &sizeOne);
assert(sizeOne == ret->vertexCount);
sqlUnsignedDynamicArray(row[9], &ret->vPositions, &sizeOne);
assert(sizeOne == ret->vertexCount);
sqlUnsignedDynamicArray(row[11], &ret->edgeStarts, &sizeOne);
assert(sizeOne == ret->edgeCount);
sqlUnsignedDynamicArray(row[12], &ret->edgeEnds, &sizeOne);
assert(sizeOne == ret->edgeCount);
sqlUnsignedDynamicArray(row[14], &ret->mrnaRefs, &sizeOne);
assert(sizeOne == ret->mrnaRefCount);
return ret;
}

struct altGraph *altGraphCommaIn(char **pS)
/* Create a altGraph out of a comma separated string. */
{
struct altGraph *ret;
char *s = *pS;
int i;

AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
ret->orientation = sqlSignedComma(&s);
ret->startBac = sqlUnsignedComma(&s);
ret->startPos = sqlUnsignedComma(&s);
ret->endBac = sqlUnsignedComma(&s);
ret->endPos = sqlUnsignedComma(&s);
ret->vertexCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->vTypes, ret->vertexCount);
for (i=0; i<ret->vertexCount; ++i)
    {
    ret->vTypes[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->vBacs, ret->vertexCount);
for (i=0; i<ret->vertexCount; ++i)
    {
    ret->vBacs[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->vPositions, ret->vertexCount);
for (i=0; i<ret->vertexCount; ++i)
    {
    ret->vPositions[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->edgeCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->edgeStarts, ret->edgeCount);
for (i=0; i<ret->edgeCount; ++i)
    {
    ret->edgeStarts[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
AllocArray(ret->edgeEnds, ret->edgeCount);
for (i=0; i<ret->edgeCount; ++i)
    {
    ret->edgeEnds[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->mrnaRefCount = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->mrnaRefs, ret->mrnaRefCount);
for (i=0; i<ret->mrnaRefCount; ++i)
    {
    ret->mrnaRefs[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void altGraphFree(struct altGraph **pEl)
/* Free a single dynamically allocated altGraph such as created
 * with altGraphLoad(). */
{
struct altGraph *el;

if ((el = *pEl) == NULL) return;
freeMem(el->vTypes);
freeMem(el->vBacs);
freeMem(el->vPositions);
freeMem(el->edgeStarts);
freeMem(el->edgeEnds);
freeMem(el->mrnaRefs);
freez(pEl);
}

void altGraphFreeList(struct altGraph **pList)
/* Free a list of dynamically allocated altGraph's */
{
struct altGraph *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    altGraphFree(&el);
    }
*pList = NULL;
}

void altGraphOutput(struct altGraph *el, FILE *f, char sep, char lastSep) 
/* Print out altGraph.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->id, sep);
fputc(sep,f);
fprintf(f, "%d", el->orientation, sep);
fputc(sep,f);
fprintf(f, "%u", el->startBac, sep);
fputc(sep,f);
fprintf(f, "%u", el->startPos, sep);
fputc(sep,f);
fprintf(f, "%u", el->endBac, sep);
fputc(sep,f);
fprintf(f, "%u", el->endPos, sep);
fputc(sep,f);
fprintf(f, "%u", el->vertexCount, sep);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->vertexCount; ++i)
    {
    fprintf(f, "%u", el->vTypes[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->vertexCount; ++i)
    {
    fprintf(f, "%u", el->vBacs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->vertexCount; ++i)
    {
    fprintf(f, "%u", el->vPositions[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->edgeCount, sep);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->edgeCount; ++i)
    {
    fprintf(f, "%u", el->edgeStarts[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->edgeCount; ++i)
    {
    fprintf(f, "%u", el->edgeEnds[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%d", el->mrnaRefCount, sep);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->mrnaRefCount; ++i)
    {
    fprintf(f, "%u", el->mrnaRefs[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

