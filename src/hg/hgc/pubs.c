/* pubs.c - display details of publiations literature track (pubsxxx tables) */

#include "common.h"
#include "jksql.h"
#include "hdb.h"
#include "hgc.h"
#include "hgColors.h"
#include "trackDb.h"
#include "web.h"
#include "hash.h"
#include "obscure.h"
//include "hgTrackUi.h"

// cgi var to activate debug output
static int pubsDebug = 0;

// internal section types in mysql table
static char* pubsSecNames[] ={
      "header", "abstract",
      "intro", "methods",
      "results", "discussion",
      "conclusions", "ack",
      "refs", "unknown" };
//
// whether a checkbox is checked by default, have to correspond to pubsSecNames
static int pubsSecChecked[] ={
      1, 1,
      1, 1,
      1, 1,
      1, 0,
      0, 1 };

static char* pubsSequenceTable;
static char* pubsArticleTable;

static char* makeSqlMarkerList(void)
/* return list of sections from cgi vars, format like "'abstract','header'" */
{
int secCount = sizeof(pubsSecNames)/sizeof(char *);
struct slName* names = NULL;
int i;
for (i=0; i<secCount; i++) 
{
    // add ' around name and add to list
    char* secName = pubsSecNames[i];
    if (cgiOptionalInt(secName, pubsSecChecked[i]))
    {
        char nameBuf[100];
        safef(nameBuf, sizeof(nameBuf), "'%s'", secName);
        slAddHead(&names, slNameNew(nameBuf));
    }
}

if (names==0)
    errAbort("You need to specify at least one article section.");

char* nameListString = slNameListToString(names, ',');
slNameFree(names);
return nameListString;
}


static struct sqlResult* queryMarkerRows(struct sqlConnection* conn, char* markerTable, \
    char* articleTable, char* item, int itemLimit, char* sectionList)
/* query marker rows from mysql, based on http parameters  */
{
char query[4000];
/* Mysql specific setting to make the group_concat function return longer strings */
sqlUpdate(conn, "SET SESSION group_concat_max_len = 100000");

safef(query, sizeof(query), "SELECT distinct %s.articleId, url, title, authors, citation," 
    "group_concat(snippet, section SEPARATOR ' (...) ') FROM %s "
    "JOIN %s USING (articleId) "
    "WHERE markerId='%s' AND section in (%s) "
    "GROUP by articleId "
    "ORDER BY year DESC "
    "LIMIT %d",
    markerTable, markerTable, articleTable, item, sectionList, itemLimit);

if (pubsDebug)
    printf("%s", query);

struct sqlResult *sr = sqlGetResult(conn, query);

return sr;
}


static void printSectionCheckboxes()
/* show a little form with checkboxes where user can select sections they want to show */
{
// labels to show to user, have to correspond to pubsSecNames
char *secLabels[] ={
      "Title", "Abstract",
      "Introduction", "Methods",
      "Results", "Discussion",
      "Conclusions", "Acknowledgements",
      "References", "Not determined" };

int labelCount = sizeof(secLabels)/sizeof(char *);

int i;
printf("<P>\n");
printf("<B>Sections of article shown:</B><BR>\n");
printf("<FORM ACTION=\"hgc?%s&o=%s&t=%s&g=%s&i=%s\" METHOD=\"get\">\n",
    cartSidUrlString(cart), cgiString("o"), cgiString("t"), cgiString("g"), cgiString("i"));

for (i=0; i<labelCount; i++) 
{
    char* name = pubsSecNames[i];
    // checkboxes default to 0 unless checked, see 
    // http://stackoverflow.com/questions/2520952/how-come-checkbox-state-is-not-always-passed-along-to-php-script
    printf("<INPUT TYPE=\"hidden\" name=\"%s\" value=\"0\" />\n", pubsSecNames[i]);
    printf("<INPUT TYPE=\"checkbox\" name=\"%s\" ", name);

    int isChecked = cgiOptionalInt(name, pubsSecChecked[i]);
    if (isChecked)
        printf("value=\"1\" checked=\"yes\">%s</INPUT>\n", secLabels[i]);
    else
        printf("value=\"1\">%s</INPUT>\n", secLabels[i]);
}

printf("<INPUT TYPE=\"hidden\" name=\"o\" value=\"%s\" />\n", cgiString("o"));
printf("<INPUT TYPE=\"hidden\" name=\"g\" value=\"%s\" />\n", cgiString("g"));
printf("<INPUT TYPE=\"hidden\" name=\"t\" value=\"%s\" />\n", cgiString("t"));
printf("<INPUT TYPE=\"hidden\" name=\"i\" value=\"%s\" />\n", cgiString("i"));
printf("<INPUT TYPE=\"hidden\" name=\"hgsid\" value=\"%d\" />\n", cart->sessionId);
printf("<BR>");
printf("<INPUT TYPE=\"submit\" VALUE=\"Submit\" />\n");
printf("</FORM><P>\n");
}

