/* docIdMetaShow - cgi to show metadata from docId table. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"
#include "htmshell.h"
#include "web.h"
#include "cheapcgi.h"
#include "cart.h"
#include "hui.h"
#include "docId.h"
#include "mdb.h"

static char const rcsid[] = "$Id: newProg.c,v 1.30 2010/03/24 21:18:33 hiram Exp $";

/* Global Variables */
struct cart *cart;             /* CGI and other variables */
struct hash *oldVars = NULL;
char *database = "encpipeline_prod";
extern char *docIdTable;
//char *docIdDir = "/hive/groups/encode/dcc/pipeline/downloads/docId";
char *docIdDir = "http://hgdownload-test.cse.ucsc.edu/goldenPath/docId";

void doMiddle(struct cart *theCart)
/* Set up globals and make web page */
{
cart = theCart;
cartWebStart(cart, database, "ENCODE DCC Submissions");
struct sqlConnection *conn = sqlConnect(database);
struct docIdSub *docIdSub;
char query[10 * 1024];
struct sqlResult *sr;
char **row;
printf("<table>");
printf("<tr><th>File</th>");
printf("<th>dataType</th>");
printf("<th>view</th>");
printf("<th>fileType</th>");
printf("<th>cell type</th>");
printf("<th>lab</th>");
printf("</tr>\n");
safef(query, sizeof query, "select * from %s", docIdTable);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    docIdSub = docIdSubLoad(row);
    verbose(2, "ix %d\n", docIdSub->ix);
    verbose(2, "submitDate %s\n", docIdSub->submitDate);
    verbose(2, "md5sum %s\n", docIdSub->md5sum);
    verbose(2, "valReport %s\n", docIdSub->valReport);
    verbose(2, "metaData %s\n", docIdSub->metaData);
    verbose(2, "submitPath %s\n", docIdSub->submitPath);
    verbose(2, "submitter %s\n", docIdSub->submitter);

    cgiDecode(docIdSub->metaData, docIdSub->metaData, strlen(docIdSub->metaData));
    char *tempFile = "../trash/temp";
    FILE *f = mustOpen(tempFile, "w");
    fwrite(docIdSub->metaData, strlen(docIdSub->metaData), 1, f);
    fclose(f);
    boolean validated;
    struct mdbObj *mdbObj = mdbObjsLoadFromFormattedFile(tempFile, &validated);
    char *docIdType = mdbObjFindValue(mdbObj, "type");
    char buffer[10 * 1024];
    safef(buffer, sizeof buffer, "%d", docIdSub->ix);
        printf("<tr><td><a href=%s> %s</a></td>", docIdGetPath(buffer, docIdDir, docIdType) , docIdDecorate(docIdSub->ix));
    printf("<td>%s</td> <td>%s</td> <td>%s</td> <td>%s</td> <td>%s</td>\n",   mdbObjFindValue(mdbObj, "dataType"), mdbObjFindValue(mdbObj, "view"),mdbObjFindValue(mdbObj, "type"), mdbObjFindValue(mdbObj, "cell"), mdbObjFindValue(mdbObj, "lab"));
    printf("</tr>\n");
    }

printf("</table>");
sqlFreeResult(&sr);
sqlDisconnect(&conn);
cartWebEnd();
}

/* Null terminated list of CGI Variables we don't want to save
 * permanently. */
char *excludeVars[] = {"Submit", "submit", NULL,};

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
cartEmptyShell(doMiddle, hUserCookie(), excludeVars, oldVars);
return 0;
}
