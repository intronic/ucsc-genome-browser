/* Built in string handling. */
#include "common.h"
#include "../compiler/pfPreamble.h"
#include "runType.h"
#include "object.h"
#include "pfString.h"


static void _pf_string_cleanup(struct _pf_string *string, int typeId)
/* Clean up string->s and string itself. */
{
// uglyf("_pf_string_cleanup %s\n", string->s);
verbose(2, "_pf_string_cleanup %s\n", string->s);
if (string->allocated != 0)
    freeMem(string->s);
freeMem(string);
}

struct _pf_string *_pf_string_new(char *s, int size)
/* Wrap string buffer of given size. */
{
struct _pf_string *string;
AllocVar(string);
string->_pf_refCount = 1;
string->_pf_cleanup = _pf_string_cleanup;
string->s = cloneStringZ(s, size);
string->size = string->allocated = size;
return string;
}

struct _pf_string *_pf_string_dupe(char *s, int size)
/* Clone string of given size and wrap string around it. */
{
return _pf_string_new(s, size);
}

int _pf_strcmp(_pf_Stack *stack)
/* Return comparison between strings.  Cleans them off
 * of stack.  Does not put result on stack because
 * the code generator may use it in different ways. */
{
_pf_String a = stack[0].String;
_pf_String b = stack[1].String;
int ret = 0;

if (a != b)
    {
    if (a == NULL)
        ret = -1;
    else if (b == NULL)
        ret = 1;
    else
        ret = strcmp(a->s, b->s);
    }

if (NULL != a && --a->_pf_refCount <= 0)
    a->_pf_cleanup(a, 0);
if (NULL != b && --b->_pf_refCount <= 0)
    b->_pf_cleanup(b, 0);
return ret;
}

static struct _pf_string *_pf_string_init(char *s)
/* Wrap string around constant. */
{
return _pf_string_new(s, strlen(s));
}

struct _pf_string *_pf_string_from_const(char *s)
/* Wrap string around constant. */
{
struct _pf_string *string = _pf_string_init(s);
return string;
}

struct _pf_string *_pf_string_from_long(_pf_Long ll)
/* Wrap string around Long. */
{
char buf[32];
safef(buf, sizeof(buf), "%lld", ll);
return _pf_string_init(buf);
}

struct _pf_string *_pf_string_from_double(_pf_Double d)
/* Wrap string around Double. */
{
char buf[32];
safef(buf, sizeof(buf), "%0.2f", d);
return _pf_string_init(buf);
}


struct _pf_string *_pf_string_from_Int(_pf_Int i)
/* Wrap string around Int. */
{
return _pf_string_from_long(i);
}

struct _pf_string *_pf_string_from_Float(_pf_Stack *stack)
/* Wrap string around Float. */
{
return _pf_string_from_long(stack->Float);
}


struct _pf_string *_pf_string_cat(_pf_Stack *stack, int count)
/* Create new string that's a concatenation of the above strings. */
{
int i;
size_t total=0;
size_t pos = 0;
_pf_String string;
char *s;
AllocVar(string);
string->_pf_refCount = 1;
string->_pf_cleanup = _pf_string_cleanup;
for (i=0; i<count; ++i)
    {
    _pf_String ss = stack[i].String;
    if (ss)
	total += ss->size;
    }
string->s = s = needLargeMem(total+1);
for (i=0; i<count; ++i)
    {
    _pf_String ss = stack[i].String;
    if (ss)
	{
	memcpy(s, ss->s, ss->size);
	s += ss->size;
	if (--ss->_pf_refCount <= 0)
	    ss->_pf_cleanup(ss, 0);
	}
    }
*s = 0;
string->size = string->allocated = total;
return string;
}

void _pf_cm_string_upper(_pf_Stack *stack)
/* Uppercase existing string */
{
_pf_String string = stack[0].String;
toUpperN(string->s, string->size);
}

void _pf_cm_string_lower(_pf_Stack *stack)
/* Lowercase existing string */
{
_pf_String string = stack[0].String;
toLowerN(string->s, string->size);
}

void _pf_cm_string_dupe(_pf_Stack *stack)
/* Return duplicate of string */
{
_pf_String string = stack[0].String;
stack[0].String = _pf_string_new(string->s, string->size);
}

