/* pgSnp.c was originally generated by the autoSql program, which also 
 * generated pgSnp.h and pgSnp.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "pgSnp.h"
#include "hdb.h"
#include "dnaseq.h"
#include "pgPhenoAssoc.h"
#include "regexHelper.h"

void pgSnpStaticLoad(char **row, struct pgSnp *ret)
/* Load a row from pgSnp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->bin = sqlUnsigned(row[0]);
ret->chrom = row[1];
ret->chromStart = sqlUnsigned(row[2]);
ret->chromEnd = sqlUnsigned(row[3]);
ret->name = row[4];
ret->alleleCount = sqlSigned(row[5]);
ret->alleleFreq = row[6];
ret->alleleScores = row[7];
}

struct pgSnp *pgSnpLoad(char **row)
/* Load a pgSnp from row fetched with select * from pgSnp
 * from database.  Dispose of this with pgSnpFree(). */
{
struct pgSnp *ret;

AllocVar(ret);
ret->bin = sqlUnsigned(row[0]);
ret->chrom = cloneString(row[1]);
ret->chromStart = sqlUnsigned(row[2]);
ret->chromEnd = sqlUnsigned(row[3]);
ret->name = cloneString(row[4]);
ret->alleleCount = sqlSigned(row[5]);
ret->alleleFreq = cloneString(row[6]);
ret->alleleScores = cloneString(row[7]);
return ret;
}

struct pgSnp *pgSnpLoadAll(char *fileName) 
/* Load all pgSnp from a whitespace-separated file.
 * Dispose of this with pgSnpFreeList(). */
{
struct pgSnp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = pgSnpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pgSnp *pgSnpLoadAllByChar(char *fileName, char chopper) 
/* Load all pgSnp from a chopper separated file.
 * Dispose of this with pgSnpFreeList(). */
{
struct pgSnp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = pgSnpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct pgSnp *pgSnpLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all pgSnp from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with pgSnpFreeList(). */
{
struct pgSnp *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = pgSnpLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void pgSnpSaveToDb(struct sqlConnection *conn, struct pgSnp *el, char *tableName, int updateSize)
/* Save pgSnp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use pgSnpSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( %u,'%s',%u,%u,'%s',%d,'%s','%s')", 
	tableName,  el->bin,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->alleleCount,  el->alleleFreq,  el->alleleScores);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void pgSnpSaveToDbEscaped(struct sqlConnection *conn, struct pgSnp *el, char *tableName, int updateSize)
/* Save pgSnp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than pgSnpSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *chrom, *name, *alleleFreq, *alleleScores;
chrom = sqlEscapeString(el->chrom);
name = sqlEscapeString(el->name);
alleleFreq = sqlEscapeString(el->alleleFreq);
alleleScores = sqlEscapeString(el->alleleScores);

dyStringPrintf(update, "insert into %s values ( %u,'%s',%u,%u,'%s',%d,'%s','%s')", 
	tableName,  el->bin,  chrom,  el->chromStart,  el->chromEnd,  name,  el->alleleCount,  alleleFreq,  alleleScores);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&chrom);
freez(&name);
freez(&alleleFreq);
freez(&alleleScores);
}

struct pgSnp *pgSnpCommaIn(char **pS, struct pgSnp *ret)
/* Create a pgSnp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pgSnp */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->bin = sqlUnsignedComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->alleleCount = sqlSignedComma(&s);
ret->alleleFreq = sqlStringComma(&s);
ret->alleleScores = sqlStringComma(&s);
*pS = s;
return ret;
}

void pgSnpFree(struct pgSnp **pEl)
/* Free a single dynamically allocated pgSnp such as created
 * with pgSnpLoad(). */
{
struct pgSnp *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->alleleFreq);
freeMem(el->alleleScores);
freez(pEl);
}

void pgSnpFreeList(struct pgSnp **pList)
/* Free a list of dynamically allocated pgSnp's */
{
struct pgSnp *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    pgSnpFree(&el);
    }
*pList = NULL;
}

