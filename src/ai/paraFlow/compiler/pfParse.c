/* pfParse build up parse tree from token stream */
/* Copyright 2005 Jim Kent.  All rights reserved. */

#include "common.h"
#include "hash.h"
#include "dystring.h"
#include "localmem.h"
#include "pfType.h"
#include "pfScope.h"
#include "pfToken.h"
#include "pfCompile.h"
#include "pfParse.h"

char *pfParseTypeAsString(enum pfParseType type)
/* Return string corresponding to pfParseType */
{
switch (type)
    {
    case pptProgram:
    	return "pptProgram";
    case pptScope:
        return "pptScope";
    case pptInclude:
        return "pptInclude";
    case pptModule:
        return "pptModule";
    case pptModuleRef:
        return "pptModuleRef";
    case pptMainModule:
        return "pptMainModule";
    case pptNop:
    	return "pptNop";
    case pptCompound:
    	return "pptCompound";
    case pptTuple:
        return "pptTuple";
    case pptOf:
        return "pptOf";
    case pptDot:
        return "pptDot";
    case pptKeyVal:
        return "pptKeyVal";
    case pptIf:
    	return "pptIf";
    case pptElse:
    	return "pptElse";
    case pptWhile:
    	return "pptWhile";
    case pptFor:
	return "pptFor";
    case pptForeach:
	return "pptForeach";
    case pptForEachCall:
	return "pptForEachCall";
    case pptBreak:
    	return "pptBreak";
    case pptContinue:
    	return "pptContinue";
    case pptClass:
	return "pptClass";
    case pptInterface:
	return "pptInterface";
    case pptVarDec:
	return "pptVarDec";
    case pptNameUse:
	return "pptNameUse";
    case pptFieldUse:
	return "pptFieldUse";
    case pptSelfUse:
	return "pptSelfUse";
    case pptVarUse:
	return "pptVarUse";
    case pptConstUse:
	return "pptConstUse";
    case pptPolymorphic:
        return "pptPolymorphic";
    case pptToDec:
	return "pptToDec";
    case pptFlowDec:
	return "pptFlowDec";
    case pptReturn:
	return "pptReturn";
    case pptCall:
	return "pptCall";
    case pptIndirectCall:
	return "pptIndirectCall";
    case pptAssignment:
	return "pptAssignment";
    case pptPlusEquals:
	return "pptPlusEquals";
    case pptMinusEquals:
	return "pptMinusEquals";
    case pptMulEquals:
	return "pptMulEquals";
    case pptDivEquals:
	return "pptDivEquals";
    case pptIndex:
	return "pptIndex";
    case pptPlus:
	return "pptPlus";
    case pptMinus:
	return "pptMinus";
    case pptMul:
	return "pptMul";
    case pptDiv:
	return "pptDiv";
    case pptShiftLeft:
        return "pptShiftLeft";
    case pptShiftRight:
        return "pptShiftRight";
    case pptMod:
	return "pptMod";
    case pptComma:
	return "pptComma";
    case pptSame:
	return "pptSame";
    case pptNotSame:
	return "pptNotSame";
    case pptGreater:
	return "pptGreater";
    case pptLess:
	return "pptLess";
    case pptGreaterOrEquals:
	return "pptGreaterOrEquals";
    case pptLessOrEquals:
	return "pptLessOrEquals";
    case pptNegate:
	return "pptNegate";
    case pptNot:
        return "pptNot";
    case pptFlipBits:
        return "pptFlipBits";
    case pptBitAnd:
	return "pptBitAnd";
    case pptBitOr:
	return "pptBitOr";
    case pptBitXor:
	return "pptBitXor";
    case pptLogAnd:
	return "pptLogAnd";
    case pptLogOr:
	return "pptLogOr";
    case pptRoot:
	return "pptRoot";
    case pptParent:
	return "pptParent";
    case pptSys:
	return "pptSys";
    case pptUser:
	return "pptUser";
    case pptSysOrUser:
	return "pptSysOrUser";
    case pptVarInit:
        return "pptVarInit";
    case pptPlaceholder:
        return "pptPlaceholder";
    case pptSymName:
	return "pptSymName";
    case pptTypeName:
	return "pptTypeName";
    case pptTypeTuple:
	return "pptTypeTuple";
    case pptTypeToPt:
	return "pptTypeToPt";
    case pptTypeFlowPt:
	return "pptTypeFlowPt";
    case pptStringCat:
	return "pptStringCat";
    case pptParaDo:
	return "pptParaDo";
    case pptParaAdd:
	return "pptParaAdd";
    case pptParaMultiply:
	return "pptParaMultiply";
    case pptParaAnd:
	return "pptParaAnd";
    case pptParaOr:
	return "pptParaOr";
    case pptParaMin:
	return "pptParaMin";
    case pptParaMax:
	return "pptParaMax";
    case pptParaArgMin:
	return "pptParaArgMin";
    case pptParaArgMax:
	return "pptParaArgMax";
    case pptParaGet:
	return "pptParaGet";
    case pptParaFilter:
	return "pptParaFilter";
    case pptUntypedElInCollection:
	return "pptUntypedElInCollection";
    case pptOperatorDec:
	return "pptOperatorDec";
    case pptArrayAppend:
	return "pptArrayAppend";
    case pptIndexRange:
	return "pptIndexRange";

    case pptCastBitToBit:
        return "pptCastBitToBit";
    case pptCastBitToByte:
        return "pptCastBitToByte";
    case pptCastBitToShort:
	return "pptCastBitToShort";
    case pptCastBitToInt:
	return "pptCastBitToInt";
    case pptCastBitToLong:
	return "pptCastBitToLong";
    case pptCastBitToFloat:
	return "pptCastBitToFloat";
    case pptCastBitToDouble:
	return "pptCastBitToDouble";
    case pptCastBitToString:
        return "pptCastBitToString";
    case pptCastByteToBit:
        return "pptCastByteToBit";
    case pptCastByteToByte:
        return "pptCastByteToByte";
    case pptCastByteToShort:
	return "pptCastByteToShort";
    case pptCastByteToInt:
	return "pptCastByteToInt";
    case pptCastByteToLong:
	return "pptCastByteToLong";
    case pptCastByteToFloat:
	return "pptCastByteToFloat";
    case pptCastByteToDouble:
	return "pptCastByteToDouble";
    case pptCastByteToString:
        return "pptCastByteToString";
    case pptCastShortToBit:
        return "pptCastShortToBit";
    case pptCastShortToByte:
	return "pptCastShortToByte";
    case pptCastShortToInt:
	return "pptCastShortToInt";
    case pptCastShortToLong:
	return "pptCastShortToLong";
    case pptCastShortToFloat:
	return "pptCastShortToFloat";
    case pptCastShortToDouble:
	return "pptCastShortToDouble";
    case pptCastShortToString:
        return "pptCastShortToString";
    case pptCastIntToBit:
        return "pptCastIntToBit";
    case pptCastIntToByte:
	return "pptCastIntToByte";
    case pptCastIntToShort:
	return "pptCastIntToShort";
    case pptCastIntToLong:
	return "pptCastIntToLong";
    case pptCastIntToFloat:
	return "pptCastIntToFloat";
    case pptCastIntToDouble:
	return "pptCastIntToDouble";
    case pptCastIntToString:
        return "pptCastIntToString";
    case pptCastLongToBit:
        return "pptCastLongToBit";
    case pptCastLongToByte:
	return "pptCastLongToByte";
    case pptCastLongToShort:
	return "pptCastLongToShort";
    case pptCastLongToInt:
	return "pptCastLongToInt";
    case pptCastLongToFloat:
	return "pptCastLongToFloat";
    case pptCastLongToDouble:
	return "pptCastLongToDouble";
    case pptCastLongToString:
        return "pptCastLongToString";
    case pptCastFloatToBit:
        return "pptCastFloatToBit";
    case pptCastFloatToByte:
	return "pptCastFloatToByte";
    case pptCastFloatToShort:
	return "pptCastFloatToShort";
    case pptCastFloatToInt:
	return "pptCastFloatToInt";
    case pptCastFloatToLong:
	return "pptCastFloatToLong";
    case pptCastFloatToDouble:
	return "pptCastFloatToDouble";
    case pptCastFloatToString:
        return "pptCastFloatToString";
    case pptCastDoubleToBit:
        return "pptCastDoubleToBit";
    case pptCastDoubleToByte:
	return "pptCastDoubleToByte";
    case pptCastDoubleToShort:
	return "pptCastDoubleToShort";
    case pptCastDoubleToInt:
	return "pptCastDoubleToInt";
    case pptCastDoubleToLong:
	return "pptCastDoubleToLong";
    case pptCastDoubleToFloat:
	return "pptCastDoubleToFloat";
    case pptCastDoubleToDouble:
	return "pptCastDoubleToDouble";
    case pptCastDoubleToString:
        return "pptCastDoubleToString";
    case pptCastStringToBit:
        return "pptCastStringToBit";
    case pptCastObjectToBit:
        return "pptCastObjectToBit";
    case pptCastClassToInterface:
    	return "pptCastClassToInterface";
    case pptCastTypedToVar:
        return "pptCastTypedToVar";
    case pptCastVarToTyped:
        return "pptCastVarToTyped";
    case pptCastCallToTuple:
	return "pptCastCallToTuple";
    case pptUniformTuple:
        return "pptUniformTuple";
    case pptConstBit:
	return "pptConstBit";
    case pptConstByte:
	return "pptConstByte";
    case pptConstShort:
	return "pptConstShort";
    case pptConstInt:
	return "pptConstInt";
    case pptConstLong:
	return "pptConstLong";
    case pptConstFloat:
	return "pptConstFloat";
    case pptConstDouble:
	return "pptConstDouble";
    case pptConstString:
	return "pptConstString";
    case pptConstZero:
	return "pptConstZero";

    default:
        internalErr();
	return NULL;
    }
}

