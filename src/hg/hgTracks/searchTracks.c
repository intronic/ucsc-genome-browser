/* Track search code used by hgTracks CGI */

#include "common.h"
#include "searchTracks.h"
#include "hCommon.h"
#include "memalloc.h"
#include "obscure.h"
#include "dystring.h"
#include "hash.h"
#include "cheapcgi.h"
#include "hPrint.h"
#include "htmshell.h"
#include "cart.h"
#include "hgTracks.h"
#include "web.h"
#include "jksql.h"
#include "hdb.h"
#include "mdb.h"
#include "fileUi.h"
#include "trix.h"
#include "jsHelper.h"
#include "imageV2.h"


#define TRACK_SEARCH_FORM        "trackSearch"
#define SEARCH_RESULTS_FORM      "searchResults"
#define TRACK_SEARCH_CURRENT_TAB "tsCurTab"
#define TRACK_SEARCH_SIMPLE      "tsSimple"
#define TRACK_SEARCH_ON_NAME     "tsName"
#define TRACK_SEARCH_ON_TYPE     "tsType"
#define TRACK_SEARCH_ON_GROUP    "tsGroup"
#define TRACK_SEARCH_ON_DESCR    "tsDescr"
#define TRACK_SEARCH_SORT        "tsSort"

//#define FILES_SEARCH
#ifdef FILES_SEARCH
    #define FILE_SEARCH_ON_FILETYPE "tsFileType"
#endif///def FILES_SEARCH

static int gCmpGroup(const void *va, const void *vb)
/* Compare groups based on label. */
{
const struct group *a = *((struct group **)va);
const struct group *b = *((struct group **)vb);
return strcmp(a->label, b->label);
}

// Would like to do a radio button choice ofsorts
enum sortBy
    {
    sbRelevance=0,
    sbAbc      =1,
    sbHierarchy=2,
    };
static int gCmpTrackHierarchy(const void *va, const void *vb)
/* Compare tracks based on longLabel. */
{
const struct slRef *aa = *((struct slRef **)va);
const struct slRef *bb = *((struct slRef **)vb);
const struct track *a = ((struct track *) aa->val);
const struct track *b = ((struct track *) bb->val);
     if ( tdbIsFolder(a->tdb) && !tdbIsFolder(b->tdb))
        return -1;
else if (!tdbIsFolder(a->tdb) &&  tdbIsFolder(b->tdb))
        return 1;
     if ( tdbIsContainer(a->tdb) && !tdbIsContainer(b->tdb))
        return -1;
else if (!tdbIsContainer(a->tdb) &&  tdbIsContainer(b->tdb))
        return 1;
     if (!tdbIsContainerChild(a->tdb) &&  tdbIsContainerChild(b->tdb))
        return -1;
else if ( tdbIsContainerChild(a->tdb) && !tdbIsContainerChild(b->tdb))
        return 1;
return strcasecmp(a->longLabel, b->longLabel);
}

static int gCmpTrack(const void *va, const void *vb)
/* Compare tracks based on longLabel. */
{
const struct slRef *aa = *((struct slRef **)va);
const struct slRef *bb = *((struct slRef **)vb);
const struct track *a = ((struct track *) aa->val);
const struct track *b = ((struct track *) bb->val);
return strcasecmp(a->longLabel, b->longLabel);
}

static void findTracksSort(struct slRef **pTrack, enum sortBy sortBy)
{
if (sortBy == sbHierarchy)
    slSort(pTrack, gCmpTrackHierarchy);
else if (sortBy == sbAbc)
    slSort(pTrack, gCmpTrack);
else
    slReverse(pTrack);
}


// XXXX make a matchString function to support "contains", "is" etc. and wildcards in contains

//    ((sameString(op, "is") && !strcasecmp(track->shortLabel, str)) ||

static boolean isNameMatch(struct track *track, char *str, char *op)
{
return str && strlen(str) &&
    ((sameString(op, "is") && !strcasecmp(track->shortLabel, str)) ||
    (sameString(op, "is") && !strcasecmp(track->longLabel, str)) ||
    (sameString(op, "contains") && containsStringNoCase(track->shortLabel, str) != NULL) ||
    (sameString(op, "contains") && containsStringNoCase(track->longLabel, str) != NULL));
}

static boolean isDescriptionMatch(struct track *track, char **words, int wordCount)
// We parse str and look for every word at the start of any word in track description (i.e. google style).
{
if(words)
    {
    // We do NOT lookup up parent hierarchy for html descriptions.
    char *html = track->tdb->html;
    if(!isEmpty(html))
        {
        /* This probably could be made more efficient by parsing the html into some kind of b-tree, but I am assuming
           that the inner html loop while only happen for 1-2 words for vast majority of the tracks. */

        int i, numMatches = 0;
        html = stripRegEx(html, "<[^>]*>", REG_ICASE);
        for(i = 0; i < wordCount; i++)
            {
            char *needle = words[i];
            char *haystack, *tmp = cloneString(html);
            boolean found = FALSE;
            while((haystack = nextWord(&tmp)))
                {
                char *ptr = strstrNoCase(haystack, needle);
                if(ptr != NULL && ptr == haystack)
                    {
                    found = TRUE;
                    break;
                    }
                }
            if(found)
                numMatches++;
            else
                break;
            }
        if(numMatches == wordCount)
            return TRUE;
        }
    }
return FALSE;
}