void pgSnpOutput(struct pgSnp *el, FILE *f, char sep, char lastSep) 
/* Print out pgSnp.  Separate fields with sep. Follow last field with lastSep. */
{
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
fputc(sep,f);
fprintf(f, "%d", el->alleleCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alleleFreq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->alleleScores);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct pgCodon *fetchCodons(char *db, struct bed *gene, unsigned chrStart, unsigned chrEnd)
/* find codons containing region, return sequence and positions */
/* gene should have coding sequence only, bedThickOnly */
{
int codNum = 0;
int cPos = 0;
struct pgCodon *rv = NULL;
AllocVar(rv);
int frame = 0; /* num of nts needed to complete codon in next exon */
int i0, iN, iInc, i;
boolean posStrand;

if (startsWith("+", gene->strand)) //positive strand
    {
    i0 = 0; iN = gene->blockCount; iInc = 1;
    posStrand = TRUE;
    }
else
    {
    i0 = gene->blockCount-1; iN=-1; iInc = -1;
    posStrand = FALSE;
    }

for (i=i0; (iInc*i)<(iInc*iN); i=i+iInc)
    {
//printf("TESTING cPos=%d i=%d frame=%d\n", cPos, i, frame);
    if (chrStart >= (gene->chromStarts[i] + gene->chromStart) && 
        chrEnd <= gene->chromStarts[i] + gene->blockSizes[i] + gene->chromStart)
        {
        unsigned d = 0;
        int codStart = cPos + 1;
        unsigned cStart = 0;
        unsigned cEnd = 0;
        if (posStrand) 
            d = chrStart - (gene->chromStarts[i] + gene->chromStart);
        else
            d = (gene->chromStarts[i] + gene->chromStart + gene->blockSizes[i]-1) - (chrEnd - 1); 
        cPos += d;
        if (d > frame) 
            codNum += trunc((d - frame)/3) + 1; 
        if (frame > 0) { codNum++; }
        rv->firstCodon = codNum;
        if (d >= frame) 
            frame = (d - frame) % 3; 
        else 
            frame = 3 - frame + d;
        /* start of codon */
        rv->cdStart = cPos - frame; 
        rv->cdEnd = rv->cdStart + 3; /* end of first codon */
        rv->regStart = cPos;
        rv->regEnd = cPos + (chrEnd - chrStart);
//printf("TESTING d = %d cdSt = %d cdEnd = %d reg %d - %d frame=%d\n", d, rv->cdStart, rv->cdEnd, rv->regStart, rv->regEnd, frame);
        /* more than 1 codon? */
        while (rv->regEnd > rv->cdEnd)
            rv->cdEnd += 3;
        struct dyString *seq = newDyString(1024);
        /* check prev exon, chrom order not cds order */
        if (rv->cdStart < codStart && posStrand) 
            {
            int st = rv->cdStart;
            int end = codStart - 1;
            cStart = gene->chromStart + gene->chromStarts[i-1] + gene->blockSizes[i-1] - (end - st);
            cEnd = gene->chromStart + gene->chromStarts[i-1] + gene->blockSizes[i-1];
            struct dnaSeq *s = hDnaFromSeq(db, gene->chrom, cStart, cEnd, dnaUpper);
            dyStringPrintf(seq, "%s", s->dna);
            //freeDnaSeq(&s);
            }
        else if (!posStrand && rv->cdEnd >= (codStart + gene->blockSizes[i])) 
            {
            int st = codStart + gene->blockSizes[i] - 1;
            int end = rv->cdEnd;
            cEnd = gene->chromStart + gene->chromStarts[i-1] + gene->blockSizes[i-1];
            cStart = gene->chromStart + gene->chromStarts[i-1] + gene->blockSizes[i-1] - (end - st);
//error here?
            struct dnaSeq *s = hDnaFromSeq(db, gene->chrom, cStart, cEnd, dnaUpper);
            dyStringPrintf(seq, "%s", s->dna);
//printf("TESTING got seq=%s<br>\n", s->dna);
            //freeDnaSeq(&s);
            }
        /* get sequence needed from this exon */
        int st = rv->cdStart;
        if (rv->cdStart < codStart)
            st = codStart - 1;
        int end = rv->cdEnd;
        if (rv->cdEnd > (codStart + gene->blockSizes[i] - 1))
            end = codStart + gene->blockSizes[i] - 1;
        if (posStrand) 
            {
            cStart = gene->chromStart + gene->chromStarts[i] + (st - codStart + 1);
            cEnd = gene->chromStart + gene->chromStarts[i] + (end - codStart + 1);
            }
        else
            {
            /* minus strand coding start == chromEnd */
            cEnd = gene->chromStart + gene->chromStarts[i] + gene->blockSizes[i] - (st - codStart + 1);
            cStart = gene->chromStart + gene->chromStarts[i] + gene->blockSizes[i] - (end - codStart + 1);
            }
//printf("TESTING fetching sequence for %s:%d-%d\n", gene->chrom, cStart, cEnd);
        struct dnaSeq *s = hDnaFromSeq(db, gene->chrom, cStart, cEnd, dnaUpper);
        dyStringPrintf(seq, "%s", s->dna);
//printf("TESTING got seq=%s<br>\n", s->dna);
        //freeDnaSeq(&s);
        /* check following exons, chrom order */
        if (posStrand && rv->cdEnd >= codStart + gene->blockSizes[i]) 
            {
            int st = codStart + gene->blockSizes[i] + 1;
            int end = rv->cdEnd;
            cStart = gene->chromStart + gene->chromStarts[i+1] - 1;
            cEnd = gene->chromStart + gene->chromStarts[i+1] + (end - st + 1);
            struct dnaSeq *s = hDnaFromSeq(db, gene->chrom, cStart, cEnd, dnaUpper);
            dyStringPrintf(seq, "%s", s->dna);
            //freeDnaSeq(&s);
            }
        else if (!posStrand && rv->cdStart < (codStart - 1))
            {
            int st = rv->cdStart;
            int end = codStart - 1;
            cStart = gene->chromStart + gene->chromStarts[i+1];
            cEnd = gene->chromStart + gene->chromStarts[i+1] + (end - st);
//printf("TESTING fetching sequence for %s:%d-%d\n", gene->chrom, cStart, cEnd);
            struct dnaSeq *s = hDnaFromSeq(db, gene->chrom, cStart, cEnd, dnaUpper);
            dyStringPrintf(seq, "%s", s->dna);
            //freeDnaSeq(&s);
//printf("TESTING got seq=%s<br>\n", s->dna);
            }
        rv->seq = dyStringCannibalize(&seq);
        break;
        }
    /* increment values past this exon */
    codNum += trunc((gene->blockSizes[i] - frame) / 3);
    if (frame > 0) codNum++;
    cPos += gene->blockSizes[i];
    frame = 3 - ((gene->blockSizes[i] - frame) % 3);
    }
if (rv->seq == NULL) 
    return NULL; /* not found */
return rv;
}

char *replaceString(char *old, int from, int to, char *rep)
/* replace part of string based on character positions */
{
char *result = NULL;
char *resultPtr = NULL;
char *ptr = old+from;
int strLen = strlen(old) + strlen(rep);
int oldLen = to - from;
result = needMem(strLen);
resultPtr = result;
strLen = from; //beg
strcpy(resultPtr, old);
old = ptr + oldLen;
resultPtr += strLen;
strcpy(resultPtr, rep); //mid
resultPtr += strlen(rep);
strcpy(resultPtr, old); //end
return result;
}

void aaProperties (char *aa1, char *aa2);

void printSeqCodDisplay(char *db, struct pgSnp *item)
/* print the display of sequence changes for a coding variant */
{
struct bed *list = NULL, *el, *th = NULL;
struct sqlResult *sr;
char **row;
char query[512];
struct sqlConnection *conn = hAllocConn(db);
safef(query, sizeof(query), "select chrom, txStart, txEnd, name, 0, strand, cdsStart, cdsEnd, 0, exonCount, exonEnds, exonStarts  from knownGene where chrom = '%s' and cdsStart <= %d and cdsEnd >= %d",
   item->chrom, item->chromStart, item->chromEnd);

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = bedLoad12(row);
    /* adjust exonStarts and exonEnds to actual bed values */
    int i;
    for (i=0;i<el->blockCount;i++) 
        {
        el->blockSizes[i] = el->blockSizes[i] - el->chromStarts[i];
        el->chromStarts[i] = el->chromStarts[i] - el->chromStart;
        }
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
hFreeConn(&conn);

int found = 0;
th = bedThickOnlyList(list);
for (el = th; el != NULL; el = el->next)
    {
    struct pgCodon *cod = fetchCodons(db, el, item->chromStart, item->chromEnd);
    if (cod == NULL) 
        continue; /* not in exon */
    if (found == 0)
	printf("\n<BR>Coding sequence changes are relative to strand of transcript:<BR>\n");
    found++;
    if (sameString(el->strand, "-")) 
        reverseComplement(cod->seq, strlen(cod->seq)); 
    /* bold changing seq */
    char old[512];
    strncpy(old, cod->seq+(cod->regStart - cod->cdStart), (cod->regEnd - cod->regStart));
    old[cod->regEnd - cod->regStart] = '\0';
    char b[512];
    safef(b, sizeof(b), "<B>%s</B>", old);
    char *bold = replaceString(cod->seq, (cod->regStart - cod->cdStart), (cod->regEnd - cod->cdStart), b);
    char *nameCopy = cloneString(item->name);
    char *allele[8];
    (void) chopByChar(nameCopy, '/', allele, item->alleleCount);
    int i;
    printf("%s:starting positions codon %d cds %d<BR>\n", el->name, cod->firstCodon, (cod->cdStart+1));
    for (i=0;i<item->alleleCount;i++)
        {
        char a[512];
        if (sameString(el->strand, "-"))
            reverseComplement(allele[i], strlen(allele[i]));
        safef(a, sizeof(a), "<B>%s</B>", allele[i]);
        char *rep = replaceString(cod->seq, (cod->regStart - cod->cdStart), (cod->regEnd - cod->cdStart), a);
        printf("%s &gt; %s<BR>\n", bold, rep);
        
        if (item->chromStart == item->chromEnd &&
            !strstr(rep, "-") && (strlen(rep)-7) % 3 != 0)
            {
            printf("&nbsp;&nbsp;&nbsp;&nbsp;frameshift<BR>\n");
            }
        else if (item->chromStart == item->chromEnd &&
                 countChars(allele[i], '-') == strlen(allele[i]))
            {
            printf("&nbsp;&nbsp;&nbsp;&nbsp;wildtype<BR>\n");
            }
        else
            {
            struct dnaSeq *dnaseq = newDnaSeq(cod->seq, strlen(cod->seq), "orig");
            aaSeq *origAa = translateSeq(dnaseq, 0, FALSE);
            if (!strstr(origAa->dna, "X"))
                {
                char *rep2 = replaceString(cod->seq, (cod->regStart - cod->cdStart), (cod->regEnd - cod->cdStart), allele[i]);
                dnaseq = newDnaSeq(rep2, strlen(rep2), "rep2");
                aaSeq *repAa = translateSeq(dnaseq, 0, FALSE);
                //freeDnaSeq(&dnaseq);
                if (!strstr(repAa->dna, "X") && isNotEmpty(repAa->dna))
                    {
                    printf("&nbsp;&nbsp;&nbsp;&nbsp;%s &gt; %s<BR>\n",
                        origAa->dna, repAa->dna);
                    if (differentString(origAa->dna, repAa->dna))
                        aaProperties(origAa->dna, repAa->dna);
                    }
                else if ((countChars(rep2, '-')) % 3 != 0)
                    {
                    printf("&nbsp;&nbsp;&nbsp;&nbsp;frameshift<BR>\n");
                    }
                }
            }
        }
    }
bedFreeList(&list);
}

char *aaPolarity (char *aa) 
/* return the polarity of the amino acid */
{
if (sameString(aa, "A") ||
    sameString(aa, "G") ||
    sameString(aa, "I") ||
    sameString(aa, "L") ||
    sameString(aa, "M") ||
    sameString(aa, "F") ||
    sameString(aa, "P") ||
    sameString(aa, "W") ||
    sameString(aa, "V"))
    return cloneString("nonpolar");
return cloneString("polar");
}

char *aaAcidity (char *aa) 
/* return the acidity */
{
if (sameString(aa, "R")) 
    return cloneString("basic (strongly)");
else if (sameString(aa, "H"))
    return cloneString("basic (weakly)");
else if (sameString(aa, "K"))
    return cloneString("basic");
else if (sameString(aa, "D") ||
         sameString(aa, "E"))
    return cloneString("acidic");
else 
    return cloneString("neutral");
}

float aaHydropathy (char *aa) 
/* return the hydropathy */
{
if (sameString(aa, "A"))
    return 1.8;
if (sameString(aa, "R"))
    return -4.5;
if (sameString(aa, "N"))
    return -3.5;
if (sameString(aa, "D"))
    return -3.5;
if (sameString(aa, "C"))
    return 2.5;
if (sameString(aa, "E"))
    return -3.5;
if (sameString(aa, "Q"))
    return -3.5;
if (sameString(aa, "G"))
    return -0.4;
if (sameString(aa, "H"))
    return -3.2;
if (sameString(aa, "I"))
    return 4.5;
if (sameString(aa, "L"))
    return 3.8;
if (sameString(aa, "K"))
    return -3.9;
if (sameString(aa, "M"))
    return 1.9;
if (sameString(aa, "F"))
    return 2.8;
if (sameString(aa, "P"))
    return -1.6;
if (sameString(aa, "S"))
    return -0.8;
if (sameString(aa, "T"))
    return -0.7;
if (sameString(aa, "W"))
    return -0.9;
if (sameString(aa, "Y"))
    return -1.3;
if (sameString(aa, "V"))
    return 4.2;
return 0;
}

void aaProperties (char *aa1, char *aa2)
/* print amino acid properties for these amino acids */
{
char *pol1 = aaPolarity(aa1);
char *pol2 = aaPolarity(aa2);
char *acid1 = aaAcidity(aa1);
char *acid2 = aaAcidity(aa2);
float hyd1 = aaHydropathy(aa1);
float hyd2 = aaHydropathy(aa2);
printf("<table border=\"1\"><caption>Amino acid properties</caption><tr><td>&nbsp;</td><td>%s</td><td>%s</td></tr>\n", aa1, aa2);
/* take out highlights, not sure what is significant change for hydropathy */
//if (differentString(pol1, pol2))
    //printf("<tr bgcolor=\"white\"><td>polarity</td><td>%s</td><td>%s</td></tr>\n", pol1, pol2);
//else 
    printf("<tr><td>polarity</td><td>%s</td><td>%s</td></tr>\n", pol1, pol2);
//if (differentString(acid1, acid2) && 
    //(!startsWith("basic", acid1) || !startsWith("basic", acid2)) )
    //printf("<tr bgcolor=\"white\"><td>acidity</td><td>%s</td><td>%s</td></tr>\n", acid1, acid2);
//else 
    printf("<tr><td>acidity</td><td>%s</td><td>%s</td></tr>\n", acid1, acid2);
//if ((hyd1 < 0 && hyd2 > 0) || (hyd1 > 0 && hyd2 < 0))
    //printf("<tr bgcolor=\"white\"><td>hydropathy</td><td>%1.1f</td><td>%1.1f</td></tr></table>\n", hyd1, hyd2);
//else 
    printf("<tr><td>hydropathy</td><td>%1.1f</td><td>%1.1f</td></tr></table>\n", hyd1, hyd2);
printf("<br>");
}

void printPgDbLink(char *db, struct trackDb *tdb, struct pgSnp *item)
/* print the links to phenotype and other databases for pgSnps */
{
struct pgPhenoAssoc *el;
struct sqlResult *sr;
char **row;
char query[512];
struct sqlConnection *conn = hAllocConn(db);
char *dbList[8];
int tot = 0, i = 0, first = 1;
char *tabs = trackDbSetting(tdb, "pgDbLink");
if (tabs == NULL)
    return;
tot = chopByWhite(tabs, dbList, ArraySize(dbList));
for(i=0;i<tot;i++)
    {
    safef(query, sizeof(query), "select chrom, chromStart, chromEnd, name, srcUrl from %s where chrom = '%s' and chromStart = %d and chromEnd = %d",
    dbList[i], item->chrom, item->chromStart, item->chromEnd);

    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
        {
        if (first == 1)
             {
             printf("<br><b>Links to phenotype databases</b><br>\n");
             first = 0;
             }
        el = pgPhenoAssocLoad(row);
        printf("<a href=\"%s\">%s</a></br>\n", el->srcUrl, el->name);
        }
    }
}

char *pgSnpAutoSqlString =
"table pgSnp"
"\"personal genome SNP\""
"   ("
"   ushort  bin;            \"A field to speed indexing\""
"   string  chrom;          \"Chromosome\""
"   uint    chromStart;     \"Start position in chrom\""
"   uint    chromEnd;       \"End position in chrom\""
"   string  name;           \"alleles ACTG[/ACTG]\""
"   int     alleleCount;    \"number of alleles\""
"   string  alleleFreq;     \"comma separated list of frequency of each allele\""
"   string  alleleScores;   \"comma separated list of quality scores\""
"   )"
;

struct pgSnp *pgSnpLoadNoBin(char **row)
/* load pgSnp struct from row without bin */
{
struct pgSnp *ret;

AllocVar(ret);
ret->bin = 0;
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->alleleCount = sqlSigned(row[4]);
ret->alleleFreq = cloneString(row[5]);
ret->alleleScores = cloneString(row[6]);
return ret;
}

struct pgSnp *pgSnpLineFileLoad(char **row, struct lineFile *lf)
/* Load pgSnp from a lineFile line, with error checking. */
/* Requires comma separated zeroes for frequency and scores. */
{
struct pgSnp *item;
AllocVar(item);
item->chrom = cloneString(row[0]);
item->chromStart = lineFileNeedNum(lf, row, 1);
item->chromEnd = lineFileNeedNum(lf, row, 2);
if (item->chromEnd < 1)
    lineFileAbort(lf, "chromEnd less than 1 (%d)", item->chromEnd);
if (item->chromEnd < item->chromStart)
    lineFileAbort(lf, "chromStart after chromEnd (%d > %d)",
        item->chromStart, item->chromEnd);
/* use pattern match to check values and counts both */
/* alleles are separated by / and can be ACTG- */
item->name = cloneString(row[3]);
/* allele count, positive integer matching # of alleles */
item->alleleCount = lineFileNeedNum(lf, row, 4);
char alleles[128]; /* pattern to match alleles */
safef(alleles, sizeof(alleles), "^[ACTG-]+(\\/[ACTG-]+){%d}$", item->alleleCount - 1);
if (! regexMatchNoCase(row[3], alleles))
    lineFileAbort(lf, "invalid alleles %s", row[3]);
/* read count, comma separated list of numbers with above # of items */
item->alleleFreq = cloneString(row[5]);
char pattern[128];
safef(pattern, sizeof(pattern), "^[0-9]+(,[0-9]+){%d}$", item->alleleCount - 1);
if (! regexMatchNoCase(row[5], pattern))
    lineFileAbort(lf, "invalid allele frequency, %s with count of %d", row[5], item->alleleCount);
/* scores, comma separated list of numbers with above # of items */
item->alleleScores = cloneString(row[6]);
safef(pattern, sizeof(pattern), "^[0-9.]+(,[0-9.]+){%d}$", item->alleleCount - 1);
if (! regexMatchNoCase(row[6], pattern))
    lineFileAbort(lf, "invalid allele scores, %s with count of %d", row[6], item->alleleCount);
return item;
}

#define VCF_MAX_ALLELE_LEN 80

static char *alleleCountsFromVcfRecord(struct vcfRecord *rec, int alDescCount)
/* Build up comma-sep list of per-allele counts, if available, up to alDescCount
 * which may be less than rec->alleleCount: */
{
static struct dyString *dy = NULL;
if (dy == NULL)
    dy = dyStringNew(0);
else
    dyStringClear(dy);
dyStringClear(dy);
int alCounts[VCF_MAX_ALLELE_LEN];
boolean gotTotalCount = FALSE, gotAltCounts = FALSE;
int i;
for (i = 0;  i < rec->infoCount;  i++)
    if (sameString(rec->infoElements[i].key, "AN"))
	{
	gotTotalCount = TRUE;
	// Set ref allele to total count, subtract alt counts below.
	alCounts[0] = rec->infoElements[i].values[0].datInt;
	break;
	}
for (i = 0;  i < rec->infoCount;  i++)
    if (sameString(rec->infoElements[i].key, "AC"))
	{
	if (rec->infoElements[i].count > 0)
	    {
	    gotAltCounts = TRUE;
	    int j;
	    for (j = 0;  j < rec->infoElements[i].count && j < alDescCount-1;  j++)
		{
		int ac = rec->infoElements[i].values[j].datInt;
		alCounts[1+j] = ac;
		if (gotTotalCount)
		    alCounts[0] -= ac;
		}
	    while (j++ < alDescCount-1)
		alCounts[1+j] = -1;
	    if (gotTotalCount)
		dyStringPrintf(dy, "%d", alCounts[0]);
	    else
		dyStringAppend(dy, "-1");
	    for (j = 1;  j < alDescCount;  j++)
		if (alCounts[j] >= 0)
		    dyStringPrintf(dy, ",%d", alCounts[j]);
		else
		    dyStringAppend(dy, ",-1");
	    }
	break;
	}
if (gotTotalCount && !gotAltCounts)
    dyStringPrintf(dy, "%d", alCounts[0]);
return cloneStringZ(dy->string, dy->stringSize+1);
}

struct pgSnp *pgSnpFromVcfRecord(struct vcfRecord *rec)
/* Convert VCF rec to pgSnp; don't free rec->file (vcfFile) until
 * you're done with pgSnp because pgSnp points to rec->chrom. */
{
static struct dyString *dy = NULL;
if (dy == NULL)
    dy = dyStringNew(0);
else
    dyStringClear(dy);
struct pgSnp *pgs;
AllocVar(pgs);
pgs->chrom = rec->chrom;
pgs->chromStart = rec->chromStart;
pgs->chromEnd = rec->chromEnd;
// Build up slash-separated allele string from rec->alleles, starting with ref allele:
dyStringAppend(dy, rec->alleles[0]);
int alCount = rec->alleleCount, i;
if (rec->alleleCount == 2 && sameString(rec->alleles[1], "."))
    // ignore N/A alternate allele
    alCount = 1;
else if (rec->alleleCount >= 2)
    {
    // append /-sep'd alternate alleles, unless/until it gets too long:
    for (i = 1;  i < rec->alleleCount;  i++)
	{
	if ((dy->stringSize + 1 + strlen(rec->alleles[i])) > VCF_MAX_ALLELE_LEN)
	    break;
	dyStringPrintf(dy, "/%s", rec->alleles[i]);
	}
    if (i < rec->alleleCount)
	alCount = i;
    }
pgs->name = cloneStringZ(dy->string, dy->stringSize+1);
pgs->alleleCount = alCount;
pgs->alleleFreq = alleleCountsFromVcfRecord(rec, alCount);
// Build up comma-sep list... supposed to be per-allele quality scores but I think
// the VCF spec only gives us one BQ... for the reference position?  should ask.
dyStringClear(dy);
for (i = 0;  i < rec->infoCount;  i++)
    if (sameString(rec->infoElements[i].key, "BQ"))
	{
	float qual = rec->infoElements[i].values[0].datFloat;
	dyStringPrintf(dy, "%.1f", qual);
	int j;
	for (j = 1;  j < rec->alleleCount;  j++)
	    dyStringPrintf(dy, ",%.1f", qual);
	break;
	}
pgs->alleleScores = cloneStringZ(dy->string, dy->stringSize+1);
return pgs;
}

