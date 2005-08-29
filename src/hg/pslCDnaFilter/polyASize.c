/* polyASize.c was originally generated by the autoSql program, which also 
 * generated polyASize.h and polyASize.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "polyASize.h"
#include "localmem.h"

static char const rcsid[] = "$Id: polyASize.c,v 1.1 2005/08/29 06:18:33 markd Exp $";

void polyASizeStaticLoad(char **row, struct polyASize *ret)
/* Load a row from polyASize table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->id = row[0];
ret->seqSize = sqlUnsigned(row[1]);
ret->tailPolyASize = sqlUnsigned(row[2]);
ret->headPolyTSize = sqlUnsigned(row[3]);
}

struct polyASize *polyASizeLoad(char **row)
/* Load a polyASize from row fetched with select * from polyASize
 * from database.  Dispose of this with polyASizeFree(). */
{
struct polyASize *ret;

AllocVar(ret);
ret->id = cloneString(row[0]);
ret->seqSize = sqlUnsigned(row[1]);
ret->tailPolyASize = sqlUnsigned(row[2]);
ret->headPolyTSize = sqlUnsigned(row[3]);
return ret;
}

struct polyASize *polyASizeLoadAll(char *fileName) 
/* Load all polyASize from a whitespace-separated file.
 * Dispose of this with polyASizeFreeList(). */
{
struct polyASize *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = polyASizeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct polyASize *polyASizeLoadAllByChar(char *fileName, char chopper) 
/* Load all polyASize from a chopper separated file.
 * Dispose of this with polyASizeFreeList(). */
{
struct polyASize *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = polyASizeLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct polyASize *polyASizeCommaIn(char **pS, struct polyASize *ret)
/* Create a polyASize out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new polyASize */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlStringComma(&s);
ret->seqSize = sqlUnsignedComma(&s);
ret->tailPolyASize = sqlUnsignedComma(&s);
ret->headPolyTSize = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void polyASizeFree(struct polyASize **pEl)
/* Free a single dynamically allocated polyASize such as created
 * with polyASizeLoad(). */
{
struct polyASize *el;

if ((el = *pEl) == NULL) return;
freeMem(el->id);
freez(pEl);
}

void polyASizeFreeList(struct polyASize **pList)
/* Free a list of dynamically allocated polyASize's */
{
struct polyASize *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    polyASizeFree(&el);
    }
*pList = NULL;
}

void polyASizeOutput(struct polyASize *el, FILE *f, char sep, char lastSep) 
/* Print out polyASize.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->id);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->seqSize);
fputc(sep,f);
fprintf(f, "%u", el->tailPolyASize);
fputc(sep,f);
fprintf(f, "%u", el->headPolyTSize);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

static void loadPolyASizeRec(struct hash *pasHash, char **row)
/* load one polyA size record into the hash table.  This
 * places in hash localmem to reduce memory usage */
{
struct polyASize pas, *pasRec;
struct hashEl *recHel;

polyASizeStaticLoad(row, &pas);
recHel = hashStore(pasHash, pas.id);
if (recHel->val != NULL)
    {
    /* record already exists for this id, validate */
    pasRec = recHel->val;
    if ((pasRec->seqSize != pas.seqSize)
        ||(pasRec->tailPolyASize != pas.tailPolyASize)
        || (pasRec->headPolyTSize != pas.headPolyTSize))
        errAbort("multiple polyA size records for %s with different data", pas.id);
    }
else
    {
    /* add new record */
    lmAllocVar(pasHash->lm, pasRec);
    pasRec->id = recHel->name;
    pasRec->seqSize = pas.seqSize;
    pasRec->tailPolyASize = pas.tailPolyASize;
    pasRec->headPolyTSize = pas.headPolyTSize;
    }
}

struct hash *polyASizeLoadHash(char *fileName)
/* load a tab-separated file of polyASize objects into a hash table.  Objects
 * are stored in the local mem of the hash, and thus freed when the hash is
 * freed. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[POLYASIZE_NUM_COLS];
struct hash *pasHash = hashNew(22);

while (lineFileNextRowTab(lf, row, ArraySize(row)))
    loadPolyASizeRec(pasHash, row);

lineFileClose(&lf);
return pasHash;
}