void _pf_cm_string_start(_pf_Stack *stack)
/* Return start of string */
{
_pf_String string = stack[0].String;
int size = stack[1].Int;
if (size < 0)
    size = 0;
if (size > string->size)
    size = string->size;
stack[0].String = _pf_string_new(string->s, size);
}

void _pf_cm_string_end(_pf_Stack *stack)
/* Return end of string */
{
_pf_String string = stack[0].String;
int size = stack[1].Int;
if (size < 0)
    size = 0;
if (size > string->size)
    size = string->size;
stack[0].String = _pf_string_new(string->s + string->size - size, size);
}

void _pf_cm_string_middle(_pf_Stack *stack)
/* Return middle of string */
{
_pf_String string = stack[0].String;
int start = stack[1].Int;
int size = stack[2].Int;
int end = start + size;
if (start < 0) start = 0;
if (end > string->size) end = string->size;
stack[0].String = _pf_string_new(string->s + start, end - start);
}

void _pf_cm_string_rest(_pf_Stack *stack)
/* Return rest of string (skipping up to start) */
{
_pf_String string = stack[0].String;
int start = stack[1].Int;
if (start < 0) start = 0;
if (start > string->size) start = string->size;
stack[0].String = _pf_string_new(string->s + start, string->size - start);
}


void _pf_cm_string_append(_pf_Stack *stack)
/* Put something on end of string. */
{
_pf_String start = stack[0].String;
_pf_String end = stack[1].String;
if (end != NULL)
    {
    int newSize = start->size + end->size;
    if (newSize > start->allocated)
	{
	char *s;
	if (newSize <= 64*1024)
	    start->allocated = 2*newSize;
	else
	    start->allocated = newSize + 64*1024;
	start->s = needLargeMemResize(start->s, start->allocated);
	}
    memcpy(start->s + start->size, end->s, end->size);
    start->s[newSize] = 0;
    start->size = newSize;

    /* Clean up references on stack.  (Not first param since it's a method). */
    if (--end->_pf_refCount <= 0)
	end->_pf_cleanup(end, 0);
    }
}

void _pf_cm_string_find(_pf_Stack *stack)
/* Find occurence of substring within string.  Return -1 if
 * not found, otherwise start location within string. */
{
_pf_String string = stack[0].String;
_pf_String sub = stack[1].String;
char *loc = stringIn(sub->s, string->s);
if (loc == NULL)
    stack[0].Int = -1;
else
    stack[0].Int = loc - string->s;

/* Clean up references on stack.  (Not first param since it's a method). */
if (--sub->_pf_refCount <= 0)
    sub->_pf_cleanup(sub, 0);
}

void _pf_cm_string_findNext(_pf_Stack *stack)
/* Find occurence of substring within string.  Return -1 if
 * not found, otherwise start location within string. */
{
/* Get input off of stack. */
_pf_String string = stack[0].String;
_pf_String sub = stack[1].String;
_pf_Int start = stack[2].Int;

/* Calculate where match is.  Don't bother looking is
 * start coordinate outside of string. */
char *loc = NULL;
if (start >= 0 && start < string->size)
    loc = stringIn(sub->s, string->s + start);
if (loc == NULL)
    stack[0].Int = -1;
else
    stack[0].Int = loc - string->s;

/* Clean up references on stack.  (Not first param since it's a method). */
if (--sub->_pf_refCount <= 0)
    sub->_pf_cleanup(sub, 0);
}


static int tokenSpan(char *s)
/* Count up number of characters in word starting at s. */
{
int count = 0;
if (s != NULL && s[0] != 0)
    {
    char *e = s;
    char c = *e++;
    if (isalnum(c) || c == '_')
        {
	for (;;)
	    {
	    c = *e;
	    if (!isalnum(c) && c != '_')
		break;
	    e += 1;
	    }
	}
    count = e - s;
    }
return count;
}

static int wordSpan(char *s)
/* Count up number of characters in space-delimited word starting at s. */
{
char *e = skipToSpaces(s);
if (e == NULL)
    return strlen(s);
else
    return e - s;
}