static void printLimitWarning(struct sqlConnection *conn, char* markerTable, 
    char* item, int itemLimit, char* sectionList)
{
char query[4000];
safef(query, sizeof(query), "SELECT COUNT(*) from %s WHERE markerId='%s' AND section in (%s) ", markerTable, item, sectionList);
if (sqlNeedQuickNum(conn, query) > itemLimit) 
{
    printf("<b>This marker is mentioned more than %d times</b><BR>\n", itemLimit);
    printf("The results would take too long to load in your browser and are "
    "therefore limited to %d articles.<P>\n", itemLimit);
}
}

static void printMarkerSnippets(struct sqlConnection *conn, char* articleTable, char* markerTable, char* item)
{

/* do not show more snippets than this limit */
int itemLimit=1000;

printSectionCheckboxes();
char* sectionList = makeSqlMarkerList();
printLimitWarning(conn, markerTable, item, itemLimit, sectionList);

printf("<H3>Snippets from Publications:</H3>");
struct sqlResult* sr = queryMarkerRows(conn, markerTable, articleTable, item, itemLimit, sectionList);

char **row;
while ((row = sqlNextRow(sr)) != NULL)
{
    char* articleId = row[0];
    char* url       = row[1];
    char* title     = row[2];
    char* authors   = row[3];
    char* citation  = row[4];
    char* snippets  = row[5];
    char* addParam  = "";
    if (strstrNoCase(url, "sciencedirect.com"))
        addParam = "?svAppaddApp=298535"; // add the "UCSC matches" sciverse application to article view
    printf("<A HREF=\"%s%s\">%s</A> ", url, addParam, title);
    printf("<SMALL>%s</SMALL>; ", authors);
    printf("<SMALL>%s</SMALL><BR>", citation);
    if (pubsDebug)
        printf("articleId=%s", articleId);
    printf("<I>%s</I><P>", snippets);
    printf("<HR>");
}

freeMem(sectionList);
sqlFreeResult(&sr);
}

static char* printArticleInfo(struct sqlConnection *conn, char* item)
/* Header with information about paper, return documentId */
{
    char query[512];

    safef(query, sizeof(query), "SELECT articleId, url, title, authors, citation, abstract, pmid FROM %s WHERE displayId='%s'", pubsArticleTable, item);

    struct sqlResult *sr = sqlGetResult(conn, query);
    char **row;
    char *articleId=NULL;
    if ((row = sqlNextRow(sr)) != NULL)
    {
        char* cit = row[4];
        char* abstract = row[5];
        char* pmid = row[6];

        if (strlen(abstract)==0) 
            {
                abstract = "(No abstract found for this article. Please use the link to the fulltext above.)";
            }
        articleId = cloneString(row[0]);
        printf("<P>%s</P>\n", row[3]);
        printf("<A TARGET=\"_blank\" HREF=\"%s\"><B>%s</B></A>\n", row[1], row[2]);
        printf("<P style=\"width:800px; font-size:80%%\">%s", cit);
        if (nullIfAllSpace(pmid)!=NULL)
            printf(", <A HREF=\"http://www.ncbi.nlm.nih.gov/pubmed/%s\">PMID%s</A>\n", pmid, pmid);
        printf("</P>\n");
        printf("<P style=\"width:800px; font-size:100%%\">%s</P>\n", abstract);
	}
    sqlFreeResult(&sr);
    return articleId;
}

