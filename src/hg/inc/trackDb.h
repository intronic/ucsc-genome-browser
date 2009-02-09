/* trackDb.h was originally generated by the autoSql program, which also
 * generated trackDb.c and trackDb.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef TRACKDB_H
#define TRACKDB_H

#include "common.h"
#ifndef JKSQL_H
#include "jksql.h"
#endif

#define TRACKDB_NUM_COLS 21

struct trackDb
/* This describes an annotation track. */
    {
    struct trackDb *next;  /* Next in singly linked list. */
    char *tableName;	/* Symbolic ID of Track */
    char *shortLabel;	/* Short label displayed on left */
    char *type;	/* Track type: bed, psl, genePred, etc. */
    char *longLabel;	/* Long label displayed in middle */
    unsigned char visibility;	/* 0=hide, 1=dense, 2=full, 3=pack, 4=squish */
    float priority;	/* 0-100 - where to position.  0 is top */
    unsigned char colorR;	/* Color red component 0-255 */
    unsigned char colorG;	/* Color green component 0-255 */
    unsigned char colorB;	/* Color blue component 0-255 */
    unsigned char altColorR;	/* Light color red component 0-255 */
    unsigned char altColorG;	/* Light color green component 0-255 */
    unsigned char altColorB;	/* Light color blue component 0-255 */
    unsigned char useScore;	/* 1 if use score, 0 if not */
#ifndef	__cplusplus
    unsigned char private;	/* 1 if only want to show it on test site */
#else
    unsigned char priv;		/* don't conflict with C++ keyword */
#endif
    int restrictCount;	/* Number of chromosomes this is on (0=all though!) */
    char **restrictList;	/* List of chromosomes this is on */
    char *url;	/* URL to link to when they click on an item */
    char *html;	/* Some html to display when they click on an item */
    char *grp;	/* Which group track belongs to */
    unsigned char canPack;	/* 1 if can pack track display, 0 otherwise */
    char *settings;	/* Name/value pairs for track-specific stuff */
    struct hash *settingsHash;  /* Hash for settings. Not saved in database.
                                 * Don't use directly, rely on trackDbSetting to access. */
    /* additional info, determined from settings */
    char treeNodeType;          /* bit map containing defining supertrack, composite and children of same (may be parent & child) */
    struct trackDb *parent;     /* parent of composite or superTracks */
    struct trackDb *subtracks;  /* children of composite (TODO: or supertrack) */ // NOTE: can only be on one sl at a time!
    char *parentName;           /* set if this is a supertrack member */
    boolean isShow;             /* for supertracks tracks: true if this is a supertrack with pseudo-vis 'show' */
    struct hash *overrides;     /* If not NULL, this is an override
                                 * entry.  It contains the names, but not the
                                 * values of the fields and settings that were
                                 * specified in the entry. */
    };

#define SUPERTRACK_MASK                 0x10
#define COMPOSITE_MASK                  0x20
#define SUPERTRACK_CHILD_MASK           0x01
#define COMPOSITE_CHILD_MASK            0x02
#define PARENT_MASK                     0xF0
#define CHILD_MASK                      0x0F
#define TREETYPE_MASK                   0xFF
#define PARENT_NODE(nodeType)           ((nodeType) & PARENT_MASK)
#define CHILD_NODE(nodeType)            ((nodeType) & CHILD_MASK)
#define SUPERTRACK_NODE(nodeType)       (((nodeType) & SUPERTRACK_MASK) == SUPERTRACK_MASK)
#define COMPOSITE_NODE(nodeType)        (((nodeType) & COMPOSITE_MASK ) == COMPOSITE_MASK )
#define SUPERTRACK_CHILD_NODE(nodeType) (((nodeType) & SUPERTRACK_CHILD_MASK) == SUPERTRACK_CHILD_MASK)
#define COMPOSITE_CHILD_NODE(nodeType)  (((nodeType) & COMPOSITE_CHILD_MASK ) == COMPOSITE_CHILD_MASK )
#define INDEPENDENT_NODE(nodeType)      (((nodeType) & TREETYPE_MASK ) == 0 )
//#define tdbIsParent(tdb)              ((tdb)->subtracks)
//#define tdbIsChild(tdb)               ((tdb)->parent   )
//#define tdbIsTreeLeaf(tdb)            ( CHILD_NODE((tdb)->treeNodeType) && !tdbIsParent(tdb))
//#define tdbIsTreeRoot(tdb)            (PARENT_NODE((tdb)->treeNodeType) && !tdbIsChild(tdb) )
//#define tdbIsTreeBranch(tdb)          (!INDEPENDENT_NODE((tdb)->treeNodeType) &&  tdbIsParent(tdb) &&  tdbIsChild(tdb))
//#define tdbIsNotInTree(tdb)           ( INDEPENDENT_NODE((tdb)->treeNodeType) && !tdbIsParent(tdb) && !tdbIsChild(tdb))

