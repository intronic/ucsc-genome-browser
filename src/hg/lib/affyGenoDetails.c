/* affyGenoDetails.c was originally generated by the autoSql program, which also 
 * generated affyGenoDetails.h and affyGenoDetails.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "affyGenoDetails.h"

static char const rcsid[] = "$Id: affyGenoDetails.c,v 1.1 2003/10/09 02:21:38 daryl Exp $";

void affyGenoDetailsStaticLoad(char **row, struct affyGenoDetails *ret)
/* Load a row from affyGenoDetails table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->affyId = sqlUnsigned(row[0]);
ret->rsId = sqlUnsigned(row[1]);
strcpy(ret->baseA, row[2]);
strcpy(ret->baseB, row[3]);
strcpy(ret->sequenceA, row[4]);
strcpy(ret->sequenceB, row[5]);
strcpy(ret->enzyme, row[6]);
ret->minFreq = atof(row[7]);
ret->hetzyg = atof(row[8]);
ret->avHetSE = atof(row[9]);
strcpy(ret->NA04477, row[10]);
strcpy(ret->NA04479, row[11]);
strcpy(ret->NA04846, row[12]);
strcpy(ret->NA11036, row[13]);
strcpy(ret->NA11038, row[14]);
strcpy(ret->NA13056, row[15]);
strcpy(ret->NA17011, row[16]);
strcpy(ret->NA17012, row[17]);
strcpy(ret->NA17013, row[18]);
strcpy(ret->NA17014, row[19]);
strcpy(ret->NA17015, row[20]);
strcpy(ret->NA17016, row[21]);
strcpy(ret->NA17101, row[22]);
strcpy(ret->NA17102, row[23]);
strcpy(ret->NA17103, row[24]);
strcpy(ret->NA17104, row[25]);
strcpy(ret->NA17105, row[26]);
strcpy(ret->NA17106, row[27]);
strcpy(ret->NA17201, row[28]);
strcpy(ret->NA17202, row[29]);
strcpy(ret->NA17203, row[30]);
strcpy(ret->NA17204, row[31]);
strcpy(ret->NA17205, row[32]);
strcpy(ret->NA17206, row[33]);
strcpy(ret->NA17207, row[34]);
strcpy(ret->NA17208, row[35]);
strcpy(ret->NA17210, row[36]);
strcpy(ret->NA17211, row[37]);
strcpy(ret->NA17212, row[38]);
strcpy(ret->NA17213, row[39]);
strcpy(ret->PD01, row[40]);
strcpy(ret->PD02, row[41]);
strcpy(ret->PD03, row[42]);
strcpy(ret->PD04, row[43]);
strcpy(ret->PD05, row[44]);
strcpy(ret->PD06, row[45]);
strcpy(ret->PD07, row[46]);
strcpy(ret->PD08, row[47]);
strcpy(ret->PD09, row[48]);
strcpy(ret->PD10, row[49]);
strcpy(ret->PD11, row[50]);
strcpy(ret->PD12, row[51]);
strcpy(ret->PD13, row[52]);
strcpy(ret->PD14, row[53]);
strcpy(ret->PD15, row[54]);
strcpy(ret->PD16, row[55]);
strcpy(ret->PD17, row[56]);
strcpy(ret->PD18, row[57]);
strcpy(ret->PD19, row[58]);
strcpy(ret->PD20, row[59]);
strcpy(ret->PD21, row[60]);
strcpy(ret->PD22, row[61]);
strcpy(ret->PD23, row[62]);
strcpy(ret->PD24, row[63]);
}

struct affyGenoDetails *affyGenoDetailsLoad(char **row)
/* Load a affyGenoDetails from row fetched with select * from affyGenoDetails
 * from database.  Dispose of this with affyGenoDetailsFree(). */
{
struct affyGenoDetails *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->affyId = sqlUnsigned(row[0]);
ret->rsId = sqlUnsigned(row[1]);
strcpy(ret->baseA, row[2]);
strcpy(ret->baseB, row[3]);
strcpy(ret->sequenceA, row[4]);
strcpy(ret->sequenceB, row[5]);
strcpy(ret->enzyme, row[6]);
ret->minFreq = atof(row[7]);
ret->hetzyg = atof(row[8]);
ret->avHetSE = atof(row[9]);
strcpy(ret->NA04477, row[10]);
strcpy(ret->NA04479, row[11]);
strcpy(ret->NA04846, row[12]);
strcpy(ret->NA11036, row[13]);
strcpy(ret->NA11038, row[14]);
strcpy(ret->NA13056, row[15]);
strcpy(ret->NA17011, row[16]);
strcpy(ret->NA17012, row[17]);
strcpy(ret->NA17013, row[18]);
strcpy(ret->NA17014, row[19]);
strcpy(ret->NA17015, row[20]);
strcpy(ret->NA17016, row[21]);
strcpy(ret->NA17101, row[22]);
strcpy(ret->NA17102, row[23]);
strcpy(ret->NA17103, row[24]);
strcpy(ret->NA17104, row[25]);
strcpy(ret->NA17105, row[26]);
strcpy(ret->NA17106, row[27]);
strcpy(ret->NA17201, row[28]);
strcpy(ret->NA17202, row[29]);
strcpy(ret->NA17203, row[30]);
strcpy(ret->NA17204, row[31]);
strcpy(ret->NA17205, row[32]);
strcpy(ret->NA17206, row[33]);
strcpy(ret->NA17207, row[34]);
strcpy(ret->NA17208, row[35]);
strcpy(ret->NA17210, row[36]);
strcpy(ret->NA17211, row[37]);
strcpy(ret->NA17212, row[38]);
strcpy(ret->NA17213, row[39]);
strcpy(ret->PD01, row[40]);
strcpy(ret->PD02, row[41]);
strcpy(ret->PD03, row[42]);
strcpy(ret->PD04, row[43]);
strcpy(ret->PD05, row[44]);
strcpy(ret->PD06, row[45]);
strcpy(ret->PD07, row[46]);
strcpy(ret->PD08, row[47]);
strcpy(ret->PD09, row[48]);
strcpy(ret->PD10, row[49]);
strcpy(ret->PD11, row[50]);
strcpy(ret->PD12, row[51]);
strcpy(ret->PD13, row[52]);
strcpy(ret->PD14, row[53]);
strcpy(ret->PD15, row[54]);
strcpy(ret->PD16, row[55]);
strcpy(ret->PD17, row[56]);
strcpy(ret->PD18, row[57]);
strcpy(ret->PD19, row[58]);
strcpy(ret->PD20, row[59]);
strcpy(ret->PD21, row[60]);
strcpy(ret->PD22, row[61]);
strcpy(ret->PD23, row[62]);
strcpy(ret->PD24, row[63]);
return ret;
}

