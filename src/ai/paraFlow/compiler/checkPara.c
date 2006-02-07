/* checkPara - Stuff to check that flow and para statements
 * are good in the sense that there are no inappropriate writes
 * to variables. */
/* Copyright 2005-6 Jim Kent.  All rights reserved. */

#include "common.h"
#include "hash.h"
#include "pfParse.h"
#include "pfCompile.h"
#include "pfType.h"
#include "checkPara.h"

static boolean enclosedScope(struct pfScope *outer, struct pfScope *inner)
/* Return true if inner is the same or is inside of outer. */
{
for ( ; inner != NULL; inner = inner->parent)
    if (inner == outer)
       return TRUE;
return FALSE;
}

static boolean isOutsideObj(struct hash *outputVars, struct pfScope *localScope,
	struct pfParse *pp)
/* Return TRUE if pp (which must be of type pptVarUse) is outside of local
 * scope, and also outside the outputVars hash (which typically is used for
 * a function's return values. */
{
return !(enclosedScope(localScope, pp->var->scope) || hashLookup(outputVars, pp->name));
}

static void checkLocal(struct hash *outputVars, struct pfScope *localScope,
	struct pfParse *pp)
/* Make sure that any pptVarUse's are local or function calls */
{
switch (pp->type)
    {
    case pptVarUse:
	{
	if (pp->var->paraTainted)
	    errAt(pp->tok, "%s may contain a non-local value, so you can no longer write to it here",
	    	pp->name); 
	if (isOutsideObj(outputVars, localScope, pp))
	    errAt(pp->tok, "write to non-local variable illegal in this context");
        break;
	}
    }
for (pp = pp->children; pp != NULL; pp = pp->next)
    checkLocal(outputVars, localScope, pp);
}

struct pfParse *firstVarInTree(struct pfParse *pp)
/* Return first variable use in parse tree */
{
struct pfParse *v;
if (pp->type == pptVarUse)
    return pp;
for (pp = pp->children; pp != NULL; pp = pp->next)
    {
    v = firstVarInTree(pp);
    if (v != NULL)
        return v;
    }
return NULL;
}


static boolean usesOutsideObj(struct hash *outputVars,
	struct pfScope *localScope, struct pfParse *pp)
/* Return TRUE if any non-local objects are involved under pp. */
{
switch (pp->type)
    {
    case pptVarUse:
        return isOutsideObj(outputVars, localScope, pp);
    case pptDot:
    case pptIndex:
        return usesOutsideObj(outputVars, localScope, pp->children);
    case pptTuple:
        {
	for (pp = pp->children; pp != NULL; pp = pp->next)
	    {
	    if (usesOutsideObj(outputVars, localScope, pp))
	        return TRUE;
	    }
	return FALSE;
	}
    }
return FALSE;
}

static void markParaTainted(struct hash *outputVars,
	struct pfScope *localScope, struct pfParse *pp)
/* If we're storing a non-local object or string inside a
 * a local object or a string var, then mark that local variable
 * as tainted so we no longer allow writes to it. */
{
if (pp->ty->base->needsCleanup)
    {
    if (pp->type == pptVarInit)
        {
	struct pfParse *type = pp->children;
	struct pfParse *name = type->next;
	struct pfParse *init = name->next;
	if (init != NULL)
	    {
	    if (usesOutsideObj(outputVars, localScope, init))
	        pp->var->paraTainted = TRUE;
	    }
	}
    else 
	{
	struct pfParse *left = pp->children;
	struct pfParse *init = left->next;
	struct pfParse *leftVar = firstVarInTree(left);
	if (hashLookup(outputVars, leftVar->name))
	    {
	    if (usesOutsideObj(outputVars, localScope, init))
	        errAt(pp->tok, "Can't assign a para or flow output to something non-local.");
	    }
	else if (!isOutsideObj(outputVars, localScope, leftVar))
	    {
	    if (usesOutsideObj(outputVars, localScope, init))
	        leftVar->var->paraTainted = TRUE;
	    }
	}
    }
}

