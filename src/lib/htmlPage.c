/* htmlPage - stuff to read, parse, and submit  htmlPages and forms. */

#include "common.h"
#include "errabort.h"
#include "memalloc.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "cheapcgi.h"
#include "obscure.h"
#include "filePath.h"
#include "net.h"
#include "htmlPage.h"

void htmlStatusFree(struct htmlStatus **pStatus)
/* Free up resources associated with status */
{
struct htmlStatus *status = *pStatus;
if (status != NULL)
    {
    freeMem(status->version);
    freez(pStatus);
    }
}

void htmlStatusFreeList(struct htmlStatus **pList)
/* Free a list of dynamically allocated htmlStatus's */
{
struct htmlStatus *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlStatusFree(&el);
    }
*pList = NULL;
}

void htmlCookieFree(struct htmlCookie **pCookie)
/* Free memory associated with cookie. */
{
struct htmlCookie *cookie = *pCookie;
if (cookie != NULL)
    {
    freeMem(cookie->name);
    freeMem(cookie->value);
    freeMem(cookie->domain);
    freeMem(cookie->path);
    freeMem(cookie->expires);
    freez(pCookie);
    }
}

void htmlCookieFreeList(struct htmlCookie **pList)
/* Free a list of dynamically allocated htmlCookie's */
{
struct htmlCookie *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlCookieFree(&el);
    }
*pList = NULL;
}

void htmlAttributeFree(struct htmlAttribute **pAttribute)
/* Free up resources associated with attribute. */
{
struct htmlAttribute *att = *pAttribute;
if (att != NULL)
    {
    freeMem(att->name);
    freeMem(att->val);
    freez(pAttribute);
    }
}

void htmlAttributeFreeList(struct htmlAttribute **pList)
/* Free a list of dynamically allocated htmlAttribute's */
{
struct htmlAttribute *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlAttributeFree(&el);
    }
*pList = NULL;
}

void htmlTagFree(struct htmlTag **pTag)
/* Free up resources associated with tag. */
{
struct htmlTag *tag = *pTag;
if (tag != NULL)
    {
    htmlAttributeFreeList(&tag->attributes);
    freeMem(tag->name);
    freez(pTag);
    }
}

void htmlTagFreeList(struct htmlTag **pList)
/* Free a list of dynamically allocated htmlTag's */
{
struct htmlTag *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlTagFree(&el);
    }
*pList = NULL;
}

void htmlFormVarFree(struct htmlFormVar **pVar)
/* Free up resources associated with form variable. */
{
struct htmlFormVar *var = *pVar;
if (var != NULL)
    {
    freeMem(var->curVal);
    slFreeList(&var->values);
    slFreeList(&var->tags);
    freez(pVar);
    }
}

void htmlFormVarFreeList(struct htmlFormVar **pList)
/* Free a list of dynamically allocated htmlFormVar's */
{
struct htmlFormVar *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlFormVarFree(&el);
    }
*pList = NULL;
}


void htmlFormFree(struct htmlForm **pForm)
/* Free up resources associated with form variable. */
{
struct htmlForm *form = *pForm;
if (form != NULL)
    {
    htmlFormVarFreeList(&form->vars);
    freez(pForm);
    }
}

void htmlFormFreeList(struct htmlForm **pList)
/* Free a list of dynamically allocated htmlForm's */
{
struct htmlForm *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlFormFree(&el);
    }
*pList = NULL;
}

void htmlPageFree(struct htmlPage **pPage)
/* Free up resources associated with htmlPage. */
{
struct htmlPage *page = *pPage;
if (page != NULL)
    {
    freez(&page->url);
    htmlStatusFree(&page->status);
    freeHashAndVals(&page->header);
    htmlCookieFreeList(&page->cookies);
    freez(&page->fullText);
    htmlTagFreeList(&page->tags);
    htmlFormFreeList(&page->forms);
    freez(pPage);
    }
}

void htmlPageFreeList(struct htmlPage **pList)
/* Free a list of dynamically allocated htmlPage's */
{
struct htmlPage *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    htmlPageFree(&el);
    }
*pList = NULL;
}

static int findLineNumber(char *start, char *pos)
/* Figure out line number of given position relative to start. */
{
char *s;
int line = 1;
for (s = start; s <= pos; ++s)
    {
    if (s[0] == '\n')
       ++line;
    }
return line;
}

static void tagVaWarn(struct htmlPage *page, struct htmlTag *tag, char *format, 
	va_list args)
/* Print warning message and some context of tag. */
{
char context[80];
strncpy(context, tag->start, sizeof(context));
context[sizeof(context)-1] = 0;
warn("Error near line %d of %s:\n %s", findLineNumber(page->htmlText, tag->start), 
	page->url, context);
vaWarn(format, args);
}

static void tagWarn(struct htmlPage *page, struct htmlTag *tag, char *format, ...)
/* Print warning message and some context of tag. */
{
va_list args;
va_start(args, format);
tagVaWarn(page, tag, format, args);
va_end(args);
}