struct affyGenoDetails *affyGenoDetailsLoadAll(char *fileName) 
/* Load all affyGenoDetails from a whitespace-separated file.
 * Dispose of this with affyGenoDetailsFreeList(). */
{
struct affyGenoDetails *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[64];

while (lineFileRow(lf, row))
    {
    el = affyGenoDetailsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyGenoDetails *affyGenoDetailsLoadAllByChar(char *fileName, char chopper) 
/* Load all affyGenoDetails from a chopper separated file.
 * Dispose of this with affyGenoDetailsFreeList(). */
{
struct affyGenoDetails *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[64];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = affyGenoDetailsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct affyGenoDetails *affyGenoDetailsCommaIn(char **pS, struct affyGenoDetails *ret)
/* Create a affyGenoDetails out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new affyGenoDetails */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->affyId = sqlUnsignedComma(&s);
ret->rsId = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, ret->baseA, sizeof(ret->baseA));
sqlFixedStringComma(&s, ret->baseB, sizeof(ret->baseB));
sqlFixedStringComma(&s, ret->sequenceA, sizeof(ret->sequenceA));
sqlFixedStringComma(&s, ret->sequenceB, sizeof(ret->sequenceB));
sqlFixedStringComma(&s, ret->enzyme, sizeof(ret->enzyme));
ret->minFreq = sqlFloatComma(&s);
ret->hetzyg = sqlFloatComma(&s);
ret->avHetSE = sqlFloatComma(&s);
sqlFixedStringComma(&s, ret->NA04477, sizeof(ret->NA04477));
sqlFixedStringComma(&s, ret->NA04479, sizeof(ret->NA04479));
sqlFixedStringComma(&s, ret->NA04846, sizeof(ret->NA04846));
sqlFixedStringComma(&s, ret->NA11036, sizeof(ret->NA11036));
sqlFixedStringComma(&s, ret->NA11038, sizeof(ret->NA11038));
sqlFixedStringComma(&s, ret->NA13056, sizeof(ret->NA13056));
sqlFixedStringComma(&s, ret->NA17011, sizeof(ret->NA17011));
sqlFixedStringComma(&s, ret->NA17012, sizeof(ret->NA17012));
sqlFixedStringComma(&s, ret->NA17013, sizeof(ret->NA17013));
sqlFixedStringComma(&s, ret->NA17014, sizeof(ret->NA17014));
sqlFixedStringComma(&s, ret->NA17015, sizeof(ret->NA17015));
sqlFixedStringComma(&s, ret->NA17016, sizeof(ret->NA17016));
sqlFixedStringComma(&s, ret->NA17101, sizeof(ret->NA17101));
sqlFixedStringComma(&s, ret->NA17102, sizeof(ret->NA17102));
sqlFixedStringComma(&s, ret->NA17103, sizeof(ret->NA17103));
sqlFixedStringComma(&s, ret->NA17104, sizeof(ret->NA17104));
sqlFixedStringComma(&s, ret->NA17105, sizeof(ret->NA17105));
sqlFixedStringComma(&s, ret->NA17106, sizeof(ret->NA17106));
sqlFixedStringComma(&s, ret->NA17201, sizeof(ret->NA17201));
sqlFixedStringComma(&s, ret->NA17202, sizeof(ret->NA17202));
sqlFixedStringComma(&s, ret->NA17203, sizeof(ret->NA17203));
sqlFixedStringComma(&s, ret->NA17204, sizeof(ret->NA17204));
sqlFixedStringComma(&s, ret->NA17205, sizeof(ret->NA17205));
sqlFixedStringComma(&s, ret->NA17206, sizeof(ret->NA17206));
sqlFixedStringComma(&s, ret->NA17207, sizeof(ret->NA17207));
sqlFixedStringComma(&s, ret->NA17208, sizeof(ret->NA17208));
sqlFixedStringComma(&s, ret->NA17210, sizeof(ret->NA17210));
sqlFixedStringComma(&s, ret->NA17211, sizeof(ret->NA17211));
sqlFixedStringComma(&s, ret->NA17212, sizeof(ret->NA17212));
sqlFixedStringComma(&s, ret->NA17213, sizeof(ret->NA17213));
sqlFixedStringComma(&s, ret->PD01, sizeof(ret->PD01));
sqlFixedStringComma(&s, ret->PD02, sizeof(ret->PD02));
sqlFixedStringComma(&s, ret->PD03, sizeof(ret->PD03));
sqlFixedStringComma(&s, ret->PD04, sizeof(ret->PD04));
sqlFixedStringComma(&s, ret->PD05, sizeof(ret->PD05));
sqlFixedStringComma(&s, ret->PD06, sizeof(ret->PD06));
sqlFixedStringComma(&s, ret->PD07, sizeof(ret->PD07));
sqlFixedStringComma(&s, ret->PD08, sizeof(ret->PD08));
sqlFixedStringComma(&s, ret->PD09, sizeof(ret->PD09));
sqlFixedStringComma(&s, ret->PD10, sizeof(ret->PD10));
sqlFixedStringComma(&s, ret->PD11, sizeof(ret->PD11));
sqlFixedStringComma(&s, ret->PD12, sizeof(ret->PD12));
sqlFixedStringComma(&s, ret->PD13, sizeof(ret->PD13));
sqlFixedStringComma(&s, ret->PD14, sizeof(ret->PD14));
sqlFixedStringComma(&s, ret->PD15, sizeof(ret->PD15));
sqlFixedStringComma(&s, ret->PD16, sizeof(ret->PD16));
sqlFixedStringComma(&s, ret->PD17, sizeof(ret->PD17));
sqlFixedStringComma(&s, ret->PD18, sizeof(ret->PD18));
sqlFixedStringComma(&s, ret->PD19, sizeof(ret->PD19));
sqlFixedStringComma(&s, ret->PD20, sizeof(ret->PD20));
sqlFixedStringComma(&s, ret->PD21, sizeof(ret->PD21));
sqlFixedStringComma(&s, ret->PD22, sizeof(ret->PD22));
sqlFixedStringComma(&s, ret->PD23, sizeof(ret->PD23));
sqlFixedStringComma(&s, ret->PD24, sizeof(ret->PD24));
*pS = s;
return ret;
}

void affyGenoDetailsFree(struct affyGenoDetails **pEl)
/* Free a single dynamically allocated affyGenoDetails such as created
 * with affyGenoDetailsLoad(). */
{
struct affyGenoDetails *el;

if ((el = *pEl) == NULL) return;
freez(pEl);
}

void affyGenoDetailsFreeList(struct affyGenoDetails **pList)
/* Free a list of dynamically allocated affyGenoDetails's */
{
struct affyGenoDetails *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    affyGenoDetailsFree(&el);
    }
*pList = NULL;
}

void affyGenoDetailsOutput(struct affyGenoDetails *el, FILE *f, char sep, char lastSep) 
/* Print out affyGenoDetails.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->affyId);
fputc(sep,f);
fprintf(f, "%u", el->rsId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->baseA);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->baseB);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->sequenceA);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->sequenceB);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->enzyme);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->minFreq);
fputc(sep,f);
fprintf(f, "%f", el->hetzyg);
fputc(sep,f);
fprintf(f, "%f", el->avHetSE);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA04477);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA04479);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA04846);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA11036);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA11038);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA13056);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17011);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17012);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17013);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17014);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17015);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17016);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17101);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17102);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17103);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17104);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17105);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17106);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17201);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17202);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17203);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17204);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17205);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17206);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17207);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17208);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17210);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17211);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17212);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->NA17213);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD01);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD02);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD03);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD04);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD05);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD06);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD07);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD08);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD09);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD10);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD11);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD12);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD13);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD14);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD15);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD16);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD17);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD18);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD19);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD20);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD21);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD22);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD23);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->PD24);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

