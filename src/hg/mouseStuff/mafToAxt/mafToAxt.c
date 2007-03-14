/* mafToAxt - Convert from maf to axt format, taking sequences tName and qName. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "axt.h"
#include "obscure.h"
#include "maf.h"
#include "string.h"

boolean stripDb = FALSE;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "mafToAxt - Convert from maf to axt format\n"
  "usage:\n"
  "   mafToAxt in.maf tName qName output\n"
  "Where tName and qName are the names for the\n"
  "target and query sequences respectively.\n"
  "tName should be maf target since it must always\n"
  "be oriented in \"+\" direction.\n"
  "  Use 'first' for tName to always use first sequence\n"
  "Options:\n"
  "  -stripDb - Strip names from start to first period.\n"
  );
}

static struct optionSpec options[] = {
   {"stripDb", OPTION_BOOLEAN},
   {NULL, 0},
};

char *stripDbFromName(char *name)
/* Optionally strip out database part of name */
{
if (stripDb)
    {
    char *afterDb = strchr(name, '.');
    if (afterDb == NULL)
	errAbort("No db. found in %s", name);
    name = afterDb+1;
    }
return cloneString(name);
}

void mafToAxt(char *in, char *tName, char *qName, char *out)
/* mafToAxt - Convert from axt to maf format. */
{
struct mafFile *mp = mafOpen(in);
FILE *f = mustOpen(out, "w");
struct mafAli *ali;
struct mafComp *comp;
boolean tIsFirst = sameString(tName, "first");

while ((ali = mafNext(mp)) != NULL)
    {
    struct axt *axt;
    AllocVar(axt);
    axt->score = ali->score;
    for (comp = ali->components; comp != NULL; comp = comp->next)
	{
	if( (tIsFirst && comp == ali->components) || startsWith( tName, comp->src ) )
	    {
	    axt->tName = stripDbFromName(comp->src);
	    cloneString(tName);	
	    axt->tStrand = comp->strand;
		    axt->tStart = comp->start;
		    axt->tEnd = comp->start + comp->size;
		    axt->tSym = comp->text;
		    comp->text = NULL;


	    if( axt->tStrand != '+' )
		errAbort("Target sequence not on positive strand.\n");
	    }
	else if( startsWith( qName, comp->src ))
	    {
	    axt->qName = stripDbFromName(comp->src);
	    axt->qStrand = comp->strand;
	    axt->qStart = comp->start;
	    axt->qEnd = comp->start + comp->size;
	    axt->qSym = comp->text;
	    comp->text = NULL;
	    }
	}

    if( axt->tSym == NULL || axt->qSym == NULL )
	{
	axtFree(&axt);
	continue;
	}
    if (strlen( axt->tSym  ) == strlen( axt->qSym )) 
	axt->symCount = strlen( axt->tSym );
    else
	{
	errAbort("Target and query sequences are different"
		 "lengths\n%s\n%s\n\n", axt->tSym, axt->qSym );
	}
    axtWrite(axt,f);
    axtFree(&axt);
    mafAliFree(&ali);
    }

carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();
stripDb = optionExists("stripDb");
mafToAxt(argv[1],argv[2],argv[3],argv[4]);
return 0;
}