static void tagAbort(struct htmlPage *page, struct htmlTag *tag, char *format, ...)
/* Print abort message and some context of tag. */
{
va_list args;
va_start(args, format);
tagVaWarn(page, tag, format, args);
va_end(args);
noWarnAbort();
}

struct htmlStatus *htmlStatusParse(char **pText)
/* Read in status from first line.  Update pText to point to next line. 
 * Note unlike many routines here, this does not insert zeros into text. */
{
char *text = *pText;
char *end = strchr(text, '\n');
struct htmlStatus *status;
if (end != NULL)
   *pText = end+1;
else
   *pText = text + strlen(text);
end = skipToSpaces(text);
if (end == NULL)
    {
    warn("Short status line.");
    return NULL;
    }
AllocVar(status);
status->version = cloneStringZ(text, end-text);
end = skipLeadingSpaces(end);
if (!isdigit(end[0]))
    {
    warn("Not a number in status field");
    return NULL;
    }
status->status = atoi(end);
return status;
}

char *htmlNextCrLfLine(char **pS)
/* Return zero-terminated line and advance *pS to start of
 * next line.  Return NULL at end of file.  Warn if there is
 * no <CR>. */
{
char *s = *pS, *e;
if (s == NULL || s[0] == 0)
    return NULL;
e = strchr(s, '\n');
if (e == NULL)
    verbose(1, "End of file in header\n");
else 
    {
    *e = 0;
    if (e == s || e[-1] != '\r')
	verbose(1, "Missing <CR> in header line\n");
    else
       e[-1] = 0;
    e += 1;
    }
*pS = e;
return s;
}

static void cookieParseNameValuePair(char *s, char **retName, char **retVal)
/* Parse out name/value pair. Warn and return FALSE if there's a problem. */
{
char *val = strchr(s, '=');
if (val == NULL)
    {
    val = s + strlen(s);
    }
*val++ = 0;
*retName = s;
*retVal = val;
}

static struct htmlCookie *parseCookie(char *s)
/* Parse out cookie line to the right of Set-Cookie. */
{
char *cookiePairs = s;
char *e, *name, *val;
char *domain = NULL, *path = NULL, *expires = NULL;
boolean secure = FALSE;
struct htmlCookie *cookie;

/* Grab up to semicolon, which is the cookie name/value pair. */
e = strchr(s, ';');
if (e == NULL)
    {
    warn("Missing ';' in cookie");
    return NULL;
    }
*e++ = 0;

/* Allocate cookie and fill out name/value pair. */
AllocVar(cookie);
cookieParseNameValuePair(s, &name, &val);
cookie->name = cloneString(name);
cookie->value = cloneString(val);

/* Loop through to grab the other info - domain and so forth. */
s = e;
for (;;)
    {
    /* Find next semicolon and zero-terminate it. */
    s = skipLeadingSpaces(s);
    e = strchr(s, ';');
    if (e == NULL)
        break;
    *e++ = 0;

    /* Parse out name/value pairs and save it away if it's one we know about. */
    cookieParseNameValuePair(s, &name, &val);
    if (sameString(name, "domain"))
        cookie->domain = cloneString(val);
    else if (sameString(name, "path"))
        cookie->path = cloneString(val);
    else if (sameString(name, "expires"))
        cookie->expires = cloneString(val);
    else if (sameString(name, "secure"))
        cookie->secure = TRUE;

    s = e;
    }
return cookie;
}

static struct hash *htmlHeaderRead(char **pHtml, struct htmlCookie **pCookies)
/* Read in from second line through first blank line and
 * save in hash.  These lines are in the form name: value. */
{
struct hash *hash = hashNew(6);
for (;;)
    {
    char *line = htmlNextCrLfLine(pHtml);
    char *word;
    if (line == NULL)
	{
        warn("End of file in header");
	break;
	}
    word = nextWord(&line);
    if (word == NULL)
        break;
    line = skipLeadingSpaces(line);
    hashAdd(hash, word, cloneString(line));
    if (sameString(word, "Set-Cookie:"))
	{
	struct htmlCookie *cookie = parseCookie(line);
	if (cookie != NULL)
	    slAddTail(pCookies, cookie);
	}
    }
return hash;
}

static char *htmlAttributeFindVal(struct htmlAttribute *list, char *name)
/* Find named attribute or return NULL. */
{
struct htmlAttribute *att;
for (att = list; att != NULL; att = att->next)
    {
    if (sameWord(att->name, name))
        return att->val;
    }
return NULL;
}


char *htmlTagAttributeVal(struct htmlPage *page, struct htmlTag *tag, 
	char *name, char *defaultVal)
/* Return value of named attribute, or defaultVal if attribute doesn't exist. */
{
char *val = htmlAttributeFindVal(tag->attributes, name);
if (val == NULL)
    val = defaultVal;
return val;
}

char *htmlTagAttributeNeeded(struct htmlPage *page, struct htmlTag *tag, char *name)
/* Return named tag attribute.  Complain and return "n/a" if it
 * doesn't exist. */
{
char *val = htmlTagAttributeVal(page, tag, name, NULL);
if (val == NULL)
    {
    tagWarn(page, tag, "Missing %s attribute", name);
    val = "n/a";
    }
return val;
}

