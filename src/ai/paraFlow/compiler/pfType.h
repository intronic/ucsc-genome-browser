/* pfType - ParaFlow type heirarchy */
#ifndef PFTYPE_H
#define PFTYPE_H

struct pfBaseType
/* A type, may have parents in object heirarchy, and
 * be composed of multiple elements. */
    {
    struct pfType *next;
    char *name;			/* Type name.  Allocated in hash. */
    struct pfScope *scope;	/* The scope this class lives in */
    struct pfBaseType *parentType;	/* Pointer to parent class if any */
    struct pfCollectedType *elType;	/* Element type for collections. */
    struct pfDef *components;	/* Component elements of this class */
    bool isCollection;		/* TRUE if it's a collection type */
    };


struct pfCollectedType
/* A type, which may involve collections. */
    {
    struct pfCollectedType *next;
    struct pfBaseType *base;		/* Base type. */
    };

void pfCollectedTypeDump(struct pfCollectedType *ct, FILE *f);
/* Write out info on ct to file. */

#endif /* PFTYPE_H */
