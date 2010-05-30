/* Track search code used by hgTracks CGI */

#include "common.h"
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

static char const rcsid[] = "$Id: searchTracks.c,v 1.6 2010/05/30 01:38:17 larrym Exp $";

#define ANYLABEL "Any"

static int gCmpGroup(const void *va, const void *vb)
/* Compare groups based on label. */
{
const struct group *a = *((struct group **)va);
const struct group *b = *((struct group **)vb);
return strcmp(a->label, b->label);
}

static int gCmpTrack(const void *va, const void *vb)
/* Compare tracks based on longLabel. */
{
const struct slRef *aa = *((struct slRef **)va);
const struct slRef *bb = *((struct slRef **)vb);
const struct track *a = ((struct track *) aa->val);
const struct track *b = ((struct track *) bb->val);
return strcmp(a->longLabel, b->longLabel);
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

static boolean isDescriptionMatch(struct track *track, char *str, struct hash *trackMetadata)
{
// We parse str and look for every word ANYWHERE in track description (i.e. google style).
// XXXX currently quite primitive; do stemming, strip html markup ??
// Also, we should fold all metadata into the description of every track.
boolean found = FALSE;

if(str && strlen(str))
    {
    char *html = track->tdb->html;
    if(html == NULL || !strlen(html))
        {
        // XXXX is there a cleaner way to find parent?
        struct trackDb *parent = track->tdb->parent;
        while(parent != NULL && (parent->html == NULL || !strlen(parent->html)))
            parent = parent->parent;
        if(parent != NULL)
            html = parent->html;
        }

    if(html && strlen(html))
        {
        char *tmp = cloneString(str);
        char *val = nextWord(&tmp);
        while (val != NULL)
            {
            if(strstrNoCase(html, val) == NULL)
                {
                struct hashEl *el;
                found = FALSE;
                for(el = hashLookup(trackMetadata, track->track); el != NULL; el = hashLookupNext(el))
                    if(sameWord((char *) el->val, val))
                        break;
                found = el != NULL;
                }
            else
                found = TRUE;
            if(found)
                val = nextWord(&tmp);
            else
                break;
            }
        }
    }
return found;
}

static int getTermList(struct sqlConnection *conn, char ***terms, char *type)
{
// Pull out all term fields from ra entries with given type
// Returns count of items found and items via the terms argument.
struct sqlResult *sr = NULL;
char **row = NULL;
char query[256];
struct slName *termList = NULL;
int i, count = 0;
char **retval;

safef(query, sizeof(query), "select distinct val from metaDb where var = '%s'", type);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    slNameAddHead(&termList, row[0]);
    count++;
    }
sqlFreeResult(&sr);
slSort(&termList, slNameCmpCase);
count++; // make room for "Any"
retval = needMem(sizeof(char *) * count);
retval[0] = cloneString(ANYLABEL);
for(i=1; termList != NULL;termList = termList->next, i++)
    {
    retval[i] = cloneString(termList->name);
    }
*terms = retval;
return count;
}

static struct slName *metaDbSearch(struct sqlConnection *conn, char *name, char *val, char *op)
{
// Search the assemblies metaDb table; If name == NULL, we search every metadata field.
struct slName *retval = NULL;
char query[256];
struct sqlResult *sr = NULL;
char **row = NULL;

if(strcmp(op, "contains"))
    if(name == NULL)
        safef(query, sizeof(query), "select distinct obj from metaDb where val = '%s'", val);
    else
        safef(query, sizeof(query), "select obj from metaDb where var = '%s' and val = '%s'", name, val);
else
    if(name == NULL)
        safef(query, sizeof(query), "select obj from metaDb where val like  '%%%s%%'", val);
    else
        safef(query, sizeof(query), "select obj from metaDb where var = '%s' and val like  '%%%s%%'", name, val);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    slNameAddHead(&retval, row[0]);
    }
sqlFreeResult(&sr);
return retval;
}

static int metaDbVars(struct sqlConnection *conn, char *** metaValues)
{
// Search the assemblies metaDb table; If name == NULL, we search every metadata field.
char query[256];
struct sqlResult *sr = NULL;
char **row = NULL;
int i;
struct slName *el, *varList = NULL;
char **retval;

safef(query, sizeof(query), "select distinct var from metaDb");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    slNameAddHead(&varList, row[0]);
sqlFreeResult(&sr);
retval = needMem(sizeof(char *) * slCount(varList));
slNameSort(&varList);
for (el = varList, i = 0; el != NULL; el = el->next, i++)
    retval[i] = el->name;
*metaValues = retval;
return i;
}