static struct htmlTag *htmlTagScan(char *html, char *dupe)
/* Scan HTML for tags and return a list of them. 
 * Html is the text to scan, and dupe is a copy of it
 * which this routine will insert 0's in in the course of
 * parsing.*/
{
char *s = dupe, c, *e, *tagName;
struct slName *list = NULL, *link;
struct htmlTag *tagList = NULL, *tag;
struct htmlAttribute *att;
int pos;

for (;;)
    {
    c = *s++;
    if (c == 0)
        break;
    if (c == '<')
        {
	if (*s == '!')	/* HTML comment. */
	    {
	    s += 1;
	    if (s[0] == '-' && s[1] == '-')
	        s = stringIn("-->", s);
	    else
		s = strchr(s, '>');
	    if (s == NULL)
		{
	        warn("End of file in comment");
		break;
		}
	    }
	else
	    {
	    /* Grab first word into tagName. */
	    e = s;
	    for (;;)
	        {
		c = *e;
		if (c == '>' || c == 0 || isspace(c))
		    break;
		e += 1;
		}
	    if (c != 0)
	       *e++ = 0;
	    tagName = s;
	    s = e;
	    
	    /* Allocate tag, fill in name, and stick it on list. */
	    AllocVar(tag);
	    tag->name = cloneString(tagName);
	    slAddHead(&tagList, tag);
	    pos = tagName - dupe - 1;
	    tag->start = html+pos;

	    /* If already got end tag (or EOF) stop processing tag. */
	    if (c == '>' || c == 0)
		{
		tag->end = html + (e - dupe);
	        continue;
		}

	    /* Process name/value pairs until get end tag. */
	    for (;;)
		{
		char *name, *val;
		boolean gotEnd = FALSE;

		/* Check for end tag. */
		s = skipLeadingSpaces(s);
		if (s[0] == '>' || s[0] == 0)
		    {
		    tag->end = html + (s - dupe);
		    if (s[0] == '>')
			tag->end += 1;
		    break;
		    }

		/* Get name - everything up to equals. */
		e = s;
		for (;;)
		    {
		    c = *e;
		    if (c == '=')
		        break;
		    else if (c == '>')
		        break;
		    else if (c == 0)
		        break;
		    else if (isspace(c))
		        break;
		    e += 1;
		    }
		if (c == 0)
		    {
		    warn("End of file in tag");
		    break;
		    }
		name = s;
		*e++ = 0;
		eraseTrailingSpaces(name);
		if (c == '>')
		    {
		    val = "";
		    gotEnd = TRUE;
		    tag->end = html + (e - dupe);
		    }
		else if (isspace(c))
		    {
		    val = "";
		    }
		else
		    {
		    val = e = skipLeadingSpaces(e);
		    if (e[0] == '"')
			{
			if (!parseQuotedString(val, val, &e))
			    break;
			}
		    else
			{
			for (;;)
			    {
			    c = *e;
			    if (c == '>')
				{
				gotEnd = TRUE;
				*e++ = 0;
				tag->end = html + (e - dupe);
				break;
				}
			    else if (isspace(c))
				{
				*e++ = 0;
				break;
				}
			    else if (c == 0)
				break;
			    ++e;
			    }
			}
		    }
		AllocVar(att);
		att->name = cloneString(name);
		att->val = cloneString(val);
		slAddTail(&tag->attributes, att);
		s = e;
		if (gotEnd)
		    break;
		}
	    }
	}
    }
slReverse(&tagList);
return tagList;
}

static struct htmlFormVar *findOrMakeVar(struct htmlPage *page, char *name, 
	struct hash *hash, struct htmlTag *tag, struct htmlFormVar **pVarList)
/* Find variable of existing name if it exists,  otherwise
 * make a new one and add to hash and list.  Add reference
 * to this tag to var. */
{
struct htmlFormVar *var = hashFindVal(hash, name);
if (var == NULL)
    {
    AllocVar(var);
    var->name = name;
    var->tagName = tag->name;
    hashAdd(hash, name, var);
    slAddHead(pVarList, var);
    }
else
    {
    if (!sameWord(var->tagName, tag->name))
        {
	tagWarn(page, tag, "Mixing FORM variable tag types %s and %s", 
		var->tagName, tag->name);
	var->tagName = tag->name;
	}
    }
refAdd(&var->tags, tag);
return var;
}

static boolean isMixableInputType(char *type)
/* Return TRUE if it's a type you can mix with others ok, like
 * button, submit, and image. */
{
return sameWord(type, "BUTTON") || sameWord(type, "SUBMIT") 
	|| sameWord(type, "IMAGE");
}

static void htmlFormVarAddValue(struct htmlFormVar *var, char *value)
/* Add value to list of predefined values for var. */
{
struct slName *name = slNameNew(value);
slAddTail(&var->values, name);
}