INLINE boolean tdbIsSuper(struct trackDb *tdb)
/* Is this trackDb struct marked as a supertrack ? */
{
return tdb && SUPERTRACK_NODE(tdb->treeNodeType);
}

INLINE boolean tdbIsSuperTrack(struct trackDb *tdb)
/* Is this trackDb struct marked as a supertrack ? */
{
return tdb && /*tdb->subtracks &&*/ SUPERTRACK_NODE(tdb->treeNodeType); // TODO: superTrack code needs rewrite to contain it's children
}

INLINE boolean tdbIsComposite( struct trackDb *tdb)
/* Is this trackDb struct marked as a composite with children ?  */
{
return tdb && tdb->subtracks && COMPOSITE_NODE( tdb->treeNodeType);
}

INLINE boolean tdbIsSuperTrackChild(struct trackDb *tdb)
/* Is this trackDb struct marked as a child of a supertrack ?  */
{
return tdb && tdb->parent && SUPERTRACK_CHILD_NODE(tdb->treeNodeType);
}

INLINE boolean tdbIsCompositeChild(struct trackDb *tdb)
/* Is this trackDb struct marked as a child of a composite track ?  */
{
return tdb && tdb->parent && COMPOSITE_CHILD_NODE( tdb->treeNodeType);
}

INLINE void tdbMarkAsSuperTrack(struct trackDb *tdb)
/* Marks a trackDb struct as a supertrack */
{
tdb->treeNodeType |= SUPERTRACK_MASK;
}

INLINE void tdbMarkAsComposite( struct trackDb *tdb)
/* Marks a trackDb struct as a composite track  */
{
tdb->treeNodeType |= COMPOSITE_MASK;
}

INLINE void tdbMarkAsSuperTrackChild(struct trackDb *tdb)
/* Marks a trackDb struct as a child of a supertrack  */
{
tdb->treeNodeType |= SUPERTRACK_CHILD_MASK;
}

INLINE void tdbMarkAsCompositeChild( struct trackDb *tdb)
/* Marks a trackDb struct as a child of a composite track  */
{
tdb->treeNodeType |= COMPOSITE_CHILD_MASK;
}

struct trackDb *trackDbLoad(char **row);
/* Load a trackDb from row fetched with select * from trackDb
 * from database.  Dispose of this with trackDbFree(). */

struct trackDb *trackDbLoadAll(char *fileName);
/* Load all trackDb from whitespace-separated file.
 * Dispose of this with trackDbFreeList(). */

struct trackDb *trackDbLoadWhere(struct sqlConnection *conn, char *table,
	char *where);
/* Load all trackDb from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with trackDbFreeList(). */

struct trackDb *trackDbLoadAllByChar(char *fileName, char chopper);
/* Load all trackDb from chopper separated file.
 * Dispose of this with trackDbFreeList(). */

#define trackDbLoadAllByTab(a) trackDbLoadAllByChar(a, '\t');
/* Load all trackDb from tab separated file.
 * Dispose of this with trackDbFreeList(). */

struct trackDb *trackDbCommaIn(char **pS, struct trackDb *ret);
/* Create a trackDb out of a comma separated string.
 * This will fill in ret if non-null, otherwise will
 * return a new trackDb */

void trackDbFree(struct trackDb **pEl);
/* Free a single dynamically allocated trackDb such as created
 * with trackDbLoad(). */

void trackDbFreeList(struct trackDb **pList);
/* Free a list of dynamically allocated trackDb's */

void trackDbOutput(struct trackDb *el, FILE *f, char sep, char lastSep);
/* Print out trackDb.  Separate fields with sep. Follow last field with lastSep. */

#define trackDbTabOut(el,f) trackDbOutput(el,f,'\t','\n');
/* Print out trackDb as a line in a tab-separated file. */

