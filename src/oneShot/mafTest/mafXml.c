/* maf.c autoXml generated file */

#include "common.h"
#include "xap.h"
#include "maf.h"

void *mafStartHandler(struct xap *xp, char *name, char **atts);
/* Called by expat with start tag.  Does most of the parsing work. */

void mafEndHandler(struct xap *xp, char *name);
/* Called by expat with end tag.  Checks all required children are loaded. */


void mafMafSave(struct mafFile *obj, int indent, FILE *f)
/* Save mafFile to file. */
{
struct mafAli *mafAli;
boolean isNode = TRUE;
if (obj == NULL) return;
xapIndent(indent, f);
fprintf(f, "<MAF");
fprintf(f, " version=\"%d\"", obj->version);
if (obj->scoring != NULL)
    fprintf(f, " scoring=\"%s\"", obj->scoring);
fprintf(f, ">");
for (mafAli = obj->mafAli; mafAli != NULL; mafAli = mafAli->next)
   {
   if (isNode)
       {
       fprintf(f, "\n");
       isNode = FALSE;
       }
   mafAliSave(mafAli, indent+2, f);
   }
if (!isNode)
    xapIndent(indent, f);
fprintf(f, "</MAF>\n");
}

struct mafFile *mafMafLoad(char *fileName)
/* Load mafFile from file. */
{
struct mafFile *obj;
xapParseAny(fileName, "MAF", mafStartHandler, mafEndHandler, NULL, &obj);
return obj;
}

void mafAliSave(struct mafAli *obj, int indent, FILE *f)
/* Save mafAli to file. */
{
struct mafS *mafS;
if (obj == NULL) return;
xapIndent(indent, f);
fprintf(f, "<ALI");
if (obj->score != 0.0)
   fprintf(f, " score=\"%f\"", obj->score);
fprintf(f, ">");
fprintf(f, "\n");
for (mafS = obj->mafS; mafS != NULL; mafS = mafS->next)
   {
   mafSSave(mafS, indent+2, f);
   }
xapIndent(indent, f);
fprintf(f, "</ALI>\n");
}

struct mafAli *mafAliLoad(char *fileName)
/* Load mafAli from file. */
{
struct mafAli *obj;
xapParseAny(fileName, "ALI", mafStartHandler, mafEndHandler, NULL, &obj);
return obj;
}

void mafSSave(struct mafS *obj, int indent, FILE *f)
/* Save mafS to file. */
{
if (obj == NULL) return;
xapIndent(indent, f);
fprintf(f, "<S");
fprintf(f, " seq=\"%s\"", obj->seq);
fprintf(f, " seqSize=\"%d\"", obj->seqSize);
fprintf(f, " strand=\"%s\"", obj->strand);
fprintf(f, " start=\"%d\"", obj->start);
fprintf(f, " size=\"%d\"", obj->size);
fprintf(f, ">");
fprintf(f, "%s", obj->text);
fprintf(f, "</S>\n");
}

struct mafS *mafSLoad(char *fileName)
/* Load mafS from file. */
{
struct mafS *obj;
xapParseAny(fileName, "S", mafStartHandler, mafEndHandler, NULL, &obj);
return obj;
}

void *mafStartHandler(struct xap *xp, char *name, char **atts)
/* Called by expat with start tag.  Does most of the parsing work. */
{
struct xapStack *st = xp->stack+1;
int depth = xp->stackDepth;
int i;

if (sameString(name, "MAF"))
    {
    struct mafFile *obj;
    AllocVar(obj);
    for (i=0; atts[i] != NULL; i += 2)
        {
        char *name = atts[i], *val = atts[i+1];
        if  (sameString(name, "version"))
            obj->version = atoi(val);
        else if (sameString(name, "scoring"))
            obj->scoring = cloneString(val);
        }
    return obj;
    }
else if (sameString(name, "ALI"))
    {
    struct mafAli *obj;
    AllocVar(obj);
    for (i=0; atts[i] != NULL; i += 2)
        {
        char *name = atts[i], *val = atts[i+1];
        if (sameString(name, "score"))
            obj->score = atof(val);
        }
    if (depth > 1)
        {
        if  (sameString(st->elName, "MAF"))
            {
            struct mafFile *parent = st->object;
            slAddHead(&parent->mafAli, obj);
            }
        }
    return obj;
    }
else if (sameString(name, "S"))
    {
    struct mafS *obj;
    AllocVar(obj);
    for (i=0; atts[i] != NULL; i += 2)
        {
        char *name = atts[i], *val = atts[i+1];
        if  (sameString(name, "seq"))
            obj->seq = cloneString(val);
        else if (sameString(name, "seqSize"))
            obj->seqSize = atoi(val);
        else if (sameString(name, "strand"))
            obj->strand = cloneString(val);
        else if (sameString(name, "start"))
            obj->start = atoi(val);
        else if (sameString(name, "size"))
            obj->size = atoi(val);
        }
    if (obj->seq == NULL)
        xapError(xp, "missing seq");
    if (obj->strand == NULL)
        xapError(xp, "missing strand");
    if (depth > 1)
        {
        if  (sameString(st->elName, "ALI"))
            {
            struct mafAli *parent = st->object;
            slAddHead(&parent->mafS, obj);
            }
        }
    return obj;
    }
else
    {
    xapSkip(xp);
    return NULL;
    }
}

void mafEndHandler(struct xap *xp, char *name)
/* Called by expat with end tag.  Checks all required children are loaded. */
{
struct xapStack *stack = xp->stack;
if (sameString(name, "MAF"))
    {
    struct mafFile *obj = stack->object;
    slReverse(&obj->mafAli);
    }
else if (sameString(name, "ALI"))
    {
    struct mafAli *obj = stack->object;
    if (obj->mafS == NULL)
        xapError(xp, "Missing S");
    slReverse(&obj->mafS);
    }
else if (sameString(name, "S"))
    {
    struct mafS *obj = stack->object;
    obj->text = cloneString(stack->text->string);
    }
}

