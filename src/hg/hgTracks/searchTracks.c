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
#include "versionInfo.h"
#include "web.h"
#include "cds.h"

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

static boolean isDescriptionMatch(struct track *track, char *str, char *op)
{
// XXXX obviously very primitive; s/d ignore html markup and do stemming etc.
char *html = track->tdb->html;
if((!html || !strlen(html)) && track->tdb->parent)
    {
    // XXXX is there a cleaner way to find parent?
    html = track->tdb->parent->html;
    }

// fprintf(stderr, "description: %s; %s\n", str, html);
return str && strlen(str) && sameString(op, "contains") && (strstrNoCase(html, str) != NULL);
}

static boolean isMetaMatch(struct track *track, char *name, char *op, char *val)
{
boolean retVal = FALSE;
char *setting = NULL;
char buf[100];
char *metadata = trackDbSetting(track->tdb, "metadata");
if(metadata)
    {
    char *ptr;
    safef(buf, sizeof(buf), "%s=", name);
    ptr = strstr(metadata, buf);
    if(ptr)
        {
        ptr += strlen(buf);
        setting = ptr;
        ptr = skipToSpaces(ptr);
        if(*ptr)
            *ptr = 0;
        }
    }
// fprintf(stderr, "%s; setting: %s\n", track->mapName, setting);
retVal = setting && strlen(setting) &&
    ((sameString(op, "is") && !strcasecmp(setting, val)) ||
     (sameString(op, "contains") && containsStringNoCase(setting, val) != NULL));
return retVal;
}

