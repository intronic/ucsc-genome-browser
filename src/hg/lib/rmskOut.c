/* rmskOut.c was originally generated by the autoSql program, which also 
 * generated rmskOut.h and rmskOut.sql.  This module links the database and the RAM 
 * representation of objects. */

#include "common.h"
#include "jksql.h"
#include "linefile.h"
#include "rmskOut.h"

void rmskOutStaticLoad(char **row, struct rmskOut *ret)
/* Load a row from rmskOut table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->swScore = sqlUnsigned(row[0]);
ret->milliDiv = sqlUnsigned(row[1]);
ret->milliDel = sqlUnsigned(row[2]);
ret->milliIns = sqlUnsigned(row[3]);
ret->genoName = row[4];
ret->genoStart = sqlUnsigned(row[5]);
ret->genoEnd = sqlUnsigned(row[6]);
ret->genoLeft = sqlSigned(row[7]);
strcpy(ret->strand, row[8]);
ret->repName = row[9];
ret->repClass = row[10];
ret->repFamily = row[11];
ret->repStart = sqlSigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repLeft = sqlSigned(row[14]);
strcpy(ret->id, row[15]);
}

struct rmskOut *rmskOutLoad(char **row)
/* Load a rmskOut from row fetched with select * from rmskOut
 * from database.  Dispose of this with rmskOutFree(). */
{
struct rmskOut *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->swScore = sqlUnsigned(row[0]);
ret->milliDiv = sqlUnsigned(row[1]);
ret->milliDel = sqlUnsigned(row[2]);
ret->milliIns = sqlUnsigned(row[3]);
ret->genoName = cloneString(row[4]);
ret->genoStart = sqlUnsigned(row[5]);
ret->genoEnd = sqlUnsigned(row[6]);
ret->genoLeft = sqlSigned(row[7]);
strcpy(ret->strand, row[8]);
ret->repName = cloneString(row[9]);
ret->repClass = cloneString(row[10]);
ret->repFamily = cloneString(row[11]);
ret->repStart = sqlSigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repLeft = sqlSigned(row[14]);
strcpy(ret->id, row[15]);
return ret;
}

struct rmskOut *rmskOutCommaIn(char **pS, struct rmskOut *ret)
/* Create a rmskOut out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rmskOut */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->swScore = sqlUnsignedComma(&s);
ret->milliDiv = sqlUnsignedComma(&s);
ret->milliDel = sqlUnsignedComma(&s);
ret->milliIns = sqlUnsignedComma(&s);
ret->genoName = sqlStringComma(&s);
ret->genoStart = sqlUnsignedComma(&s);
ret->genoEnd = sqlUnsignedComma(&s);
ret->genoLeft = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->repName = sqlStringComma(&s);
ret->repClass = sqlStringComma(&s);
ret->repFamily = sqlStringComma(&s);
ret->repStart = sqlSignedComma(&s);
ret->repEnd = sqlUnsignedComma(&s);
ret->repLeft = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->id, sizeof(ret->id));
*pS = s;
return ret;
}

void rmskOutFree(struct rmskOut **pEl)
/* Free a single dynamically allocated rmskOut such as created
 * with rmskOutLoad(). */
{
struct rmskOut *el;

if ((el = *pEl) == NULL) return;
freeMem(el->genoName);
freeMem(el->repName);
freeMem(el->repClass);
freeMem(el->repFamily);
freez(pEl);
}

void rmskOutFreeList(struct rmskOut **pList)
/* Free a list of dynamically allocated rmskOut's */
{
struct rmskOut *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rmskOutFree(&el);
    }
*pList = NULL;
}

void rmskOutOutput(struct rmskOut *el, FILE *f, char sep, char lastSep) 
/* Print out rmskOut.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->swScore, sep);
fputc(sep,f);
fprintf(f, "%u", el->milliDiv, sep);
fputc(sep,f);
fprintf(f, "%u", el->milliDel, sep);
fputc(sep,f);
fprintf(f, "%u", el->milliIns, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genoName, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->genoStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->genoEnd, sep);
fputc(sep,f);
fprintf(f, "%d", el->genoLeft, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repName, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repClass, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repFamily, sep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->repStart, sep);
fputc(sep,f);
fprintf(f, "%u", el->repEnd, sep);
fputc(sep,f);
fprintf(f, "%d", el->repLeft, sep);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->id, lastSep);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* ------------ End of AutoSQL generated code. ------------------ */

void rmskOutOpenVerify(char *fileName, struct lineFile **retFile, boolean *retEmpty)
/* Open repeat masker .out file and verify that it is good.
 * Set retEmpty if it has header characteristic of an empty file. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line;
int lineSize;

lineFileNeedNext(lf, &line, &lineSize);
if (startsWith("There were no", line))
    *retEmpty = TRUE;
if (startsWith("   SW", line))
    {
    line = skipLeadingSpaces(line);
    if (!startsWith("SW", line))
        errAbort("%s doesn't seem to be a RepeatMasker .out file", fileName);
    lineFileSkip(lf, 2);
    *retEmpty = FALSE;
    }
*retFile = lf;
}

static int negParenNum(struct lineFile *lf, char *s)
/* Return number where negative is shown by parenthization. */
{
boolean hasParen = FALSE;
int result;
if (*s == '(')
   {
   hasParen = TRUE;
   ++s;
   }
if (!isdigit(s[0]) && s[0] != '-')
   errAbort("Expecting digit line %d of %s got %s\n", 
   	lf->lineIx, lf->fileName, s);
result = atoi(s);
if (hasParen) 
    result = -result;
return result;
}