static void pfDumpConst(struct pfToken *tok, FILE *f)
/* Dump out constant to file */
{
switch (tok->type)
    {
    case pftString:
	{
	#define MAXLEN 32
	char buf[MAXLEN+4];
	char *s = tok->val.s;
	int len = strlen(s);
	if (len > MAXLEN) len = MAXLEN;
	memcpy(buf, s, len);
	buf[len] = 0;
	strcpy(buf+MAXLEN, "...");
	printEscapedString(f, buf);
	break;
	#undef MAXLEN
	}
    case pftLong:
        fprintf(f, "%lld", tok->val.l);
	break;
    case pftInt:
        fprintf(f, "%d", tok->val.i);
	break;
    case pftFloat:
        fprintf(f, "%g", tok->val.x);
	break;
    case pftNil:
        fprintf(f, "nil");
	break;
    default:
        fprintf(f, "unknownType");
	break;
    }
}

void pfParseDumpOne(struct pfParse *pp, int level, FILE *f)
/* Dump out single pfParse record at given level of indent. */
{
spaceOut(f, level*3);
fprintf(f, "%s", pfParseTypeAsString(pp->type));
if (pp->isStatic)
    fprintf(f, " s");
if (pp->ty != NULL)
    {
    fprintf(f, " ");
    pfTypeDump(pp->ty, f);
    }
if (pp->name != NULL)
    fprintf(f, " %s", pp->name);
switch (pp->type)
    {
    case pptConstUse:
    case pptConstBit:
    case pptConstByte:
    case pptConstShort:
    case pptConstInt:
    case pptConstLong:
    case pptConstFloat:
    case pptConstDouble:
    case pptConstString:
    case pptConstZero:
	fprintf(f, " ");
	pfDumpConst(pp->tok, f);
	break;
    }
fprintf(f, "\n");
}

void pfParseDump(struct pfParse *pp, int level, FILE *f)
/* Write out pp (and it's children) to file at given level of indent */
{
pfParseDumpOne(pp, level, f);
level += 1;
for (pp = pp->children; pp != NULL; pp = pp->next)
    pfParseDump(pp, level, f);
}

struct pfParse *pfParseStatement(struct pfCompile *pfc, struct pfParse *parent, 
	struct pfToken **pTokList, struct pfScope *scope);

struct pfParse *pfParseExpression(struct pfCompile *pfc, 
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope);

static void syntaxError(struct pfToken *tok, int code)
/* Complain about syntax error and quit. */
{
errAt(tok, "Syntax error #%d", code);
}

void eatSemi(struct pfToken **pTokList)
/* Make sure there's a semicolon, and eat it. */
{
struct pfToken *tok = *pTokList;
if (tok->type != ';')
    expectingGot(";", tok);
tok = tok->next;
*pTokList = tok;
}
    	
struct pfParse *pfParseNew(enum pfParseType type,
	struct pfToken *tok, struct pfParse *parent, struct pfScope *scope)
/* Return new parse node.  It's up to caller to fill in
 * children later. */
{
struct pfParse *pp;
AllocVar(pp);
pp->type = type;
pp->tok = tok;
pp->parent = parent;
pp->scope = scope;
return pp;
}

void pfParsePutChildrenInPlaceOfSelf(struct pfParse **pPp)
/* Replace self with children. */
{
struct pfParse *pp = *pPp;
if (pp->children == NULL)
    *pPp = pp->next;
else
    {
    struct pfParse *lastChild = slLastEl(pp->children);
    lastChild->next = pp->next;
    *pPp = pp->children;
    }
}

struct pfParse *pfParseEnclosingClass(struct pfParse *pp)
/* Find enclosing class if any.  It treats self as enclosing self.  */
{
for (pp = pp->parent; pp != NULL; pp = pp->parent)
    {
    if (pp->type == pptClass)
	{
        return pp;
	}
    }
return NULL;
}


struct pfParse *emptyTuple(struct pfParse *parent, struct pfToken *tok,
	struct pfScope *scope)
/* Return empty tuple. */
{
return pfParseNew(pptTuple, tok, parent, scope);
}

struct pfParse *emptyStatement(struct pfParse *parent, struct pfToken *tok,
	struct pfScope *scope)
/* Return no-op statement. */
{
return pfParseNew(pptNop, tok, parent, scope);
}

struct pfParse *pfSingleTuple(struct pfParse *parent, struct pfToken *tok, 
	struct pfParse *single)