static void nextSkippingWhite(_pf_Stack *stack, int (*calcSpan)(char *s))
{
_pf_String string = stack[0].String;
int pos = stack[1].Int;
char *s, c;

if (string == NULL || pos < 0)
    {
    stack[0].Int = -1;
    stack[1].String = NULL;
    return;
    }
s = skipLeadingSpaces(string->s + pos);
c = s[0];
if (c == 0)
    {
    stack[0].String = NULL;
    stack[1].Int = string->size;
    }
else
    {
    int span = calcSpan(s);
    stack[0].String = _pf_string_new(s, span);
    stack[1].Int = s + span - string->s;
    }
}

void _pf_cm_string_nextToken(_pf_Stack *stack)
/* Return next white space or punctuation separated word. */
{
nextSkippingWhite(stack, tokenSpan);
}

void _pf_cm_string_nextWord(_pf_Stack *stack)
/* Return next white-space separated word. */
{
nextSkippingWhite(stack, wordSpan);
}

static int countWordsSkippingWhite(char *s, int (*calcSpan)(char *s))
/* Count words - not in the space-delimited sense, but in the
 * paraFlow word since - where punctuation also separates words,
 * and punctuation marks are returned as a separate word. */
{
int count = 0, span;
char c;
for (;;)
    {
    s = skipLeadingSpaces(s);
    span = calcSpan(s);
    if (span == 0)
        break;
    count += 1;
    s += span;
    }
return count;
}

static void stringSplitSkippingWhite(_pf_Stack *stack, int (*calcSpan)(char *s))
/* Implements flow string.words() into array of string words */
{
_pf_String string = stack[0].String, *strings;
_pf_Array words;
int i, wordCount, span;
char *s;

_pf_nil_check(string);
s = string->s;
wordCount = countWordsSkippingWhite(s, calcSpan);
words = _pf_dim_array(wordCount, _pf_find_string_type_id());
strings =  (_pf_String *)words->elements;
for (i=0; i<wordCount; ++i)
    {
    s = skipLeadingSpaces(s);
    span = calcSpan(s);
    strings[i] = _pf_string_new(s, span);
    s += span;
    }
stack[0].Array = words;
}

void _pf_cm_string_tokens(_pf_Stack *stack)
/* Implements flow string.tokens() into array of string words */
{
stringSplitSkippingWhite(stack, tokenSpan);
}

void _pf_cm_string_words(_pf_Stack *stack)
/* Implements flow string.words() into array of string words */
{
stringSplitSkippingWhite(stack, wordSpan);
}

static void fitString(_pf_Stack *stack, boolean right)
/* Fit string on either side. */
{
_pf_String string = stack[0].String;
_pf_Int size = stack[1].Int;
_pf_String out;
AllocVar(out);
out->_pf_refCount = 1;
out->_pf_cleanup = _pf_string_cleanup;
out->s = needMem(size+1);
out->size = string->allocated = size;
if (string->size >= size)
     memcpy(out->s, string->s, size);
else
     {
     if (right)
         {
	 int diff = size - string->size;
	 memset(out->s, ' ', diff);
	 memcpy(out->s + diff, string->s, string->size);
	 }
     else
	 {
	 memcpy(out->s, string->s, string->size);
	 memset(out->s + string->size, ' ', size - string->size);
	 }
     }
stack[0].String = out;
}

void _pf_cm_string_fitLeft(_pf_Stack *stack)
/* Expand string to size, keeping string on left, and adding spaces as
 * needed. 
 *     string.fitLeft(size) into (string new)*/
{
fitString(stack, FALSE);
}

void _pf_cm_string_fitRight(_pf_Stack *stack)
/* Expand string to size, keeping string on right, and adding spaces as
 * needed. 
 *     string.fitRight(size) into (string new)*/
{
fitString(stack, TRUE);
}

void pf_floatString(_pf_Stack *stack)
/* floatString(double f, int digitsBeforeDecimal, int digitsAfterDecimal, 
 *         bit forceScientific) into string s
 * This returns a floating point number formatted just how you like it. */
{
_pf_Double f = stack[0].Double;
_pf_Int digitsBeforeDecimal = stack[1].Int;
_pf_Int digitsAfterDecimal = stack[2].Int;
_pf_Bit forceScientific = stack[3].Bit;
char buf[32];
char format[16];
char *sci = (forceScientific ? "e" : "f");

safef(format, sizeof(format), "%%%d.%d%s", 
	digitsBeforeDecimal+digitsAfterDecimal+1,
	digitsAfterDecimal, sci);
safef(buf, sizeof(buf), format, f);
stack[0].String = _pf_string_new(buf, strlen(buf));
}

