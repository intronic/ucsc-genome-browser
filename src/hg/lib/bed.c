/* bed.c was originally generated by the autoSql program, which also 
 * generated bed.h and bed.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "linefile.h"
#include "bed.h"

void bedStaticLoad(char **row, struct bed *ret)
/* Load a row from bed table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
}

struct bed *bedLoad(char **row)
/* Load a bed from row fetched with select * from bed
 * from database.  Dispose of this with bedFree(). */
{
struct bed *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
return ret;
}

struct bed *bedCommaIn(char **pS, struct bed *ret)
/* Create a bed out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
*pS = s;
return ret;
}

void bedFree(struct bed **pEl)
/* Free a single dynamically allocated bed such as created
 * with bedLoad(). */
{
struct bed *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->blockSizes);
freeMem(el->chromStarts);
freez(pEl);
}

void bedFreeList(struct bed **pList)
/* Free a list of dynamically allocated bed's */
{
struct bed *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bedFree(&el);
    }
*pList = NULL;
}

void bedOutput(struct bed *el, FILE *f, char sep, char lastSep) 
/* Print out bed.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* --------------- End of AutoSQL generated code. --------------- */

int bedCmp(const void *va, const void *vb)
/* Compare to sort based on chrom,chromStart. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}

int bedCmpScore(const void *va, const void *vb)
/* Compare to sort based on score - lowest first. */
{
const struct bed *a = *((struct bed **)va);
const struct bed *b = *((struct bed **)vb);
return a->score - b->score;
}

struct bedLine *bedLineNew(char *line)
/* Create a new bedLine based on tab-separated string s. */
{
struct bedLine *bl;
char *s, c;

AllocVar(bl);
bl->chrom = cloneString(line);
s = strchr(bl->chrom, '\t');
if (s == NULL)
    errAbort("Expecting tab in bed line %s", line);
*s++ = 0;
c = *s;
if (isdigit(c) || (c == '-' && isdigit(s[1])))
    {
    bl->chromStart = atoi(s);
    bl->line = s;
    return bl;
    }
else
    {
    errAbort("Expecting start position in second field of %s", line);
    return NULL;
    }
}

void bedLineFree(struct bedLine **pBl)
/* Free up memory associated with bedLine. */
{
struct bedLine *bl;

if ((bl = *pBl) != NULL)
    {
    freeMem(bl->chrom);
    freez(pBl);
    }
}

void bedLineFreeList(struct bedLine **pList)
/* Free a list of dynamically allocated bedLine's */
{
struct bedLine *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bedLineFree(&el);
    }
*pList = NULL;
}


int bedLineCmp(const void *va, const void *vb)
/* Compare to sort based on query. */
{
const struct bedLine *a = *((struct bedLine **)va);
const struct bedLine *b = *((struct bedLine **)vb);
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}


void bedSortFile(char *inFile, char *outFile)
/* Sort a bed file (in place, overwrites old file. */
{
struct lineFile *lf = NULL;
FILE *f = NULL;
struct bedLine *blList = NULL, *bl;
char *line;
int lineSize;

printf("Reading %s\n", inFile);
lf = lineFileOpen(inFile, TRUE);
while (lineFileNext(lf, &line, &lineSize))
    {
    if (line[0] == '#')
        continue;
    bl = bedLineNew(line);
    slAddHead(&blList, bl);
    }
lineFileClose(&lf);

printf("Sorting\n");
slSort(&blList, bedLineCmp);

printf("Writing %s\n", outFile);
f = mustOpen(outFile, "w");
for (bl = blList; bl != NULL; bl = bl->next)
    {
    fprintf(f, "%s\t%s\n", bl->chrom, bl->line);
    if (ferror(f))
        {
	perror("Writing error\n");
	errAbort("%s is truncated, sorry.", outFile);
	}
    }
fclose(f);
}

struct bed *bedLoadWScore(char **row)
/* Load a bed from row fetched with select * from bed
 * from database.  Dispose of this with bedFree(). */
{
struct bed *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
return ret;
}
struct bed *bedLoad3(char **row)
/* Load first three fields of bed. */
{
struct bed *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
return ret;
}

struct bed *bedLoad5(char **row)
/* Load first five fields of bed. */
{
struct bed *ret;
AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
return ret;
}