static struct hash* getSeqIdHash(struct sqlConnection* conn, char* trackTable, \
    char* articleId, char *item, char* seqName, int start)
/* return a hash with the sequence IDs for a given chain of BLAT matches */
{
    char query[512];
    /* check first if the column exists (some debugging tables on hgwdev don't have seqIds) */
    safef(query, sizeof(query), "SHOW COLUMNS FROM %s LIKE 'seqIds';", trackTable);
    char* seqIdPresent = sqlQuickString(conn, query);
    if (!seqIdPresent) {
        return NULL;
    }

    /* get sequence-Ids for feature that was clicked (item&startPos are unique) and return as hash*/
    safef(query, sizeof(query), "SELECT seqIds,'' FROM %s WHERE name='%s' "
        "and chrom='%s' and chromStart=%d", trackTable, item, seqName, start);
    if (pubsDebug)
        puts(query);
    
    // split comma-sep list into parts
    char* seqIdCoordString = sqlQuickString(conn, query);
    char* seqIdCoords[1024];
    int partCount = chopString(seqIdCoordString, ",", seqIdCoords, ArraySize(seqIdCoords));
    int i;

    // format of part <fileId:3><annotId:5>:<start>-<end>, e.g. 12312345:0-23
    struct hash *seqIdHash = NULL;
    seqIdHash = newHash(0);
    for (i=0; i<partCount; i++) 
    {
        char* annotIdHitCoord[1024];
        chopString(seqIdCoords[i], ":", annotIdHitCoord, ArraySize(annotIdHitCoord));
        if (pubsDebug)
            printf("%s, %s<br>", annotIdHitCoord[0], annotIdHitCoord[1]);

        // put articleId and annotId together to create annotation(=sequence) Id
        char fullAnnotId[4096];
        safef(fullAnnotId, sizeof(fullAnnotId), "%s%s", articleId, annotIdHitCoord[0]);
        hashAdd(seqIdHash, fullAnnotId, annotIdHitCoord[1]);
    }
    freeMem(seqIdCoordString);
    return seqIdHash;
}

static void printSeqHeaders(bool showDesc, bool isClickedSection) 
{
    printf("<TABLE style=\"background-color: #%s\" WIDTH=\"100%%\" CELLPADDING=\"2\">\n", HG_COL_BORDER);
    printf("<TR style=\"background-color: #%s; color: #FFFFFF\">\n", HG_COL_TABLE_LABEL);
    if (showDesc)
        puts("  <TH style=\"width: 10%\">Article file</TH>\n");
    puts("  <TH style=\"width: 70%\">One table row per sequence, with flanking text, sequence in bold</TH>\n");
    if (pubsDebug)
        puts("  <TH style=\"width: 30%\">Identifiers</TH>\n");

    if (!isClickedSection && !pubsDebug)
        puts("  <TH style=\"width: 20%\">Feature that includes this match</TH>\n");
    puts("</TR>\n");
}

static void printAddWbr(char* text, int distance) 
/* a crazy hack for firefox/mozilla that is unable to break long words in tables
 * We need to add a <wbr> tag every x characters in the text to make text breakable.
 */
{
int i;
i = 0;
char* c;
c = text;
bool doNotBreak = FALSE;
while (*c != 0){
    {
    if (*c=='&')
       doNotBreak = TRUE;
    if (*c==';')
       doNotBreak = FALSE;

    if (i % distance == 0 && ! doNotBreak) 
        printf("<wbr>");
    printf("%c", *c);
    c++;
    i++;
    }
}
}