void pf_intString(_pf_Stack *stack)
/* Format an integer just how you like it. *?
 * intString(long l, int minWidth, bit zeroPad, bit commas) into String s
 */
{
_pf_Long l = stack[0].Long;
_pf_Int minWidth = stack[1].Int;
_pf_Bit zeroPad = stack[2].Bit;
_pf_Bit commas = stack[3].Bit;
char buf[32];
char format[16];
safef(format, sizeof(format), "%%%s%dlld", 
	(zeroPad ? "0" : ""), minWidth);
safef(buf, sizeof(buf), format, l);
if (commas)
    {
    int len = strlen(buf);
    char commaBuf[48];
    int sourcePos = 0, destPos = 0;
    int size = len%3;
    if (size == 0) size = 3;
    for (;;)
        {
	memcpy(commaBuf+destPos, buf+sourcePos, size);
	destPos += size;
	sourcePos += size;
	if (sourcePos >= len)
	    break;
	if (commaBuf[destPos-1] != ' ')
	    {
	    commaBuf[destPos] = ',';
	    destPos += 1;
	    }
	size = 3;
	}
    stack[0].String = _pf_string_new(commaBuf, destPos);
    }
else
    stack[0].String = _pf_string_new(buf, strlen(buf));
}

void _pf_string_substitute(_pf_Stack *stack)
/* Create new string on top of stack based on all of the
 * substitution parameters already on the stack.  The
 * first param is the string with the $ signs. The rest
 * of the parameters are strings  to substitute in, in the
 * same order as the $'s in the first param.  */
{
_pf_String sourceString = stack[0].String;
_pf_String destString = NULL;
_pf_String varString;
int varCount = 0, varIx = 0;
int estimatedSize = 0;  /*  Estimate will be a little high. */
int destSize = 0;
char *destBuf;
char *s, *e, *d, *sEnd;

/* Count up number of substitutions, and estimate the resulting
 * string size.  Make a buffer big enough to hold substituted result. */
s = sourceString->s;
e = s + sourceString->size;
while (s < e)
    {
    if (s[0] == '$')
        {
	if (s[1] == '$')
	    s += 1;
	else
	    {
	    varCount += 1;
	    varString = stack[varCount].String;
	    estimatedSize += varString->size;
	    }
	}
    s += 1;
    }
estimatedSize += sourceString->size;
destBuf = needMem(estimatedSize+1);

/* Loop through copying source with substitutions to dest. */
s = sourceString->s;
sEnd = s + sourceString->size;
d = destBuf;
while (s < sEnd)
    {
    char c = *s;
    if (c == '$')
        {
	s += 1;
	c = *s;
	if (c == '$')
	    {
	    *d++ = '$';
	    s += 1;
	    }
	else 
	    {
	    if (c == '(')
		{
		/* Skip to closing paren in s */
		s = strchr(s, ')');
		assert(s != NULL);
		s += 1;
		}
	    else
	        {
		/* Skip to character after var name. */
		while ((isalnum(c) || c == '_') && s < sEnd)
		    {
		    c = *(++s);
		    }
		}
	    ++varIx;
	    varString = stack[varIx].String;
	    memcpy(d, varString->s, varString->size);
	    d += varString->size;
	    }
	}
    else
        {
	*d++ = c;
	s += 1;
	}
    }
assert(varIx == varCount);
destSize = d - destBuf;

/* Clean up all the input references on stack. */
for (varIx=0; varIx <= varCount; ++varIx)  /* <= intentional here */
    {
    struct _pf_string *s = stack[varIx].String;
    if (--s->_pf_refCount <= 0)
	s->_pf_cleanup(s, 0);
    }

/* Create a string object to return and put it on stack. */
stack[0].String = _pf_string_new(destBuf, destSize);
freeMem(destBuf);
}