static struct htmlFormVar *formParseVars(struct htmlPage *page, struct htmlForm *form)
/* Return a list of variables parsed out of form.  
 * A form variable is something that may appear in the name
 * side of the name=value pairs that serves as input to a CGI
 * script.  The variables may be constructed from buttons, 
 * INPUT tags, OPTION lists, or TEXTAREAs. */
{
struct htmlTag *tag;
struct htmlFormVar *varList = NULL, *var;
struct hash *hash = newHash(0);
for (tag = form->startTag->next; tag != form->endTag; tag = tag->next)
    {
    if (sameWord(tag->name, "INPUT"))
        {
	char *type = htmlTagAttributeVal(page, tag, "TYPE", NULL);
	char *varName = htmlTagAttributeVal(page, tag, "NAME", NULL);
	char *value = htmlTagAttributeVal(page, tag, "VALUE", NULL);
	if (type == NULL)
	    type = "TEXT";
	if (varName == NULL)
	    {
	    if (!sameWord(type, "SUBMIT") && !sameWord(type, "CLEAR")
	    	&& !sameWord(type, "BUTTON") && !sameWord(type, "RESET")
		&& !sameWord(type, "IMAGE"))
		tagWarn(page, tag, "Missing NAME attribute");
	    varName = "n/a";
	    }
	var = findOrMakeVar(page, varName, hash, tag, &varList); 
	if (var->type != NULL && !sameWord(var->type, type))
	    {
	    if (!isMixableInputType(var->type) || !isMixableInputType(type))
		tagWarn(page, tag, "Mixing input types %s and %s", var->type, type);
	    }
	var->type = type;
	if (sameWord(type, "TEXT") || sameWord(type, "PASSWORD") 
		|| sameWord(type, "FILE") || sameWord(type, "HIDDEN")
		|| sameWord(type, "IMAGE"))
	    {
	    var->curVal = cloneString(value);
	    }
	else if (sameWord(type, "CHECKBOX"))
	    {
	    if (htmlTagAttributeVal(page, tag, "CHECKED", NULL) != NULL)
	        var->curVal = cloneString("on");
	    }
	else if (sameWord(type, "RADIO"))
	    {
	    if (htmlTagAttributeVal(page, tag, "CHECKED", NULL) != NULL)
	        var->curVal = cloneString(value);
	    htmlFormVarAddValue(var, value);
	    }
	else if ( sameWord(type, "RESET") || sameWord(type, "BUTTON") ||
		sameWord(type, "SUBMIT") || sameWord(type, "IMAGE") ||
		sameWord(type, "n/a"))
	    {
	    /* Do nothing. */
	    }
	else
	    {
	    tagWarn(page, tag, "Unrecognized INPUT TYPE %s", type);
	    }
	}
    else if (sameWord(tag->name, "SELECT"))
        {
	char *varName = htmlTagAttributeNeeded(page, tag, "NAME");
	struct htmlTag *subTag;
	var = findOrMakeVar(page, varName, hash, tag, &varList); 
	for (subTag = tag->next; subTag != form->endTag; subTag = subTag->next)
	    {
	    if (sameWord(subTag->name, "/SELECT"))
		{
		if (var->curVal == NULL && var->values != NULL)
		    {
		    var->curVal = cloneString(var->values->name);
		    }
		break;
		}
	    else if (sameWord(subTag->name, "OPTION"))
	        {
		char *val = cloneString(htmlTagAttributeVal(page, subTag, "VALUE", NULL));
		if (val == NULL)
		    {
		    char *e = strchr(subTag->end, '<');
		    if (e != NULL)
			val = cloneStringZ(subTag->end, e - subTag->end);
		    }
		if (val != NULL)
		    htmlFormVarAddValue(var, val);
		if (htmlTagAttributeVal(page, subTag, "SELECTED", NULL) != NULL)
		    {
		    if (val != NULL)
			var->curVal = cloneString(val);
		    }
		freez(&val);
		}
	    }
	}
    else if (sameWord(tag->name, "TEXTAREA"))
        {
	char *varName = htmlTagAttributeNeeded(page, tag, "NAME");
	char *e = strchr(tag->end, '<');
	var = findOrMakeVar(page, varName, hash, tag, &varList); 
	if (e != NULL)
	    var->curVal = cloneStringZ(tag->end, e - tag->end);
	}
    }
slReverse(&varList);
for (var = varList; var != NULL; var = var->next)
    {
    slReverse(&var->tags);
    }
return varList;
}

static struct htmlForm *htmlParseForms(struct htmlPage *page,
	struct htmlTag *startTag, struct htmlTag *endTag)
/* Parse out list of forms from tag stream. */
{
struct htmlForm *formList = NULL, *form = NULL;
struct htmlTag *tag;
for (tag = startTag; tag != endTag; tag = tag->next)
    {
    if (sameWord(tag->name, "FORM"))
        {
	if (form != NULL)
	    tagWarn(page, tag, "FORM inside of FORM");
	AllocVar(form);
	form->startTag = tag;
	slAddHead(&formList, form);
	form->name = htmlTagAttributeVal(page, tag, "name", "n/a");
	form->action = htmlTagAttributeNeeded(page, tag, "action");
	form->method = htmlTagAttributeVal(page, tag, "method", "GET");
	}
    else if (sameWord(tag->name, "/FORM"))
        {
	if (form == NULL)
	    tagWarn(page, tag, "/FORM outside of FORM");
	else
	    {
	    form->endTag = tag->next;
	    form = NULL;
	    }
	}
    }
slReverse(&formList);
for (form = formList; form != NULL; form = form->next)
    {
    form->vars = formParseVars(page, form);
    }
return formList;
}