static bool printSeqSection(char* articleId, char* title, bool showDesc, struct sqlConnection* conn, struct hash* clickedSeqs, bool isClickedSection, bool fasta)
/* print a table of sequences, show only sequences with IDs in hash,
 * There are two sections, respective sequences are shown depending on isClickedSection and clickedSeqs 
 *   - seqs that were clicked on (isClickedSection=True) -> show only seqs in clickedSeqs
 *   - other seqs (isClickedSection=False) -> show all other seqs
 * 
 * */
{
    // get data from mysql
    char query[4096];
    safef(query, sizeof(query), 
    "SELECT fileDesc, snippet, locations, articleId, fileId, seqId, sequence "
    "FROM %s WHERE articleId='%s';", pubsSequenceTable, articleId);
    if (pubsDebug)
        puts(query);
    struct sqlResult *sr = sqlGetResult(conn, query);

    // construct title for section
    char fullTitle[5000];
    safef(fullTitle, sizeof(fullTitle), 
    "%s&nbsp;<A HREF=\"../cgi-bin/hgc?%s&o=%s&t=%s&g=%s&i=%s&fasta=%d\"><SMALL>(switch fasta format)</SMALL></A>", 
    title, cartSidUrlString(cart), cgiString("o"), cgiString("t"), cgiString("g"), cgiString("i"), 
    !fasta);

    webNewSection(fullTitle);

    if (!fasta) 
        printSeqHeaders(showDesc, isClickedSection);

    char **row;
    bool foundSkippedRows = FALSE;
    while ((row = sqlNextRow(sr)) != NULL)
    {
        char* fileDesc = row[0];
        char* snippet  = row[1];
        char* locList  = row[2];
        char* artId    = row[3];
        char* fileId   = row[4];
        char* seqId    = row[5];
        char* seq      = row[6];

        // annotation (=sequence) ID is a 64 bit int with 10 digits for 
        // article, 3 digits for file, 5 for annotation
        char annotId[100];
        safef(annotId, 100, "%010d%03d%05d", atoi(artId), atoi(fileId), atoi(seqId));

        // only display this sequence if we're in the right section
        if (clickedSeqs!=NULL && ((hashLookup(clickedSeqs, annotId)!=NULL) != isClickedSection)) {
            foundSkippedRows = TRUE;
            continue;
        }

        if (fasta)
        {
            printf("<TT>>%s<BR>%s<BR></TT>", annotId, seq);
        }
        else
        {
            printf("<TR style=\"background-color: #%s\">\n", HG_COL_LOCAL_TABLE);
            if (showDesc)
                printf("<TD style=\"word-break:break-all\">%s\n", fileDesc);
            //printf("<TD><I>%s</I></TD>\n", snippet); 
            printf("<TD style=\"word-break:break-all;\"><I>");
            printAddWbr(snippet, 40);
            printf("</I></TD>\n"); 
            if (pubsDebug) 
            {
                printf("<TD>article %s, file %s, seq %s, annotId %s", artId, fileId, seqId, annotId);
            }

            // print links to locations 
            if (!isClickedSection && !pubsDebug) {
                struct slName *locs;
                // format: hg19/chr1:300-400,mm9/chr1:60006-23234
                // split on "," then split on "/"
                locs = charSepToSlNames(locList, ',');
                printf("<TD>");
                if (locs==NULL)
                    printf("No matches");
                for ( ; locs!=NULL; locs = locs->next) 
                {
                    char* locString = locs->name;
                    char* parts[2];
                    int partCount;
                    partCount = chopString(locString, "/", parts, ArraySize(parts));
                    assert(partCount==2);
                    char* db = parts[0];
                    char* pos = parts[1];
                    printf("<A HREF=\"../cgi-bin/hgTracks?%s&amp;db=%s&amp;position=%s\">%s (%s)</A>", cartSidUrlString(cart), db, pos, pos, db);
                    printf("<BR>");
                }
                printf("</TD>\n");
            }
        printf("</TR>\n");
        }
	}
    printf("</TR>\n");
    webEndSectionTables();
    sqlFreeResult(&sr);
    return foundSkippedRows;
}