void doSearchTracks(struct group *groupList)
{
struct group *group;
char *ops[] = {"is", "contains"};
char *op_labels[] = {"is", "contains"};
char *metaNames[] = {"antibody", "cell line"};
char *metaValues[] = {"antibody", "cell"};
char *groups[128];
char *labels[128];
int numGroups = 1;
groups[0] = "any";
labels[0] = "Any";
char *nameSearch = cartOptionalString(cart, "hgt.nameSearch");
char *nameOp = cartOptionalString(cart, "hgt.nameOp");
char *descSearch = cartOptionalString(cart, "hgt.descSearch");
char *descOp = cartOptionalString(cart, "hgt.descOp");
char *groupSearch = cartOptionalString(cart, "hgt.groupSearch");
char *metaName = cartOptionalString(cart, "hgt.metaName");
char *metaOp = cartOptionalString(cart, "hgt.metaOp");
char *metaSearch = cartOptionalString(cart, "hgt.metaSearch");

// struct track *trackList = 
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

cartWebStart(cart, database, "Search/Select Annotation Tracks (very rough prototype!)");

hPrintf("<form action='%s' name='SearchTracks' method='post'>\n\n", hgTracksName());
hPrintf("<table>\n");

hPrintf("<tr><td></td><td><b>Name:</b></td><td>\n");
cgiMakeDropListFull("hgt.nameOp", op_labels, ops, ArraySize(ops), nameOp == NULL ? "contains" : nameOp, NULL);
hPrintf("</td>\n<td><input type='text' name='hgt.nameSearch' value='%s'></td></tr>\n", nameSearch == NULL ? "" : nameSearch);

hPrintf("<tr><td>and</td><td><b>Description:</b></td><td>\n");
cgiMakeDropListFull("hgt.descOp", op_labels, ops, ArraySize(ops), descOp == NULL ? "contains" : descOp, NULL);
hPrintf("</td>\n<td><input type='text' name='hgt.descSearch' value='%s'></td></tr>\n", descSearch == NULL ? "" : descSearch);

hPrintf("<tr><td>and</td>\n");
hPrintf("<td><b>Group</b></td><td>is</td>\n<td>\n");

cgiMakeDropListFull("hgt.groupSearch", labels, groups, numGroups, groupSearch, NULL);

hPrintf("</td></tr>\n");

hPrintf("<tr><td>and</td><td>\n");
cgiMakeDropListFull("hgt.metaName", metaNames, metaValues, ArraySize(metaNames), metaName, NULL);
hPrintf("</td><td>\n");
cgiMakeDropListFull("hgt.metaOp", op_labels, ops, ArraySize(ops), metaOp == NULL ? "contains" : metaOp, NULL);
hPrintf("</td><td>\n");
hPrintf("<input type='text' name='hgt.metaSearch' value='%s'></td></tr>\n", metaSearch == NULL ? "" : metaSearch);
hPrintf("</td></tr>\n");

hPrintf("</table>\n");

hPrintf("<input type='submit' name='%s' value='Search'>\n", searchTracks);
hPrintf("<input type='submit' name='submit' value='Cancel'>\n");
hPrintf("</form>\n");

struct slRef *tracks = NULL;
int tracksFound = 0;
struct hash *parents = newHash(4);
if(groupSearch != NULL && sameString(groupSearch, "any"))
    {
    groupSearch = NULL;
    }
if(metaSearch != NULL && !strlen(metaSearch))
    {
    metaSearch = NULL;
    }
if((nameSearch != NULL && strlen(nameSearch)) || descSearch != NULL || groupSearch != NULL || metaSearch != NULL)
    {
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
                       (isEmpty(descSearch) || isDescriptionMatch(track, descSearch, descOp)) &&
                       (isEmpty(metaName) || isEmpty(metaSearch) || isMetaMatch(track, metaName, metaOp, metaSearch)))
                        {
                        tracksFound++;
                        if(tracks == NULL)
                            tracks = slRefNew(track);
                        else 
                            refAdd(&tracks, track);
                        }
                    if (track->subtracks != NULL)
                        {
                        struct track *subTrack;
                        for (subTrack = track->subtracks; subTrack != NULL; subTrack = subTrack->next)
                            {
//                            fprintf(stderr, "search track: %s\n", subTrack->shortLabel);
                            if((isEmpty(nameSearch) || isNameMatch(subTrack, nameSearch, nameOp)) &&
                               (isEmpty(descSearch) || isDescriptionMatch(subTrack, descSearch, descOp)) &&
                               (isEmpty(metaName) || isEmpty(metaSearch) || isMetaMatch(subTrack, metaName, metaOp, metaSearch)))
                                {
                                // XXXX to parent hash. - use tdb->parent instead.
                                hashAdd(parents, subTrack->mapName, track);
//                                fprintf(stderr, "found subtrack: %s\n", subTrack->shortLabel);
                                tracksFound++;
                                if(tracks == NULL)
                                    tracks = slRefNew(subTrack);
                                else 
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
        hPrintf("<table>\n");
        hPrintf("<tr bgcolor='#666666'><td><b>Name</b></td><td><b>Description</b></td><td><b>Group</b></td><td><br /></td></tr>\n");
        struct slRef *ptr;
        while((ptr = slPopHead(&tracks)))
            {
            struct track *track = (struct track *) ptr->val;
            // trackDbOutput(track->tdb, stderr, ',', '\n');
            hPrintf("<tr bgcolor='#EEEEEE'>\n");
            hPrintf("<td>%s</td>\n", track->shortLabel);
            hPrintf("<td>%s</td>\n", track->longLabel);
            // How do we get subtrack's parent?
            struct track *parent = NULL;
            if(hashLookup(parents, track->mapName) != NULL)
                parent = (struct track *) hashLookup(parents, track->mapName)->val;
            hPrintf("<td>%s</td>\n", parent != NULL ? parent->longLabel : track->group != NULL ? track->group->label : "");
            hPrintf("<td>\n");
            if (tdbIsSuper(track->tdb))
                {
                superTrackDropDown(cart, track->tdb,
                                   superTrackHasVisibleMembers(track->tdb));
                }
            else
                {
                hTvDropDownClassVisOnly(track->mapName, track->visibility,
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
