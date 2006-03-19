/* isx - intermediate code -  hexes of
 *     label destType destName left op right 
 * For instance:
 *     100 + int t1 1 t2  (means int t1 = 1 + t2)
 *     101 * double v 0.5 t3  (means double v = 0.5 * t3)
 */

#include "common.h"
#include "dlist.h"
#include "pfParse.h"
#include "pfToken.h"
#include "pfType.h"
#include "pfScope.h"
#include "pfCompile.h"
#include "isx.h"

static struct isxAddress *isxExpression(struct pfCompile *pfc, 
	struct pfParse *pp, struct hash *varHash, struct dlList *iList);
/* Generate intermediate code for expression. Return destination */

void isxStatement(struct pfCompile *pfc, struct pfParse *pp, 
	struct hash *varHash, struct dlList *iList);
/* Generate intermediate code for statement. */

char *isxValTypeToString(enum isxValType val)
/* Convert isxValType to string. */
{
switch (val)
    {
    case ivZero:
	return "zer";
    case ivByte:
	return "byt";
    case ivShort:
	return "sho";
    case ivInt:
	return "int";
    case ivLong:
	return "lng";
    case ivFloat:
	return "flt";
    case ivDouble:
	return "dbl";
    case ivObject:
	return "obj";
    case ivJump:
	return "jmp";
    default:
        internalErr();
	return NULL;
    }
}

char *isxOpTypeToString(enum isxOpType val)
/* Convert isxValType to string. */
{
switch (val)
    {
    case poInit:
        return "poInit";
    case poAssign:
        return "poAssign";
    case poPlus:
	return "poPlus";
    case poMinus:
	return "poMinus";
    case poMul:
	return "poMul";
    case poDiv:
	return "poDiv";
    case poMod:
	return "poMod";
    case poBitAnd:
	return "poBitAnd";
    case poBitOr:
	return "poBitOr";
    case poBitXor:
	return "poBitXor";
    case poShiftLeft:
	return "poShiftLeft";
    case poShiftRight:
	return "poShiftRight";
    case poGoTo:
	return "poGoTo";
    case poBranch:
	return "poBranch";
    case poCall:
	return "poCall";
    default:
        internalErr();
	return NULL;
    }
}

char *isxRegName(struct isxReg *reg, enum isxValType valType)
/* Get name of reg for given type. */
{
switch (valType)
    {
    case ivByte:
	return reg->byteName;
    case ivShort:
	return reg->shortName;
    case ivInt:
	return reg->intName;
    case ivLong:
	return reg->longName;
    case ivFloat:
	return reg->floatName;
    case ivDouble:
	return reg->doubleName;
    case ivObject:
	return reg->pointerName;
    default:
	internalErr();
	return NULL;
    }
}



static void isxAddrDump(struct isxAddress *iad, FILE *f)
/* Print variable name or n/a */
{
fprintf(f, "%s:", isxValTypeToString(iad->valType));
/* Convert isxValType to string. */
switch (iad->adType)
    {
    case iadZero:
	fprintf(f, "#zero");
	break;
    case iadConst:
	{
	struct pfToken *tok = iad->val.tok;
	union pfTokVal val = tok->val;
	switch(tok->type)
	    {
	    case pftInt:
		fprintf(f, "#%d", val.i);
		break;
	    case pftLong:
		fprintf(f, "#%lld", val.l);
		break;
	    case pftFloat:
		fprintf(f, "#%f", val.x);
		break;
	    case pftString:
		fprintf(f, "#'%s'", val.s);
		break;
	    }
	break;
	}
    case iadRealVar:
    case iadTempVar:
	fprintf(f, "%s", iad->name);
	break;
    case iadOperator:
        fprintf(f, "#%s", iad->name);
	break;
    }
if (iad->reg != NULL)
    fprintf(f, "@%s", isxRegName(iad->reg, iad->valType));
}