static void checkReadOnlyOutsideLocals(struct hash *outputVars,
	struct pfScope *localScope, struct pfParse *pp)
/* Check that anything on left hand side of an assignment
 * is local or in the outputVars hash. */
{
switch (pp->type)
    {
    case pptAssignment:
    case pptPlusEquals:
    case pptMinusEquals:
    case pptMulEquals:
    case pptDivEquals:
         {
	 checkLocal(outputVars, localScope, pp->children);
	 markParaTainted(outputVars, localScope, pp);
	 break;
	 }
    case pptVarInit:
        {
	markParaTainted(outputVars, localScope, pp);
	break;
	}
    }
for (pp = pp->children; pp != NULL; pp = pp->next)
    checkReadOnlyOutsideLocals(outputVars, localScope, pp);
}

static void checkCalls(struct pfCompile *pfc, struct pfParse *pp, boolean paraOk)
/* Check that all calls are of type flow, or optionally
 * of type flow or type para. */
{
switch (pp->type)
    {
    case pptCall:
         {
	 struct pfParse *ppFuncVar = pp->children;
	 struct pfBaseType *base = ppFuncVar->ty->base;
	 if (base == pfc->flowType)
	     ;
	 else if (paraOk && base == pfc->paraType)
	     ;
	 else
	     {
	     if (paraOk)
	         errAt(pp->tok, 
		    "Only calls to para and flow functions allowed inside para");
	     else
	         errAt(pp->tok, 
		    "Only calls to flow functions allowed inside flow");
	     }
	 break;
	 }
    }
for (pp = pp->children; pp != NULL; pp = pp->next)
    checkCalls(pfc, pp, paraOk);
}

static void checkParaBody(struct pfCompile *pfc, struct hash *outputVars,
	struct pfScope *scope, struct pfParse *body)
/* Make sure that body does not write to anything but
 *   1) Output vars.
 *   2) Local vars..
 */
{
checkCalls(pfc, body, TRUE);
checkReadOnlyOutsideLocals(outputVars, scope, body);
}

static void checkParaDec(struct pfCompile *pfc, struct pfParse *paraDec)
/* Make sure para function declaration is ok. */
{
struct pfParse *input = paraDec->children->next;
struct pfParse *output = input->next;
struct pfParse *body = output->next;
if (body != NULL)
    {
    struct pfParse *pp;
    struct hash *outputVars = hashNew(6);

    /* Build up hash of variables that it's ok to write to. */
    for (pp = output->children; pp != NULL; pp = pp->next)
	hashAdd(outputVars, pp->name, pp->var);

    checkParaBody(pfc, outputVars, body->scope, body);

    /* Clean up */
    hashFree(&outputVars);
    }
}

static void checkParaAction(struct pfCompile *pfc, struct pfParse *para)
/* Make sure para action is ok. */
{
struct pfParse *element = para->children;
struct pfParse *collection = element->next;
struct pfParse *body = collection->next;
struct hash *emptyHash = hashNew(2);
checkParaBody(pfc, emptyHash, para->scope, body);
hashFree(&emptyHash);
}

void checkParaFlow(struct pfCompile *pfc, struct pfParse *pp)
/* Check para and flow declarations throughout program. */
{
switch (pp->type)
    {
    case pptParaDo:
    case pptParaAdd:
    case pptParaMultiply:
    case pptParaAnd:
    case pptParaOr:
    case pptParaMin:
    case pptParaMax:
    case pptParaGet:
    case pptParaFilter:
	checkParaAction(pfc, pp);
        break;
    case pptParaDec:
	checkParaDec(pfc, pp);
        break;
    }
for (pp = pp->children; pp != NULL; pp = pp->next)
    checkParaFlow(pfc, pp);
}

