/* A more-or-less generic container class that hopefully will someday encompass
 * the superTrack and subTrack systems.  Tracks that have the tag "container"
 * get sent here for processing. */

#include "common.h"
#include "hash.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "hgTracks.h"
#include "container.h"


static void containerLoad(struct track *track)
/* containerLoad - call load routine on all children. This one is generic for all containers. */
{
struct track *subtrack;
for (subtrack = track->subtracks; subtrack != NULL; subtrack = subtrack->next)
    subtrack->loadItems(subtrack);
}

static void containerFree(struct track *track)
/* containerFree - call free routine on all children. */
{
struct track *subtrack;
for (subtrack = track->subtracks; subtrack != NULL; subtrack = subtrack->next)
    subtrack->freeItems(subtrack);
}

static void containerDraw(struct track *track, int seqStart, int seqEnd,
        struct hvGfx *hvg, int xOff, int yOff, int width, 
        MgFont *font, Color color, enum trackVisibility vis)
/* Draw items in container. */
{
struct track *subtrack;
int y = yOff;
for (subtrack = track->subtracks; subtrack != NULL; subtrack = subtrack->next)
    {
    if (isSubtrackVisible(subtrack))
	{
	subtrack->drawItems(subtrack, seqStart, seqEnd, hvg, xOff, y, width, font, color, vis);
	y += subtrack->totalHeight(subtrack, subtrack->limitedVis);
	}
    }
}

static int containerTotalHeight(struct track *track, enum trackVisibility vis)
/* Return total height of container. */
{
int totalHeight = 0;
struct track *subtrack;
for (subtrack = track->subtracks; subtrack != NULL; subtrack = subtrack->next)
    {
    if (isSubtrackVisible(subtrack))
	{
	totalHeight += subtrack->totalHeight(subtrack, vis);
	track->lineHeight = subtrack->lineHeight;
	track->heightPer = subtrack->heightPer;
	}
    }
track->height = totalHeight;
return totalHeight;
}

void makeContainerTrack(struct track *track, struct trackDb *tdb)
/* Construct track subtrack list from trackDb entry for container tracks. */
{
/* Wrap tracks around child tdb's, maintaining same heirarchy as in tdb. */
struct trackDb *subtdb;
for (subtdb = tdb->subtracks; subtdb != NULL; subtdb = subtdb->next)
    {
    struct track *subtrack = trackFromTrackDb(subtdb);
    TrackHandler handler = lookupTrackHandler(subtdb->tableName);
    if (handler != NULL)
	handler(subtrack);
    slAddHead(&track->subtracks, subtrack);
    if (subtdb->subtracks != NULL)
	makeContainerTrack(subtrack, subtdb);
    }
slSort(&track->subtracks, trackPriCmp);

/* Set methods that may be shared by all containers. */
track->loadItems = containerLoad;
track->freeItems = containerFree;
track->drawItems = containerDraw;
track->totalHeight = containerTotalHeight;

/* Set methods specific to containers. */
char *containerType = trackDbSetting(tdb, "container");
if (sameString(containerType, "multiWig"))
    {
    multiWigContainerMethods(track);
    }
else if (sameString(containerType, "folder"))
    {
    /* Folder's just use the default methods. */
    }
else
    errAbort("unknown container type %s in trackDb for %s", containerType, tdb->tableName);
}