static void printSeqInfo(struct sqlConnection* conn, char* trackTable,
    char* articleId, char* item, char* seqName, int start, bool fileDesc, bool fasta)
    /* print sequences, split into two sections 
     * two sections: one for sequences that were clicked, one for all others*/
{
    struct hash* clickedSeqs = getSeqIdHash(conn, trackTable, articleId, item, seqName, start);

    bool skippedRows;
    if (clickedSeqs) 
        skippedRows = printSeqSection(articleId, "Sequences used to construct this feature", \
            fileDesc, conn, clickedSeqs, 1, fasta);
    else 
        skippedRows=1;

    if (skippedRows)
        printSeqSection(articleId, "Other Sequences in this article", \
            fileDesc, conn, clickedSeqs, 0, fasta);
    printf("<P><SMALL>Copyright 2011 Elsevier B.V. All rights reserved.</SMALL><P>");
    freeHash(&clickedSeqs);

}

static void printTrackVersion(struct trackDb *tdb, struct sqlConnection* conn, char* item) 
{
    char versionString[256];
    char dateReference[256];
    char headerTitle[512];
    /* see if hgFixed.trackVersion exists */
    boolean trackVersionExists = hTableExists("hgFixed", "trackVersion");

    if (trackVersionExists)
        {
        char query[256];
        safef(query, sizeof(query), \
        "SELECT version,dateReference FROM hgFixed.trackVersion "
        "WHERE db = '%s' AND name = 'pubs' ORDER BY updateTime DESC limit 1", database);
        struct sqlResult *sr = sqlGetResult(conn, query);
        char **row;

        /* in case of NULL result from the table */
        versionString[0] = 0;
        while ((row = sqlNextRow(sr)) != NULL)
        {
        safef(versionString, sizeof(versionString), "version %s",
            row[0]);
        safef(dateReference, sizeof(dateReference), "%s",
            row[1]);
        }
        sqlFreeResult(&sr);
        }
    else
        {
        versionString[0] = 0;
        dateReference[0] = 0;
        }

    if (versionString[0])
        safef(headerTitle, sizeof(headerTitle), "%s - %s", item, versionString);
    else
        safef(headerTitle, sizeof(headerTitle), "%s", item);

    genericHeader(tdb, headerTitle);
}

static void printPositionAndSize(int start, int end, bool showSize)
{
    printf("<B>Position:</B>&nbsp;"
               "<A HREF=\"%s&amp;db=%s&amp;position=%s%%3A%d-%d\">",
                      hgTracksPathAndSettings(), database, seqName, start+1, end);
    char startBuf[64], endBuf[64];
    sprintLongWithCommas(startBuf, start + 1);
    sprintLongWithCommas(endBuf, end);
    printf("%s:%s-%s</A><BR>\n", seqName, startBuf, endBuf);
    long size = end - start;
    sprintLongWithCommas(startBuf, size);
    if (showSize)
        printf("<B>Genomic Size:</B>&nbsp;%s<BR>\n", startBuf);
}

void doPubsDetails(struct trackDb *tdb, char *item)
/* publications custom display */
{

int start = cgiInt("o");
int end = cgiInt("t");
char* trackTable = cgiString("g");
int fasta = cgiOptionalInt("fasta", 0);

pubsDebug = cgiOptionalInt("debug", 0);

struct sqlConnection *conn = hAllocConn(database);
printTrackVersion(tdb, conn, item);

if (startsWith("pubsMarker", trackTable)) 
{
    char* markerTable = hashMustFindVal(tdb->settingsHash, "pubsMarkerTable");
    char* articleTable = hashMustFindVal(tdb->settingsHash, "pubsArticleTable");
    printPositionAndSize(start, end, 0);
    printMarkerSnippets(conn, articleTable, markerTable, item);
}
else 
{
    printPositionAndSize(start, end, 1);
    pubsSequenceTable = hashMustFindVal(tdb->settingsHash, "pubsSequenceTable");
    pubsArticleTable = hashMustFindVal(tdb->settingsHash, "pubsArticleTable");

    char* articleId = printArticleInfo(conn, item);
    if (articleId!=NULL) 
    {
        bool showDesc; 
        showDesc = (! endsWith(trackTable, "Elsevier")); 
        // avoid clutter: Elsevier has only main text
        printSeqInfo(conn, trackTable, articleId, item, seqName, start, showDesc, fasta);
    }
}

printTrackHtml(tdb);
hFreeConn(&conn);
}