void doSearchTracks(struct group *groupList)
{
struct group *group;
char *ops[] = {"is", "contains"};
char *op_labels[] = {"is", "contains"};
char *groups[128];
char *labels[128];
int numGroups = 1;
groups[0] = ANYLABEL;
labels[0] = ANYLABEL;
char *nameSearch = cartOptionalString(cart, "hgt.nameSearch");
char *nameOp = cartOptionalString(cart, "hgt.nameOp");
char *descSearch = cartOptionalString(cart, "hgt.descSearch");
char *groupSearch = cartOptionalString(cart, "hgt.groupSearch");
char *metaName = cartUsualString(cart, "hgt.metaName", "cell");
char *metaOp = cartOptionalString(cart, "hgt.metaOp");
char *metaSearch = cartOptionalString(cart, "hgt.metaSearch");
char *antibodySearch = cartOptionalString(cart, "hgt.antibodySearch");
char **terms;
struct sqlConnection *conn = hAllocConn(database);
boolean metaDbExists = sqlTableExists(conn, "metaDb");
struct slRef *tracks = NULL;
int tracksFound = 0;
struct hash *parents = newHash(4);

getTrackList(&groupList, -2);
slSort(&groupList, gCmpGroup);
for (group = groupList; group != NULL; group = group->next)
    {
    if (group->trackList != NULL)
        {
        groups[numGroups] = cloneString(group->name);
        labels[numGroups] = cloneString(group->label);
        numGroups++;
        if (numGroups >= ArraySize(groups))
            internalErr();
        }
    }

// cartWebStart(cart, database, );
webStartWrapperDetailedNoArgs(cart, database, "", "Track Search (prototype!)", FALSE, FALSE, FALSE, FALSE);

hPrintf("<input type='hidden' name='db' value='%s'>\n", database);

hPrintf("<form action='%s' name='SearchTracks' method='post'>\n\n", hgTracksName());
hPrintf("<table>\n");

hPrintf("<tr><td></td><td><b>Description:</b></td><td>contains</td>\n");
hPrintf("<td><input type='text' name='hgt.descSearch' value='%s' size='80'></td></tr>\n", descSearch == NULL ? "" : descSearch);

hPrintf("<tr><td>and</td><td><b>Track Name:</b></td><td>\n");
cgiMakeDropListFull("hgt.nameOp", op_labels, ops, ArraySize(ops), nameOp == NULL ? "contains" : nameOp, NULL);
hPrintf("</td>\n<td><input type='text' name='hgt.nameSearch' value='%s'></td></tr>\n", nameSearch == NULL ? "" : nameSearch);

hPrintf("<tr><td>and</td>\n");
hPrintf("<td><b>Group</b></td><td>is</td>\n<td>\n");
cgiMakeDropListFull("hgt.groupSearch", labels, groups, numGroups, groupSearch, NULL);
hPrintf("</td></tr>\n");

if(metaDbExists)
    {
    int len;
    char **metaValues = NULL;
    int count = metaDbVars(conn, &metaValues);

    hPrintf("<tr><td>and</td>\n");
    hPrintf("<td><b>Antibody</b></td><td>is</td>\n<td>\n");
    len = getTermList(conn, &terms, "antibody");
    cgiMakeDropListFull("hgt.antibodySearch", terms, terms, len, antibodySearch, NULL);
    hPrintf("</td></tr>\n");

    hPrintf("<tr><td>and</td>\n");
    hPrintf("</td><td>\n");
    cgiMakeDropListClassWithStyleAndJavascript("hgt.metaName", metaValues, count, metaName, 
                                               NULL, NULL, "onchange=metaPulldownChanged(this)");
    hPrintf("</td><td>is</td>\n<td>\n");
    len = getTermList(conn, &terms, metaName);
    cgiMakeDropListFull("hgt.metaSearch", terms, terms, len, metaSearch, NULL);
    hPrintf("</td></tr>\n");
    }

hPrintf("</table>\n");

hPrintf("<input type='submit' name='%s' value='Search'>\n", searchTracks);
hPrintf("<input type='submit' name='submit' value='Cancel'>\n");
hPrintf("</form>\n");

if(descSearch != NULL && !strlen(descSearch))
    descSearch = NULL;
if(groupSearch != NULL && sameString(groupSearch, ANYLABEL))
    groupSearch = NULL;
if(metaSearch != NULL && (!strlen(metaSearch) || sameString(metaSearch, ANYLABEL)))
    metaSearch = NULL;
if(antibodySearch != NULL && sameString(antibodySearch, ANYLABEL))
    antibodySearch = NULL;
if((nameSearch != NULL && strlen(nameSearch)) || descSearch != NULL || groupSearch != NULL || metaSearch != NULL || antibodySearch != NULL)
    {
    // First do the metaDb searches, which can be done quickly for all tracks with db queries.
    struct hash *matchingTracks = newHash(0);
    struct hash *trackMetadata = newHash(0);
    struct slName *el, *metaTracks = NULL;
    boolean checkMeta = FALSE;
    if(antibodySearch != NULL)
        {
        metaTracks = metaDbSearch(conn, "antibody", antibodySearch, "is");
        checkMeta++;
        }
    if(metaSearch != NULL && strlen(metaSearch) && metaName != NULL && strlen(metaName))
        {
        struct slName *tmp = metaDbSearch(conn, metaName, metaSearch, metaOp);
        if(metaTracks == NULL)
            metaTracks = tmp;
        else
            metaTracks = slNameIntersection(metaTracks, tmp);
        checkMeta++;
        }
    for (el = metaTracks; el != NULL; el = el->next)
        hashAddInt(matchingTracks, el->name, 1);

    if(metaDbExists && !isEmpty(descSearch))
        {
        // Load all metadata words for each track to facilitate metadata search.
        char query[256];
        struct sqlResult *sr = NULL;
        char **row;
        safef(query, sizeof(query), "select obj, val from metaDb");
        sr = sqlGetResult(conn, query);
        while ((row = sqlNextRow(sr)) != NULL)
            {
            char *str = cloneString(row[1]);
            hashAdd(trackMetadata, row[0], str);
            }
        sqlFreeResult(&sr);
        }

    for (group = groupList; group != NULL; group = group->next)
        {
        if(groupSearch == NULL || !strcmp(group->name, groupSearch))
            {
            if (group->trackList != NULL)
                {
                struct trackRef *tr;
                for (tr = group->trackList; tr != NULL; tr = tr->next)
                    {
                    struct track *track = tr->track;
                    if((isEmpty(nameSearch) || isNameMatch(track, nameSearch, nameOp)) && 
                       (isEmpty(descSearch) || isDescriptionMatch(track, descSearch, trackMetadata)) &&
                       (!checkMeta || hashLookup(matchingTracks, track->track) != NULL))
                        {
                        tracksFound++;
                        refAdd(&tracks, track);
                        }
                    if (track->subtracks != NULL)
                        {
                        struct track *subTrack;
                        for (subTrack = track->subtracks; subTrack != NULL; subTrack = subTrack->next)
                            {
                            if((isEmpty(nameSearch) || isNameMatch(subTrack, nameSearch, nameOp)) &&
                               (isEmpty(descSearch) || isDescriptionMatch(subTrack, descSearch, trackMetadata)) &&
                               (!checkMeta || hashLookup(matchingTracks, subTrack->track) != NULL))
                                {
                                // XXXX to parent hash. - use tdb->parent instead.
                                hashAdd(parents, subTrack->track, track);
                                tracksFound++;
                                refAdd(&tracks, subTrack);
                                }
                            }                        
                        }
                    }
                }
            }
        }
    slSort(&tracks, gCmpTrack);
    if(tracksFound)
        {
        hPrintf("<p>%d tracks found:</p>\n", tracksFound);
        hPrintf("<form action='%s' name='SearchTracks' method='post'>\n\n", hgTracksName());
        hButton("submit", "save");
        hButtonWithOnClick("hgt.ignoreme", "show all", "show all found tracks", "alert('show all not yet implemented'); return false;");
        hPrintf("<table>\n");
        hPrintf("<tr bgcolor='#666666'><td><b>Name</b></td><td><b>Description</b></td><td><b>Group</b></td><td><br /></td></tr>\n");
        struct slRef *ptr;
        while((ptr = slPopHead(&tracks)))
            {
            struct track *track = (struct track *) ptr->val;
            // trackDbOutput(track->tdb, stderr, ',', '\n');
            hPrintf("<tr bgcolor='#EEEEEE'>\n");
            hPrintf("<td>%s</td>\n", track->shortLabel);
            hPrintf("<td><a target='_top' href='%s'>%s</a></td>\n", trackUrl(track->track, NULL), track->longLabel);
            // How do we get subtrack's parent?
            struct track *parent = NULL;
            if(hashLookup(parents, track->track) != NULL)
                parent = (struct track *) hashLookup(parents, track->track)->val;
            hPrintf("<td>%s</td>\n", parent != NULL ? parent->longLabel : track->group != NULL ? track->group->label : "");
            hPrintf("<td>\n");
            if (tdbIsSuper(track->tdb))
                {
                superTrackDropDown(cart, track->tdb,
                                   superTrackHasVisibleMembers(track->tdb));
                }
            else
                {
                hTvDropDownClassVisOnly(track->track, track->visibility,
                                        track->canPack, (track->visibility == tvHide) ? 
                                        "hiddenText" : "normalText", 
                                        trackDbSetting(track->tdb, "onlyVisibility"));
                }
            hPrintf("</td></tr>\n");
            }
        hPrintf("</table>\n");
        hButton("submit", "save");
        hPrintf("</form>\n");
        } 
    else
        {
        hPrintf("<p>No tracks found</p>\n");
        }
    }
}