struct htmlPage *htmlPageParse(char *url, char *fullText)
/* Parse out page and return. */
{
struct htmlPage *page;
char *dupe = cloneString(fullText);
char *s = dupe;
struct htmlStatus *status = htmlStatusParse(&s);
char *contentType;

if (status == NULL)
    return NULL;
if (status->status != 200)
   {
   warn("%s returned with status code %d", url, status->status);
   htmlStatusFree(&status);
   return NULL;
   }

AllocVar(page);
page->url = cloneString(url);
page->fullText = fullText;
page->status = status;
page->header = htmlHeaderRead(&s, &page->cookies);
contentType = hashFindVal(page->header, "Content-Type:");
if (startsWith("text/html", contentType))
    {
    page->htmlText = fullText + (s - dupe);
    page->tags = htmlTagScan(page->htmlText, s);
    page->forms = htmlParseForms(page, page->tags, NULL);
    }
freez(&dupe);
return page;
}

struct htmlPage *htmlPageParseNoHead(char *url, char *htmlText)
/* Parse out page in memory (past http header if any) and return. */
{
char *dupe = cloneString(htmlText);
struct htmlPage *page;
AllocVar(page);
page->url = cloneString(url);
page->htmlText = htmlText;
page->tags = htmlTagScan(page->htmlText, dupe);
page->forms = htmlParseForms(page, page->tags, NULL);
freez(&dupe);
return page;
}

struct htmlPage *htmlPageParseOk(char *url, char *fullText)
/* Parse out page and return only if status ok. */
{
struct htmlPage *page = htmlPageParse(url, fullText);
if (page == NULL)
   noWarnAbort();
return page;
}

char *htmlExpandUrl(char *base, char *url)
/* Expand URL that is relative to base to stand on it's own. 
 * Return NULL if it's not http or https. */
{
struct dyString *dy = NULL;
char *hostName, *pastHostName;

/* In easiest case URL is actually absolute and begins with
 * protocol.  Just return clone of url. */
if (startsWith("http:", url) || startsWith("https:", url))
    return cloneString(url);

/* If it's got a colon, but no http or https, then it's some
 * protocol we don't understand, like a mailto.  Just return NULL. */
if (strchr(url, ':') != NULL)
    return NULL;

/* Figure out first character past host name. Load up
 * return string with protocol (if any) and host name. */
dy = dyStringNew(256);
if (startsWith("http:", base) || startsWith("https:", base))
    hostName = (strchr(base, ':') + 3);
else
    hostName = base;
pastHostName = strchr(hostName, '/');
if (pastHostName == NULL)
    pastHostName = hostName + strlen(hostName);
dyStringAppendN(dy, base, pastHostName - base);

/* Add url to return string after host name. */
if (startsWith("/", url))	/* New URL is absolute, just append to hostName */
    {
    dyStringAppend(dy, url);
    }
else
    {
    char *curDir = pastHostName;
    char *endDir;
    if (curDir[0] == '/')
        curDir += 1;
    dyStringAppendC(dy, '/');
    endDir = strrchr(curDir, '/');
    if (endDir == NULL)
	endDir = curDir;
    if (startsWith("../", url))
	{
	char *dir = cloneStringZ(curDir, endDir-curDir);
	char *path = expandRelativePath(dir, url);
	if (path != NULL)
	     {
	     dyStringAppend(dy, path);
	     }
	freez(&dir);
	freez(&path);
	}
    else
	{
	dyStringAppendN(dy, curDir, endDir-curDir);
	if (lastChar(dy->string) != '/')
	    dyStringAppendC(dy, '/');
	dyStringAppend(dy, url);
	}
    }
return dyStringCannibalize(&dy);
}

static void appendCgiVar(struct dyString *dy, char *name, char *value)
/* Append cgiVar with cgi-encoded value to dy. */
{
char *enc = NULL;
if (value == NULL)
    value = "";
enc = cgiEncode(value);
if (dy->stringSize == 0)
    dyStringAppendC(dy, '?');
else
    dyStringAppendC(dy, '&');
dyStringAppend(dy, name);
dyStringAppendC(dy, '=');
dyStringAppend(dy, enc);
freez(&enc);
}

static char *cgiVarsFromForm(struct htmlPage *page, struct htmlForm *form, 
	char *buttonName, char *buttonVal)
