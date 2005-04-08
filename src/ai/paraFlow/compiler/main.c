/* Command line main driver module for paraFlow compiler. */
/* Copyright 2005 Jim Kent.  All rights reserved. */
#include "common.h"
#include "linefile.h"
#include "options.h"
#include "hash.h"
#include "dystring.h"
#include "localmem.h"
#include "pfType.h"
#include "pfScope.h"
#include "pfToken.h"
#include "pfCompile.h"
#include "pfParse.h"
#include "pfBindVars.h"
#include "pfCheck.h"

boolean parseOnly = FALSE;

void usage()
/* Explain command line and exit. */
{
errAbort(
"This program compiles paraFlow code.  ParaFlow is a parallel programming\n"
"language designed by Jim Kent.\n"
"Usage:\n"
"    paraFlow input.pf\n"
"options:\n"
"    -parseOnly  - Just parse, don't do type checking or anything\n"
);

}

static struct optionSpec options[] = {
   {"parseOnly", OPTION_BOOLEAN},
   {NULL, 0},
};

static struct hash *createReservedWords()
/* Create reserved words table. */
{
struct hash *hash = hashNew(7);
hashAddInt(hash, "class", pftClass);
hashAddInt(hash, "to", pftTo);
hashAddInt(hash, "para", pftPara);
hashAddInt(hash, "flow", pftFlow);
hashAddInt(hash, "into", pftInto);
hashAddInt(hash, "for", pftFor);
hashAddInt(hash, "foreach", pftForeach);
hashAddInt(hash, "while", pftWhile);
hashAddInt(hash, "of", pftOf);
hashAddInt(hash, "if", pftIf);
hashAddInt(hash, "else", pftElse);
hashAddInt(hash, "break", pftBreak);
hashAddInt(hash, "return", pftReturn);
hashAddInt(hash, "continue", pftContinue);
return hash;
}

static void rParseCount(int *pCount, struct pfParse *pp)
/* Recursively count. */
{
*pCount += 1;
for (pp = pp->children; pp != NULL; pp = pp->next)
   rParseCount(pCount, pp);
}

int pfParseCount(struct pfParse *pp)
/* Count up number of parses in this el and children. */
{
int parseCount = 0;
rParseCount(&parseCount, pp);
return parseCount;
}

static void printScopeInfo(int level, struct pfParse *pp)
/* Print out info on each new scope. */
{
switch (pp->type)
    {
    case pptProgram:
    case pptModule:
    case pptFor:
    case pptForeach:
    case pptToDec:
    case pptFlowDec:
    case pptParaDec:
    case pptCompound:
	{
	char *name = pp->name;
	if (name == NULL)
	    name = "";
        spaceOut(stdout, 2*level);
	printf("scope %s %s: ", 
		pfParseTypeAsString(pp->type), name);
	pfScopeDump(pp->scope, stdout);
	printf("\n");
	break;
	}
    }
for (pp = pp->children; pp != NULL; pp = pp->next)
    printScopeInfo(level+1, pp);
}

static void addBuiltInTypes(struct pfCompile *pfc)
/* Add built in types . */
{
struct pfScope *scope = pfc->scope;

/* Declare some basic types.  Types with names in parenthesis
 * are never declared by user directly */
pfc->varType = pfScopeAddType(scope, "var", FALSE, NULL);
pfc->keyValType = pfScopeAddType(scope, "(keyVal)", FALSE, pfc->varType);
pfc->streamType = pfScopeAddType(scope, "(stream)", FALSE, pfc->varType);
pfc->numType = pfScopeAddType(scope, "(number)", FALSE, pfc->varType);
pfc->collectionType = pfScopeAddType(scope, "(collection)", TRUE, pfc->varType);
pfc->tupleType = pfScopeAddType(scope, "(tuple)", TRUE, pfc->collectionType);
pfc->classType = pfScopeAddType(scope, "(class)", TRUE, pfc->collectionType);
pfc->functionType = pfScopeAddType(scope, "(function)", TRUE, pfc->varType);
pfc->toType = pfScopeAddType(scope, "to", TRUE, pfc->functionType);
pfc->paraType = pfScopeAddType(scope, "para", TRUE, pfc->functionType);
pfc->flowType = pfScopeAddType(scope, "flow", TRUE, pfc->functionType);

pfc->bitType = pfScopeAddType(scope, "bit", FALSE, pfc->numType);
pfc->byteType = pfScopeAddType(scope, "byte", FALSE, pfc->numType);
pfc->shortType = pfScopeAddType(scope, "short", FALSE, pfc->numType);
pfc->intType = pfScopeAddType(scope, "int", FALSE, pfc->numType);
pfc->longType = pfScopeAddType(scope, "long", FALSE, pfc->numType);
pfc->floatType = pfScopeAddType(scope, "float", FALSE, pfc->numType);
pfc->doubleType = pfScopeAddType(scope, "double", FALSE, pfc->numType);

pfc->stringType = pfScopeAddType(scope, "string", FALSE, pfc->streamType);

pfc->arrayType = pfScopeAddType(scope, "array", TRUE, pfc->collectionType);
pfc->listType = pfScopeAddType(scope, "list", TRUE, pfc->collectionType);
pfc->treeType = pfScopeAddType(scope, "tree", TRUE, pfc->collectionType);
pfc->treeType->keyedBy = pfc->doubleType;
pfc->dirType = pfScopeAddType(scope, "dir", TRUE, pfc->collectionType);
pfc->dirType->keyedBy = pfc->stringType;
}

struct pfCompile *pfCompileNew(char *fileName)
/* Make new compiler object. */
{
struct pfCompile *pfc;
AllocVar(pfc);
pfc->baseFile = cloneString(fileName);
pfc->modules = hashNew(0);
pfc->reservedWords = createReservedWords();
pfc->scope = pfScopeNew(NULL, 8);
addBuiltInTypes(pfc);
pfc->tkz = pfTokenizerNew(fileName, pfc->reservedWords);
return pfc;
}

void paraFlow(char *fileName)
/* parse and dump. */
{
struct pfCompile *pfc = pfCompileNew(fileName);
struct pfParse *program = pfParseProgram(fileName, pfc);

if (!parseOnly)
    {
    pfBindVars(pfc, program);
    pfTypeCheck(pfc, &program);
    pfCheckParaFlow(pfc, program);
    }

pfParseDump(program, 0, stdout);

printf("%d modules, %d tokens, %d parseNodes\n",
	pfc->modules->elCount, pfc->tkz->tokenCount, pfParseCount(program));
printScopeInfo(0, program);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 2)
    usage();
parseOnly = optionExists("parseOnly");
paraFlow(argv[1]);
return 0;
}

