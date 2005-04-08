/* ccdsCommon - common functions used by CCDS programs */
#include "common.h"
#include "ccdsCommon.h"
#include "jksql.h"

void ccdsGetTblFileNames(char *tblFile, char *table, char *tabFile)
/* Get table and tab file name from a single input name.  If specified names
 * looks like a file name, use it as-is, otherwise generate a file name.
 * Output buffers should be PATH_LEN bytes long.  Either output argument maybe
 * be NULL. */
{
if ((strchr(tblFile, '/') != NULL) || (strchr(tblFile, '.') != NULL))
    {
    if (tabFile != NULL)
        safef(tabFile, PATH_LEN, "%s", tblFile);
    if (table != NULL)
        splitPath(tblFile, NULL, table, NULL);
    }
else
    {
    if (tabFile != NULL)
        safef(tabFile, PATH_LEN, "%s.tab", tblFile);
    if (table != NULL)
        safef(table, PATH_LEN, "%s", tblFile);
    }
}

void ccdsRenameTable(struct sqlConnection *conn, char *oldTable, char *newTable)
/* rename a database table */
{
char query[2048];
safef(query, sizeof(query), "RENAME TABLE %s TO %s", oldTable, newTable);
sqlDropTable(conn, newTable);
sqlUpdate(conn, query);
}

