#include "common.h"
#include "gbSql.h"
#include "jksql.h"
#include "psl.h"
#include "dystring.h"

void gbSqlDupTableDef(struct sqlConnection *conn, char* table,
                      char* newTable)
/* create a duplicate of a table with the right indices. */
{
char query[2048], *createArgs;
struct sqlResult* sr;
char** row;

/* Returns two columns of table name and command */
safef(query, sizeof(query), "SHOW CREATE TABLE %s", table);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    if (sameString(row[0], table))
        break;
    }
if (row == NULL)
    errAbort("can't get create table command for: %s", table);
createArgs = strchr(row[1], '(');
if (createArgs == NULL)
    errAbort("don't understand returned create table command: %s", row[1]);
safef(query, sizeof(query), "CREATE TABLE %s %s", newTable, createArgs);
sqlFreeResult(&sr);
sqlUpdate(conn, query);
}

void gbAddTableIfExists(struct sqlConnection *conn, char* table,
                        struct slName** tableList)
{
/* Add a table to a list, if it exists */
if (sqlTableExists(conn, table))
    slSafeAddHead(tableList, slNameNew(table));
}

unsigned gbSqlUnsignedNull(char *s)
/* parse an unsigned, allowing empty string to return zero */
{
if (s[0] == '\0')
    return 0;
else
    return sqlUnsigned(s);
}

int gbSqlSignedNull(char *s)
/* parse an int, allowing empty string to return zero */
{
if (s[0] == '\0')
    return 0;
else
    return sqlSigned(s);
}

struct slName* gbSqlListTablesLike(struct sqlConnection *conn, char *like)
/* get list of tables matching a pattern */
{
char query[256];
safef(query, sizeof(query), "show tables like '%s'", like);
return sqlQuickList(conn, query);
}

/* mysql lock suffix, database if prefixed, just in case we want multiple
 * gbLoadRnas on different databases*/
static char *GB_LOCK_NAME = "genbank";

void gbLockDb(struct sqlConnection *conn, char *db)
/* get an advisory lock to keep two genbank process from updating
 * the database at the same time.  If db is null, use the database
 * associated with the connection. */
{
char query[128];
int got;
if (db == NULL)
    db = sqlGetDatabase(conn);
safef(query, sizeof(query), "SELECT GET_LOCK(\"%s.%s\", 0)", db, GB_LOCK_NAME);
got = sqlNeedQuickNum(conn, query);
if (!got)
    errAbort("failed to get lock %s.%s", db, GB_LOCK_NAME);
}

void gbUnlockDb(struct sqlConnection *conn, char *db)
/* free genbank advisory lock on database */
{
char query[128];
if (db == NULL)
    db = sqlGetDatabase(conn);
safef(query, sizeof(query), "SELECT RELEASE_LOCK(\"%s.%s\")", db, GB_LOCK_NAME);
sqlUpdate(conn, query);
}

char *gbSqlStrOrNullTabVar(char *str)
/* If str is null, return \N for loading tab file, otherwise str */
{
return (str == NULL) ? "\\N" : str;
}

void tblBldGetTmpName(char *tmpTable, int tmpBufSize, char *table)
/* generate the temporary table name */
{
safef(tmpTable, tmpBufSize, "%s_tmp", table);
}

void tblBldDrop(struct sqlConnection *conn, char *table, unsigned selFlags)
/* Drop a tables based on the set of select flags: TBLBLD_REAL_TABLE,
 * TBLBLD_TMP_TABLE, TBLBLD_OLD_TABLE */
{
char table2[64];
if (selFlags & TBLBLD_REAL_TABLE)
    sqlDropTable(conn, table);
if (selFlags & TBLBLD_TMP_TABLE)
    {
    safef(table2, sizeof(table2), "%s_tmp", table);
    sqlDropTable(conn, table2);
    }
if (selFlags & TBLBLD_OLD_TABLE)
    {
    safef(table2, sizeof(table2), "%s_old", table);
    sqlDropTable(conn, table2);
    }
}

void tblBldDropTables(struct sqlConnection *conn, char **tables, unsigned selFlags)
/* Drop a list of tables based on the set of select flags: TBLBLD_REAL_TABLE,
 * TBLBLD_TMP_TABLE, TBLBLD_OLD_TABLE */
{
int i;
for (i = 0; tables[i] != NULL; i++)
    tblBldDrop(conn, tables[i], selFlags);
}

void tblBldRemakePslTable(struct sqlConnection *conn, char *table, char *insertTable)
/* remake a PSL table based on another PSL that is going to be inserted into
 * it. */
{
/* create with tName index and bin and if the specified table has bin.  This
 * is done so insert from the other table works. */
boolean useBin = (sqlFieldIndex(conn, insertTable, "bin") >= 0);
unsigned options = PSL_TNAMEIX | ((useBin ? PSL_WITH_BIN : 0));
char *sqlCmd = pslGetCreateSql(table, options, 0);
sqlRemakeTable(conn, table, sqlCmd);
freez(&sqlCmd);
}

void tblBldAtomicInstall(struct sqlConnection *conn, char **tables)
/* Install the tables in the NULL terminated in an atomic manner.  Drop
 * under tbl_old first, then renametbl to tbl_old, and tbl_tmp to tbl. */
{
int i;

for (i = 0; tables[i] != NULL; i++)
    tblBldDrop(conn, tables[i], TBLBLD_OLD_TABLE);

struct dyString* sql = dyStringNew(1024);
dyStringAppend(sql, "RENAME TABLE ");
for (i = 0; tables[i] != NULL; i++)
    {
    if (i > 0)
        dyStringAppend(sql, ", ");
    if (sqlTableExists(conn, tables[i]))
        dyStringPrintf(sql, "%s TO %s_old, ", tables[i], tables[i]);
    dyStringPrintf(sql, "%s_tmp TO %s", tables[i], tables[i]);
    }
sqlUpdate(conn, sql->string);
dyStringFree(&sql);
}

/*
 * Local Variables:
 * c-file-style: "jkent-c"
 * End:
 */
