/*****************************************************************************
 * Copyright (C) 2000 Jim Kent.  This source code may be freely used         *
 * for personal, academic, and non-profit purposes.  Commercial use          *
 * permitted only by explicit agreement with Jim Kent (jim_kent@pacbell.net) *
 *****************************************************************************/
/* Hash - a simple hash table that provides name/value pairs */
#ifndef HASH_H
#define HASH_H

struct hashEl
    {
    struct hashEl *next;
    char *name;
    void *val;
    };

struct hash
    {
    bits32 mask;
    struct hashEl **table;
    int powerOfTwoSize;
    int size;
    struct lm *lm;
    };

bits32 hashCrc(char *string);
/* Returns a CRC value on string. */

struct hashEl *hashLookup(struct hash *hash, char *name);
/* Looks for name in hash table. Returns associated element,
 * if found, or NULL if not. */

struct hashEl *hashAdd(struct hash *hash, char *name, void *val);
/* Add new element to hash table. */

struct hashEl *hashAddUnique(struct hash *hash, char *name, void *val);
/* Add new element to hash table. Squawk and die if is already in table. */

struct hashEl *hashAddSaveName(struct hash *hash, char *name, void *val, char **saveName);
/* Add new element to hash table.  Save the name of the element, which is now
 * allocated in the hash table, to *saveName.  A typical usage would be:
 *    AllocVar(el);
 *    hashAddSaveName(hash, name, el, &el->name);
 */

struct hashEl *hashStore(struct hash *hash, char *name);
/* If element in hash already return it, otherwise add it
 * and return it. */

char  *hashStoreName(struct hash *hash, char *name);
/* Put name into hash table. */

char *hashMustFindName(struct hash *hash, char *name);
/* Return name as stored in hash table (in hel->name). 
 * Abort if not found. */

void *hashMustFindVal(struct hash *hash, char *name);
/* Lookup name in hash and return val.  Abort if not found. */

void *hashFindVal(struct hash *hash, char *name);
/* Look up name in hash and return val or NULL if not found. */

void hashTraverseEls(struct hash *hash, void (*func)(struct hashEl *hel));
/* Apply func to every element of hash with hashEl as parameter. */

void hashTraverseVals(struct hash *hash, void (*func)(void *val));
/* Apply func to every element of hash with hashEl->val as parameter. */

struct hash *newHash(int powerOfTwoSize);
/* Returns new hash table. */

void freeHash(struct hash **pHash);
/* Free up hash table. */

void freeHashAndVals(struct hash **pHash);
/* Free up hash table and all values associated with it.
 * (Just calls freeMem on each hel->val) */

#endif /* HASH_H */