/* Wrap tuple around single and return it.  . */
{
struct pfParse *tuple = pfParseNew(pptTuple, tok, parent, parent->scope);
tuple->children = single;
tuple->next = single->next;
single->parent = tuple;
single->next = NULL;
return tuple;
}

static void skipRequiredName(char *name, struct pfToken **pTokList)
/* Make sure next token matches name, and then skip it. */
{
struct pfToken *tok = *pTokList;
if (tok->type != pftName || !sameString(tok->val.s, name))
    expectingGot(name, tok);
*pTokList = tok->next;
}

static void skipRequiredCharType(enum pfTokType type, struct pfToken **pTokList)
/* Make sure next token is of given type simple char type, and then skip it. */
{
struct pfToken *tok = *pTokList;
if (tok->type != type)
    {
    char sym[2];
    sym[0] = type;
    sym[1] = 0;
    expectingGot(sym, tok);
    }
*pTokList = tok->next;
}

struct pfParse *parseNameUse(struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope)
/* Make sure have a name, and create a varUse type node
 * based on it. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(pptNameUse, tok, parent, scope);
if (tok->type != pftName)
    {
    errAt(tok, "Expecting variable.");
    }
pp->name = tok->val.s;
*pTokList = tok->next;
return pp;
}

struct pfParse *parseDottedNames(struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope)
/* Parse this.that.the.other */
{
struct pfToken *tok = *pTokList;
struct pfParse *dots, *first = NULL, *pp;
boolean startsSpecial = FALSE;

/* If it's a name try and take shortcut if there's no
 * following dot,  otherwise set up first from name. */
if (tok->type == pftName)
    {
    first = parseNameUse(parent, &tok, scope);
    if (tok->type != '.')
	{
	*pTokList = tok;
	return first;
	}
    else
        tok = tok->next;
    }
/* Eat through special token at front of name. */
else
    {
    enum pfParseType type = pptNone;
    startsSpecial = TRUE;
    switch (tok->type)
	{
	case pftRoot:
	    type = pptRoot;
	    break;
	case pftParent:
	    type = pptParent;
	    break;
	case pftSys:
	    type = pptSys;
	    break;
	case pftUser:
	    type = pptUser;
	    break;
	case pftSysOrUser:
	    type = pptSysOrUser;
	    break;
	default:
	    expectingGot("name", tok);
	    break;
	}
    first = pfParseNew(type, tok, NULL, scope);
    tok = tok->next;
    }

/* Create dot list parse and add first element to it. */
dots = pfParseNew(pptDot, tok, parent, scope);
first->parent = dots;
slAddHead(&dots->children, first);

/* In some cases there may be additional parents to add in front. */
if (startsSpecial)
    {
    while (tok->type == pftParent)
	{
	tok = tok->next;
	pp = pfParseNew(pptParent, tok, dots, scope);
	slAddHead(&dots->children, pp);
	}
    }

/* Keep adding names until we run out of dots. */
for (;;)
    {
    if (tok->type != pftName)
        expectingGot("name", tok);
    pp = parseNameUse(dots, &tok, scope);
    slAddHead(&dots->children, pp);
    if (tok->type != '.')
        break;
    tok = tok->next;
    }

/* Clean up and go home. */
slReverse(&dots->children);
*pTokList = tok;
return dots;
}