static int getFormatTypes(char ***pLabels, char ***pTypes)
{
char *crudeTypes[] = {
    ANYLABEL,
    "bam",
    "psl",
    "chain",
    "netAlign",
    "maf",
    "bed",
    "bigBed",
    "ctgPos",
    "expRatio",
    "genePred",
    "broadPeak",
    "narrowPeak",
    "rmsk",
    "bedGraph",
    "bigWig",
    "wig",
    "wigMaf"
};
// Non-standard:
// type altGraphX
// type axt
// type bed5FloatScore
// type bed5FloatScoreWithFdr
// type chromGraph
// type clonePos
// type coloredExon
// type encodeFiveC
// type factorSource
// type ld2
// type logo
// type maf
// type sample
// type wigMafProt 0.0 1.0

char *nicerTypes[] = {
    ANYLABEL,
    "Alignment binary (bam) - binary SAM",
    "Alignment Blast (psl) - Blast output",
    "Alignment Chains (chain) - Pairwise alignment",
    "Alignment Nets (netAlign) - Net alignments",
    "Alignments (maf) - multiple alignment format",
    "bed - browser extensible data",
    "bigBed - self index, often remote bed format",
    "ctgPos - Contigs",
    "expRatio - Expression ratios",
    "Genes (genePred) - Gene prediction and annotation",
    "Peaks Broad (broadPeak) - ENCODE large region peak format",
    "Peaks Narrow (narrowPeak) - ENCODE small region peak format",
    "Repeats (rmsk) - Repeat masking",
    "Signal (bedGraph) - graphically represented bed data",
    "Signal (bigWig) - self index, often remote wiggle format",
    "Signal (wig) - wiggle format",
    "Signal (wigMaf) - multiple alignment wiggle"
};

int ix = 0, count = sizeof(crudeTypes)/sizeof(char *);
char **labels;
char **values;
AllocArray(labels, count);
AllocArray(values, count);
for(ix=0;ix<count;ix++)
    {
    labels[ix] = cloneString(nicerTypes[ix]);
    values[ix] = cloneString(crudeTypes[ix]);
    }
*pLabels = labels;
*pTypes = values;
return count;
}

static struct slRef *simpleSearchForTracksstruct(struct trix *trix,char **descWords,int descWordCount)
// Performs the simple search and returns the found tracks.
{
struct slRef *tracks = NULL;

struct trixSearchResult *tsList;
for(tsList = trixSearch(trix, descWordCount, descWords, TRUE); tsList != NULL; tsList = tsList->next)
    {
    struct track *track = (struct track *) hashFindVal(trackHash, tsList->itemId);
    if (track != NULL)  // It is expected that this is NULL (e.g. when the trix references trackDb tracks which have no tables)
        {
        refAdd(&tracks, track);
        }
    }
return tracks;
}

static struct slRef *advancedSearchForTracks(struct sqlConnection *conn,struct group *groupList, char **descWords,int descWordCount,
                                             char *nameSearch, char *typeSearch, char *descSearch, char *groupSearch, struct slPair *mdbPairs)
