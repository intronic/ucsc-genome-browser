/* tokenizer - A tokenizer structure that will chop up file into
 * tokens.  It is aware of quoted strings and otherwise tends to return
 * white-space or punctuated-separated words, with punctuation in
 * a separate token.  This is used by autoSql. */

#include "common.h"
#include "errabort.h"
#include "linefile.h"
#include "tokenizer.h"

static char const rcsid[] = "$Id: tokenizer.c,v 1.1 2004/03/22 14:17:09 kent Exp $";

struct tokenizer *tokenizerNew(char *fileName)
/* Return a new tokenizer. */
{
struct tokenizer *tkz;
AllocVar(tkz);
tkz->sAlloc = 128;
tkz->string = needMem(tkz->sAlloc);
tkz->lf = lineFileOpen(fileName, TRUE);
tkz->curLine = tkz->linePt = "";
return tkz;
}

void tokenizerFree(struct tokenizer **pTkz)
/* Tear down a tokenizer. */
{
struct tokenizer *tkz;
if ((tkz = *pTkz) != NULL)
    {
    freeMem(tkz->string);
    lineFileClose(&tkz->lf);
    freez(pTkz);
    }
}

void tokenizerReuse(struct tokenizer *tkz)
/* Reuse token. */
{
tkz->reuse = TRUE;
}

int tokenizerLineCount(struct tokenizer *tkz)
/* Return line of current token. */
{
return tkz->lf->lineIx;
}

char *tokenizerFileName(struct tokenizer *tkz)
/* Return name of file. */
{
return tkz->lf->fileName;
}

char *tokenizerNext(struct tokenizer *tkz)
/* Return token's next string (also available as tkz->string) or
 * NULL at EOF. */
{
char *start, *end;
char c, *s;
int size;
if (tkz->reuse)
    {
    tkz->reuse = FALSE;
    return tkz->string;
    }
for (;;)	/* Skip over white space. */
    {
    int lineSize;
    s = start = skipLeadingSpaces(tkz->linePt);
    if ((c = start[0]) != 0)
	break;
    if (!lineFileNext(tkz->lf, &tkz->curLine, &lineSize))
	{
	tkz->eof = TRUE;
	return NULL;
	}
    tkz->linePt = tkz->curLine;
    }
if (isalnum(c) || (c == '_'))
    {
    for (;;)
	{
        s++;
	if (!(isalnum(*s) || (*s == '_')))
	    break;
	}
    end = s;
    }
else if (c == '"')
    {
    start = s+1;
    for (;;)
	{
	c = *(++s);
	if (c == '"')
	    break;
	}
    end = s;
    ++s;
    }
else
    {
    end = ++s;
    }
tkz->linePt = s;
size = end - start;
if (size >= tkz->sAlloc)
    {
    tkz->sAlloc = size+128;
    tkz->string = needMoreMem(tkz->string, 0, tkz->sAlloc);
    }
memcpy(tkz->string, start, size);
tkz->string[size] = 0;
return tkz->string;
}

void tokenizerErrAbort(struct tokenizer *tkz, char *format, ...)
/* Print error message followed by file and line number and
 * abort. */
{
va_list args;
va_start(args, format);
vaWarn(format, args);
errAbort("line %d of %s:\n%s", 
	tokenizerLineCount(tkz), tokenizerFileName(tkz), tkz->curLine);
}

void tokenizerNotEnd(struct tokenizer *tkz)
/* Squawk if at end. */
{
if (tkz->eof)
    errAbort("Unexpected end of file");
}

void tokenizerMustHaveNext(struct tokenizer *tkz)
/* Get next token, which must be there. */
{
if (tokenizerNext(tkz) == NULL)
    errAbort("Unexpected end of file");
}

void tokenizerMustMatch(struct tokenizer *tkz, char *string)
/* Require next token to match string.  Return next token
 * if it does, otherwise abort. */
{
if (sameWord(tkz->string, string))
    tokenizerMustHaveNext(tkz);
else
    tokenizerErrAbort(tkz, "Expecting %s got %s", string, tkz->string);
}