#define trackDbCommaOut(el,f) trackDbOutput(el,f,',',',');
/* Print out trackDb as a comma separated list including final comma. */

/* ----------- End of AutoSQL generated code --------------------- */

int trackDbCmp(const void *va, const void *vb);
/* Sort track by priority. */

void trackDbOverrideVisbility(struct hash *tdHash, char *visibilityRa,
			      boolean hideFirst);
/* Override visbility settings using a ra file.  If hideFirst, set all
 * visibilities to hide before applying visibilityRa. */

void trackDbOverridePriority(struct hash *tdHash, char *priorityRa);
/* Override priority settings using a ra file. */

struct trackDb *trackDbFromRa(char *raFile);
/* Load track info from ra file into list. */

void trackDbPolish(struct trackDb *bt);
/* Fill in missing values with defaults. */

struct hash *trackDbHashSettings(struct trackDb *tdb);
/* Force trackDb to hash up it's settings.  Usually this is just
 * done on demand. Returns settings hash. */

char *trackDbSetting(struct trackDb *tdb, char *name);
/* Return setting string or NULL if none exists. */

boolean trackDbSettingOn(struct trackDb *tdb, char *name);
/* Return true if a tdb setting is "on" "true" or "enabled". */

char *trackDbRequiredSetting(struct trackDb *tdb, char *name);
/* Return setting string or squawk and die. */

char *trackDbSettingOrDefault(struct trackDb *tdb, char *name, char *defaultVal);
/* Return setting string, or defaultVal if none exists */

struct hashEl *trackDbSettingsLike(struct trackDb *tdb, char *wildStr);
/* Return a list of settings whose names match wildStr (may contain wildcard
 * characters).  Free the result with hashElFreeList. */

char *trackDbGetSupertrackName(struct trackDb *tdb);
/* Find name of supertrack if this track is a member */

float trackDbFloatSettingOrDefault(struct trackDb *tdb, char *name, float defaultVal);
/* Return setting, convert to a float, or defaultVal if none exists */

void trackDbSuperMemberSettings(struct trackDb *tdb);
/* Set fields in trackDb to indicate this is a member of a
 * supertrack. */

void trackDbSuperSettings(struct trackDb *tdbList);
/* Get info from supertrack setting.  There are 2 forms:
 * Parent:   'supertrack on [show]'
 * Child:    'supertrack <parent> [vis]
 * Returns NULL if there is no such setting */

char *trackDbInclude(char *raFile, char *line);
/* Get include filename from trackDb line.
   Return NULL if line doesn't contain include */

char *trackDbOrigAssembly(struct trackDb *tdb);
/* return setting from trackDb, if any */

void trackDbPrintOrigAssembly(struct trackDb *tdb, char *database);
/* Print lift information from trackDb, if any */

// Not all track types have separate configuration
typedef enum _eCfgType
{
    cfgNone     =0,
    cfgBedScore =1,
    cfgWig      =2,
    cfgWigMaf   =3,
    cfgPeak     =4,
    cfgGenePred =5
} eCfgType;

eCfgType cfgTypeFromTdb(struct trackDb *tdb, boolean warnIfNecessary);
/* determine what kind of track specific configuration is needed,
   warn if not multi-view compatible */

void trackDbOverride(struct trackDb *td, struct trackDb *overTd);
/* apply an trackOverride trackDb entry to a trackDb entry */

char *trackDbCompositeSettingByView(struct trackDb *parentTdb, char* view, char *name);
/* Get a trackDb setting at the view level for a multiview composite.
   returns a string that must be freed */

char *trackDbSettingByView(struct trackDb *tdb, char *name);
/* For a subtrack of a multiview composite, get a setting stored in the parent settingByView.
   returns a string that must be freed */

char *trackDbSettingClosestToHome(struct trackDb *tdb, char *name);
/* Look for a trackDb setting from lowest level on up:
   from subtrack, then composite, then settingsByView, then composite */

char *trackDbSettingClosestToHomeOrDefault(struct trackDb *tdb, char *name, char *defaultVal);
/* Look for a trackDb setting (or default) from lowest level on up:
   from subtrack, then composite, then settingsByView, then composite */

boolean trackDbSettingClosestToHomeOn(struct trackDb *tdb, char *name);
/* Return true if a tdb setting closest to home is "on" "true" or "enabled". */

#endif /* TRACKDB_H */