// Performs the advanced search and returns the found tracks.
{
int tracksFound = 0;
struct slRef *tracks = NULL;
int numMetadataNonEmpty = slCount(mdbPairs);

    if(!isEmpty(nameSearch) || typeSearch != NULL || descSearch != NULL || groupSearch != NULL || numMetadataNonEmpty)
        {
        // First do the metaDb searches, which can be done quickly for all tracks with db queries.
        struct hash *matchingTracks = newHash(0);

        if (numMetadataNonEmpty)
            {

            struct mdbObj *mdbObj, *mdbObjs = mdbObjRepeatedSearch(conn,mdbPairs,TRUE,FALSE);
            if (mdbObjs)
                {
                for (mdbObj = mdbObjs; mdbObj != NULL; mdbObj = mdbObj->next)
                    hashAddInt(matchingTracks, mdbObj->obj, 1);
                mdbObjsFree(&mdbObjs);
                }
            }

        struct group *group;
        for (group = groupList; group != NULL; group = group->next)
            {
            if(groupSearch == NULL || sameString(group->name, groupSearch))
                {
                if (group->trackList != NULL)
                    {
                    struct trackRef *tr;
                    for (tr = group->trackList; tr != NULL; tr = tr->next)
                        {
                        struct track *track = tr->track;
                        char *trackType = cloneFirstWord(track->tdb->type); // will be spilled
                        if((isEmpty(nameSearch) || isNameMatch(track, nameSearch, "contains")) &&
                           (isEmpty(typeSearch) || (sameWord(typeSearch, trackType) && !tdbIsComposite(track->tdb))) &&
                           (isEmpty(descSearch) || isDescriptionMatch(track, descWords, descWordCount)) &&
                          (!numMetadataNonEmpty || hashLookup(matchingTracks, track->track) != NULL))
                            {
                            if (track != NULL)
                                {
                                tracksFound++;
                                refAdd(&tracks, track);
                                }
                            else
                                warn("found group track is NULL.");
                            }
                        if (track->subtracks != NULL)
                            {
                            struct track *subTrack;
                            for (subTrack = track->subtracks; subTrack != NULL; subTrack = subTrack->next)
                                {
                                trackType = cloneFirstWord(subTrack->tdb->type); // will be spilled
                                if((isEmpty(nameSearch) || isNameMatch(subTrack, nameSearch, "contains")) &&
                                   (isEmpty(typeSearch) || sameWord(typeSearch, trackType)) &&
                                   (isEmpty(descSearch) || isDescriptionMatch(subTrack, descWords, descWordCount)) &&
                                   (!numMetadataNonEmpty || hashLookup(matchingTracks, subTrack->track) != NULL))
                                    {
                                    if (track != NULL)
                                        {
                                        tracksFound++;
                                        refAdd(&tracks, subTrack);
                                        }
                                    else
                                        warn("found subtrack is NULL.");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

return tracks;
}

#define MAX_FOUND_TRACKS 100
static void findTracksPageLinks(int tracksFound, int startFrom)
{
if (tracksFound <= MAX_FOUND_TRACKS)
    return;

// Opener
int willStartAt = 0;
int curPage  = (startFrom/MAX_FOUND_TRACKS) + 1;
int endAt = startFrom+MAX_FOUND_TRACKS;
if (endAt > tracksFound)
    endAt = tracksFound;
hPrintf("<span><em style='font-size:.9em;'>Listing %d - %d of %d tracks</em>&nbsp;&nbsp;&nbsp;",startFrom+1,endAt,tracksFound);

// << and <
if (startFrom >= MAX_FOUND_TRACKS)
    {
    hPrintf("<a href='../cgi-bin/hgTracks?%s=Search&%s=0' title='First page of found tracks' onclick='return findTracksPage(\"%s\",0);'>&#171;</a>&nbsp;",
            TRACK_SEARCH,TRACK_SEARCH_PAGER,TRACK_SEARCH_PAGER);
    willStartAt = startFrom - MAX_FOUND_TRACKS;
    hPrintf("&nbsp;<a href='../cgi-bin/hgTracks?%s=Search&%s=%d' title='Previous page of found tracks' onclick='return findTracksPage(\"%s\",%d);'>&#139;</a>&nbsp;",
            TRACK_SEARCH,TRACK_SEARCH_PAGER,willStartAt,TRACK_SEARCH_PAGER,willStartAt);
    }

// page number links
int lastPage = (tracksFound/MAX_FOUND_TRACKS);
if ((tracksFound % MAX_FOUND_TRACKS) > 0)
    lastPage++;

int thisPage = curPage - 3; // Window of 3 pages above and below
if (thisPage < 1)
    thisPage = 1;
for (;thisPage <= lastPage && thisPage <= curPage + 3; thisPage++)
    {
    if (thisPage != curPage)
        {
        willStartAt = ((thisPage - 1) * MAX_FOUND_TRACKS);
        endAt = willStartAt+ MAX_FOUND_TRACKS;
        if (endAt > tracksFound)
            endAt = tracksFound;
        hPrintf("&nbsp;<a href='../cgi-bin/hgTracks?%s=Search&%s=%d' title='Page %d (%d - %d) tracks' onclick='return findTracksPage(\"%s\",%d);'>%d</a>&nbsp;",
                TRACK_SEARCH,TRACK_SEARCH_PAGER,willStartAt,thisPage,willStartAt+1,endAt,TRACK_SEARCH_PAGER,willStartAt,thisPage);
        }
    else
        hPrintf("&nbsp;<em style='color:%s;'>%d</em>&nbsp;",COLOR_DARKGREY,thisPage);
    }

// > and >>
if ((startFrom + MAX_FOUND_TRACKS) < tracksFound)
    {
    willStartAt = startFrom + MAX_FOUND_TRACKS;
    hPrintf("&nbsp;<a href='../cgi-bin/hgTracks?%s=Search&%s=%d' title='Next page of found tracks' onclick='return findTracksPage(\"%s\",%d);'>&#155;</a>&nbsp;",
            TRACK_SEARCH,TRACK_SEARCH_PAGER,willStartAt,TRACK_SEARCH_PAGER,willStartAt);
    willStartAt =  tracksFound - (tracksFound % MAX_FOUND_TRACKS);
    if (willStartAt == tracksFound)
        willStartAt -= MAX_FOUND_TRACKS;
    hPrintf("&nbsp;<a href='../cgi-bin/hgTracks?%s=Search&%s=%d' title='Last page of found tracks' onclick='return findTracksPage(\"%s\",%d);'>&#187;</a></span>\n",
            TRACK_SEARCH,TRACK_SEARCH_PAGER,willStartAt,TRACK_SEARCH_PAGER,willStartAt);
    }
}

static void displayFoundTracks(struct cart *cart, struct slRef *tracks, int tracksFound,enum sortBy sortBy)
// Routine for displaying found tracks
{
hPrintf("<div id='found' style='display:none;'>\n"); // This div allows the clear button to empty it
if(tracksFound < 1)
    {
    hPrintf("<p>No tracks found</p>\n");
    }
else
    {
    struct hash *tdbHash = makeTrackHash(database, chromName);
    hPrintf("<form action='%s' name='%s' id='%s' method='post'>\n\n", hgTracksName(),SEARCH_RESULTS_FORM,SEARCH_RESULTS_FORM);
    cartSaveSession(cart);  // Creates hidden var of hgsid to avoid bad voodoo

    int startFrom = 0;
    hPrintf("<table id='foundTracks'>\n");

    // Opening view in browser button and foundTracks count
    #define ENOUGH_FOUND_TRACKS 10
    if(tracksFound >= ENOUGH_FOUND_TRACKS)
        {
        hPrintf("<tr><td nowrap colspan=3>\n");
        hPrintf("<INPUT TYPE=SUBMIT NAME='submit' VALUE='return to browser' class='viewBtn' style='font-size:.8em;'>");
        hPrintf("&nbsp;&nbsp;&nbsp;&nbsp;<FONT class='selCbCount'></font>\n");

        startFrom = cartUsualInt(cart,TRACK_SEARCH_PAGER,0);
        if (startFrom > 0 && startFrom < tracksFound)
            {
            int countUp = 0;
            for(countUp=0; countUp < startFrom;countUp++)
                {
                if (slPopHead(&tracks) == NULL) // memory waste
                    break;
                }
            }
        hPrintf("</td><td align='right' valign='bottom'>\n");
        findTracksPageLinks(tracksFound,startFrom);
        hPrintf("</td></tr>\n");
        }

    // Begin foundTracks table
    //hPrintf("<table id='foundTracks'><tr><td colspan='2'>\n");
    hPrintf("<tr><td colspan='2'>\n");
    hPrintf("</td><td align='right'>\n");
    #define PM_BUTTON "<IMG height=18 width=18 onclick=\"return findTracksCheckAllWithWait(%s);\" id='btn_%s' src='../images/%s' title='%s all found tracks'>"
    hPrintf("</td></tr><tr bgcolor='#%s'><td>",HG_COL_HEADER);
    hPrintf(PM_BUTTON,"true",  "plus_all",   "add_sm.gif",  "Select");
    hPrintf(PM_BUTTON,"false","minus_all","remove_sm.gif","Unselect");
    hPrintf("</td><td><b>Visibility</b></td><td colspan=2>&nbsp;&nbsp;<b>Track Name</b>\n");

    // Sort options?
    if(tracksFound >= ENOUGH_FOUND_TRACKS)
        {
        hPrintf("<span style='float:right;'>Sort:");
        cgiMakeOnClickRadioButton(TRACK_SEARCH_SORT, "0", (sortBy == sbRelevance),"onclick=\"findTracksSortNow(this);\"");
        hPrintf("by Relevance");
        cgiMakeOnClickRadioButton(TRACK_SEARCH_SORT, "1", (sortBy == sbAbc),      "onclick=\"findTracksSortNow(this);\"");
        hPrintf("Alphabetically");
        cgiMakeOnClickRadioButton(TRACK_SEARCH_SORT, "2",(sortBy == sbHierarchy), "onclick=\"findTracksSortNow(this);\"");
        hPrintf("by Hierarchy&nbsp;&nbsp;</span>\n");
        }
    hPrintf("</td></tr>\n");

    // Set up json for js functionality
    struct dyString *jsonTdbVars = NULL;

    int trackCount=0;
    boolean containerTrackCount = 0;
    struct slRef *ptr;
    while((ptr = slPopHead(&tracks)))
        {
        if(++trackCount > MAX_FOUND_TRACKS)
            break;

        struct track *track = (struct track *) ptr->val;
        jsonTdbSettingsBuild(&jsonTdbVars, track, FALSE); // FALSE: No configuration from track search

        if (tdbIsFolder(track->tdb)) // supertrack
            hPrintf("<tr bgcolor='%s' valign='top' class='found'>\n","#EED5B7");//"#DEB887");//"#E6B426");//#FCECC0//COLOR_LTGREY);//COLOR_LTGREEN);//COLOR_TRACKLIST_LEVEL1);
        else if (tdbIsContainer(track->tdb))
            hPrintf("<tr bgcolor='%s' valign='top' class='found'>\n",COLOR_TRACKLIST_LEVEL3);
        else
            hPrintf("<tr bgcolor='%s' valign='top' class='found'>\n",COLOR_TRACKLIST_LEVEL2);

        hPrintf("<td align='center'>\n");

        // Determine visibility and checked state
        track->visibility = tdbVisLimitedByAncestors(cart, track->tdb, TRUE, TRUE);
        boolean checked = ( track->visibility != tvHide );
        if(tdbIsContainerChild(track->tdb))
            {
            checked = fourStateVisible(subtrackFourStateChecked(track->tdb,cart)); // Don't need all 4 states here.  Visible=checked&&enabled
            checked = (checked && ( track->visibility != tvHide )); // Checked is only if subtrack level vis is also set!
            }

        // Setup the check box
        #define CB_HIDDEN_VAR "<INPUT TYPE=HIDDEN disabled=true NAME='%s_sel' VALUE='%s'>"
        if (tdbIsContainerChild(track->tdb) || tdbIsFolderContent(track->tdb))  // subtracks and folder children get "_sel" var.  ("_sel" var is temporary on folder children)
            hPrintf(CB_HIDDEN_VAR,track->track,checked?"1":CART_VAR_EMPTY);
        #define CB_SEEN "<INPUT TYPE=CHECKBOX id='%s_sel_id' VALUE='on' class='selCb' onclick='findTracksClickedOne(this,true);'%s>"
        hPrintf(CB_SEEN,track->track,(checked?" CHECKED":""));
        hPrintf("</td><td>\n");

        // Setup the visibility drop down
        #define VIS_HIDDEN_VAR "<INPUT TYPE=HIDDEN disabled=true NAME='%s' VALUE='%s'>"
        hPrintf(VIS_HIDDEN_VAR,track->track,CART_VAR_EMPTY); // All tracks get vis hidden var
        char extra[512];
        if (tdbIsFolder(track->tdb))
            {
            safef(extra,sizeof(extra),"id='%s_id' onchange='findTracksChangeVis(this)'",track->track);
            hideShowDropDownWithClassAndExtra(track->track, (track->visibility != tvHide), "normalText visDD",extra);
            }
        else
            {
            safef(extra,sizeof(extra),"id='%s_id' onchange='findTracksChangeVis(this)'",track->track);
            hTvDropDownClassWithJavascript(NULL, track->visibility,track->canPack,"normalText seenVis",extra);
            }

        // If this is a container track, allow configuring...
        if (tdbIsContainer(track->tdb) || tdbIsFolder(track->tdb))
            {
            containerTrackCount++; // Using onclick ensures return to search tracks on submit
            hPrintf("&nbsp;<IMG SRC='../images/folderWrench.png' style='cursor:pointer;' title='Configure this track container...' onclick='findTracksConfigureSet(\"%s\");'>&nbsp;", track->track);
            }
//#define SHOW_PARENT_FOLDER
#ifdef SHOW_PARENT_FOLDER
        else if (tdbIsContainerChild(track->tdb) || tdbIsFolderContent(track->tdb))
            {
            struct trackDb *parentTdb = tdbIsContainerChild(track->tdb) ? tdbGetContainer(track->tdb) : tdbGetImmediateFolder(track->tdb);
            if (parentTdb != NULL) // Using href will not return to search tracks on submit
                hPrintf("&nbsp;<A HREF='../cgi-bin/hgTrackUi?g=%s'><IMG SRC='../images/folderC.png' title='Navigate to parent container...'></A>&nbsp;", parentTdb->track);
            }
#endif///def SHOW_PARENT_FOLDER
        hPrintf("</td>\n");

        // shortLabel has description popup and longLabel has "..." metadata
        hPrintf("<td><a target='_top' onclick=\"hgTrackUiPopUp('%s',true); return false;\" href='%s' title='Display track details'>%s</a></td>\n", track->track, trackUrl(track->track, NULL), track->shortLabel);
        hPrintf("<td>%s", track->longLabel);
        compositeMetadataToggle(database, track->tdb, "...", TRUE, FALSE, tdbHash);
        hPrintf("</td></tr>\n");
        }
    //hPrintf("</table>\n");

    // Closing view in browser button and foundTracks count
    hPrintf("<tr><td nowrap colspan=3>");
    hPrintf("<INPUT TYPE=SUBMIT NAME='submit' VALUE='Return to Browser' class='viewBtn' style='font-size:.8em;'>");
    hPrintf("&nbsp;&nbsp;&nbsp;&nbsp;<FONT class='selCbCount'></font>");
    if(tracksFound >= ENOUGH_FOUND_TRACKS)
        {
        hPrintf("</td><td align='right' valign='top'>\n");
        findTracksPageLinks(tracksFound,startFrom);
        hPrintf("</td></tr>\n");
        }
    hPrintf("</table>\n");

    if(containerTrackCount > 0)
        hPrintf("<BR><IMG SRC='../images/folderWrench.png'>&nbsp;Tracks so marked are containers which group related data tracks.  Containers may need additional configuration (by clicking on the <IMG SRC='../images/folderWrench.png'> icon) before they can be viewed in the browser.<BR>\n");
        //hPrintf("* Tracks so marked are containers which group related data tracks.  These may not be visible unless further configuration is done.  Click on the * to configure these.<BR><BR>\n");
    hPrintf("\n</form>\n");

    // be done with json
    hWrites(jsonTdbSettingsUse(&jsonTdbVars));
    }
hPrintf("</div>"); // This div allows the clear button to empty it
}

void doSearchTracks(struct group *groupList)
{
if (!advancedJavascriptFeaturesEnabled(cart))
    {
    warn("Requires advanced javascript features.");
    return;
    }

webIncludeResourceFile("ui.dropdownchecklist.css");
//jsIncludeFile("ui.core.js",NULL);   // NOTE: This appears to be not needed as long as jquery-ui.js comes before ui.dropdownchecklist.js
jsIncludeFile("ui.dropdownchecklist.js",NULL);
// This line is needed to get the multi-selects initialized
hPrintf("<script type='text/javascript'>$(document).ready(function() { $('.filterBy').each( function(i) { $(this).dropdownchecklist({ firstItemChecksAll: true, noneIsAll: true });});});</script>\n");

struct group *group;
char *groups[128];
char *labels[128];
int numGroups = 1;
groups[0] = ANYLABEL;
labels[0] = ANYLABEL;
char *nameSearch = cartOptionalString(cart, TRACK_SEARCH_ON_NAME);
char *typeSearch = cartOptionalString(cart, TRACK_SEARCH_ON_TYPE);
#ifdef FILES_SEARCH
char *fileTypeSearch = cartOptionalString(cart, FILE_SEARCH_ON_FILETYPE);
#endif///def FILES_SEARCH
char *descSearch=FALSE;
char *groupSearch = cartOptionalString(cart, TRACK_SEARCH_ON_GROUP);
boolean doSearch = sameString(cartOptionalString(cart, TRACK_SEARCH), "Search") || cartUsualInt(cart, TRACK_SEARCH_PAGER, -1) >= 0;
struct sqlConnection *conn = hAllocConn(database);
boolean metaDbExists = sqlTableExists(conn, "metaDb");
int tracksFound = 0;
struct trix *trix;
char trixFile[HDB_MAX_PATH_STRING];
char **descWords = NULL;
int descWordCount = 0;
boolean searchTermsExist = FALSE;
int cols;
char buf[512];

enum searchTab selectedTab = simpleTab;
char *currentTab = cartUsualString(cart, TRACK_SEARCH_CURRENT_TAB, "simpleTab");
if(sameString(currentTab, "simpleTab"))
    {
    selectedTab = simpleTab;
    descSearch = cartOptionalString(cart, TRACK_SEARCH_SIMPLE);
    freez(&nameSearch);
    }
else if(sameString(currentTab, "advancedTab"))
    {
    selectedTab = advancedTab;
    descSearch = cartOptionalString(cart, TRACK_SEARCH_ON_DESCR);
    }
#ifdef FILES_SEARCH
else if(sameString(currentTab, "filesTab"))
    {
    selectedTab = filesTab;
    descSearch = cartOptionalString(cart, TRACK_SEARCH_ON_DESCR);
    }
#endif///def FILES_SEARCH

if(descSearch)
    stripChar(descSearch, '"');
trackList = getTrackList(&groupList, -2); // global
makeGlobalTrackHash(trackList);

// NOTE: This is necessary when container cfg by '*' results in vis changes
// This will handle composite/view override when subtrack specific vis exists, AND superTrack reshaping.
parentChildCartCleanup(trackList,cart,oldVars); // Subtrack settings must be removed when composite/view settings are updated

getSearchTrixFile(database, trixFile, sizeof(trixFile));
trix = trixOpen(trixFile);
slSort(&groupList, gCmpGroup);
for (group = groupList; group != NULL; group = group->next)
    {
    groupTrackListAddSuper(cart, group);
    if (group->trackList != NULL)
        {
        groups[numGroups] = cloneString(group->name);
        labels[numGroups] = cloneString(group->label);
        numGroups++;
        if (numGroups >= ArraySize(groups))
            internalErr();
        }
    }

safef(buf, sizeof(buf),"Search for Tracks in the %s %s Assembly", organism, hFreezeFromDb(database));
webStartWrapperDetailedNoArgs(cart, database, "", buf, FALSE, FALSE, FALSE, FALSE);

hPrintf("<div style='max-width:1080px;'>");
hPrintf("<form action='%s' name='%s' id='%s' method='get'>\n\n", hgTracksName(),TRACK_SEARCH_FORM,TRACK_SEARCH_FORM);
cartSaveSession(cart);  // Creates hidden var of hgsid to avoid bad voodoo
safef(buf, sizeof(buf), "%lu", clock1());
cgiMakeHiddenVar("hgt_", buf);  // timestamps page to avoid browser cache


hPrintf("<input type='hidden' name='db' value='%s'>\n", database);
hPrintf("<input type='hidden' name='%s' id='currentTab' value='%s'>\n", TRACK_SEARCH_CURRENT_TAB, currentTab);
hPrintf("<input type='hidden' name='%s' value=''>\n",TRACK_SEARCH_DEL_ROW);
hPrintf("<input type='hidden' name='%s' value=''>\n",TRACK_SEARCH_ADD_ROW);
hPrintf("<input type='hidden' name='%s' value=''>\n",TRACK_SEARCH_PAGER);

hPrintf("<div id='tabs' style='display:none; %s'>\n"
        "<ul>\n"
        "<li><a href='#simpleTab'><B style='font-size:.9em;font-family: arial, Geneva, Helvetica, san-serif;'>Search</B></a></li>\n"
        "<li><a href='#advancedTab'><B style='font-size:.9em;font-family: arial, Geneva, Helvetica, san-serif;'>Advanced</B></a></li>\n"
#ifdef FILES_SEARCH
        "<li><a href='#filesTab'><B style='font-size:.9em;font-family: arial, Geneva, Helvetica, san-serif;'>Files</B></a></li>\n"
#endif///def FILES_SEARCH
        "</ul>\n"
        "<div id='simpleTab' style='max-width:inherit;'>\n",cgiBrowser()==btIE?"width:1060px;":"max-width:inherit;");

hPrintf("<table id='simpleTable' style='width:100%%; font-size:.9em;'><tr><td colspan='2'>");
hPrintf("<input type='text' name='%s' id='simpleSearch' class='submitOnEnter' value='%s' style='max-width:1000px; width:100%%;' onkeyup='findTracksSearchButtonsEnable(true);'>\n",
        TRACK_SEARCH_SIMPLE,descSearch == NULL ? "" : descSearch);
if (selectedTab==simpleTab && descSearch)
    searchTermsExist = TRUE;

hPrintf("</td></tr><td style='max-height:4px;'></td></tr></table>");
//hPrintf("</td></tr></table>");
hPrintf("<input type='submit' name='%s' id='searchSubmit' value='search' style='font-size:.8em;'>\n", TRACK_SEARCH);
hPrintf("<input type='button' name='clear' value='clear' class='clear' style='font-size:.8em;' onclick='findTracksClear();'>\n");
hPrintf("<input type='submit' name='submit' value='cancel' class='cancel' style='font-size:.8em;'>\n");
hPrintf("</div>\n");

// Advanced tab
hPrintf("<div id='advancedTab' style='width:inherit;'>\n"
        "<table id='advancedTable' cellSpacing=0 style='width:inherit; font-size:.9em;'>\n");
cols = 8;

// Track Name contains
hPrintf("<tr><td colspan=3></td>");
hPrintf("<td nowrap><b style='max-width:100px;'>Track&nbsp;Name:</b></td>");
hPrintf("<td align='right'>contains</td>\n");
hPrintf("<td colspan='%d'>", cols - 4);
hPrintf("<input type='text' name='%s' id='nameSearch' class='submitOnEnter' value='%s' onkeyup='findTracksSearchButtonsEnable(true);' style='min-width:326px; font-size:.9em;'>",
        TRACK_SEARCH_ON_NAME, nameSearch == NULL ? "" : nameSearch);
hPrintf("</td></tr>\n");

// Description contains
hPrintf("<tr><td colspan=2></td><td align='right'>and&nbsp;</td>");
hPrintf("<td><b style='max-width:100px;'>Description:</b></td>");
hPrintf("<td align='right'>contains</td>\n");
hPrintf("<td colspan='%d'>", cols - 4);
hPrintf("<input type='text' name='%s' id='descSearch' value='%s' class='submitOnEnter' onkeyup='findTracksSearchButtonsEnable(true);' style='max-width:536px; width:536px; font-size:.9em;'>",
        TRACK_SEARCH_ON_DESCR, descSearch == NULL ? "" : descSearch);
hPrintf("</td></tr>\n");
if (selectedTab==advancedTab && descSearch)
    searchTermsExist = TRUE;

hPrintf("<tr><td colspan=2></td><td align='right'>and&nbsp;</td>\n");
hPrintf("<td><b style='max-width:100px;'>Group:</b></td>");
hPrintf("<td align='right'>is</td>\n");
hPrintf("<td colspan='%d'>", cols - 4);
cgiMakeDropListFull(TRACK_SEARCH_ON_GROUP, labels, groups, numGroups, groupSearch, "class='groupSearch' style='min-width:40%; font-size:.9em;'");
hPrintf("</td></tr>\n");
if (selectedTab==advancedTab && groupSearch)
    searchTermsExist = TRUE;

// Track Type is (drop down)
hPrintf("<tr><td colspan=2></td><td align='right'>and&nbsp;</td>\n");
hPrintf("<td nowrap><b style='max-width:100px;'>Data Format:</b></td>");
hPrintf("<td align='right'>is</td>\n");
hPrintf("<td colspan='%d'>", cols - 4);
char **formatTypes = NULL;
char **formatLabels = NULL;
int formatCount = getFormatTypes(&formatLabels, &formatTypes);
cgiMakeDropListFull(TRACK_SEARCH_ON_TYPE, formatLabels, formatTypes, formatCount, typeSearch, "class='typeSearch' style='min-width:40%; font-size:.9em;'");
hPrintf("</td></tr>\n");
if (selectedTab==advancedTab && typeSearch)
    searchTermsExist = TRUE;

// mdb selects
struct slPair *mdbSelects = NULL;
if(metaDbExists)
    {
    struct slPair *mdbVars = mdbVarsSearchable(conn,TRUE,FALSE); // Tables but not file only objects
    mdbSelects = mdbSelectPairs(cart,selectedTab, mdbVars);
    char *output = mdbSelectsHtmlRows(conn,mdbSelects,mdbVars,cols,FALSE);  // not a fileSearch
    if (output)
        {
        puts(output);
        freeMem(output);
        }
    slPairFreeList(&mdbVars);
    }

hPrintf("</table>\n");
hPrintf("<input type='submit' name='%s' id='searchSubmit' value='search' style='font-size:.8em;'>\n", TRACK_SEARCH);
hPrintf("<input type='button' name='clear' value='clear' class='clear' style='font-size:.8em;' onclick='findTracksClear();'>\n");
hPrintf("<input type='submit' name='submit' value='cancel' class='cancel' style='font-size:.8em;'>\n");
//hPrintf("<a target='_blank' href='../goldenPath/help/trackSearch.html'>help</a>\n");
hPrintf("</div>\n");

#ifdef FILES_SEARCH
// Files tab
hPrintf("<div id='filesTab' style='width:inherit;'>\n"
        "<table id='filesTable' cellSpacing=0 style='width:inherit; font-size:.9em;'>\n");
cols = 8;

//// Track Name contains
//hPrintf("<tr><td colspan=3></td>");
//hPrintf("<td nowrap><b style='max-width:100px;'>Track&nbsp;Name:</b></td>");
//hPrintf("<td align='right'>contains</td>\n");
//hPrintf("<td colspan='%d'>", cols - 4);
//hPrintf("<input type='text' name='%s' id='nameSearch' class='submitOnEnter' value='%s' onkeyup='findTracksSearchButtonsEnable(true);' style='min-width:326px; font-size:.9em;'>",
//        TRACK_SEARCH_ON_NAME, nameSearch == NULL ? "" : nameSearch);
//hPrintf("</td></tr>\n");
//
//// Description contains
//hPrintf("<tr><td colspan=2></td><td align='right'>and&nbsp;</td>");
//hPrintf("<td><b style='max-width:100px;'>Description:</b></td>");
//hPrintf("<td align='right'>contains</td>\n");
//hPrintf("<td colspan='%d'>", cols - 4);
//hPrintf("<input type='text' name='%s' id='descSearch' value='%s' class='submitOnEnter' onkeyup='findTracksSearchButtonsEnable(true);' style='max-width:536px; width:536px; font-size:.9em;'>",
//        TRACK_SEARCH_ON_DESCR, descSearch == NULL ? "" : descSearch);
//hPrintf("</td></tr>\n");
//if (selectedTab==fileTab && descSearch)
//    searchTermsExist = TRUE;
//
//hPrintf("<tr><td colspan=2></td><td align='right'>and&nbsp;</td>\n");
//hPrintf("<td><b style='max-width:100px;'>Group:</b></td>");
//hPrintf("<td align='right'>is</td>\n");
//hPrintf("<td colspan='%d'>", cols - 4);
//cgiMakeDropListFull(TRACK_SEARCH_ON_GROUP, labels, groups, numGroups, groupSearch, "class='groupSearch' style='min-width:40%; font-size:.9em;'");
//hPrintf("</td></tr>\n");
//if (selectedTab==fileTab && groupSearch)
//    searchTermsExist = TRUE;

// Track Type is (drop down)
hPrintf("<tr><td colspan=2></td><td align='right'>&nbsp;</td>\n");
//hPrintf("<tr><td colspan=2></td><td align='right'>and&nbsp;</td>\n"); // Bring back "and" if using "Track Name,Description or Group
hPrintf("<td nowrap><b style='max-width:100px;'>Data Format:</b></td>");
hPrintf("<td align='right'>is</td>\n");
hPrintf("<td colspan='%d'>", cols - 4);
char *dropDownHtml = fileFormatSelectHtml(FILE_SEARCH_ON_FILETYPE,fileTypeSearch,"style='min-width:40%; font-size:.9em;'");
if (dropDownHtml)
    {
    puts(dropDownHtml);
    freeMem(dropDownHtml);
    }
hPrintf("</td></tr>\n");
if (selectedTab==filesTab && fileTypeSearch)
    searchTermsExist = TRUE;

// mdb selects
if(metaDbExists)
    {
    struct slPair *mdbVars = mdbVarsSearchable(conn,TRUE,FALSE); // Tables but not file only objects
    mdbSelects = mdbSelectPairs(cart,selectedTab, mdbVars);
    char *output = mdbSelectsHtmlRows(conn,mdbSelects,mdbVars,colsFALSE);  // not a fileSearch
    if (output)
        {
        puts(output);
        freeMem(output);
        }
    slPairFreeList(&mdbVars);
    }

hPrintf("</table>\n");
hPrintf("<input type='submit' name='%s' id='searchSubmit' value='search' style='font-size:.8em;'>\n", TRACK_SEARCH);
hPrintf("<input type='button' name='clear' value='clear' class='clear' style='font-size:.8em;' onclick='findTracksClear();'>\n");
hPrintf("<input type='submit' name='submit' value='cancel' class='cancel' style='font-size:.8em;'>\n");
//hPrintf("<a target='_blank' href='../goldenPath/help/trackSearch.html'>help</a>\n");
hPrintf("</div>\n");

#endif///def FILES_SEARCH
hPrintf("</div>\n");

hPrintf("</form>\n");
hPrintf("</div>"); // Restricts to max-width:1000px;

if (measureTiming)
    uglyTime("Rendered tabs");

if(descSearch != NULL && !strlen(descSearch))
    descSearch = NULL;
if(groupSearch != NULL && sameString(groupSearch, ANYLABEL))
    groupSearch = NULL;
if(typeSearch != NULL && sameString(typeSearch, ANYLABEL))
    typeSearch = NULL;

if(!isEmpty(descSearch))
    {
    char *tmp = cloneString(descSearch);
    char *val = nextWord(&tmp);
    struct slName *el, *descList = NULL;
    int i;
    while (val != NULL)
        {
        slNameAddTail(&descList, val);
        descWordCount++;
        val = nextWord(&tmp);
        }
    descWords = needMem(sizeof(char *) * descWordCount);
    for(i = 0, el = descList; el != NULL; i++, el = el->next)
        descWords[i] = strLower(el->name);
    }
if (doSearch && selectedTab==simpleTab && descWordCount <= 0)
    doSearch = FALSE;

if(doSearch)
    {
    // Now search
    struct slRef *tracks = NULL;
    if(selectedTab==simpleTab)
        tracks = simpleSearchForTracksstruct(trix,descWords,descWordCount);
    else if(selectedTab==advancedTab)
        tracks = advancedSearchForTracks(conn,groupList,descWords,descWordCount,nameSearch,typeSearch,descSearch,groupSearch,mdbSelects);
#ifdef FILES_SEARCH
    else if(selectedTab==filesTab && mdbSelects != NULL)
        fileSearchResults(database, conn, mdbSelects, fileTypeSearch);
#endif///def FILES_SEARCH

    if (measureTiming)
        uglyTime("Searched for tracks");

    // Sort and Print results
    if(selectedTab!=filesTab)
        {
        enum sortBy sortBy = cartUsualInt(cart,TRACK_SEARCH_SORT,sbRelevance);
        tracksFound = slCount(tracks);
        if(tracksFound > 1)
            findTracksSort(&tracks,sortBy);

        displayFoundTracks(cart,tracks,tracksFound,sortBy);

        if (measureTiming)
            uglyTime("Displayed found tracks");
        }
    slPairFreeList(&mdbSelects);
    }
hFreeConn(&conn);

webNewSection("About Track Search");
if(metaDbExists)
    hPrintf("<p>Search for terms in track names, descriptions, groups, and ENCODE "
            "metadata.  If multiple terms are entered, only tracks with all terms "
            "will be part of the results.");
else
    hPrintf("<p>Search for terms in track descriptions, groups, and names. "
            "If multiple terms are entered, only tracks with all terms "
            "will be part of the results.");
hPrintf("<BR><a target='_blank' href='../goldenPath/help/trackSearch.html'>more help</a></p>\n");
webEndSectionTables();
}