/* Return cgi vars in name=val format from use having pressed
 * submit button of given name and value. */
{
struct dyString *dy = newDyString(0);
struct htmlFormVar *var;

if (buttonName != NULL)
    appendCgiVar(dy, buttonName, buttonVal);
for (var = form->vars; var != NULL; var = var->next)
    {
    if (sameWord(var->tagName, "SELECT") || var->type != NULL &&
        (sameWord(var->type, "CHECKBOX") || sameWord(var->type, "SELECT")
    	|| sameWord(var->type, "RADIO") || sameWord(var->type, "TEXTBOX")
	|| sameWord(var->type, "PASSWORD") || sameWord(var->type, "HIDDEN")
	|| sameWord(var->type, "TEXT") || sameWord(var->type, "FILE")))
        {
	char *val = var->curVal;
	if (val == NULL)
	    val = "";
	appendCgiVar(dy, var->name, val);
	}
    }
return dyStringCannibalize(&dy);
}

static void cookieOutput(struct dyString *dy, struct htmlCookie *cookieList)
/* Write cookies to dy. */
{
struct htmlCookie *cookie;
if (cookieList != NULL)
    {
    dyStringAppend(dy, "Cookie:");
    for (cookie = cookieList; cookie != NULL; cookie = cookie->next)
	{
	dyStringAppendC(dy, ' ');
	dyStringAppend(dy, cookie->name);
	dyStringAppendC(dy, '=');
	dyStringAppend(dy, cookie->value);
	dyStringAppendC(dy, ';');
	}
    dyStringAppend(dy, "\r\n");
    }
}

struct htmlPage *htmlPageFromForm(struct htmlPage *origPage, struct htmlForm *form, 
	char *buttonName, char *buttonVal)
/* Return a new htmlPage based on response to pressing indicated button
 * on indicated form in origPage. */
{
struct htmlPage *newPage = NULL;
struct dyString *dyUrl = dyStringNew(0);
struct dyString *dyHeader = dyStringNew(0);
struct dyString *dyText = NULL;
char *url = htmlExpandUrl(origPage->url, form->action);
char *cgiVars = NULL;
int contentLength = 0;
int sd = -1;

dyStringAppend(dyUrl, url);
cookieOutput(dyHeader, origPage->cookies);
if (sameWord(form->method, "GET"))
    {
    cgiVars = cgiVarsFromForm(origPage, form, buttonName, buttonVal);
    dyStringAppend(dyUrl, cgiVars);
    sd = netOpenHttpExt(dyUrl->string, form->method, FALSE);
    dyStringAppend(dyHeader, "\r\n");
    write(sd, dyHeader->string, dyHeader->stringSize);
    }
else if (sameWord(form->method, "POST"))
    {
    cgiVars = cgiVarsFromForm(origPage, form, buttonName, buttonVal);
    contentLength = strlen(cgiVars);
    sd = netOpenHttpExt(dyUrl->string, form->method, FALSE);
    dyStringPrintf(dyHeader, "Content-length: %d\r\n", contentLength);
    dyStringAppend(dyHeader, "\r\n");
    write(sd, dyHeader->string, dyHeader->stringSize);
    write(sd, cgiVars, contentLength);
    }
dyText = netSlurpFile(sd);
close(sd);
newPage = htmlPageParse(url, dyText->string);
freez(&url);
dyStringFree(&dyUrl);
dyStringFree(&dyHeader);
dyStringFree(&dyText);
return newPage;
}

struct slName *htmlPageScanAttribute(struct htmlPage *page, 
	char *tagName, char *attribute)
/* Scan page for values of particular attribute in particular tag.
 * if tag is NULL then scans in all tags. */
{
struct htmlTag *tag;
struct htmlAttribute *att;
struct slName *list = NULL, *el;

for (tag = page->tags; tag != NULL; tag = tag->next)
    {
    if (tagName == NULL || sameWord(tagName, tag->name))
        {
	for (att = tag->attributes; att != NULL; att = att->next)
	    {
	    if (sameWord(attribute, att->name))
	        {
		el = slNameNew(att->val);
		slAddHead(&list, el);
		}
	    }
	}
    }
slReverse(&list);
return list;
}

struct slName *htmlPageLinks(struct htmlPage *page)
/* Scan through tags list and pull out HREF attributes. */
{
return htmlPageScanAttribute(page, NULL, "HREF");
}

struct htmlTableRow
/* Data on a row */
    {
    struct htmlTableRow *next;
    int tdCount;
    int inTd;
    };

struct htmlTable 
/* Data on a table. */
    {
    struct htmlTable *next;
    struct htmlTableRow *row;
    int rowCount;
    };

static void validateTables(struct htmlPage *page, 
	struct htmlTag *startTag, struct htmlTag *endTag)