static void parseClassAndFamily(char *s, char **retClass, char **retFamily)
/* Separate repeatMasker class/family .*/
{
char *e = strchr(s, '/');
if (e == NULL)
    *retClass = *retFamily = s;
else
    {
    *e++ = 0;
    *retClass = s;
    *retFamily = e;
    }
}

struct rmskOut *rmskOutReadNext(struct lineFile *lf)
/* Read next record from repeat masker file.  Return NULL at EOF. */
{
char *words[32];
int wordCount;
char id;
struct rmskOut *ret;
char *class, *family;

if ((wordCount = lineFileChop(lf, words)) == 0)
    return NULL;
if (wordCount < 14 )
    errAbort("Expecting at least 14 words line %d of %s", lf->lineIx, lf->fileName);
if (wordCount >= 15)
    id = words[14][0];
else
    id = 0;
AllocVar(ret);
ret->swScore = lineFileNeedNum(lf, words, 0);
ret->milliDiv = round(10.0*atof(words[1]));
ret->milliDel = round(10.0*atof(words[2]));
ret->milliIns = round(10.0*atof(words[3]));
ret->genoName = cloneString(words[4]);
ret->genoStart = lineFileNeedNum(lf, words, 5)-1;
ret->genoEnd = lineFileNeedNum(lf, words, 6);
ret->genoLeft = -negParenNum(lf, words[7]);
if (sameString(words[8], "C"))
    ret->strand[0] = '-';
else if (sameString(words[8], "+"))
    ret->strand[0] = '+';
else
    errAbort("Unexpected strand char line %d of %s", lf->lineIx, lf->fileName);
ret->repName = cloneString(words[9]);
parseClassAndFamily(words[10], &class, &family);
ret->repClass = cloneString(class);
ret->repFamily = cloneString(family);
ret->repStart = negParenNum(lf, words[11])-1;
ret->repEnd = sqlUnsigned(words[12]);
ret->repLeft = -negParenNum(lf, words[13]);
return ret;
}

struct rmskOut *rmskOutRead(char *fileName)
/* Read all records in .out file and return as list. */
{
struct lineFile *lf;
boolean isEmpty;
struct rmskOut *list = NULL, *el;

rmskOutOpenVerify(fileName, &lf, &isEmpty);
if (!isEmpty)
    {
    while ((el = rmskOutReadNext(lf)) != NULL)
       {
       slAddHead(&list, el);
       }
    slReverse(&list);
    }
lineFileClose(&lf);
return list;
}

void rmskOutWriteHead(FILE *f)
/* Write out rmsk header lines. */
{
fprintf(f,
"   SW  perc perc perc  query      position in query           matching       repeat              position in  repeat\n"
"score  div. del. ins.  sequence    begin     end    (left)    repeat         class/family         begin  end (left)   ID\n"
"\n");
}

static void parenNeg(int num, char *s)
/* Write number to s, parenthesizing if negative. */
{
if (num <= 0)
   sprintf(s, "(%d)", -num);
else
   sprintf(s, "%d", num);
}

void rmskOutWriteOneOut(struct rmskOut *rmsk, FILE *f)
/* Write one rmsk in .out format to file. */
{
char genoLeft[24], repStart[24], repLeft[24];
char classFam[128];

parenNeg(-rmsk->genoLeft, genoLeft);
parenNeg(rmsk->repStart+1, repStart);
parenNeg(-rmsk->repLeft, repLeft);
if (sameString(rmsk->repClass, rmsk->repFamily))
    sprintf(classFam, "%s", rmsk->repClass);
else
    sprintf(classFam, "%s/%s", rmsk->repClass, rmsk->repFamily);
fprintf(f, 
  "%5d %5.1f %4.1f %4.1f  %-9s %7d %7d %9s %1s  %-14s %-19s %6s %4d %6s %6s\n",
  rmsk->swScore, 0.1*rmsk->milliDiv, 0.1*rmsk->milliDel, 0.1*rmsk->milliIns, 
  rmsk->genoName, rmsk->genoStart+1, rmsk->genoEnd, genoLeft,
  (rmsk->strand[0] == '+' ? "+" : "C"),
  rmsk->repName, classFam, repStart, rmsk->repEnd, repLeft, rmsk->id);
}

void rmskOutWriteAllOut(char *fileName, struct rmskOut *rmskList)
/* Write .out format file containing all in rmskList. */
{
FILE *f = mustOpen(fileName, "w");
struct rmskOut *rmsk;

rmskOutWriteHead(f);
for (rmsk = rmskList; rmsk != NULL; rmsk = rmsk->next)
    rmskOutWriteOneOut(rmsk, f);
fclose(f);
}