void isxDump(struct isx *isx, FILE *f)
/* Dump out isx code */
{
struct isxAddress *iad;
struct slRef *ref;
fprintf(f, "%d: %s ", isx->label, isxOpTypeToString(isx->opType));
for (ref = isx->destList; ref != NULL; ref = ref->next)
    {
    iad = ref->val;
    fprintf(f, " ");
    isxAddrDump(iad, f);
    }
fprintf(f, " =");
for (ref = isx->sourceList; ref != NULL; ref = ref->next)
    {
    iad = ref->val;
    fprintf(f, " ");
    isxAddrDump(iad, f);
    }
if (isx->liveList != NULL)
    {
    fprintf(f, "\t{");
    for (ref = isx->liveList; ref != NULL; ref = ref->next)
        {
	iad = ref->val;
	fprintf(f, "%s", iad->name);
	if (iad->reg != NULL)
	    fprintf(f, "@%s", isxRegName(iad->reg, iad->valType));
	if (ref->next != NULL)
	    fprintf(f, ",");
	}
    fprintf(f, "}");
    }

}

void isxDumpList(struct dlList *list, FILE *f)
/* Dump all of isx in list */
{
struct dlNode *node;
for (node = list->head; !dlEnd(node); node = node->next)
    {
    isxDump(node->val, f);
    fprintf(f, "\n");
    }
}

struct isx *isxNew(struct pfCompile *pfc, 
	enum isxOpType opType,
	struct slRef *destList,
	struct slRef *sourceList,
	struct dlList *iList)
/* Return new isx */
{
struct isx *isx;
AllocVar(isx);
isx->label = ++pfc->isxLabelMaker;
isx->opType = opType;
isx->destList = destList;
isx->sourceList = sourceList;
dlAddValTail(iList, isx);
return isx;
}

static enum isxValType tyToIsxValType(struct pfCompile *pfc, 
	struct pfType *ty)
/* Return isxValType corresponding to pp */
{
struct pfBaseType *base = ty->base;
if (base == pfc->bitType || base == pfc->byteType)
    return ivByte;
else if (base == pfc->shortType)
    return ivShort;
else if (base == pfc->intType)
    return ivInt;
else if (base == pfc->longType)
    return ivLong;
else if (base == pfc->floatType)
    return ivFloat;
else if (base == pfc->doubleType)
    return ivDouble;
else
    return ivObject;
}

static enum isxValType ppToIsxValType(struct pfCompile *pfc, 
	struct pfParse *pp)
/* Return isxValType corresponding to pp */
{
return tyToIsxValType(pfc, pp->ty);
}

static struct isxAddress *constAddress(struct pfToken *tok, 
	enum isxValType valType)
/* Get place to put constant. */
{
struct isxAddress *iad;
AllocVar(iad);
iad->adType = iadConst;
iad->valType = valType;
iad->val.tok = tok;
return iad;
}

static struct isxAddress *zeroAddress()
/* Get place representing zero or nil. */
{
static struct isxAddress zero;
return &zero;
}

static struct isxAddress *tempAddress(struct pfCompile *pfc, struct hash *hash,
	enum isxValType valType)
/* Create a new temporary */
{
struct isxAddress *iad;
char buf[18];
safef(buf, sizeof(buf), "$t%X", ++(pfc->tempLabelMaker));
AllocVar(iad);
iad->adType = iadTempVar;
iad->valType = valType;
hashAddSaveName(hash, buf, iad, &iad->name);
return iad;
}

static struct isxAddress *varAddress(struct pfVar *var, struct hash *hash,
	enum isxValType valType)
/* Create reference to a real var */
{
struct isxAddress *iad = hashFindVal(hash, var->cName);
if (iad == NULL)
    {
    AllocVar(iad);
    iad->adType = iadRealVar;
    iad->valType = valType;
    iad->val.var = var;
    hashAddSaveName(hash, var->cName, iad, &iad->name);
    }
return iad;
}

