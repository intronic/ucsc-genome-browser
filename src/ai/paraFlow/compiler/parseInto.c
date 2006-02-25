/* parseInto - Handle parsing across multiple modules. */

#include "common.h"
#include "hash.h"
#include "dystring.h"
#include "localmem.h"
#include "portable.h"
#include "pfType.h"
#include "pfScope.h"
#include "pfToken.h"
#include "pfCompile.h"
#include "pfParse.h"
#include "pfBindVars.h"
#include "defFile.h"
#include "parseInto.h"


static struct pfType *findTypeInModule(struct pfParse *module, char *typeName)
/* Return base type in module or die. */
{
struct pfBaseType *base = pfScopeFindType(module->scope, typeName);
if (base == NULL)
    errAbort("Can't find class %s in %s", typeName, module->name);
return pfTypeNew(base);
}

static void attachStringsAndThings(struct pfCompile *pfc, 
	struct pfParse *stringModule)
/* Finish parsing, binding and type checking the string module.  
 * Rummage around for string class and some other things we need
 * and attach them to the pfCompile struct. */
{
pfBindVars(pfc, stringModule);
pfTypeCheck(pfc, &stringModule);
pfc->elTypeFullType = findTypeInModule(stringModule, "_pf_elType");
pfc->strFullType = findTypeInModule(stringModule, "_pf_string");
pfc->dyStrFullType = findTypeInModule(stringModule, "_pf_dyString");
pfc->arrayFullType = findTypeInModule(stringModule, "_pf_array");
pfc->arrayType->methods = pfc->arrayFullType->base->methods;
pfc->dirFullType = findTypeInModule(stringModule, "_pf_dir");
pfc->dirType->methods = pfc->dirFullType->base->methods;
}

static void addCompoundScopes(struct pfCompile *pfc, struct pfToken *tokList,
	struct pfScope *scope)
/* Add scoping info to opening braces in token list */
{
int braceDepth = 0;
struct pfToken *tok;
for (tok = tokList; tok != NULL; tok = tok->next)
    {
    if (tok->type == '{')
        {
	++braceDepth;
	scope = pfScopeNew(pfc, scope, 0, FALSE);
	tok->val.scope = scope;
	}
    else if (tok->type == '}')
        {
	if (braceDepth == 0)
	    errAt(tok, "} without preceding {");
	--braceDepth;
	scope = scope->parent;
	tok->val.scope = scope;
	}
    }
if (braceDepth != 0)
    errAbort("Unclosed brace in %s", tokList->source->name);
}

static void addClasses(struct pfCompile *pfc, struct pfToken *tokList, 
	struct pfScope *scope)
/* Add types in classes to appropriate scope.  We do this as
 * a first pass to help disambiguate the grammar. */
{
struct pfToken *tok;
for (tok = tokList; tok->type != pftEnd; tok = tok->next)
    {
    if (tok->type == '{' || tok->type == '}')
	{
        scope = tok->val.scope;
	}
    else if (tok->type == pftClass)
	{
	struct pfBaseType *base;
	tok = tok->next;
	if (tok->type != pftName)
	    expectingGot("class name", tok);
	base = pfScopeAddType(scope, tok->val.s, FALSE, 
		pfc->classType, sizeof(void *), TRUE);
	base->isClass = TRUE;
	}
    else if (tok->type == pftInterface)
	{
	struct pfBaseType *base;
	tok = tok->next;
	if (tok->type != pftName)
	    expectingGot("interface name", tok);
	base = pfScopeAddType(scope, tok->val.s, FALSE,
	    pfc->interfaceType, sizeof(void *), TRUE);
	base->isInterface = TRUE;
	}
    }
}

struct pfParse *pfParseInto(struct pfCompile *pfc)
/* Parse file.  Also parse .pfh files for any modules the main file goes into.
 * If the .pfh files don't exist then create them.  Returns parse tree
 * of file and associated .pfh files.  Variables are not yet bound. */
{
struct pfParse *program = pfParseNew(pptProgram, NULL, NULL, pfc->scope);
struct pfModule *stringModule = hashMustFindVal(pfc->moduleHash, "<string>");
struct pfModule *builtInModule = hashMustFindVal(pfc->moduleHash, "<builtIn>");
struct pfScope *sysScope = pfc->scope;
struct pfScope *userScope  = pfScopeNew(pfc, pfc->scope, 16, TRUE);
struct pfModule *mod;

/* Loop through assigning scopes and recording classes. */
for (mod = pfc->moduleList; mod != NULL; mod = mod->next)
    {
    struct pfScope *scope = (mod->name[0] == '<' ? sysScope : userScope);
    addCompoundScopes(pfc, mod->tokList, scope);
    addClasses(pfc, mod->tokList, scope);
    }

/* Compile everything. */
for (mod = pfc->moduleList; mod != NULL; mod = mod->next)
    {
    struct pfParse *modPp;
    boolean isSys = (mod->name[0] == '<');
    struct pfScope *scope = (isSys ? sysScope : userScope);
    enum pfParseType type = pptMainModule;
    if (mod != pfc->moduleList)
	type = (mod->isPfh ? pptModuleRef : pptModule);
    pfc->isSys = isSys;
    modPp = pfParseModule(pfc, mod, program, scope, type);
    modPp->name = mod->name;
    /* Exclude string module since code generator can't handle it. */
    if (mod != stringModule)
	slAddHead(&program->children, modPp);
    if (type != pptModuleRef)
        {
	char defFile[PATH_LEN];
	safef(defFile, sizeof(defFile), "%s%s.pfh", pfc->baseDir, mod->name);
	pfMakeDefFile(pfc, modPp, defFile);
	}
    }
slReverse(&program->children);
attachStringsAndThings(pfc, stringModule->pp);

/* Restore order of scopes. */
slReverse(&pfc->scopeRefList);

return program;
}


