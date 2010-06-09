/* Make a track ixIxx file for the given to be used in the track search tool. */

#include "common.h"
#include "options.h"
#include "hdb.h"
#include "grp.h"
#include "hui.h"
#include "ra.h"
#include "jsHelper.h"

static char const rcsid[] = "$Id: makeTrackIndex.c,v 1.2 2010/06/09 22:26:24 larrym Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "makeTrackIndex - make a ixIxx input file for the tracks in given assembly.\n"
  "usage:\n"
  "   makeTrackIndex database\n\n");
}

static void printTrack(char *database, struct sqlConnection *conn, struct trackDb *tdb, char *html, 
                       struct hash *grpHash, struct hash *cvHash)
{
char query[256];
struct sqlResult *sr = NULL;
char **row = NULL;
char *metadataList[] = {"longLabel", NULL}; // include a fixed set of metadata (i.e. not stuff like subGroup).
int i;
char *grpLabel, *tmp;
boolean metaDbExists = sqlTableExists(conn, "metaDb");

printf("%s ", tdb->track);
if(metaDbExists)
    {
    safef(query, sizeof(query), "select val from metaDb where obj = '%s'", tdb->track);
    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
        {
        printf("%s ", row[0]);
        }
    sqlFreeResult(&sr);
    }

// We include the long descriptions for these terms, b/c we assume people will search for the long labels.
// Jim want's cv data after short/long label

if(metaDbExists && cvHash)
    {
    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
        {
        struct hash *hash = hashFindVal(cvHash, row[0]);
        if(hash != NULL)
            {
            char *str = hashFindVal(hash, "description");
            if(str)
                printf("%s ", str);
            }
        }
    sqlFreeResult(&sr);
    }

for(i = 0; metadataList[i] != NULL; i++)
    {
    char *str = trackDbLocalSetting(tdb, metadataList[i]);
    if(str != NULL)
        printf("%s ", str);
    }

grpLabel = tdb->grp;
tmp = hashFindVal(grpHash, grpLabel);
if(tmp != NULL)
    grpLabel = tmp;
printf("%s ", grpLabel);


if(html == NULL)
    {
    html = tdb->html;
    if(html != NULL)
        // strip out html markup
        html = stripRegEx(html, "<[^>]*>", REG_ICASE);
    }
if(html != NULL)
    {
    char *tmp = cloneString(html);
    char *val = nextWord(&tmp);
    while(val != NULL)
        {
        printf("%s ", val);
        val = nextWord(&tmp);
        }
    }
printf("\n");

// if(tdbIsComposite(tdb) && tdb->subtracks != NULL)
if(tdbIsSuper(tdb))
    hTrackDbLoadSuper(database, tdb);
if(tdb->subtracks != NULL)
    {
    struct trackDb *subtrack;
    for (subtrack = tdb->subtracks; subtrack != NULL; subtrack = subtrack->next)
        // We have decided NOT to populate html down to children.
        printTrack(database, conn, subtrack, NULL, grpHash, cvHash);
    }
}

int main(int argc, char *argv[])
{
char *database;
struct trackDb *tdb, *tdbList = NULL;
struct hash *grpHash = newHash(0);
struct grp *grp, *grps;
char filePath[PATH_LEN];
struct hash *cvHash;

if (argc < 2)
    usage();
database = argv[1];
tdbList = hTrackDb(database, NULL);
struct sqlConnection *conn = hAllocConn(database);

grps = hLoadGrps(database);
for (grp = grps; grp != NULL; grp = grp->next)
    hashAdd(grpHash, grp->name, grp->label);

safef(filePath, sizeof(filePath), "/usr/local/apache/cgi-bin/encode/cv.ra");
if(!fileExists(filePath))
    errAbort("Error: can't locate cv.ra; %s doesn't exist\n", filePath);
cvHash = raReadAll(filePath, "term");

for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    printTrack(database, conn, tdb, NULL, grpHash, cvHash);

return 0;
}