static struct isxAddress *operatorAddress(char *name)
/* Create reference to a build-in operator call */
{
struct isxAddress *iad;
AllocVar(iad);
iad->adType = iadOperator;
iad->valType = ivJump;
iad->name = name;
return iad;
}


static struct isxAddress *isxBinaryOp(struct pfCompile *pfc, 
	struct pfParse *pp, struct hash *varHash, 
	enum isxOpType op, struct dlList *iList)
/* Generate intermediate code for expression. Return destination */
{
struct isxAddress *left = isxExpression(pfc, pp->children,
	varHash, iList);
struct isxAddress *right = isxExpression(pfc, pp->children->next,
	varHash, iList);
struct isxAddress *dest = tempAddress(pfc, varHash, ppToIsxValType(pfc,pp));
struct slRef *sourceList = NULL;
refAdd(&sourceList, right);
refAdd(&sourceList, left);
isxNew(pfc, op, slRefNew(dest), sourceList, iList);
return dest;
}

static struct isxAddress *isxStringCat(struct pfCompile *pfc,
	struct pfParse *pp, struct hash *varHash, struct dlList *iList)
/* Create string cat op */
{
enum isxValType valType = ppToIsxValType(pfc, pp);
struct isxAddress *dest = tempAddress(pfc, varHash, valType);
struct isxAddress *source;
struct slRef *sourceList = slRefNew(operatorAddress("string_cat"));
for (pp = pp->children; pp != NULL; pp = pp->next)
    {
    source = isxExpression(pfc, pp, varHash, iList);
    refAdd(&sourceList, source);
    }
slReverse(&sourceList);
isxNew(pfc, poCall, slRefNew(dest), sourceList, iList);
return dest;
}

static struct isxAddress *isxCall(struct pfCompile *pfc,
	struct pfParse *pp, struct hash *varHash, struct dlList *iList)
/* Create call op */
{
struct pfParse *function = pp->children;
struct pfParse *inTuple = function->next;
struct pfParse *p;
struct pfType *outTuple = function->ty->children->next, *ty;
struct slRef *sourceList, *destList = NULL;
struct isxAddress *iad;

sourceList = slRefNew(varAddress(function->var, varHash, ivJump));
for (p = inTuple->children; p != NULL; p = p->next)
    {
    iad = isxExpression(pfc, p, varHash, iList);
    refAdd(&sourceList, iad);
    }
slReverse(&sourceList);
for (ty = outTuple->children; ty != NULL; ty = ty->next)
    {
    iad = tempAddress(pfc, varHash, tyToIsxValType(pfc, ty));
    refAdd(&destList, iad);
    }
slReverse(&destList);
isxNew(pfc, poCall, destList, sourceList, iList);
return NULL;	// TODO - fixme.
}

static struct isxAddress *isxExpression(struct pfCompile *pfc, 
	struct pfParse *pp, struct hash *varHash, struct dlList *iList)
/* Generate intermediate code for expression. Return destination */
{
switch (pp->type)
    {
    case pptConstBit:
    case pptConstByte:
    case pptConstChar:
    case pptConstShort:
    case pptConstInt:
    case pptConstLong:
    case pptConstFloat:
    case pptConstDouble:
    case pptConstString:
	return constAddress(pp->tok, ppToIsxValType(pfc, pp));
    case pptVarUse:
	return varAddress(pp->var, varHash, ppToIsxValType(pfc, pp));
    case pptPlus:
	return isxBinaryOp(pfc, pp, varHash, poPlus, iList);
    case pptMinus:
	return isxBinaryOp(pfc, pp, varHash, poMinus, iList);
    case pptMul:
	return isxBinaryOp(pfc, pp, varHash, poMul, iList);
    case pptDiv:
	return isxBinaryOp(pfc, pp, varHash, poDiv, iList);
    case pptMod:
	return isxBinaryOp(pfc, pp, varHash, poMod, iList);
    case pptBitAnd:
	return isxBinaryOp(pfc, pp, varHash, poBitAnd, iList);
    case pptBitOr:
	return isxBinaryOp(pfc, pp, varHash, poBitOr, iList);
    case pptBitXor:
	return isxBinaryOp(pfc, pp, varHash, poBitXor, iList);
    case pptShiftLeft:
	return isxBinaryOp(pfc, pp, varHash, poShiftLeft, iList);
    case pptShiftRight:
	return isxBinaryOp(pfc, pp, varHash, poShiftRight, iList);
    case pptStringCat:
        return isxStringCat(pfc, pp, varHash, iList);
    case pptCall:
        return isxCall(pfc, pp, varHash, iList);
    default:
	pfParseDump(pp, 3, uglyOut);
        internalErr();
	return NULL;
    }
}

