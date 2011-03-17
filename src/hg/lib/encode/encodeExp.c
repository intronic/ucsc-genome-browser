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
ret->factors = row[6];
ret->lastUpdated = row[7];
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
dyStringPrintf(update, "insert into %s values ( %d,'%s','%s','%s','%s','%s','%s','%s')", 
	tableName,  el->ix,  el->organism,  el->accession,  el->lab,  el->dataType,  el->cellType,  el->factors,  el->lastUpdated);
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
char  *organism, *accession, *lab, *dataType, *cellType, *factors, *lastUpdated;
organism = sqlEscapeString(el->organism);
accession = sqlEscapeString(el->accession);
lab = sqlEscapeString(el->lab);
dataType = sqlEscapeString(el->dataType);
cellType = sqlEscapeString(el->cellType);
factors = sqlEscapeString(el->factors);
lastUpdated = sqlEscapeString(el->lastUpdated);

dyStringPrintf(update, "insert into %s values ( %d,'%s','%s','%s','%s','%s','%s','%s')", 
	tableName,  el->ix,  organism,  accession,  lab,  dataType,  cellType,  factors,  lastUpdated);
sqlUpdate(conn, update->string);
freeDyString(&update);
freez(&organism);
freez(&accession);
freez(&lab);
freez(&dataType);
freez(&cellType);
freez(&factors);
freez(&lastUpdated);
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
ret->factors = cloneString(row[6]);
ret->lastUpdated = cloneString(row[7]);
return ret;
}