/* Validate <TABLE><TR><TD> are all properly nested, and that there
 * are no empty rows. */
{
struct htmlTable *tableStack = NULL, *table;
struct htmlTableRow *row;
struct htmlTag *tag;

for (tag = startTag; tag != endTag; tag = tag->next)
    {
    if (sameWord(tag->name, "TABLE"))
        {
	if (tableStack != NULL)
	    {
	    if (tableStack->row == NULL || !tableStack->row->inTd)
	    tagAbort(page, tag, "TABLE inside of another table, but not inside of <TR><TD>\n");
	    }
	AllocVar(table);
	slAddHead(&tableStack, table);
	}
    else if (sameWord(tag->name, "/TABLE"))
        {
	if ((table = tableStack) == NULL)
	    tagAbort(page, tag, "Extra </TABLE> tag");
	if (table->rowCount == 0)
	    tagAbort(page, tag, "<TABLE> with no <TR>'s");
	if (table->row != NULL)
	    tagAbort(page, tag, "</TABLE> inside of a row");
	tableStack = table->next;
	freez(&table);
	}
    else if (sameWord(tag->name, "TR"))
        {
	if ((table = tableStack) == NULL)
	    tagAbort(page, tag, "<TR> outside of TABLE");
	if (table->row != NULL)
	    tagAbort(page, tag, "<TR>...<TR> with no </TR> in between");
	AllocVar(table->row);
	table->rowCount += 1;
	}
    else if (sameWord(tag->name, "/TR"))
        {
	if ((table = tableStack) == NULL)
	    tagAbort(page, tag, "</TR> outside of TABLE");
	if (table->row == NULL)
	    tagAbort(page, tag, "</TR> with no <TR>");
	if (table->row->inTd)
	    {
	    tagAbort(page, tag, "</TR> while <TD> is open");
	    }
	if (table->row->tdCount == 0)
	    tagAbort(page, tag, "Empty row in <TABLE>");
	freez(&table->row);
	}
    else if (sameWord(tag->name, "TD") || sameWord(tag->name, "TH"))
        {
	if ((table = tableStack) == NULL)
	    tagAbort(page, tag, "<%s> outside of <TABLE>", tag->name);
	if ((row = table->row) == NULL)
	    tagAbort(page, tag, "<%s> outside of <TR>", tag->name);
	if (row->inTd)
	    tagAbort(page, tag, "<%s>...<%s> with no </%s> in between", 
	    	tag->name, tag->name, tag->name);
	row->inTd = TRUE;
	row->tdCount += 1;
	}
    else if (sameWord(tag->name, "/TD") || sameWord(tag->name, "/TH"))
        {
	if ((table = tableStack) == NULL)
	    tagAbort(page, tag, "<%s> outside of <TABLE>", tag->name);
	if ((row = table->row) == NULL)
	    tagAbort(page, tag, "<%s> outside of <TR>", tag->name);
	if (!row->inTd)
	    tagAbort(page, tag, "<%s> with no <%s>", tag->name, tag->name+1);
	row->inTd = FALSE;
	}
    }
if (tableStack != NULL)
    tagAbort(page, tag, "Missing </TABLE>");
}

static void checkTagIsInside(struct htmlPage *page, char *outsiders, char *insiders,  
	struct htmlTag *startTag, struct htmlTag *endTag)
/* Check that insiders are all bracketed by outsiders. */
{
char *outDupe = cloneString(outsiders);
char *inDupe = cloneString(insiders);
char *line, *word;
int depth = 0;
struct htmlTag *tag;
struct hash *outOpen = newHash(8);
struct hash *outClose = newHash(8);
struct hash *inHash = newHash(8);
char buf[256];

/* Create hashes of all insiders */
line = inDupe;
while ((word = nextWord(&line)) != NULL)
    {
    touppers(word);
    hashAdd(inHash, word, NULL);
    }

/* Create hash of open and close outsiders. */
line = outDupe;
while ((word = nextWord(&line)) != NULL)
    {
    touppers(word);
    hashAdd(outOpen, word, NULL);
    safef(buf, sizeof(buf), "/%s", word);
    hashAdd(outClose, buf, NULL);
    }

/* Stream through tags making sure that insiders are
 * at least one deep inside of outsiders. */
for (tag = startTag; tag != NULL; tag = tag->next)
    {
    char *type = tag->name;
    if (hashLookup(outOpen, type ))
        ++depth;
    else if (hashLookup(outClose, type))
        --depth;
    else if (hashLookup(inHash, type))
        {
	if (depth <= 0)
	    tagAbort(page, tag, "%s outside of any of %s", type, outsiders);
	}
    }
freeHash(&inHash);
freeHash(&outOpen);
freeHash(&outClose);
freeMem(outDupe);
freeMem(inDupe);
}

static void checkNest(struct htmlPage *page,
	char *type, struct htmlTag *startTag, struct htmlTag *endTag)
/* Check that <type> and </type> tags are properly nested. */
{
struct htmlTag *tag;
int depth = 0;
char endType[256];
safef(endType, sizeof(endType), "/%s", type);
for (tag = startTag; tag != endTag; tag = tag->next)
    {
    if (sameWord(tag->name, type))
	++depth;
    else if (sameWord(tag->name, endType))
        {
	--depth;
	if (depth < 0)
	   tagAbort(page, tag, "<%s> without preceding <%s>", endType, type);
	}
    }
if (depth != 0)
    errAbort("Missing <%s> tag", endType);
}

static void validateNestingTags(struct htmlPage *page,
	struct htmlTag *startTag, struct htmlTag *endTag,
	char *nesters[], int nesterCount)
