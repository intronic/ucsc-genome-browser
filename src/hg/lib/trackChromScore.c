/* trackChromScore.c was originally generated by the autoSql program, which also 
 * generated trackChromScore.h and trackChromScore.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "trackChromScore.h"

void trackChromScoreStaticLoad(char **row, struct trackChromScore *ret)
/* Load a row from trackChromScore table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->track = row[0];
ret->chromosome = row[1];
ret->score = atof(row[2]);
}

struct trackChromScore *trackChromScoreLoad(char **row)
/* Load a trackChromScore from row fetched with select * from trackChromScore
 * from database.  Dispose of this with trackChromScoreFree(). */
{
struct trackChromScore *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->track = cloneString(row[0]);
ret->chromosome = cloneString(row[1]);
ret->score = atof(row[2]);
return ret;
}

struct trackChromScore *trackChromScoreLoadAll(char *fileName) 
/* Load all trackChromScore from a whitespace-separated file.
 * Dispose of this with trackChromScoreFreeList(). */
{
struct trackChromScore *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = trackChromScoreLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct trackChromScore *trackChromScoreCommaIn(char **pS, struct trackChromScore *ret)
/* Create a trackChromScore out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new trackChromScore */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->track = sqlStringComma(&s);
ret->chromosome = sqlStringComma(&s);
ret->score = sqlDoubleComma(&s);
*pS = s;
return ret;
}

void trackChromScoreFree(struct trackChromScore **pEl)
/* Free a single dynamically allocated trackChromScore such as created
 * with trackChromScoreLoad(). */
{
struct trackChromScore *el;

if ((el = *pEl) == NULL) return;
freeMem(el->track);
freeMem(el->chromosome);
freez(pEl);
}

void trackChromScoreFreeList(struct trackChromScore **pList)
/* Free a list of dynamically allocated trackChromScore's */
{
struct trackChromScore *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    trackChromScoreFree(&el);
    }
*pList = NULL;
}

void trackChromScoreOutput(struct trackChromScore *el, FILE *f, char sep, char lastSep) 
/* Print out trackChromScore.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->track);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chromosome);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->score);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


boolean trackChromScoreMakeTable(char *db)
/* Make trackChromScore table.  Return FALSE but print
 * no warning if table already exists. */
{
static char *create = "CREATE TABLE trackChromScore (\n"
"    track varchar(255) not null,	# What track this is associated with\n"
"    chromosome varchar(255) not null,	# Chromosome name\n"
"    score double not null,	# Average score per base for chromosome"
"              #Indices\n"
"    INDEX(track(32)),\n"
"    INDEX(chromosome(16))\n"
")\n";
struct sqlConnection *conn = sqlConnect(db);
int ret = sqlMaybeMakeTable(conn, "trackChromScore", create);
sqlDisconnect(&conn);
return ret;
}
