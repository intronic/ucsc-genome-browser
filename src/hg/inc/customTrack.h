/* Data structure for dealing with custom tracks in the browser. */
#ifndef CUSTOMTRACK_H
#define CUSTOMTRACK_H

#ifndef BED_H
#include "bed.h"
#endif

#ifndef BROWSERTABLE_H
#include "browserTable.h"
#endif

struct customTrack
/* A custom track. */
    {
    struct customTrack *next;	/* Next in list. */
    struct browserTable *bt;	/* Browser table description of track. */
    struct bed *bedList;	/* List of beds. */
    int fieldCount;		/* Number of fields in bed. */
    boolean needsLift;		/* True if coordinates need lifting. */
    };

struct customTrack *customTracksFromText(char *text);
/* Parse text into a custom set of tracks. */

struct customTrack *customTracksFromFile(char *text);
/* Parse file into a custom set of tracks. */

struct customTrack *customTracksParse(char *text, boolean isFile);
/* Parse text into a custom set of tracks.  Text parameter is a
 * file name if 'isFile' is set.*/

void customTrackSave(struct customTrack *trackList, char *fileName);
/* Save out custom tracks. */

void customTrackLift(struct customTrack *trackList, struct hash *ctgPosHash);
/* Lift tracks based on hash of ctgPos. */

boolean customTrackNeedsLift(struct customTrack *trackList);
/* Return TRUE if any track in list needs a lift. */

struct bed *customTrackBed(char *row[13], int wordCount, struct hash *chromHash, int lineIx);
/* Convert a row of strings to a bed. */

boolean customTrackTest();
/* Tests module - returns FALSE and prints warning message on failure. */

#endif
