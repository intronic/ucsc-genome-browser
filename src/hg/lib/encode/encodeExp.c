/* encodeExp.c was originally generated by the autoSql program, which also 
 * generated encodeExp.h and encodeExp.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encode/encodeExp.h"

void encodeExpStaticLoad(char **row, struct encodeExp *ret)
/* Load a row from encodeExp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->ix = sqlSigned(row[0]);
ret->organism = row[1];
ret->accession = row[2];
ret->lab = row[3];
ret->dataType = row[4];
ret->cellType = row[5];
ret->vars = row[6];
}

struct encodeExp *encodeExpLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all encodeExp from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with encodeExpFreeList(). */
{
struct encodeExp *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = encodeExpLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void encodeExpSaveToDb(struct sqlConnection *conn, struct encodeExp *el, char *tableName, int updateSize)
/* Save encodeExp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use encodeExpSaveToDbEscaped() */
{
struct dyString *update = newDyString(updateSize);
dyStringPrintf(update, "insert into %s values ( %d,'%s','%s','%s','%s','%s','%s')", 
	tableName,  el->ix,  el->organism,  el->accession,  el->lab,  el->dataType,  el->cellType,  el->vars);
sqlUpdate(conn, update->string);
freeDyString(&update);
}

void encodeExpSaveToDbEscaped(struct sqlConnection *conn, struct encodeExp *el, char *tableName, int updateSize)
/* Save encodeExp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than encodeExpSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 
{
struct dyString *update = newDyString(updateSize);
char  *organism, *accession, *lab, *dataType, *cellType, *vars;
organism = sqlEscapeString(el->organism);
accession = sqlEscapeString(el->accession);
lab = sqlEscapeString(el->lab);
dataType = sqlEscapeString(el->dataType);
cellType = sqlEscapeString(el->cellType);
vars = sqlEscapeString(el->vars);

dyStringPrintf(update, "insert into %s values ( %d,'%s','%s','%s','%s','%s','%s')", 
	tableName,  el->ix,  organism,  accession,  lab,  dataType,  cellType,  vars);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&organism);
freez(&accession);
freez(&lab);
freez(&dataType);
freez(&cellType);
freez(&vars);
}

struct encodeExp *encodeExpLoad(char **row)
/* Load a encodeExp from row fetched with select * from encodeExp
 * from database.  Dispose of this with encodeExpFree(). */
{
struct encodeExp *ret;

AllocVar(ret);
ret->ix = sqlSigned(row[0]);
ret->organism = cloneString(row[1]);
ret->accession = cloneString(row[2]);
ret->lab = cloneString(row[3]);
ret->dataType = cloneString(row[4]);
ret->cellType = cloneString(row[5]);
ret->vars = cloneString(row[6]);
return ret;
}

struct encodeExp *encodeExpLoadAll(char *fileName) 
/* Load all encodeExp from a whitespace-separated file.
 * Dispose of this with encodeExpFreeList(). */
{
struct encodeExp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = encodeExpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeExp *encodeExpLoadAllByChar(char *fileName, char chopper) 
/* Load all encodeExp from a chopper separated file.
 * Dispose of this with encodeExpFreeList(). */
{
struct encodeExp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodeExpLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodeExp *encodeExpCommaIn(char **pS, struct encodeExp *ret)
/* Create a encodeExp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodeExp */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->ix = sqlSignedComma(&s);
ret->organism = sqlStringComma(&s);
ret->accession = sqlStringComma(&s);
ret->lab = sqlStringComma(&s);
ret->dataType = sqlStringComma(&s);
ret->cellType = sqlStringComma(&s);
ret->vars = sqlStringComma(&s);
*pS = s;
return ret;
}

void encodeExpFree(struct encodeExp **pEl)
/* Free a single dynamically allocated encodeExp such as created
 * with encodeExpLoad(). */
{
struct encodeExp *el;

if ((el = *pEl) == NULL) return;
freeMem(el->organism);
freeMem(el->accession);
freeMem(el->lab);
freeMem(el->dataType);
freeMem(el->cellType);
freeMem(el->vars);
freez(pEl);
}

void encodeExpFreeList(struct encodeExp **pList)
/* Free a list of dynamically allocated encodeExp's */
{
struct encodeExp *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodeExpFree(&el);
    }
*pList = NULL;
}

void encodeExpOutput(struct encodeExp *el, FILE *f, char sep, char lastSep) 
/* Print out encodeExp.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->ix);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->organism);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->accession);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->lab);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dataType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->cellType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->vars);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

void encodeExpJsonOutput(struct encodeExp *el, FILE *f) 
/* Print out encodeExp in JSON format. */
{
fputc('{',f);
fputc('"',f);
fprintf(f,"ix");
fputc('"',f);
fputc(':',f);
fprintf(f, "%d", el->ix);
fputc(',',f);
fputc('"',f);
fprintf(f,"organism");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->organism);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"accession");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->accession);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"lab");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->lab);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"dataType");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->dataType);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"cellType");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->cellType);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"vars");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->vars);
fputc('"',f);
fputc('}',f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

void encodeExpTableCreate(struct sqlConnection *conn, char *tableName)
/* Create an encodeExp table */
{
static char *sqlCreate =
"CREATE TABLE %s (\n"
"    ix int not null AUTO_INCREMENT,     # auto-increment ID\n"
"    organism varchar(255) not null,     # human | mouse\n"
"    accession varchar(255) not null,    # wgEncodeE[H|M]00000N\n"
"    lab varchar(255) not null,  # lab name from ENCODE cv.ra\n"
"    dataType varchar(255) not null,     # dataType from ENCODE cv.ra\n"
"    cellType varchar(255) not null,     # cellType from ENCODE cv.ra\n"
"    vars text,                          # typeOfTerm=term list of experiment-defining variables\n"
"              #Indices\n"
"    PRIMARY KEY(ix)\n"
")";

struct dyString *dy = newDyString(1024);
dyStringPrintf(dy, sqlCreate, tableName, dyStringContents(dy));
sqlRemakeTable(conn, tableName, dyStringContents(dy));
//dyStringFree(&dy);
}

struct encodeExp *encodeExpFromRa(struct hash *ra)
/* Load an encodeExp from a .ra */
{
struct encodeExp *ret;

AllocVar(ret);
ret->organism = cloneString(hashMustFindVal(ra, "organism"));
ret->lab = cloneString(hashMustFindVal(ra, "lab"));
ret->dataType = cloneString(hashMustFindVal(ra, "dataType"));
ret->cellType = cloneString(hashMustFindVal(ra, "cellType"));

char *vars = hashFindVal(ra, "vars");
ret->vars = (vars != NULL ? cloneString(vars) : NULL);
return ret;
}

struct hash *encodeExpToRa(struct encodeExp *el)
/* Create a .ra from an encodeExp */
{
struct hash *ra = hashNew(0);
hashAdd(ra, "organism", el->organism);
hashAdd(ra, "lab", el->lab);
hashAdd(ra, "dataType", el->dataType);
hashAdd(ra, "cellType", el->cellType);
if (el->vars != NULL)
    hashAdd(ra, "vars", el->vars);
return ra;
}

static char *encodeExpAccession(struct encodeExp *el)
/* Make accession string from prefix + organism + id */
{
char org = 'H';
#define BUF_SIZE 32
char accession[BUF_SIZE];
if (sameString(el->organism, "human"))
    org = 'H';
else if (sameString(el->organism, "mouse"))
    org = 'M';
else
    errAbort("Invalid organism %s", el->organism);
safef(accession, BUF_SIZE, "%s%c%06d", ENCODE_EXP_ACC_PREFIX, org, el->ix);
return cloneString(accession);
}

void encodeExpSave(struct sqlConnection *conn, struct encodeExp *el, char *tableName)
/* Save encodeExp as a row to the table specified by tableName. Update accession using
 * index assigned with autoincrement */
{
struct dyString *query;
char *accession;

encodeExpSaveToDb(conn, el, tableName, 2048);
query = newDyString(1024);
dyStringPrintf(query, "select max(ix) from %s", tableName);
el->ix = sqlQuickNum(conn, query->string);
freeDyString(&query);
query = newDyString(1024);
accession = encodeExpAccession(el);
dyStringPrintf(query, "update %s set accession=\'%s\' where ix=%d", 
                        tableName, accession, el->ix);
sqlUpdate(conn, query->string);
freeDyString(&query);
}
