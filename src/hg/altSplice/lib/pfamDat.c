/* pfamDat.c was originally generated by the autoSql program, which also 
 * generated pfamDat.h and pfamDat.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "pfamDat.h"


void pfamHitStaticLoad(char **row, struct pfamHit *ret)
/* Load a row from pfamHit table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->model = row[0];
ret->descript = row[1];
ret->score = atof(row[2]);
ret->eval = atof(row[3]);
ret->numTimesHit = sqlUnsigned(row[4]);
}

struct pfamHit *pfamHitLoad(char **row)
/* Load a pfamHit from row fetched with select * from pfamHit
 * from database.  Dispose of this with pfamHitFree(). */
{
struct pfamHit *ret;

AllocVar(ret);
ret->model = cloneString(row[0]);
ret->descript = cloneString(row[1]);
ret->score = atof(row[2]);
ret->eval = atof(row[3]);
ret->numTimesHit = sqlUnsigned(row[4]);
return ret;
}

struct pfamHit *pfamHitLoadAll(char *fileName) 
/* Load all pfamHit from a whitespace-separated file.
 * Dispose of this with pfamHitFreeList(). */
{
struct pfamHit *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileRow(lf, row))
    {
    el = pfamHitLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pfamHit *pfamHitLoadAllByChar(char *fileName, char chopper) 
/* Load all pfamHit from a chopper separated file.
 * Dispose of this with pfamHitFreeList(). */
{
struct pfamHit *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[5];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pfamHitLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pfamHit *pfamHitCommaIn(char **pS, struct pfamHit *ret)
/* Create a pfamHit out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pfamHit */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->model = sqlStringComma(&s);
ret->descript = sqlStringComma(&s);
ret->score = sqlDoubleComma(&s);
ret->eval = sqlDoubleComma(&s);
ret->numTimesHit = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void pfamHitFree(struct pfamHit **pEl)
/* Free a single dynamically allocated pfamHit such as created
 * with pfamHitLoad(). */
{
struct pfamHit *el;

if ((el = *pEl) == NULL) return;
freeMem(el->model);
freeMem(el->descript);
freez(pEl);
}

void pfamHitFreeList(struct pfamHit **pList)
/* Free a list of dynamically allocated pfamHit's */
{
struct pfamHit *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pfamHitFree(&el);
    }
*pList = NULL;
}

void pfamHitOutput(struct pfamHit *el, FILE *f, char sep, char lastSep) 
/* Print out pfamHit.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->model);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->descript);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->score);
fputc(sep,f);
fprintf(f, "%f", el->eval);
fputc(sep,f);
fprintf(f, "%u", el->numTimesHit);
fputc(lastSep,f);
}

void pfamDHitStaticLoad(char **row, struct pfamDHit *ret)
/* Load a row from pfamDHit table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->model = row[0];
ret->domain = sqlSigned(row[1]);
ret->numDomain = sqlSigned(row[2]);
ret->seqStart = sqlUnsigned(row[3]);
ret->seqEnd = sqlUnsigned(row[4]);
ret->seqRep = row[5];
ret->hmmStart = sqlUnsigned(row[6]);
ret->hmmEnd = sqlUnsigned(row[7]);
ret->hmmRep = row[8];
ret->dScore = atof(row[9]);
ret->dEval = atof(row[10]);
ret->alignment = row[11];
}

struct pfamDHit *pfamDHitLoad(char **row)
/* Load a pfamDHit from row fetched with select * from pfamDHit
 * from database.  Dispose of this with pfamDHitFree(). */
{
struct pfamDHit *ret;

AllocVar(ret);
ret->model = cloneString(row[0]);
ret->domain = sqlSigned(row[1]);
ret->numDomain = sqlSigned(row[2]);
ret->seqStart = sqlUnsigned(row[3]);
ret->seqEnd = sqlUnsigned(row[4]);
ret->seqRep = cloneString(row[5]);
ret->hmmStart = sqlUnsigned(row[6]);
ret->hmmEnd = sqlUnsigned(row[7]);
ret->hmmRep = cloneString(row[8]);
ret->dScore = atof(row[9]);
ret->dEval = atof(row[10]);
ret->alignment = cloneString(row[11]);
return ret;
}

struct pfamDHit *pfamDHitLoadAll(char *fileName) 
/* Load all pfamDHit from a whitespace-separated file.
 * Dispose of this with pfamDHitFreeList(). */
{
struct pfamDHit *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileRow(lf, row))
    {
    el = pfamDHitLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pfamDHit *pfamDHitLoadAllByChar(char *fileName, char chopper) 
/* Load all pfamDHit from a chopper separated file.
 * Dispose of this with pfamDHitFreeList(). */
{
struct pfamDHit *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pfamDHitLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pfamDHit *pfamDHitCommaIn(char **pS, struct pfamDHit *ret)
/* Create a pfamDHit out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pfamDHit */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->model = sqlStringComma(&s);
ret->domain = sqlSignedComma(&s);
ret->numDomain = sqlSignedComma(&s);
ret->seqStart = sqlUnsignedComma(&s);
ret->seqEnd = sqlUnsignedComma(&s);
ret->seqRep = sqlStringComma(&s);
ret->hmmStart = sqlUnsignedComma(&s);
ret->hmmEnd = sqlUnsignedComma(&s);
ret->hmmRep = sqlStringComma(&s);
ret->dScore = sqlDoubleComma(&s);
ret->dEval = sqlDoubleComma(&s);
ret->alignment = sqlStringComma(&s);
*pS = s;
return ret;
}

void pfamDHitFree(struct pfamDHit **pEl)
/* Free a single dynamically allocated pfamDHit such as created
 * with pfamDHitLoad(). */
{
struct pfamDHit *el;

if ((el = *pEl) == NULL) return;
freeMem(el->model);
freeMem(el->seqRep);
freeMem(el->hmmRep);
freeMem(el->alignment);
freez(pEl);
}

void pfamDHitFreeList(struct pfamDHit **pList)
/* Free a list of dynamically allocated pfamDHit's */
{
struct pfamDHit *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pfamDHitFree(&el);
    }
*pList = NULL;
}

void pfamDHitOutput(struct pfamDHit *el, FILE *f, char sep, char lastSep) 
/* Print out pfamDHit.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->model);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->domain);
fputc(sep,f);
fprintf(f, "%d", el->numDomain);
fputc(sep,f);
fprintf(f, "%u", el->seqStart);
fputc(sep,f);
fprintf(f, "%u", el->seqEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->seqRep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->hmmStart);
fputc(sep,f);
fprintf(f, "%u", el->hmmEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hmmRep);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->dScore);
fputc(sep,f);
fprintf(f, "%f", el->dEval);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alignment);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

struct pfamDat *pfamDatLoad(char **row)
/* Load a pfamDat from row fetched with select * from pfamDat
 * from database.  Dispose of this with pfamDatFree(). */
{
struct pfamDat *ret;
char *s;

AllocVar(ret);
ret->seqName = cloneString(row[0]);
ret->sequence = cloneString(row[1]);
s = row[2];
if(s != NULL && differentString(s, ""))
   ret->pfamHitList = pfamHitCommaIn(&s, NULL);
s = row[3];
if(s != NULL && differentString(s, ""))
   ret->pfamDHitList = pfamDHitCommaIn(&s, NULL);
return ret;
}

struct pfamDat *pfamDatLoadAll(char *fileName) 
/* Load all pfamDat from a whitespace-separated file.
 * Dispose of this with pfamDatFreeList(). */
{
struct pfamDat *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = pfamDatLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pfamDat *pfamDatLoadAllByChar(char *fileName, char chopper) 
/* Load all pfamDat from a chopper separated file.
 * Dispose of this with pfamDatFreeList(). */
{
struct pfamDat *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pfamDatLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pfamDat *pfamDatCommaIn(char **pS, struct pfamDat *ret)
/* Create a pfamDat out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pfamDat */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->seqName = sqlStringComma(&s);
ret->sequence = sqlStringComma(&s);
s = sqlEatChar(s, '{');
if(s[0] != '}')    slSafeAddHead(&ret->pfamHitList, pfamHitCommaIn(&s,NULL));
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
s = sqlEatChar(s, '{');
if(s[0] != '}')    slSafeAddHead(&ret->pfamDHitList, pfamDHitCommaIn(&s,NULL));
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
*pS = s;
return ret;
}

void pfamDatFree(struct pfamDat **pEl)
/* Free a single dynamically allocated pfamDat such as created
 * with pfamDatLoad(). */
{
struct pfamDat *el;

if ((el = *pEl) == NULL) return;
freez(&el->header);
freeMem(el->seqName);
freeMem(el->sequence);
pfamHitFreeList(&el->pfamHitList);
pfamDHitFreeList(&el->pfamDHitList);
freez(pEl);
}

void pfamDatFreeList(struct pfamDat **pList)
/* Free a list of dynamically allocated pfamDat's */
{
struct pfamDat *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pfamDatFree(&el);
    }
*pList = NULL;
}

void pfamDatOutput(struct pfamDat *el, FILE *f, char sep, char lastSep) 
/* Print out pfamDat.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->seqName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->sequence);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
if(el->pfamHitList != NULL)    pfamHitCommaOut(el->pfamHitList,f);
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('{',f);
if(el->pfamDHitList != NULL)    pfamDHitCommaOut(el->pfamDHitList,f);
if (sep == ',') fputc('}',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


static void addHitTabs(char *line, int *offsets, int offsetCount)
/* Add a tab at the correct offsets for a pfam file. */
{
int i;
int tmpOffset = 0;
int maxFuzzy =2;
for(i=0; i < offsetCount; i++)
    {
    maxFuzzy = 2;
    tmpOffset = offsets[i];
    /* Sometimes we have to back up a space or two. */
    while(line[tmpOffset] != ' ' && maxFuzzy >= 0)
	{
	tmpOffset--;
	maxFuzzy--;
	}
    if(line[tmpOffset] != ' ')
	errAbort("pfamDat::addHitTabs() - Found a '%c' where expecting a ' ' at position %d in line %s",
		 line[tmpOffset], tmpOffset, line);
    line[tmpOffset] = '\t';
    }
}

void determineOffsets(char *line, int **offsets, int *offsetCount)
/* Take the pfam line with the underlines '-----   ------' and 
   determine their field starting positions from it. Free offsets
   with freez(). */
{
int numBlocks = chopByWhite(line, NULL, 0);
int i = 0, length = strlen(line); //  int offSets[] = {15,54,62,72}; int offSets[] = {15,19,23,29,35,38,44,50,54,61};
AllocArray(*offsets, numBlocks - 1);
*offsetCount = 0;
for(i = 0; i < length -1; i++)
    {
    if(line[i] != '-' && line[i+1] == '-') 
	{
	if(*offsetCount > numBlocks -1)
	    errAbort("pfamDat::determineOffsets() - Found too many offset.");
	(*offsets)[(*offsetCount)++] = i;
	}
    }
}

struct pfamDat *pfamDatFromPfamFile(char *fileName)
/* Parse a hmmpfam generated file and return a pfamDat structure 
   containing hits. */
{
struct lineFile *lf = NULL;
struct pfamDat *pfamDat = NULL;
char *line=NULL, *mark=NULL;
char *startToken = "Query sequence: ";
char *pfamToken = "hmmpfam";
char *domainToken = "Parsed for domains:";
char *noResToken = "[no hits above thresholds]";
char *alignmentToken = "Alignments";
int numStrings=0;
struct dyString *header = newDyString(512);
struct pfamHit *pfamHit = NULL;
struct pfamDHit *pfamDHit = NULL;
struct dyString *alignment = newDyString(1024);
char *slash = NULL;
int i;
char **row;
int *offsets = NULL;
int offsetCount = 0;

AllocArray(row, 15);

lf = lineFileOpen(fileName, TRUE);
/* Make sure this is a pfam file. */

lineFileNextReal(lf, &line);
if(startsWith(pfamToken, line) == FALSE)
    errAbort("pfamDat::pfamDatFromPfamFile() - %s doesn't start with %s. "
	     "Probably not a hmmpfam generated file.", fileName, pfamToken);

/* Find end of headers. */
dyStringPrintf(header, "%s\n",line);
while(lineFileNextReal(lf, &line))
    {
    if(startsWith(startToken, line))
	break;
    else
	dyStringPrintf(header, "%s\n",line);
    }

/* Parse out seq name. */
mark = line+strlen(startToken);
AllocVar(pfamDat);
pfamDat->seqName = cloneString(mark);
pfamDat->sequence = cloneString("NA");

/* Get the global hits. */
while(lineFileNextReal(lf, &line))
    if(startsWith("--------",line))
	break;
determineOffsets(line, &offsets, &offsetCount);
while(lineFileNextReal(lf, &line))
    {
    if(strlen(line) < 2 || strstr(line,noResToken) || startsWith(domainToken,line))
	break;
    addHitTabs(line, offsets, offsetCount);
    numStrings = chopByChar(line, '\t', row, 15);
    for(i=0;i<numStrings;i++)
	row[i] = trimSpaces(row[i]);
    pfamHit = pfamHitLoad(row);
    slAddHead(&pfamDat->pfamHitList, pfamHit);
    }
slReverse(&pfamDat->pfamHitList);
freez(&offsets);
offsetCount = 0;
/* Get the domain hits. */
while(lineFileNextReal(lf, &line))
    if(startsWith("--------",line))
	break;
determineOffsets(line, &offsets, &offsetCount);
while(lineFileNextReal(lf, &line))
    {
    if(strlen(line) < 2 || strstr(line,noResToken) || startsWith(alignmentToken,line))
	break;
//    addDHitTabs(line, offsets, offsetCount);
    /* Change the slash in the first field to a space. */
    slash = strchr(line+offsets[0], '/');
    if(slash != NULL)
	slash[0] = ' ';
    numStrings = chopByWhite(line, row, 15);
    for(i=0;i<numStrings;i++)
	row[i] = trimSpaces(row[i]);
    pfamDHit = pfamDHitLoad(row);
    slAddHead(&pfamDat->pfamDHitList, pfamDHit);
    }
slReverse(&pfamDat->pfamDHitList);
freez(&offsets);
offsetCount = 0;

/* Get the domain hits. */
lineFileNextReal(lf, &line);

for(pfamDHit = pfamDat->pfamDHitList; pfamDHit != NULL; pfamDHit=pfamDHit->next)
    {
    dyStringClear(alignment);
    dyStringPrintf(alignment, "%s", line);
    while(lineFileNextReal(lf, &line))
	{
	if(strstr(line, ":") || startsWith("//", line))
	    {
	    pfamDHit->alignment = cloneString(alignment->string);
	    break;
	    }
	dyStringPrintf(alignment, "@%s", line);
	}
    }
lineFileClose(&lf);
dyStringFree(&alignment);
freez(&row);
pfamDat->header = cloneString(header->string);
dyStringFree(&header);
return pfamDat;
}

void pfamDatWritePfamFile(struct pfamDat *pfamDat, char *fileName)
/* Write out a pfam file, mimicking pfam file format enough to diff. */
{

FILE *out = mustOpen(fileName,"w");
int nameWidth = 15, descWidth=38;
struct pfamHit *pHit = NULL;
struct pfamDHit *pDHit = NULL;

if(pfamDat->header == NULL)
    fputs("hmmpfam - search one or more sequences against HMM database\n"
      "Freely distributed under the GNU General Public License (GPL)\n"
      "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
      "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\nQuery sequence: ", out);
else
    fprintf(out, "%s\nQuery sequence: ", pfamDat->header);
fprintf(out,"%s\n", pfamDat->seqName);
fputs("Accession:      [none]\n"
      "Description:    [none]\n"
      "\n"
      "Scores for sequence family classification (score includes all domains):\n"
      "Model           Description                             Score    E-value  N \n"
      "--------        -----------                             -----    ------- ---\n", out);
if(pfamDat->pfamHitList == NULL)
    fprintf(out, "	[no hits above thresholds]\n");
else
    {
    for(pHit = pfamDat->pfamHitList; pHit != NULL; pHit = pHit->next)
	{
	fprintf(out, "%-*s %-*.*s %7.1f %10.2g %3d\n",
		nameWidth, 
		pHit->model,
		descWidth, descWidth, pHit->descript,
		pHit->score, pHit->eval, pHit->numTimesHit);
	}
    }
fputs( "Parsed for domains:\n"
      "Model           Domain  seq-f seq-t    hmm-f hmm-t      score  E-value\n"
      "--------        ------- ----- -----    ----- -----      -----  -------\n",out);
if(pfamDat->pfamDHitList == NULL)
    fprintf(out, "	[no hits above thresholds]\n");
for(pDHit = pfamDat->pfamDHitList; pDHit != NULL; pDHit = pDHit->next)
    {
    fprintf(out, "%-*s %3d/%-3d %5d %5d %s %5d %5d %s %7.1f %8.2g\n",
	    nameWidth, 
	    pDHit->model,
	    pDHit->domain, pDHit->numDomain,
	    pDHit->seqStart, pDHit->seqEnd, 
	    pDHit->seqRep,
	    pDHit->hmmStart, pDHit->hmmEnd, 
	    pDHit->hmmRep,
	    pDHit->dScore, pDHit->dEval);
    }
fputs("\nAlignments of top-scoring domains:\n",out);
if(pfamDat->pfamDHitList == NULL)
    fprintf(out, "	[no hits above thresholds]\n");
for(pDHit = pfamDat->pfamDHitList; pDHit != NULL; pDHit = pDHit->next)
    {
    subChar(pDHit->alignment, '@', '\n');
    fprintf(out,"%s\n\n", pDHit->alignment);
    }
fprintf(out, "//");
carefulClose(&out);
}