struct encodeExp *encodeExpLoadAll(char *fileName) 
/* Load all encodeExp from a whitespace-separated file.
 * Dispose of this with encodeExpFreeList(). */
{
struct encodeExp *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

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
char *row[8];

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
ret->factors = sqlStringComma(&s);
ret->lastUpdated = sqlStringComma(&s);
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
freeMem(el->factors);
freeMem(el->lastUpdated);
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
fprintf(f, "%s", el->factors);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->lastUpdated);
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
fprintf(f,"factors");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->factors);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"lastUpdated");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->lastUpdated);
fputc('"',f);
fputc('}',f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#include "hdb.h"
#include "mdb.h"

/* Schema in alternate format with additional properties.
   For each field, there is a 'get' function and an entry in the fields table.
   WARNING:  Must parallel .sql */

/* BEGIN schema-dependent section */

char *encodeExpGetIx(struct encodeExp *exp)
/* Return ix field of encodeExp */
{
char buf[64];
safef(buf, 64, "%d", exp->ix);
return cloneString(buf);
}

char *encodeExpGetOrganism(struct encodeExp *exp)
/* Return organism field of encodeExp */
{
return cloneString(exp->organism);
}

char *encodeExpGetAccession(struct encodeExp *exp)
/* Return accession field of encodeExp */
{
return cloneString(exp->accession);
}

char *encodeExpGetLab(struct encodeExp *exp)
/* Return lab field of encodeExp */
{
return cloneString(exp->lab);
}

char *encodeExpGetDataType(struct encodeExp *exp)
/* Return dataType field of encodeExp */
{
return cloneString(exp->dataType);
}

char *encodeExpGetCellType(struct encodeExp *exp)
/* Return cellType field of encodeExp */
{
return cloneString(exp->cellType);
}

char *encodeExpGetFactors(struct encodeExp *exp)
/* Return factors field of encodeExp */
{
return cloneString(exp->factors);
}

char *encodeExpGetLastUpdated(struct encodeExp *exp)
/* Return lastUpdated field of encodeExp */
{
return cloneString(exp->lastUpdated);
}

typedef char * (*encodeExpGetField)(struct encodeExp *exp);

struct encodeExpField {
    char *name;
    encodeExpGetField get; 
    boolean required;
} encodeExpField;

struct encodeExpField encodeExpFields[] = 
   { {ENCODE_EXP_FIELD_IX, &encodeExpGetIx, TRUE},                  //required, set to 0
     {ENCODE_EXP_FIELD_ORGANISM, &encodeExpGetOrganism, TRUE},      //required
     {ENCODE_EXP_FIELD_ACCESSION, &encodeExpGetAccession, FALSE},
     {ENCODE_EXP_FIELD_LAB, &encodeExpGetLab, TRUE},                 //required
     {ENCODE_EXP_FIELD_DATA_TYPE, &encodeExpGetDataType, TRUE},      //required
     {ENCODE_EXP_FIELD_CELL_TYPE, &encodeExpGetCellType, TRUE},      //required
     {ENCODE_EXP_FIELD_FACTORS, &encodeExpGetFactors, FALSE},
     {ENCODE_EXP_FIELD_LAST_UPDATED, &encodeExpGetLastUpdated, FALSE},
     {NULL, 0, 0} };

static char *sqlCreate =
"CREATE TABLE %s (\n"
"    ix int not null AUTO_INCREMENT,     # auto-increment ID\n"
"    organism varchar(255) not null,     # human | mouse\n"
"    accession varchar(255) not null,    # ENC[H|M]E00000N\n"
"    lab varchar(255) not null,  # lab name from ENCODE cv.ra\n"
"    dataType varchar(255) not null,     # dataType from ENCODE cv.ra\n"
"    cellType varchar(255) not null,     # cellType from ENCODE cv.ra\n"
"    factors text,                       # var=val list of experiment-defining variables\n"
"    lastUpdated timestamp default current_timestamp on update current_timestamp,  # last update date-time"
"              #Indices\n"
"    PRIMARY KEY(ix)\n"
")";

/* END schema-dependent section */

static char *expFactors[] = {
    /* alpha sorted list, includes all exp-defining variables */
    // TODO: -> whitelist from cv.ra
    "antibody",
    "insertLength",
    "localization",
    "protocol",
    "readType",
    "restrictionEnzyme",
    "ripTgtProtein",
    "rnaExtract",
    "treatment",
    0,
};

void encodeExpTableCreate(struct sqlConnection *conn, char *tableName)
/* Create an encodeExp table */
{
struct dyString *dy = newDyString(0);
dyStringPrintf(dy, sqlCreate, tableName, dyStringContents(dy));
sqlRemakeTable(conn, tableName, dyStringContents(dy));
dyStringFree(&dy);
}

struct encodeExp *encodeExpLoadAllFromTable(struct sqlConnection *conn, char *tableName)
/* Load all encodeExp in table */
{
struct encodeExp *exps;

struct dyString *dy = newDyString(0);
dyStringPrintf(dy, "select * from %s", tableName);
exps = encodeExpLoadByQuery(conn, dyStringContents(dy));
dyStringFree(&dy);
return exps;
}

struct encodeExp *encodeExpFromMdb(struct mdbObj *mdb)
/* Create an encodeExp from an ENCODE metaDb object */
{
struct encodeExp *exp;

if (!mdbObjIsEncode(mdb))
    errAbort("Metadata object is not from ENCODE");

AllocVar(exp);

/* extract lab */
exp->lab = mdbObjFindValue(mdb, MDB_FIELD_LAB);
if (exp->lab == NULL)
    errAbort("ENCODE metadata object \'%s\' missing lab\n", mdb->obj);

// TODO: -> lib (stripPiFromLab)
/* Strip off trailing parenthesized PI name if present */
chopSuffixAt(exp->lab, '(');

/* extract data type */
exp->dataType = mdbObjFindValue(mdb, MDB_FIELD_DATA_TYPE);
if (exp->dataType == NULL)
    errAbort("ENCODE metadata object \'%s\' missing dataType\n", mdb->obj);

exp->cellType = mdbObjFindValue(mdb, MDB_FIELD_CELL_TYPE);
if (exp->cellType == NULL)
    {
    exp->cellType = ENCODE_EXP_NO_CELL;
    }

/* experimental factors (variables) */
int i;
char *var, *val;
struct dyString *factors = newDyString(0);
for (i = 0; expFactors[i] != NULL; i++)
    {
    var = expFactors[i];
    val = mdbObjFindValue(mdb, var);
    if (val == NULL || sameString(val, ENCODE_EXP_NO_VAR))
        continue;
    dyStringPrintf(factors, "%s=%s ", var, val);
    }
exp->factors = dyStringCannibalize(&factors);
eraseTrailingSpaces(exp->factors);

return exp;
}

struct encodeExp *encodeExpFromRa(struct hash *ra)
/* Load an encodeExp from a Ra hash. */
{
char *rows[ENCODEEXP_NUM_COLS];
struct encodeExp *exp;
int i;

AllocVar(exp);
for (i = 0; i < ENCODEEXP_NUM_COLS; i++)
    {
    struct encodeExpField *fp = &encodeExpFields[i];
    assert(fp->name != NULL);
    char *val = hashFindVal(ra, fp->name);
    if (val == NULL && fp->required)
        errAbort("Required field \'%s\' not found in .ra", fp->name);
    rows[i] = cloneString(val);
    }
encodeExpStaticLoad(rows, exp);
return exp;
}

struct hash *encodeExpToRaFile(struct encodeExp *exp, FILE *f)
/* Create a Ra hash from an encodeExp.  Print to file if non NULL */
{
struct hash *ra = hashNew(0);
int i;
for (i = 0; i < ENCODEEXP_NUM_COLS; i++)
    {
    struct encodeExpField *fp = &encodeExpFields[i];
    assert(fp->name != NULL);
    char *val = fp->get(exp);
    if (val != NULL)
        {
        hashAdd(ra, fp->name, val);
        if (f != NULL)
            fprintf(f, "%s %s\n", fp->name, val);
        }
    }
fputs("\n", f);
return ra;
}

struct hash *encodeExpToRa(struct encodeExp *exp)
/* Create a Ra hash from an encodeExp */
{
return encodeExpToRaFile(exp, NULL);
}

static char *encodeExpMakeAccession(struct encodeExp *exp)
/* Make accession string from prefix + organism + id */
{
#define BUF_SIZE 64 
char accession[BUF_SIZE];

char org = (char) NULL;
if (sameString(exp->organism, "human"))
    org = 'H';
else if (sameString(exp->organism, "mouse"))
    org = 'M';
else
    errAbort("Invalid organism %s", exp->organism);
safef(accession, BUF_SIZE, "%s%c%06d", ENCODE_EXP_ACC_PREFIX, org, exp->ix);
return cloneString(accession);
}

void encodeExpAdd(struct sqlConnection *conn, char *tableName, struct encodeExp *exp)
/* Add encodeExp as a new row to the table specified by tableName. 
   Update accession using index assigned with autoincrement
*/
{
struct dyString *query = dyStringNew(0);
char *accession = NULL;

sqlGetLock(conn, ENCODE_EXP_TABLE_LOCK);
encodeExpSaveToDb(conn, exp, tableName, 0);

dyStringPrintf(query, "select max(ix) from %s", tableName);
exp->ix = sqlQuickNum(conn, dyStringContents(query));
accession = encodeExpMakeAccession(exp);
freeDyString(&query);

query = newDyString(0);
dyStringPrintf(query, "update %s set accession=\'%s\' where ix=%d", 
                        tableName, accession, exp->ix);
sqlUpdate(conn, dyStringContents(query));
sqlReleaseLock(conn, ENCODE_EXP_TABLE_LOCK);

freez(&accession);
freeDyString(&query);
}

void encodeExpUpdateField(struct sqlConnection *conn, char *tableName, 
                                char *accession, char *field, char *val)
/* Update field in encodeExp identified by accession with value.
   Only supported for a few non-interdependent fields */
{
struct dyString *query = NULL;

if (field != ENCODE_EXP_FIELD_LAB &&
    field != ENCODE_EXP_FIELD_DATA_TYPE &&
    field != ENCODE_EXP_FIELD_CELL_TYPE)
        errAbort("Unsupported encodeExp field update: %s", field);

query = dyStringCreate("update %s set %s=\'%s\' where accession=\'%s\'", 
                        tableName, field, val, accession);
sqlGetLock(conn, ENCODE_EXP_TABLE_LOCK);
sqlUpdate(conn, dyStringContents(query));
sqlReleaseLock(conn, ENCODE_EXP_TABLE_LOCK);
freeDyString(&query);
}

void encodeExpUpdateFactors(struct sqlConnection *conn, char *tableName, 
                                char *accession, struct slPair *factorPairs)
/* Update factors in encodeExp identified by accession */
{
struct dyString *dy = dyStringNew(0);
char *factors;
struct slPair *pair;

slPairSortCase(&factorPairs);
for (pair = factorPairs; pair != NULL; pair = pair->next)
    dyStringPrintf(dy, "%s=%s ", pair->name, (char *)pair->val);
factors = dyStringCannibalize(&dy);
eraseTrailingSpaces(factors);
dy = dyStringCreate("update %s set %s=\'%s\' where accession=\'%s\'", 
                        tableName, ENCODE_EXP_FIELD_FACTORS, factors, accession);
sqlGetLock(conn, ENCODE_EXP_TABLE_LOCK);
sqlUpdate(conn, dyStringContents(dy));
sqlReleaseLock(conn, ENCODE_EXP_TABLE_LOCK);
freeDyString(&dy);
}

char *encodeExpKey(struct encodeExp *exp)
/* Create a hash key from an encodeExp */
{
struct dyString *dy = newDyString(0);
dyStringPrintf(dy, "lab:%s dataType:%s cellType:%s", exp->lab, exp->dataType, exp->cellType);
if (exp->factors != NULL)
    dyStringPrintf(dy, " factors:%s", exp->factors);
return dyStringCannibalize(&dy);
}

struct encodeExp *encodeExpGetFromTable(char *organism, char *lab, char *dataType, 
                                char *cell, struct slPair *factorPairs, char *table)
/* Return experiments matching args in named experiment table.
 * Organism, Lab and DataType must be non-null */
{
char *factors;
struct slPair *pair;
struct encodeExp *exps = NULL;

if (organism == NULL || lab == NULL || dataType == NULL)
    errAbort("Need organism, lab, and dataType to query experiment table");

if (cell == NULL)
    cell = ENCODE_EXP_NO_CELL;

struct sqlConnection *conn = sqlConnect(ENCODE_EXP_DATABASE);

/* construct factor string var=val from pairs */
struct dyString *dy = dyStringNew(0);
for (pair = factorPairs; pair != NULL; pair = pair->next)
    dyStringPrintf(dy, "%s=%s ", pair->name, (char *)pair->val);
factors = dyStringCannibalize(&dy);
eraseTrailingSpaces(factors);

dy = dyStringCreate(
        "select * from %s where %s=\'%s\' and %s=\'%s\' and %s=\'%s\' and %s=\'%s\' and %s=\'%s\'",
                table, 
                ENCODE_EXP_FIELD_ORGANISM, organism, 
                ENCODE_EXP_FIELD_LAB, lab, 
                ENCODE_EXP_FIELD_DATA_TYPE, dataType, 
                ENCODE_EXP_FIELD_CELL_TYPE, cell, 
                ENCODE_EXP_FIELD_FACTORS, factors);
exps = encodeExpLoadByQuery(conn, dyStringCannibalize(&dy));
sqlDisconnect(&conn);
return exps;
}

struct encodeExp *encodeExpGet(char *organism, char *lab, char *dataType, char *cell, 
                                        struct slPair *factorPairs)
/* Return experiments matching args in default experiment table.
 * Organism, Lab and DataType must be non-null */
{
return encodeExpGetFromTable(organism, lab, dataType, cell, factorPairs, ENCODE_EXP_TABLE);
}

struct encodeExp *encodeExpGetByMdbVarsFromTable(char *db, struct mdbVar *vars, char *table)
/* Return experiments by looking up mdb var list from the named experiment table */
{
struct mdbObj *mdb;
int i;
char *var, *val;
struct slPair *factorPairs;

if (db == NULL)
    errAbort("Missing assembly");

// FIXME: centralize treatment of organism/lower-casing
char *organism = hOrganism(db);
strLower(organism);

mdb = mdbObjNew(NULL, vars);

/* extract factor vars into an slPair list */
for (i = 0; expFactors[i] != NULL; i++)
    {
    var = expFactors[i];
    val = mdbObjFindValue(mdb, var);
    if (val == NULL || sameString(val, ENCODE_EXP_NO_VAR))
        continue;
    slPairAdd(&factorPairs, var, val);
    }
slPairSortCase(&factorPairs);
/* TODO: free up mdbObj */
return encodeExpGetFromTable(organism, 
            mdbObjFindValue(mdb, MDB_FIELD_LAB), mdbObjFindValue(mdb, MDB_FIELD_DATA_TYPE),
            mdbObjFindValue(mdb, MDB_FIELD_CELL_TYPE), factorPairs, table);
}

struct encodeExp *encodeExpGetByMdbVars(char *db, struct mdbVar *vars)
/* Return experiments by looking up mdb var list from the default experiment table */
{
return encodeExpGetByMdbVarsFromTable(db, vars, ENCODE_EXP_TABLE);
}

int encodeExpExists(char *db, struct mdbVar *vars)
/* Return TRUE if at least one experiment exists for these vars */
{
struct encodeExp *exp = encodeExpGetByMdbVars(db, vars);
int found = (exp != NULL);
freez(&exp);
return found;
}

char *encodeGetAccessionByMdbVars(char *db, struct mdbVar *vars)
/* Return accession of (first) experiment matching vars, or NULL if not found */
{
struct encodeExp *exp = encodeExpGetByMdbVars(db, vars);
char *acc = encodeExpGetAccession(exp);
freez(&exp);
return acc;
}