struct pfParse *parseTypeDotsAndBraces(struct pfCompile *pfc, struct pfParse *parent, 
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse this.that.the.other[expression] */
{
struct pfParse *dotted = parseDottedNames(parent, pTokList, scope);
struct pfToken *tok = *pTokList;
if (tok->type == '[')
    {
    tok = tok->next;
    dotted->children = pfParseExpression(pfc, dotted, &tok, scope);
    if (tok->type != ']')
        expectingGot("]", tok);
    else
        tok = tok->next;
    *pTokList = tok;
    }
return dotted;
}

void parseFunctionIo(struct pfCompile *pfc,
    struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope, 
    struct pfParse **retInput, struct pfParse **retOutput)
/* Parse out something like (int x, int y) into (int z)
 * into input and output, both of which will be (possibly empty)
 * pptTuples. */
{
struct pfToken *tok = *pTokList;
struct pfToken *outToken;
struct pfParse *input, *output = NULL;
skipRequiredCharType('(', &tok);
if (tok->type == ')')
    {
    input = emptyTuple(parent, tok, scope);
    outToken = tok;
    tok = tok->next;
    }
else
    {
    input = pfParseExpression(pfc, parent, &tok, scope);
    if (input->type != pptTuple)
	input = pfSingleTuple(parent, tok, input);
    if (tok->type != ')')
	expectingGot(")", tok);
    outToken = tok;
    tok = tok->next;
    }
if (tok->type == pftInto)
    {
    tok = tok->next;
    outToken = tok;
    output = pfParseExpression(pfc, parent, &tok, scope);
    if (output->type != pptTuple)
	output = pfSingleTuple(parent, outToken, output);
    }
else
    output = emptyTuple(parent, outToken, scope);
input->type  = pptTypeTuple;
output->type  = pptTypeTuple;

*pTokList = tok;
*retInput = input;
*retOutput = output;
}
	

struct pfParse *parseShortFuncType(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse out something like:
 *      to (int x,y=2) into (int f) 
 * Into a parse tree that looks like
 *      pptTypeToPt	 (or pptTypeFlowPt)
 *         pptTypeTuple
 *            pptVarInit x
 *               pptTypeName int
 *               pptSymName x
 *            pptVarInit y
 *               pptTypeName int
 *               pptSymName y
 *               pptConstUse 2 
 *         pptTypeTuple
 *            pptVarInit f
 *               pptTypeName int
 *               pptSymName f
 */
{
struct pfToken *tok = *pTokList;
enum pfParseType ppt = (tok->type == pftTo ? pptTypeToPt : pptTypeFlowPt);
struct pfScope *funcScope = pfScopeNew(pfc, scope, 0, FALSE);
struct pfParse *pp = pfParseNew(ppt, tok, parent, funcScope);
struct pfParse *input, *output;

tok = tok->next;	/* Skip over 'to' or 'flow' */
parseFunctionIo(pfc, pp, &tok, funcScope, &input, &output);
pp->children = input;
input->next = output;
*pTokList = tok;
return pp;
}


struct pfParse *parseVarOfFunc(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* If it starts with 'var of to/flow' then parse out a function pointer
 * declaration.  Else ust parseTypeDotsAndBraces. */
{
struct pfToken *tok = *pTokList;
if (tok->type == pftName && sameString(tok->val.s, "var"))
    {
    struct pfToken *ofTok = tok->next;
    if (ofTok->type == pftOf)
        {
	struct pfToken *funcTok = ofTok->next;
	if (funcTok->type == pftFlow || funcTok->type == pftTo)
	    {
	    *pTokList = funcTok;
	    return parseShortFuncType(pfc, parent, pTokList, scope);
	    }
	}
    }
return parseTypeDotsAndBraces(pfc, parent, pTokList, scope);
}

struct pfParse *parseOfs(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse of separated expression. */
{
struct pfParse *pp = parseVarOfFunc(pfc, parent, pTokList, scope);
struct pfToken *tok = *pTokList;
struct pfParse *ofs = NULL;
if (tok->type == pftOf)
    {
    ofs = pfParseNew(pptOf, tok, parent, scope);
    ofs->children = pp;
    pp->parent = ofs;
    while (tok->type == pftOf)
	{
	tok = tok->next;
	pp = parseVarOfFunc(pfc, ofs, &tok, scope);
	slAddHead(&ofs->children, pp);
	}
    }
*pTokList = tok;
if (ofs != NULL)
    {
    slReverse(&ofs->children);
    return ofs;
    }
else
    return pp;
}

struct pfToken *nextMatchingTok(struct pfToken *tokList, enum pfTokType type)
/* Return next token of type,  NULL if not found. */
{
struct pfToken *tok;
for (tok = tokList; tok != NULL; tok = tok->next)
    if (tok->type == type)
        break;
return tok;
}

struct pfParse *varUseOrDeclare(struct pfCompile *pfc, struct pfParse *parent, 
	struct pfToken **pTokList, struct pfScope *scope)
/* Make sure have a name, and create a varUse type node
 * based on it. */
{
struct pfToken *tok = *pTokList;
struct pfBaseType *baseType = NULL;

if (tok->type == pftName)
    baseType = pfScopeFindType(scope, tok->val.s);
if (baseType != NULL)
    {
    struct pfToken *nextTok = tok->next;
    if (nextTok->type == '[')
        {
	struct pfToken *matchTok = nextMatchingTok(nextTok->next, ']');
	nextTok = matchTok->next;
	}
    if (nextTok->type == pftName || nextTok->type == pftOf)
	{
	struct pfParse *name, *type;
	struct pfParse *pp = pfParseNew(pptVarDec, tok, parent, scope);
	type = parseOfs(pfc, pp, &tok, scope);
	name = parseNameUse(pp, &tok, scope);
	pp->children = type;
	pp->name = name->name;
	type->next = name;
	*pTokList = tok;
	return pp;
	}
    }
return parseDottedNames(parent, pTokList, scope);
}

boolean inFunction(struct pfParse *pp)
/* Return TRUE if pp or one of it's parents is a function. */
{
while (pp != NULL)
    {
    if (pp->type == pptToDec)
	return TRUE;
    pp = pp->parent;
    }
return FALSE;
}

static struct pfParse *varStorageOrUse(struct pfCompile *pfc,
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope)
/* Parse out storage class if any, and then rest of function. */
{
struct pfToken *tok = *pTokList;

if (tok->type == pftStatic)
    {
    struct pfToken *staticTok = tok;
    struct pfParse *pp;
    if (!inFunction(parent))
        errAt(tok, "'static' outside of 'to'");
    tok = tok->next;
    pp = varUseOrDeclare(pfc, parent, &tok, scope);
    if (pp->type != pptVarDec)
        errAt(staticTok, 
		"'static' only allowed in front of a variable declaration");
    pp->isStatic = TRUE;
    *pTokList = tok;
    return pp;
    }
else
    return varUseOrDeclare(pfc, parent, pTokList, scope);
}

struct pfParse *constUse(struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope)
/* Create constant use */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(pptConstUse, tok, parent, scope);
*pTokList = tok->next;
return pp;
}


static struct pfParse *parseAtom(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse atomic expression - const, variable, or parenthesized expression. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = NULL;
switch (tok->type)
    {
    case pftName:
    case pftRoot:
    case pftParent:
    case pftSys:
    case pftUser:
    case pftSysOrUser:
    case pftStatic:
	pp = varStorageOrUse(pfc, parent, &tok, scope);
	break;
    case pftString:
    case pftInt:
    case pftLong:
    case pftFloat:
    case pftNil:
        pp = constUse(parent, &tok, scope);
	break;
    case '(':
        {
	struct pfToken *startTok = tok;
	tok = tok->next;
	if (tok->type == ')')
	    {
	    pp = emptyTuple(parent, tok, scope);
	    }
	else
	    {
	    pp = pfParseExpression(pfc, parent, &tok, scope);
	    if (tok->type != ')')
		errAt(tok, "Unclosed parenthesis.");
	    }
	tok = tok->next;
	break;
	}
    default:
        syntaxError(tok, 1);
	break;
    }
*pTokList = tok;
return pp;
}


struct pfParse *parseCallOrIndex(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse out function call. */
{
struct pfParse *pp = parseAtom(pfc, parent, pTokList, scope);
struct pfToken *tok = *pTokList;
if (tok->type == '(')
    {
    struct pfParse *func = pp;
    struct pfParse *parameters = NULL;
    pp = pfParseNew(pptCall, tok, parent, scope);
    func->parent = pp;
    pp->children = func;
    tok = tok->next;
    if (tok->type != ')')
        {
	parameters = pfParseExpression(pfc, pp, &tok, scope);
	if (parameters->type != pptTuple)
	    {
	    struct pfParse *tuple = pfSingleTuple(pp, tok, parameters);
	    parameters->parent = tuple;
	    parameters = tuple;
	    }
	}
    else
        {
	parameters = emptyTuple(pp, tok, scope);
	}
    func->next = parameters;
    if (tok->type != ')')
        expectingGot(")", tok);
    tok = tok->next;
    *pTokList = tok;
    }
else if (tok->type == '[')
    {
    while (tok->type == '[')
	{
	struct pfParse *collection = pp;
	pp = pfParseNew(pptIndex, tok, parent, scope);
	pp->children = collection;
	tok = tok->next;
	collection->next = pfParseExpression(pfc, pp, &tok, scope);
	if (tok->type != ']')
	    expectingGot("]", tok);
	tok = tok->next;
	}
    *pTokList = tok;
    }
return pp;
}

static struct pfParse *parseParaInvoke(struct pfCompile *pfc, 
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope, boolean isStatement)
/* Parse para invokation.  This is of general form:
 *    'para' type var 'in' collection action expression
 * where action can be do, add, multiply or, and, min, max, top intExpression,
 * sample intExpression, filter, or get.  In the case where the action is 'do'
 * then the action is followed by a statement rather than an expression. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
struct pfParse *element;
struct pfParse *collection;
struct pfParse *body;
boolean actionNeedsStatement = FALSE;
char *action = NULL;
enum pfParseType paraType = pptNone;

/* Preliminaries, scope and as of yet untyped parse node for para. */
scope = pfScopeNew(pfc, scope, 1, FALSE);
tok = tok->next;	/* Skip over para. */
skipRequiredCharType('(', &tok);
pp = pfParseNew(pptNone, tok, parent, scope);

/* Parse out the element in collection */
element = parseNameUse(pp, &tok, scope);
skipRequiredName("in", &tok);
collection = pfParseExpression(pfc, pp, &tok, scope);

skipRequiredCharType(')', &tok);

/* Parse out the action keyword, and figure out what to do depending on
 * action.  Determine parse node type from action. */
action = pfTokenAsString(tok);
if (sameString("do", action))
    {
    paraType = pptParaDo;
    actionNeedsStatement = TRUE;
    }
else if (sameString("+", action))
    paraType = pptParaAdd;
else if (sameString("*", action))
    paraType = pptParaMultiply;
else if (sameString("&&", action))
    paraType = pptParaAnd;
else if (sameString("||", action))
    paraType = pptParaOr;
else if (sameString("min", action))
    paraType = pptParaMin;
else if (sameString("max", action))
    paraType = pptParaMax;
else if (sameString("get", action))
    paraType = pptParaGet;
else if (sameString("argmax", action))
    paraType = pptParaArgMax;
else if (sameString("argmin", action))
    paraType = pptParaArgMin;
else if (sameString("filter", action))
    paraType = pptParaFilter;
else
    errAt(tok, "unrecognized para action");
if (isStatement != actionNeedsStatement)
    {
    if (isStatement)
        errAt(tok, "para action \"%s\" has a value that needs to be used", action);
    else
        errAt(tok, "para action \"%s\" has no value", action);
    }
freez(&action);
tok = tok->next;
pp->type = paraType;

/* Get expression or statement that gets executed in parallel. */
if (actionNeedsStatement)
    body = pfParseStatement(pfc, pp, &tok, scope);
else
    {
    body = pfParseExpression(pfc, pp, &tok, scope);
    if (isStatement)
	eatSemi(&tok);
    }

/* Hang various things off of para parse node. */
slAddHead(&pp->children, body);
slAddHead(&pp->children, element);
slAddHead(&pp->children, collection);
*pTokList = tok;
return pp;
}



struct pfParse *parseParaExp(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse expression that might include 'para' */
{
struct pfToken *tok = *pTokList;
if (tok->type == pftPara)
    {
    return parseParaInvoke(pfc, parent, pTokList, scope, FALSE);
    }
else
    return parseCallOrIndex(pfc, parent, pTokList, scope);
}

static void makeIncrementNode(struct pfParse *pp,
	struct pfParse *exp, struct pfToken *incTok, struct pfScope *scope)
/* Add children to pp, which is a += node, so as to
 * make it into an increment/decrement operation. */
{
struct pfToken *constantTok;
struct pfParse *constant;

/* We have to fake a token because of the way the constant
 * handling code works. */
constantTok = CloneVar(incTok);
constantTok->type = pftInt;
constantTok->val.i = (incTok->type == pftPlusPlus ? 1 : -1);
constant = pfParseNew(pptConstUse, constantTok, pp, scope);

/* Hook everything onto pp. */
pp->children = exp;
exp->next = constant;
}

static struct pfParse *parseIncrement(struct pfCompile *pfc, 
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope)
/* Parse ++exp or --exp */
{
struct pfToken *tok = *pTokList;
struct pfParse *exp, *pp;
struct pfToken *incTok;

switch (tok->type)
    {
    case pftPlusPlus:
    case pftMinusMinus:
	{
	incTok = tok;
	tok = tok->next;
	pp = pfParseNew(pptPlusEquals, tok, parent, scope);
	exp = parseParaExp(pfc, pp, &tok, scope);
	makeIncrementNode(pp, exp, incTok, scope);
	*pTokList = tok;
	return pp;
	}
    }
exp = parseParaExp(pfc, parent, pTokList, scope);
tok = *pTokList;
switch (tok->type)
    {
    case pftPlusPlus:
    case pftMinusMinus:
	{
	incTok = tok;
	tok = tok->next;
	pp = pfParseNew(pptPlusEquals, tok, parent, scope);
	makeIncrementNode(pp, exp, incTok, scope);
	*pTokList = tok;
	return pp;
	}
    }
return exp;
}

struct pfParse *parseNegation(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse unary minus, and other things at that level. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
if (tok->type == '-')
    {
    pp = pfParseNew(pptNegate, tok, parent, scope);
    tok = tok->next;
    pp->children = parseNegation(pfc, pp, &tok, scope);
    }
else if (tok->type == '!')
    {
    pp = pfParseNew(pptNot, tok, parent, scope);
    tok = tok->next;
    pp->children = parseNegation(pfc, pp, &tok, scope);
    }
else if (tok->type == '~')
    {
    pp = pfParseNew(pptFlipBits, tok, parent, scope);
    tok = tok->next;
    pp->children = parseNegation(pfc, pp, &tok, scope);
    }
else
    {
    pp = parseIncrement(pfc, parent, &tok, scope);
    }
*pTokList = tok;
return pp;
}

struct pfParse *parseProduct(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse things separated by '*' or '/' or '%'. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseNegation(pfc, parent, &tok, scope);
while (tok->type == '*' || tok->type == '/' || tok->type == '%'
	|| tok->type == pftShiftLeft || tok->type == pftShiftRight
	|| tok->type == '&' && tok->next->type != ')')
    {
    struct pfParse *left = pp, *right;
    enum pfTokType tt = pptNone;
    switch (tok->type)
        {
	case '*':
	   tt = pptMul;
	   break;
	case '/':
	   tt = pptDiv;
	   break;
	case '%':
	   tt = pptMod;
	   break;
	case pftShiftLeft:
	   tt = pptShiftLeft;
	   break;
	case pftShiftRight:
	   tt = pptShiftRight;
	   break;
	case '&':
	   tt = pptBitAnd;
	   break;
	}
    pp = pfParseNew(tt, tok, parent, scope);
    left->parent = pp;
    tok = tok->next;
    right = parseNegation(pfc, pp, &tok, scope);
    pp->children = left;
    left->next = right;
    }
*pTokList = tok;
return pp;
}

struct pfParse *parseSum(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse + and - part of expression. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseProduct(pfc, parent, &tok, scope);
while (tok->type == '+' || tok->type == '-' || tok->type == '|' || tok->type == '^' && tok->next->type != ')')
    {
    struct pfParse *left = pp, *right;
    enum pfTokType tt = pptNone;
    switch (tok->type)
        {
	case '+':
	   tt = pptPlus;
	   break;
	case '-':
	   tt = pptMinus;
	   break;
	case '|':
	   tt = pptBitOr;
	   break;
	case '^':
	   tt = pptBitXor;
	   break;
	}
    pp = pfParseNew(tt, tok, parent, scope);
    left->parent = pp;
    tok = tok->next;
    right = parseProduct(pfc, pp, &tok, scope);
    pp->children = left;
    left->next = right;
    }
*pTokList = tok;
return pp;
}

struct pfParse *parseCmp(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse == >=, <= > < and != part of expression. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseSum(pfc, parent, &tok, scope);
if (tok->type == pftEqualsEquals || tok->type == pftNotEquals 
	|| tok->type == pftGreaterOrEquals || tok->type == pftLessOrEquals
	|| tok->type == '<' || tok->type == '>')
    {
    struct pfParse *left = pp, *right;
    enum pfTokType tt = 0;
    switch (tok->type)
	{
	case pftEqualsEquals:
	    tt = pptSame;
	    break;
	case pftNotEquals:
	    tt = pptNotSame;
	    break;
	case pftGreaterOrEquals:
	    tt = pptGreaterOrEquals;
	    break;
	case pftLessOrEquals:
	    tt = pptLessOrEquals;
	    break;
	case '>':
	    tt = pptGreater;
	    break;
	case '<':
	    tt = pptLess;
	    break;
	}
    pp = pfParseNew(tt, tok, parent, scope);
    left->parent = pp;
    tok = tok->next;
    right = parseSum(pfc, pp, &tok, scope);
    pp->children = left;
    left->next = right;
    }
*pTokList = tok;
return pp;
}

struct pfParse *parseLogAnd(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse logical and. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseCmp(pfc, parent, &tok, scope);
while (tok->type == pftLogAnd && tok->next->type != ')')
    {
    struct pfParse *left = pp, *right;
    pp = pfParseNew(pptLogAnd, tok, parent, scope);
    left->parent = pp;
    tok = tok->next;
    right = parseCmp(pfc, pp, &tok, scope);
    pp->children = left;
    left->next = right;
    }
*pTokList = tok;
return pp;
}

struct pfParse *parseLogOr(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse logical and. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseLogAnd(pfc, parent, &tok, scope);
while (tok->type == pftLogOr && tok->next->type != ')')
    {
    struct pfParse *left = pp, *right;
    pp = pfParseNew(pptLogOr, tok, parent, scope);
    left->parent = pp;
    tok = tok->next;
    right = parseLogAnd(pfc, pp, &tok, scope);
    pp->children = left;
    left->next = right;
    }
*pTokList = tok;
return pp;
}

struct pfParse *parseIndexRange(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse range (X to Y) */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseLogOr(pfc, parent, &tok, scope);
if (tok->type == pftTo)
     {
     struct pfParse *left = pp, *right;
     pp = pfParseNew(pptIndexRange, tok, parent, scope);
     left->parent = pp;
     tok = tok->next;
     right = parseLogOr(pfc, parent, &tok, scope);
     pp->children = left;
     left->next = right;
     }
*pTokList = tok;
return pp;
}

static void checkNoNestedAssigns(struct pfParse *pp)
/* Make sure that there are no assignments inside of subtree. */
{
switch (pp->type)
    {
    case pptAssignment:
    case pptPlusEquals:
    case pptMinusEquals:
    case pptMulEquals:
    case pptDivEquals:
        errAt(pp->tok, "nested assignments not allowed in paraFlow");
	break;
    }
for (pp = pp->children; pp != NULL; pp = pp->next)
    checkNoNestedAssigns(pp);
}

struct pfParse *parseAssign(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse '=' separated expression */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseIndexRange(pfc, parent, &tok, scope);
struct pfParse *assign = NULL;
enum pfParseType type = pptNone;

switch (tok->type)
    {
    case '=':
        type = pptAssignment;
	break;
    case pftPlusEquals:
	type = pptPlusEquals;
	break;
    case pftMinusEquals:
	type = pptMinusEquals;
	break;
    case pftMulEquals:
	type = pptMulEquals;
	break;
    case pftDivEquals:
	type = pptDivEquals;
	break;
    case ':':
        type = pptKeyVal;
	break;
    }
if (type != pptNone)
    {
    assign = pfParseNew(type, tok, parent, scope);
    assign->children = pp;
    assign->isStatic = pp->isStatic;
    pp->parent = assign;
    for (;;)
	{
	tok = tok->next;
	pp = parseIndexRange(pfc, assign, &tok, scope);
	checkNoNestedAssigns(pp);
	slAddHead(&assign->children, pp);
	if (tok->type != '=' || type != pptAssignment)
	    break;
	}
    }
*pTokList = tok;
if (assign != NULL)
    {
    slReverse(&assign->children);
    return assign;
    }
else
    return pp;
}

static void flipNameUseToVarDec(struct pfParse *pp, struct pfParse *type, struct pfParse *parent)
{
struct pfParse *dupeType, *dupeName;

AllocVar(dupeName);
*dupeName = *pp;

AllocVar(dupeType);
*dupeType = *type;
pp->children = dupeType;
dupeType->next = dupeName;
dupeName->next = NULL;
pp->type = pptVarDec;
}

static void addMissingTypesInDeclareTuple(struct pfParse *tuple)
/* If first element of tuple is typed, then make sure rest
 * is typed too. */
{
struct pfParse *vars = tuple->children, *next;
if (vars != NULL)
    {
    if (vars->type == pptVarDec || 
        vars->type == pptAssignment && vars->children->type == pptVarDec)
	{
	struct pfParse *type = NULL;
	struct pfParse *pp;
	boolean isStatic = vars->isStatic;
	for (pp = vars; pp != NULL; pp = pp->next)
	    {
	    if (pp->type == pptVarDec)
		{
	        type = pp->children;
		pp->name = type->next->name;
		pp->isStatic = isStatic;
		}
	    else if (pp->type == pptNameUse)
	        {
		flipNameUseToVarDec(pp, type, tuple);
		pp->isStatic = isStatic;
		}
	    else if (pp->type == pptAssignment)
	        {
		struct pfParse *sub = pp->children;
		if (sub->type == pptVarDec)
		    {
		    type = pp->children->children;
		    sub->name = type->next->name;
		    }
		else if (sub->type == pptNameUse)
		    {
		    flipNameUseToVarDec(sub, type, pp);
		    }
		sub->isStatic = pp->isStatic = isStatic;
		}
	    }
	tuple->isStatic = isStatic;
	}
    }
}


struct pfParse *parseTuple(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse , separated expression. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = parseAssign(pfc, parent, &tok, scope);
struct pfParse *tuple = NULL;
if (tok->type == ',')
    {
    tuple = pfParseNew(pptTuple, tok, parent, scope);
    tuple->children = pp;
    pp->parent = tuple;
    while (tok->type == ',')
	{
	tok = tok->next;
	if (tok->type == ')')
	    break;
	pp = parseAssign(pfc, tuple, &tok, scope);
	slAddHead(&tuple->children, pp);
	}
    }
*pTokList = tok;
if (tuple != NULL)
    {
    slReverse(&tuple->children);
    addMissingTypesInDeclareTuple(tuple);
    return tuple;
    }
else
    return pp;
}


struct pfParse *pfParseExpression(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse expression. */
{
return parseTuple(pfc, parent, pTokList, scope);
}

static struct pfParse *parseCompound(struct pfCompile *pfc,
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope)
/* Parse a compound statement (statement list surrounded
 * by brackets */
{
struct pfParse *pp;
struct pfToken *tok = *pTokList;
struct pfParse *statement;
if (tok->type != '{')
    expectingGot("{", tok);
tok->val.scope->parent = scope;
scope = tok->val.scope;
pp = pfParseNew(pptCompound, *pTokList, parent, scope);
tok = tok->next;
for (;;)
    {
    if (tok == NULL)
	{
	errAt(*pTokList, "End of file in compound statement");
	}
    if (tok->type == '}')
        {
	tok = tok->next;
	break;
	}
    statement = pfParseStatement(pfc, pp, &tok, scope);
    slAddHead(&pp->children, statement);
    }
*pTokList = tok;
slReverse(&pp->children);
return pp;
}

static struct pfParse *parseFunction(struct pfCompile *pfc, 
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope, enum pfParseType type)
/* Parse something (...) [into (...)] */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
struct pfParse *name, *input, *output = NULL, *body = NULL;

scope = pfScopeNew(pfc, scope, 0, FALSE);
pp = pfParseNew(type, tok, parent, scope);
tok = tok->next;	/* Skip to/flow (implicit in type) */
name = parseNameUse(parent, &tok, scope);
pp->name = name->name;

parseFunctionIo(pfc, pp, &tok, scope, &input, &output);

if (tok->type == ';')
    {
    tok = tok->next;
    }
else
    {
    body = parseCompound(pfc, pp, &tok, scope);
    slAddHead(&pp->children, body);
    }

slAddHead(&pp->children, output);
slAddHead(&pp->children, input);
slAddHead(&pp->children, name);
*pTokList = tok;
return pp;
}

static struct pfParse *parseTo(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse to (...) [into (...)] */
{
return parseFunction(pfc, parent, pTokList, scope, pptToDec);
}


static struct pfParse *parseFlow(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse flow (...) [into (...)] */
{
return parseFunction(pfc, parent, pTokList, scope, pptFlowDec);
}

static struct pfParse *parseOperator(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse to (...) [into (...)] */
{
if (!pfc->isSys)
    {
    struct pfToken *tok = *pTokList;
    errAt(tok, "the word '%s' is reserved for system use", 
    	pfTokenAsString(tok));
    }
return parseFunction(pfc, parent, pTokList, scope, pptOperatorDec);
}


static struct pfParse *parsePolymorphic(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse flow (...) [into (...)] */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(pptPolymorphic, tok, parent, scope);
if (pfParseEnclosingClass(parent) == NULL)
    errAt(tok, "polymorphic keyword outside of class");
tok = tok->next;
switch (tok->type)
    {
    case pftTo:
    case pftPara:
    case pftFlow:
        pp->children = pfParseStatement(pfc, pp, &tok, scope);
	break;
    default:
        expectingGot("Expecting function declaration after polymorphic", tok);
	break;
    }
*pTokList = tok;
return pp;
}



static void checkForCircularity(struct pfBaseType *parent, struct pfBaseType *newBase,
	struct pfToken *tok)
/* Make sure that newBase is not parent, grandparent, etc of itself. */
{
while (parent != NULL)
    {
    if (parent == newBase)
        errAt(tok, "loop in inheritance tree");
    parent = parent->parent;
    }
}

static struct pfParse *parseClassOrInterface(struct pfCompile *pfc,
	struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope,
	enum pfParseType pptType)
/* Parse class declaration statement. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
struct pfParse *name, *body, *extends = NULL;
struct pfBaseType *myBase;
pp = pfParseNew(pptType, tok, parent, scope);
tok = tok->next;	/* Skip 'class' or 'interface' token */
name = parseNameUse(pp, &tok, scope);
name->type = pptTypeName;
pp->name = name->name;
myBase = pfScopeFindType(scope, name->name);
if (myBase == NULL)
    internalErr();
myBase->def = pp;
if (tok->type == pftExtends)
    {
    struct pfBaseType *parentBase;
    tok = tok->next;
    if (tok->type != pftName)
        expectingGot("name", tok);
    parentBase = pfScopeFindType(scope, tok->val.s);
    if (parentBase == NULL)
	errAt(tok, "undefined parent %s", tok->val.s);
    if (parentBase->isClass != myBase->isClass)
        errAt(tok, "can't mix classes and interfaces in extensions");
    extends = pfParseNew(pptTypeName, tok, pp, scope);
    extends->ty = pfTypeNew(parentBase);
    extends->name = tok->val.s;
    checkForCircularity(parentBase, myBase, tok);
    myBase->parent = parentBase;
    slAddHead(&pp->children, extends);
    tok = tok->next;
    }
body = parseCompound(pfc, pp, &tok, scope);
body->scope->class = myBase;
slAddHead(&pp->children, body);
slAddHead(&pp->children, name);
*pTokList = tok;
return pp;
}

static struct pfParse *parseClass(struct pfCompile *pfc,
	struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope)
/* Parse class declaration statement. */
{
return parseClassOrInterface(pfc, parent, pTokList, scope, pptClass);
}

static struct pfParse *parseInterface(struct pfCompile *pfc,
	struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope)
/* Parse interface declaration statement. */
{
return parseClassOrInterface(pfc, parent, pTokList, scope, pptInterface);
}



static struct pfParse *parseIf(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse if statement (which may include else) */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(pptIf, tok, parent, scope);
struct pfParse *conditional, *trueBody, *falseBody = NULL;
tok = tok->next;	/* Skip 'if' */

/* Get conditional. */
if (tok->type != '(')
    expectingGot("(", tok);
tok = tok->next;
conditional = pfParseExpression(pfc, pp, &tok, scope);
skipRequiredCharType(')', &tok);

/* Body */
trueBody = pfParseStatement(pfc, pp, &tok, scope);

/* Else clause. */
if (tok->type == pftElse)
    {
    tok = tok->next;
    falseBody = pfParseStatement(pfc, pp, &tok, scope);
    }
*pTokList = tok;

pp->children = conditional;
conditional->next = trueBody;
trueBody->next = falseBody;
return pp;
}

static struct pfParse *parseWhile(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse if statement (which may include else) */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(pptWhile, tok, parent, scope);
struct pfParse *conditional, *body;

tok = tok->next;	/* Skip 'while' */

/* Get conditional. */
if (tok->type != '(')
    expectingGot("(", tok);
tok = tok->next;
conditional = pfParseExpression(pfc, pp, &tok, scope);
if (tok->type != ')')
    expectingGot(")", tok);
tok = tok->next;

/* Body */
body = pfParseStatement(pfc, pp, &tok, scope);

*pTokList = tok;

pp->children = conditional;
conditional->next = body;
return pp;
}


static struct pfParse *parseForeach(struct pfCompile *pfc, 
	struct pfParse *parent, struct pfToken **pTokList, struct pfScope *scope)
/* Parse foreach statement */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
struct pfParse *element;
struct pfParse *collection;
struct pfParse *statement;

scope = pfScopeNew(pfc, scope, 1, FALSE);
pp = pfParseNew(pptForeach, tok, parent, scope);
tok = tok->next;	/* Skip over 'foreach' */
skipRequiredCharType('(', &tok);
element = pfParseExpression(pfc, pp, &tok, scope);

skipRequiredName("in", &tok);
collection = pfParseExpression(pfc, pp, &tok, scope);
skipRequiredCharType(')', &tok);
statement = pfParseStatement(pfc, pp, &tok, scope);
slAddHead(&pp->children, statement);
slAddHead(&pp->children, element);
slAddHead(&pp->children, collection);
*pTokList = tok;
return pp;
}

static struct pfParse *parseExpressionAndSemi(struct pfCompile *pfc,
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope)
/* Parse expression and following semicolon if any. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
if (tok->type == ';')
    pp = emptyStatement(parent, tok, scope);
else
    pp = pfParseExpression(pfc, parent, &tok, scope);
eatSemi(&tok);
*pTokList = tok;
return pp;
}


static struct pfParse *parseParaStatement(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse when it starts with 'para' - will either be a para function declaration
 * or a para invokation. */
{
struct pfToken *tok = *pTokList;	/* "para" */
tok = tok->next;
if (tok->type != '(')
    expectingGot("(", tok);
return parseParaInvoke(pfc, parent, pTokList, scope, TRUE);
}



static struct pfParse *parseFor(struct pfCompile *pfc, struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse for statement */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp;
struct pfParse *init;
struct pfParse *check;
struct pfParse *advance;
struct pfParse *statement;

scope = pfScopeNew(pfc, scope, 1, FALSE);
pp = pfParseNew(pptFor, tok, parent, scope);
tok = tok->next;	/* Skip over 'for' */
if (tok->type != '(')
    expectingGot("(", tok);
tok = tok->next;

init = parseExpressionAndSemi(pfc, pp, &tok, scope);
check = parseExpressionAndSemi(pfc, pp, &tok, scope);

/* Parse advance statment and closing ')' */
if (tok->type == ')')
    advance = emptyStatement(pp, tok, scope);
else
    advance = pfParseExpression(pfc, pp, &tok, scope);
if (tok->type != ')')
    expectingGot(")", tok);
tok = tok->next;

/* Get body of loop. */
statement = pfParseStatement(pfc, pp, &tok, scope);

/* Hang various subparts of for statement onto children list. */
slAddHead(&pp->children, statement);
slAddHead(&pp->children, advance);
slAddHead(&pp->children, check);
slAddHead(&pp->children, init);
*pTokList = tok;
return pp;
}


static struct pfParse *parseWordStatement(struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope, enum pfParseType type)
/* Parse one word statement. */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(type, tok, parent, scope);
tok = tok->next;
eatSemi(&tok);
*pTokList = tok;
return pp;
}

static struct pfParse *parseBreak(struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse break statement */
{
return parseWordStatement(parent, pTokList, scope, pptBreak);
}

static struct pfParse *parseContinue(struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse continue statement */
{
return parseWordStatement(parent, pTokList, scope, pptContinue);
}
    	
static struct pfParse *parseReturn(struct pfParse *parent,
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse return statement */
{
return parseWordStatement(parent, pTokList, scope, pptReturn);
}

static struct pfParse *parseInclude(struct pfCompile *pfc, 
	struct pfParse *parent, struct pfToken **pTokList, 
	struct pfScope *scope)
/* Parse into statement */
{
struct pfToken *tok = *pTokList;
struct pfParse *pp = pfParseNew(pptInclude, tok, parent, scope);

tok = tok->next;	/* Have covered 'into' */
pp->children = parseDottedNames(pp, &tok, scope);
pp->name = pp->children->name;	/* Need to make this cope with dots FIXME */

*pTokList = tok;
return pp;
}

struct pfParse *pfParseStatement(struct pfCompile *pfc, struct pfParse *parent, 
	struct pfToken **pTokList, struct pfScope *scope)
/* Parse a single statement up to but not including semicolon,
 * and add results to (head) of
 * parent->childList. */
{
struct pfToken *tok = *pTokList;
struct pfParse *statement;

switch (tok->type)
    {
    case pftInclude:
        statement = parseInclude(pfc, parent, &tok, scope);
	break;
    case ';':
	statement = emptyStatement(parent, tok, scope);
	tok = tok->next;
	break;
    case '{':
	statement = parseCompound(pfc, parent, &tok, scope);
	break;
    case pftIf:
        statement = parseIf(pfc, parent, &tok, scope);
	break;
    case pftWhile:
        statement = parseWhile(pfc, parent, &tok, scope);
	break;
    case pftForeach:
	statement = parseForeach(pfc, parent, &tok, scope);
	break;
    case pftFor:
	statement = parseFor(pfc, parent, &tok, scope);
	break;
    case pftBreak:
    	statement = parseBreak(parent, &tok, scope);
	break;
    case pftContinue:
        statement = parseContinue(parent, &tok, scope);
        break;
    case pftReturn:
    	statement = parseReturn(parent, &tok, scope);
	break;
    case pftClass:
	statement = parseClass(pfc, parent, &tok, scope);
	break;
    case pftInterface:
        statement = parseInterface(pfc, parent, &tok, scope);
	break;
    case pftTo:
	statement = parseTo(pfc, parent, &tok, scope);
	break;
    case pftOperator:
        statement = parseOperator(pfc, parent, &tok, scope);
	break;
    case pftPara:
	statement = parseParaStatement(pfc, parent, &tok, scope);
	break;
    case pftFlow:
	statement = parseFlow(pfc, parent, &tok, scope);
	break;
    case pftPolymorphic:
    	statement = parsePolymorphic(pfc, parent, &tok, scope);
	break;
    case pftStatic:
    case pftName:
    case pftPlusPlus:
    case pftMinusMinus:
    case '(':
	statement = pfParseExpression(pfc, parent, &tok, scope);
	eatSemi(&tok);
	break;
    default:
        expectingGot("statement", tok);
	statement = NULL;
	break;
    }
*pTokList = tok;
return statement;
}


static struct pfParse *pfParseTokens(struct pfCompile *pfc,
	struct pfToken *tokList, struct pfScope *scope,
	struct pfParse *program)
/* Convert token list to parsed program. */
{
struct pfToken *tok = tokList;

while (tok->type != pftEnd)
    {
    struct pfParse *statement = pfParseStatement(pfc, program, &tok, scope);
    slAddHead(&program->children, statement);
    }
slReverse(&program->children);
return program;
}


static void subTypeForName(struct pfParse *pp)
/* Substititute pptTypeName for pptName use where appropriate. */
{
if (pp->type == pptNameUse)
    pp->type = pptTypeName;
else if (pp->type == pptOf)
    {
    for (pp = pp->children; pp != NULL; pp = pp->next)
	{
	subTypeForName(pp);
	}
    }
}

void varDecAndAssignToVarInit(struct pfParse *pp)
/* Convert pptVarDec and when appropriate pptAssignment to pptVarInit. */
{
if (pp->type == pptVarDec)
    {
    pp->type = pptVarInit;
    pp->name = pp->children->next->name;
    }
else if (pp->type == pptAssignment)
    {
    struct pfParse *left = pp->children;
    if (left->type == pptVarDec)
        {
	struct pfParse *right = left->next;
	pp->type = pptVarInit;
	pp->isStatic = left->isStatic;
	pp->children = left->children;
	pp->name = left->name;
	slAddTail(&pp->children, right);
	}
    }
if (pp->type == pptVarInit)
    {
    struct pfParse *type = pp->children;
    struct pfParse *name = type->next;
    subTypeForName(type);
    name->type = pptSymName;
    pp->name = name->name;
    }
/* Note in this case we can't process children before self.
 * This is one of the reasons this is done in a separate
 * pass actually. */
for (pp = pp->children; pp != NULL; pp = pp->next)
    varDecAndAssignToVarInit(pp);
}

struct pfParse *pfParseModule(struct pfCompile *pfc, struct pfModule *module,
	struct pfParse *parent, struct pfScope *scope, enum pfParseType modType)
/* Parse a module and return parse tree associated with it. */
{
struct pfParse *modParse = pfParseNew(modType, NULL, parent, scope);
pfParseTokens(pfc, module->tokList, scope, modParse);
varDecAndAssignToVarInit(modParse);
module->pp = modParse;
return modParse;
}