/* Validate many tags that do need to nest. */
{
int i;
for (i=0; i<nesterCount; ++i)
    checkNest(page, nesters[i], startTag, endTag);
}

static char *bodyNesters[] = 
/* Nesting tags that appear in body. */
{
    "ADDRESS", "DIV", "H1", "H2", "H3", "H4", "H5", "H6",
    "ACRONYM", "BLOCKQUOTE", "CITE", "CODE", "DEL", "DFN"
    "DIR", "DL", "MENU", "OL", "UL", "CAPTION", "TABLE", 
    "A", "MAP", "OBJECT", "FORM"
};

static char *headNesters[] =
/* Nesting tags that appear in header. */
{
    "TITLE",
};

static struct htmlTag *validateBody(struct htmlPage *page, struct htmlTag *startTag)
/* Go through tags from current position (just past <BODY>)
 * up to and including </BODY> and check some things. */
{
struct htmlTag *tag, *endTag = NULL;

/* First search for end tag. */
for (tag = startTag; tag != NULL; tag = tag->next)
    {
    if (sameWord(tag->name, "/BODY"))
        {
	endTag = tag;
	break;
	}
    }
if (endTag == NULL)
    errAbort("Missing </BODY>");
validateTables(page, startTag, endTag);
checkTagIsInside(page, "DIR MENU OL UL", "LI", startTag, endTag);
checkTagIsInside(page, "DL", "DD DT", startTag, endTag);
checkTagIsInside(page, "COLGROUP TABLE", "COL", startTag, endTag);
checkTagIsInside(page, "MAP", "AREA", startTag, endTag);
checkTagIsInside(page, "FORM", 
	"INPUT BUTTON /BUTTON OPTION SELECT /SELECT TEXTAREA /TEXTAREA"
	"FIELDSET /FIELDSET"
	, 
	startTag, endTag);
validateNestingTags(page, startTag, endTag, bodyNesters, ArraySize(bodyNesters));
return endTag->next;
}

static char *urlOkChars()
/* Return array character indexed array that has
 * 1 for characters that are ok in URLs and 0
 * elsewhere. */
{
char *okChars;
int c;
AllocArray(okChars, 256);
for (c=0; c<256; ++c)
    if (isalnum(c))
        okChars[c] = 1;
/* This list is a little more inclusive than W3's. */
okChars['='] = 1;
okChars['-'] = 1;
okChars['/'] = 1;
okChars['%'] = 1;
okChars['.'] = 1;
okChars[';'] = 1;
okChars['_'] = 1;
okChars['&'] = 1;
okChars['+'] = 1;
return okChars;
}

static void validateCgiUrl(char *url)
/* Make sure URL follows basic CGI encoding rules. */
{
if (startsWith("http:", url) || startsWith("https:", url))
    {
    static char *okChars = NULL;
    UBYTE c, *s;
    if (okChars == NULL)
	okChars = urlOkChars();
    url = strchr(url, '?');
    if (url != NULL)
	{
	s = (UBYTE*)url+1;
	while ((c = *s++) != 0)
	    {
	    if (!okChars[c])
		{
		errAbort("Character %c not allowed in URL %s", c, url);
		}
	    }
	}
    }
}

static void validateCgiUrls(struct htmlPage *page)
/* Make sure URLs in page follow basic CGI encoding rules. */
{
struct htmlForm *form;
struct slName *linkList = htmlPageLinks(page), *link;

for (form = page->forms; form != NULL; form = form->next)
    validateCgiUrl(form->action);
for (link = linkList; link != NULL; link = link->next)
    validateCgiUrl(link->name);
slFreeList(&linkList);
}

void htmlPageValidateOrAbort(struct htmlPage *page)
/* Do some basic validations.  Aborts if there is a problem. */
{
struct htmlTag *tag;
boolean gotTitle = FALSE;

/* To simplify things upper case all tag names. */
for (tag = page->tags; tag != NULL; tag = tag->next)
    touppers(tag->name);

/* Validate header, and make a suggestion or two */
if ((tag = page->tags) == NULL)
    errAbort("No tags");
if (!sameWord(tag->name, "HTML"))
    errAbort("Doesn't start with <HTML> tag");
tag = tag->next;
if (tag == NULL || !sameWord(tag->name, "HEAD"))
    errAbort("<HEAD> tag does not follow <HTML> tag");
for (;;)
    {
    tag = tag->next;
    if (tag == NULL)
        errAbort("Missing </HEAD>");
    if (sameWord(tag->name, "TITLE"))
        gotTitle = TRUE;
    if (sameWord(tag->name, "/HEAD"))
        break;
    }
if (!gotTitle)
    warn("No title in <HEAD>");
validateNestingTags(page, page->tags, tag, headNesters, ArraySize(headNesters));
tag = tag->next;
if (tag == NULL || !sameWord(tag->name, "BODY"))
    errAbort("<BODY> tag does not follow <HTML> tag");
tag = validateBody(page, tag->next);
if (tag == NULL || !sameWord(tag->name, "/HTML"))
    errAbort("Missing </HTML>");
validateCgiUrls(page);
verbose(1, "ok\n");
}

