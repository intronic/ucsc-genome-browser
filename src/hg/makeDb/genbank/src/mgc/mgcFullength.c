/* mgcFullength.c was originally generated by the autoSql program, which also 
 * generated mgcFullength.h and mgcFullength.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "mgcFullength.h"

static char const rcsid[] = "$Id: mgcFullength.c,v 1.1 2003/06/03 01:27:47 markd Exp $";

void mgcFullengthStaticLoad(char **row, struct mgcFullength *ret)
/* Load a row from mgcFullength table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
ret->id_clone = sqlSigned(row[0]);
ret->id_vendor = sqlSigned(row[1]);
ret->plate = row[2];
ret->row = row[3];
ret->col = sqlSigned(row[4]);
ret->id_parent = sqlSigned(row[5]);
ret->clone_sent = row[6];
ret->seq_back = row[7];
ret->id_sequencer = sqlSigned(row[8]);
ret->seq_verified = sqlSigned(row[9]);
ret->colony_pure = sqlSigned(row[10]);
ret->id_purif = sqlSigned(row[11]);
ret->full_length = sqlSigned(row[12]);
ret->gb_acc = row[13];
ret->why_full = row[14];
ret->known = sqlSigned(row[15]);
ret->cdstart = sqlSigned(row[16]);
ret->cdstop = sqlSigned(row[17]);
ret->pr_acc = row[18];
ret->dump_seq = sqlSigned(row[19]);
ret->series = row[20];
ret->organism = row[21];
ret->orient = sqlSigned(row[22]);
ret->defline = row[23];
ret->locuslink = sqlSigned(row[24]);
ret->commnt = sqlSigned(row[25]);
ret->pass = sqlSigned(row[26]);
ret->genesymbol = row[27];
ret->by_indexer = sqlSigned(row[28]);
}

struct mgcFullength *mgcFullengthLoad(char **row)
/* Load a mgcFullength from row fetched with select * from mgcFullength
 * from database.  Dispose of this with mgcFullengthFree(). */
{
struct mgcFullength *ret;

AllocVar(ret);
ret->id_clone = sqlSigned(row[0]);
ret->id_vendor = sqlSigned(row[1]);
ret->plate = cloneString(row[2]);
ret->row = cloneString(row[3]);
ret->col = sqlSigned(row[4]);
ret->id_parent = sqlSigned(row[5]);
ret->clone_sent = cloneString(row[6]);
ret->seq_back = cloneString(row[7]);
ret->id_sequencer = sqlSigned(row[8]);
ret->seq_verified = sqlSigned(row[9]);
ret->colony_pure = sqlSigned(row[10]);
ret->id_purif = sqlSigned(row[11]);
ret->full_length = sqlSigned(row[12]);
ret->gb_acc = cloneString(row[13]);
ret->why_full = cloneString(row[14]);
ret->known = sqlSigned(row[15]);
ret->cdstart = sqlSigned(row[16]);
ret->cdstop = sqlSigned(row[17]);
ret->pr_acc = cloneString(row[18]);
ret->dump_seq = sqlSigned(row[19]);
ret->series = cloneString(row[20]);
ret->organism = cloneString(row[21]);
ret->orient = sqlSigned(row[22]);
ret->defline = cloneString(row[23]);
ret->locuslink = sqlSigned(row[24]);
ret->commnt = sqlSigned(row[25]);
ret->pass = sqlSigned(row[26]);
ret->genesymbol = cloneString(row[27]);
ret->by_indexer = sqlSigned(row[28]);
return ret;
}

struct mgcFullength *mgcFullengthLoadAll(char *fileName) 
/* Load all mgcFullength from a whitespace-separated file.
 * Dispose of this with mgcFullengthFreeList(). */
{
struct mgcFullength *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[29];

while (lineFileRow(lf, row))
    {
    el = mgcFullengthLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct mgcFullength *mgcFullengthCommaIn(char **pS, struct mgcFullength *ret)
/* Create a mgcFullength out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new mgcFullength */
{
char *s = *pS;
if (ret == NULL)
    AllocVar(ret);
ret->id_clone = sqlSignedComma(&s);
ret->id_vendor = sqlSignedComma(&s);
ret->plate = sqlStringComma(&s);
ret->row = sqlStringComma(&s);
ret->col = sqlSignedComma(&s);
ret->id_parent = sqlSignedComma(&s);
ret->clone_sent = sqlStringComma(&s);
ret->seq_back = sqlStringComma(&s);
ret->id_sequencer = sqlSignedComma(&s);
ret->seq_verified = sqlSignedComma(&s);
ret->colony_pure = sqlSignedComma(&s);
ret->id_purif = sqlSignedComma(&s);
ret->full_length = sqlSignedComma(&s);
ret->gb_acc = sqlStringComma(&s);
ret->why_full = sqlStringComma(&s);
ret->known = sqlSignedComma(&s);
ret->cdstart = sqlSignedComma(&s);
ret->cdstop = sqlSignedComma(&s);
ret->pr_acc = sqlStringComma(&s);
ret->dump_seq = sqlSignedComma(&s);
ret->series = sqlStringComma(&s);
ret->organism = sqlStringComma(&s);
ret->orient = sqlSignedComma(&s);
ret->defline = sqlStringComma(&s);
ret->locuslink = sqlSignedComma(&s);
ret->commnt = sqlSignedComma(&s);
ret->pass = sqlSignedComma(&s);
ret->genesymbol = sqlStringComma(&s);
ret->by_indexer = sqlSignedComma(&s);
*pS = s;
return ret;
}

void mgcFullengthFree(struct mgcFullength **pEl)
/* Free a single dynamically allocated mgcFullength such as created
 * with mgcFullengthLoad(). */
{
struct mgcFullength *el;

if ((el = *pEl) == NULL) return;
freeMem(el->plate);
freeMem(el->row);
freeMem(el->clone_sent);
freeMem(el->seq_back);
freeMem(el->gb_acc);
freeMem(el->why_full);
freeMem(el->pr_acc);
freeMem(el->series);
freeMem(el->organism);
freeMem(el->defline);
freeMem(el->genesymbol);
freez(pEl);
}

void mgcFullengthFreeList(struct mgcFullength **pList)
/* Free a list of dynamically allocated mgcFullength's */
{
struct mgcFullength *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    mgcFullengthFree(&el);
    }
*pList = NULL;
}

void mgcFullengthOutput(struct mgcFullength *el, FILE *f, char sep, char lastSep) 
/* Print out mgcFullength.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->id_clone);
fputc(sep,f);
fprintf(f, "%d", el->id_vendor);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->plate);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->row);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->col);
fputc(sep,f);
fprintf(f, "%d", el->id_parent);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->clone_sent);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->seq_back);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->id_sequencer);
fputc(sep,f);
fprintf(f, "%d", el->seq_verified);
fputc(sep,f);
fprintf(f, "%d", el->colony_pure);
fputc(sep,f);
fprintf(f, "%d", el->id_purif);
fputc(sep,f);
fprintf(f, "%d", el->full_length);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gb_acc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->why_full);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->known);
fputc(sep,f);
fprintf(f, "%d", el->cdstart);
fputc(sep,f);
fprintf(f, "%d", el->cdstop);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->pr_acc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->dump_seq);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->series);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->organism);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->orient);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->defline);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->locuslink);
fputc(sep,f);
fprintf(f, "%d", el->commnt);
fputc(sep,f);
fprintf(f, "%d", el->pass);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genesymbol);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->by_indexer);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