void isxStatement(struct pfCompile *pfc, struct pfParse *pp, 
	struct hash *varHash, struct dlList *iList)
/* Generate intermediate code for statement. */
{
switch (pp->type)
    {
    case pptVarInit:
	{
	struct pfParse *init = pp->children->next->next;
	struct isxAddress *dest = varAddress(pp->var, varHash, 
		ppToIsxValType(pfc, pp));
	struct isxAddress *source;
	if (init)
	    source = isxExpression(pfc, init, varHash, iList);
	else
	    source = zeroAddress();
	isxNew(pfc, poInit, slRefNew(dest), slRefNew(source), iList);
	break;
	}
    case pptAssign:
        {
	struct pfParse *use = pp->children;
	struct pfParse *val = use->next;
	struct isxAddress *source = isxExpression(pfc, val, varHash, iList);
	struct isxAddress *dest = varAddress(use->var, varHash, 
		ppToIsxValType(pfc, use));
	isxNew(pfc, poAssign, slRefNew(dest), slRefNew(source), iList);
	break;
	}
    default:
	pfParseDump(pp, 3, uglyOut);
        errAt(pp->tok, "Unrecognized statement in isxStatement");
	break;
    }
}

static void isxLiveList(struct dlList *iList)
/* Create list of live variables at each instruction by scanning
 * backwards. */
{
struct dlNode *node;
struct slRef *liveList = NULL;
for (node = iList->tail; !dlStart(node); node = node->prev)
    {
    struct slRef *newList = NULL, *ref;
    struct isx *isx = node->val;

    /* Save away current live list. */
    isx->liveList = liveList;

    /* Make copy of live list minus any overwritten dests. */
    for (ref = liveList; ref != NULL; ref = ref->next)
	{
	struct isxAddress *iad = ref->val;
	if (!refOnList(isx->destList, iad))
	    refAdd(&newList, iad);
	}

    /* Add sources to live list */
    for (ref = isx->sourceList; ref != NULL; ref = ref->next)
	{
	struct isxAddress *iad = ref->val;
	if (iad->adType == iadRealVar || iad->adType == iadTempVar)
	    {
	    refAddUnique(&newList, iad);
	    }
	}

    /* Flip to new live list */
    liveList = newList;
    }
slFreeList(&liveList);
}

void isxModule(struct pfCompile *pfc, struct pfParse *pp, 
	struct dlList *iList)
/* Generate instructions for module. */
{
struct hash *varHash = hashNew(16);
for (pp = pp->children; pp != NULL; pp = pp->next)
    {
    isxStatement(pfc, pp, varHash, iList);
    }
isxLiveList(iList);
}

struct dlList *isxFromParse(struct pfCompile *pfc, struct pfParse *pp)
/* Convert parse tree to isx. */
{
struct dlList *iList = dlListNew(0);
for (pp = pp->children; pp != NULL; pp = pp->next)
    {
    switch (pp->type)
        {
	case pptMainModule:
	case pptModule:
	    isxModule(pfc, pp, iList);
	    break;
	}
    }
return iList;
}

