/* pfType - ParaFlow type heirarchy and type checking. */
/* Copyright 2005 Jim Kent.  All rights reserved. */

#ifndef PFTYPE_H
#define PFTYPE_H

#ifndef PFSCOPE_H
#include "pfScope.h"
#endif

struct pfType;

struct pfBaseType
/* A type, may have parents in object heirarchy, and
 * be composed of multiple elements. */
    {
    struct pfBaseType *next;		/* Sibling in class heirarchy */
    struct pfBaseType *children;	/* Derived classes. */
    struct pfBaseType *parent;		/* Pointer to parent class if any */
    struct pfBaseType *keyedBy;	/* Pointer to class of index if any */
    char *name;			/* Type name.  Allocated in hash. */
    struct pfScope *scope;	/* The scope this class lives in */
    struct pfParse *def;	/* Parse node for class definition */
    bool isCollection;		/* TRUE if it's a collection type */
    bool isClass;		/* TRUE if it's a class */
    bool needsCleanup;		/* TRUE if it needs refCount/cleanup */
    bool reserved;		/* Currently zero. */
    int id;			/* Unique id. */
    struct pfType *fields;	/* List of (data) fields. */
    struct pfType *methods;	/* List of associated functions. */
    int size;			/* Type size. */
    short selfPolyCount;	/* Count of polymorphic functions. */
    struct pfPolyFunRef *polyList;	/* List of polymorphic functions
                                           (Including in parents) */
    };

struct pfBaseType *pfBaseTypeNew(struct pfScope *scope, char *name, 
	boolean isCollection, struct pfBaseType *parent, int size, 
	boolean needsCleanup);
/* Create new base type. */

int pfBaseTypeCount();
/* Return base type count. */

struct pfPolyFunRef
/* A reference to a polymorphic function.  This helps
 * us sort out whether to use functions from the base
 * class or from the current class. */
    {
    struct pfPolyFunRef *next;
    struct pfBaseType *class;	/* Class defined in. */
    struct pfType *method;	/* Method field. */
    };

enum pfTyty
/* The overall type of a specific type. */
    {
    tytyVariable,
    tytyTuple,
    tytyFunction,
    tytyVirtualFunction,
    tytyModule,
    };

struct pfType
/* A type tree that represents typed tuples and collections. 
 * The class heirarchy is not here, but instead in pfBaseType. */
    {
    struct pfType *next;	/* Next sibling. */
    struct pfType *children;	/* Children. */
    struct pfBaseType *base;	/* Type of this node in type tree. */
    char *fieldName;		/* Field name associated with this node. */
    struct pfParse *init;	/* Initialization if any. */
    UBYTE tyty;			/* Some PfTyty value. */
    bool isStatic;		/* True if it's a static variable. */
    short polyOffset;		/* Offset in polymorphic function table. */
    };

struct pfType *pfTypeNew(struct pfBaseType *base);
/* Create new high level type object from base type.  Pass in NULL to
 * create a tuple. */

void pfTypeDump(struct pfType *ty, FILE *f);
/* Write out info on ty to file.  (No newlines written) */

boolean pfTypeSame(struct pfType *a, struct pfType *b);
/* Return TRUE if a and b are same type logically */

struct pfCompile;
struct pfParse;

void pfTypeOnTuple(struct pfCompile *compile, struct pfParse *pp);
/* Create tuple type and link in types of all children. */

void pfTypeCheck(struct pfCompile *pfc, struct pfParse **pPp);
/* Check types (adding conversions where needed) on tree,
 * which should have variables bound already. */

#endif /* PFTYPE_H */
