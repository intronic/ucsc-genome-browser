/* spaceSaver - routines that help layout 1-D objects into a
 * minimum number of tracks so that no two objects overlap
 * within a single track. */
#ifndef SPACESAVER_H
#define SPACESAVER_H

struct spaceSaver
/* Help layout 1-D objects onto multiple tracks so that
 * no two objects overlap on a single track. */
    {
    struct spaceSaver *next;	/* Next in list. */
    struct spaceNode *nodeList; /* List of things put in space saver. */
    struct spaceRowTracker *rowList; /* List of rows. */
    int rowCount;              /* Number of rows. */
    int winStart,winEnd;	/* Start and end of area we're modeling. */
    int cellsInRow;             /* Number of cells per row. */
    float scale;                /* What to scale by to get to cell coordinates. */
    };

struct spaceNode
/* Which row is this one on? */
    {
    struct spaceNode *next;	/* Next in list. */
    int row;			/* Which row, starting at zero. */
    void *val;
    };

struct spaceRowTracker 
/* Keeps track of how much of row is used. */
    {
    struct spaceRowTracker *next;	/* Next in list. */
    bool *used;                 /* A flag for each spot used. */
    };

struct spaceSaver *spaceSaverNew(int winStart, int winEnd);
/* Create a new space saver around the given window.   */

void spaceSaverFree(struct spaceSaver **pSs);
/* Free up a space saver. */

struct spaceNode *spaceSaverAdd(struct spaceSaver *ss, int start, int end, void *val);
/* Add a new node to space saver. */

void spaceSaverFinish(struct spaceSaver *ss);
/* Tell spaceSaver done adding nodes. */
#endif /* SPACESAVER_H */

