/* LocalMem.h - local memory routines. 
 * 
 * These routines are meant for the sort of scenario where
 * a lot of little to medium size pieces of memory are
 * allocated, and then disposed of all at once.
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

struct lm *lmInit(int blockSize);
/* Create a local memory pool. Parameters are:
 *      blockSize - how much system memory to allocate at a time.  Can
 *                  pass in zero and a reasonable default will be used.
 */

void lmCleanup(struct lm **pLm);
/* Clean up a local memory pool. */

void *lmAlloc(struct lm *lm, size_t size);
/* Allocate memory from local pool. */

char *lmCloneString(struct lm *lm, char *string);
/* Return local mem copy of string. */

void *lmCloneMem(struct lm *lm, void *pt, size_t size);
/* Return a local mem copy of memory block. */

#define lmAllocVar(lm, pt) (pt = lmAlloc(lm, sizeof(*pt)));
/* Shortcut to allocating a single variable in local mem
 * assigning pointer to it. */
