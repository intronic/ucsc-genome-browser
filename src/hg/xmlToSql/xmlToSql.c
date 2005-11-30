/* xmlToSql - Convert XML dump into a fairly normalized relational database.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "options.h"
#include "portable.h"
#include "xap.h"
#include "dtdParse.h"
#include "elStat.h"
#include "rename.h"

static char const rcsid[] = "$Id: xmlToSql.c,v 1.23 2005/11/30 07:50:45 kent Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "xmlToSql - Convert XML dump into a fairly normalized relational database\n"
  "   in the form of a directory full of tab-separated files and table\n"
  "   creation SQL.  You'll need to run autoDtd on the XML file first to\n"
  "   get the dtd and stats files.\n"
  "usage:\n"
  "   xmlToSql in.xml in.dtd in.stats outDir\n"
  "options:\n"
  "   -prefix=name - A name to prefix all tables with\n"
  "   -textField=name - Name to use for text field (default 'text')\n"
  );
}

char *globalPrefix = "";
char *textField = "text";

static struct optionSpec options[] = {
   {"prefix", OPTION_STRING},
   {"textField", OPTION_STRING},
   {NULL, 0},
};

struct table
/* Information about one of the tables we are making. */
    {
    struct table *next;		/* Next in list. */
    char *name;			/* Name of table. */
    struct field *fieldList;	/* Information about each field. */
    struct hash *fieldHash;	/* Fields keyed by field name. */
    struct hash *fieldMixedHash;/* Fields keyed by field mixed case name. */
    struct elStat *elStat;	/* Associated elStat structure. */
    struct dtdElement *dtdElement; /* Associated dtd element. */
    struct field *primaryKey;	/* Primary key if any. */
    boolean madeUpPrimary;	/* True if we are creating primary key. */
    int lastId;			/* Last id value if we create key. */
    struct assocRef *parentAssocs;  /* Association table linking to parent. */
    struct fieldRef *parentKeys; /* References to possible parents. */
    boolean linkedParents;	/* True if we have linked parents. */
    boolean isAssoc;		/* True if an association we've created. */
    int usesAsChild;		/* Number of times this is a child of another
                                 * table. */
    FILE *tabFile;		/* Tab oriented file associated with table */
    struct hash *uniqHash;	/* Table to insure unique output. */
    struct dyString *uniqString;/* Key into unique hash. Also most of output */
    struct assoc *assocList;    /* List of pending associations. */
    };

struct field
/* Information about a field. */
    {
    struct field *next;		/* Next in list. */
    char *name;			/* Name of field as it is in XML. */
    char *mixedCaseName;	/* Mixed case name - as it is in SQL. */
    struct table *table;	/* Table this is part of. */
    struct attStat *attStat;	/* Associated attStat structure. */
    struct dtdAttribute *dtdAttribute;	/* Associated dtd attribute. */
    boolean isMadeUpKey;	/* True if it's a made up key. */
    boolean isPrimaryKey;	/* True if it's table's primary key. */
    boolean isString;		/* Truf if it's a string field. */
    struct dyString *dy;	/* Current value of field during parsing. */
    };

struct fieldRef
/* A reference to a field. */
    {
    struct fieldRef *next;	/* Next in list */
    struct field *field;	/* Associated field. */
    };

struct assocRef
/* A reference to a table. */
    {
    struct assocRef *next;	/* Next in list. */
    struct table *assoc;	/* Association table */
    struct table *parent;	/* Parent table we're associated with */
    };

struct dtdAttribute *findDtdAttribute(struct dtdElement *element, char *name)
/* Find named attribute in element, or NULL if no such attribute. */
{
struct dtdAttribute *dtdAtt;
for (dtdAtt = element->attributes; dtdAtt != NULL; dtdAtt = dtdAtt->next)
    {
    if (sameString(dtdAtt->name, name))
        break;
    }
return dtdAtt;
}

struct dtdAttribute *findDtdAttributeMixed(struct dtdElement *element, char *name)
/* Find named attribute in element, or NULL if no such attribute. */
{
struct dtdAttribute *dtdAtt;
for (dtdAtt = element->attributes; dtdAtt != NULL; dtdAtt = dtdAtt->next)
    {
    if (sameString(dtdAtt->mixedCaseName, name))
        break;
    }
return dtdAtt;
}


struct field *addFieldToTable(struct table *table, 
	char *name, char *mixedCaseName,
	struct attStat *att, boolean isMadeUpKey, boolean atTail, 
	boolean isString)
/* Add field to end of table.  Use proposedName if it's unique,
 * otherwise proposed name with some numerical suffix. */
{
struct field *field;

AllocVar(field);
field->name = cloneString(name);
field->mixedCaseName = cloneString(mixedCaseName);
field->table = table;
field->attStat = att;
field->isMadeUpKey = isMadeUpKey;
field->isString = isString;
if (!isMadeUpKey)
    {
    field->dtdAttribute = findDtdAttribute(table->dtdElement, name);
    if (field->dtdAttribute == NULL)
        {
	if (att != NULL && !sameString(name, textField))
	    errAbort("%s.%s is in .stats but not in .dtd file", 
		table->name, field->name);
	}
    }
field->dy = dyStringNew(16);
hashAdd(table->fieldHash, name, field);
hashAdd(table->fieldMixedHash, mixedCaseName, field);
if (atTail)
    {
    slAddTail(&table->fieldList, field);
    }
else
    {
    slAddHead(&table->fieldList, field);
    }
return field;
}

void makePrimaryKey(struct table *table)
/* Figure out primary key, using an existing field if
 * possible, otherwise making one up. */
{
struct field *primaryKey = NULL, *field;
int rowCount = table->elStat->count;

/* First scan for an int key, which we prefer. */
for (field = table->fieldList; field != NULL; field = field->next)
    {
    struct attStat *att = field->attStat;
    if (att->uniqCount == rowCount && sameString(att->type, "int"))
        {
	primaryKey = field;
	break;
	}
    }

/* If no int key, try for a short string key */
if (primaryKey == NULL)
    {
    for (field = table->fieldList; field != NULL; field = field->next)
	{
	struct attStat *att = field->attStat;
	if (att->uniqCount == rowCount && sameString(att->type, "string")
	    && att->maxLen <= 12 && !sameString(field->name, textField))
	    {
	    primaryKey = field;
	    break;
	    }
	}
    }

/* If still no key, we have to make one up */
if (primaryKey == NULL)
    {
    char *fieldName = renameUnique(table->fieldMixedHash, "id");
    primaryKey = addFieldToTable(table, fieldName, fieldName,
    	NULL, TRUE, FALSE, FALSE);
    table->madeUpPrimary = TRUE;
    }
table->primaryKey = primaryKey;
primaryKey->isPrimaryKey = TRUE;
}

boolean attIsString(struct attStat *att)
/* Return TRUE if att is of string type. */
{
return sameString(att->type, "string");
}

struct table *tableNew(char *name, struct elStat *elStat,
	struct dtdElement *dtdElement)
/* Create a new table structure. */
{
struct table *table;
AllocVar(table);
table->name = cloneString(name);
table->elStat = elStat;
table->dtdElement = dtdElement;
table->fieldHash = hashNew(8);
table->fieldMixedHash = hashNew(8);
table->uniqHash = hashNew(17);
table->uniqString = dyStringNew(0);
return table;
}

struct table *elsIntoTables(struct elStat *elList, struct hash *dtdHash)
/* Create table and field data structures from element/attribute
 * data structures. */
{
struct elStat *el;
struct attStat *att;
struct table *tableList = NULL, *table;
struct field *field;

for (el = elList; el != NULL; el = el->next)
    {
    struct dtdElement *dtdElement = hashFindVal(dtdHash, el->name);
    if (dtdElement == NULL)
        errAbort("Element %s is in .stats but not in .dtd file", el->name);
    table = tableNew(dtdElement->mixedCaseName, el, dtdElement);
    for (att = el->attList; att != NULL; att = att->next)
        {
	char *name = att->name;
	char *mixedName = NULL;
	if (sameString(name, "<text>"))
	    name = mixedName = textField;
	else
	    {
	    struct dtdAttribute *dtdAtt = findDtdAttribute(dtdElement, name);
	    if (dtdAtt == NULL)
		errAbort("Element %s attribute %s is in .stats but not in .dtd file", 
			el->name, name);
	    mixedName = dtdAtt->mixedCaseName;
	    }
	field = addFieldToTable(table, name, mixedName, att, 
		FALSE, TRUE, attIsString(att));
	}
    makePrimaryKey(table);
    slAddHead(&tableList, table);
    }
slReverse(&tableList);
return tableList;
}

void countUsesAsChild(struct dtdElement *dtdList, struct hash *tableHash)
/* Count up how many times each table is used as a child. */
{
struct dtdElement *dtdEl;
struct table *table;
for (dtdEl = dtdList; dtdEl != NULL; dtdEl = dtdEl->next)
    {
    struct dtdElChild *child;

    /* Make sure table exists in table hash - not really necessary
     * for this function, but provides a further reality check that
     * dtd and spec go together. */
    table = hashFindVal(tableHash, dtdEl->mixedCaseName);
    if (table == NULL)
        errAbort("Table %s is in .dtd but not .stat file", dtdEl->name);

    /* Loop through dtd's children and add counts. */
    for (child = dtdEl->children; child != NULL; child = child->next)
        {
	table = hashMustFindVal(tableHash, child->el->mixedCaseName);
	table->usesAsChild += 1;
	}
    }
}

static struct hash *rUniqParentLinkHash;

void rAddParentKeys(struct dtdElement *parent, struct dtdElChild *elAsChild,
	struct hash *tableHash, struct table **pTableList, int level)
/* Recursively add parentKeys. */
{
struct dtdElement *element = elAsChild->el;
struct dtdElChild *child;
struct table *table = hashMustFindVal(tableHash, element->mixedCaseName);
struct table *parentTable = hashMustFindVal(tableHash, parent->mixedCaseName);
struct field *field;
char linkUniqName[256];
int i;
static struct dtdElChild *parentStack[256];

if (level >= ArraySize(parentStack))
    errAbort("Recursion too deep in rAddParentKeys");
parentStack[level] = elAsChild;

for (i=level-1; i>= 0; i -= 1)
    if (elAsChild == parentStack[i])
	{
	warn("WARNING: self-referential data structure %s.", elAsChild->name);
	warn("         Entering untested code.");
        return;	/* Avoid cycling on self. */
	}

/* Add new field in parent. */
safef(linkUniqName, sizeof(linkUniqName), "%s.%s", 
	parentTable->name, table->name);
if (!hashLookup(rUniqParentLinkHash, linkUniqName))
    {
    hashAdd(rUniqParentLinkHash, linkUniqName, NULL);
    verbose(3, "Linking %s to parent %s\n", table->name, parentTable->name);
    if (elAsChild->copyCode == '1' || elAsChild->copyCode == '?')
	{
	struct fieldRef *ref;
	char *fieldName = renameUnique(parentTable->fieldHash, element->name);
	char *fieldMixedName = renameUnique(parentTable->fieldMixedHash, 
		element->mixedCaseName);
	field = addFieldToTable(parentTable, 
		    fieldName, fieldMixedName,
		    table->primaryKey->attStat, TRUE, TRUE,
		    table->primaryKey->isString);
	AllocVar(ref);
	ref->field = field;
	slAddHead(&table->parentKeys, ref);
	}
    else
	{
	/* Need to handle association here. */
	struct table *assocTable;
	struct assocRef *ref;
	char joinedName[256];
	char *assocTableName;
	int upperAt;
	safef(joinedName, sizeof(joinedName), "%sTo%s", parentTable->name,
	  table->name);
	upperAt = strlen(parentTable->name) + 2;
	joinedName[upperAt] = toupper(joinedName[upperAt]);
	assocTableName = renameUnique(tableHash, joinedName);
	assocTable = tableNew(joinedName, NULL, NULL);
	assocTable->isAssoc = TRUE;
	addFieldToTable(assocTable, parentTable->name, parentTable->name,
	    parentTable->primaryKey->attStat, TRUE, TRUE, 
	    parentTable->primaryKey->isString);
	addFieldToTable(assocTable, table->name, table->name,
	    table->primaryKey->attStat, TRUE, TRUE, 
	    table->primaryKey->isString);
	slAddHead(pTableList, assocTable);
	AllocVar(ref);
	ref->assoc = assocTable;
	ref->parent = parentTable;
	slAddHead(&table->parentAssocs, ref);
	}
    table->linkedParents = TRUE;
    }
else
    verbose(3, "skipping link %s to parent %s\n", table->name, parentTable->name);
for (child = element->children;  child != NULL; child = child->next)
    rAddParentKeys(element, child, tableHash, pTableList, level+1);
}

void addParentKeys(struct dtdElement *dtdRoot, struct hash *tableHash,
	struct table **pTableList)
/* Use dtd to guide creation of field->parentKeys  */
{
struct dtdElChild *topLevel;

rUniqParentLinkHash = newHash(0);
for (topLevel = dtdRoot->children; topLevel != NULL; topLevel = topLevel->next)
    {
    struct dtdElement *mainIterator = topLevel->el;
    struct dtdElChild *child;
    for (child = mainIterator->children; child != NULL; child = child->next)
        {
	rAddParentKeys(mainIterator, child, tableHash, pTableList, 0);
	}
    }
hashFree(&rUniqParentLinkHash);
}


struct table *findRootTable(struct table *tableList)
/* Find root table (looking for one that has no uses as child) */
{
struct table *root = NULL, *table;
for (table = tableList; table != NULL; table = table->next)
    {
    if (table->usesAsChild == 0)
        {
        if (root != NULL)
            errAbort(".dtd file has two root tables: %s and %s", 
	      root->name, table->name);
	root = table;
	}
    }
if (root == NULL)
    errAbort("Can't find root table in dtd.  Circular linkage?");
return root;
}

/* Globals used by the streaming parser callbacks. */
struct hash *xmlTableHash;  /* Hash of tables keyed by XML tag names */
struct table *rootTable;    /* Highest level tag. */

void dyStringAppendEscapedForTabFile(struct dyString *dy, char *string)
/* Append string to dy, escaping if need be */
{
char c, *s;
boolean needsEscape = FALSE;

s =  string;
while ((c = *s++) != 0)
    {
    switch (c)
       {
       case '\\':
       case '\t':
       case '\n':
           needsEscape = TRUE;
       }
    }

if (needsEscape)
    {
    s = string;
    while ((c = *s++) != 0)
	{
	switch (c)
	   {
	   case '\\':
	      dyStringAppendN(dy, "\\\\", 2);
	      break;
	   case '\t':
	      dyStringAppendN(dy, "\\t", 2);
	      break;
	   case '\n':
	      dyStringAppendN(dy, "\\n", 2);
	      break;
	   default:
	      dyStringAppendC(dy, c);
	      break;
	   }
	}
    }
else
    dyStringAppend(dy, string);
}

void *startHandler(struct xap *xap, char *tagName, char **atts)
/* Called at the start of a tag after attributes are parsed. */
{
struct table *table = hashFindVal(xmlTableHash, tagName);
struct field *field;
int i;
boolean uniq = FALSE;

if (table == NULL)
    errAbort("Tag %s is in xml file but not dtd file", tagName);
/* Clear all fields. */
for (field = table->fieldList; field != NULL; field = field->next)
    {
    dyStringClear(field->dy);
    }

for (i=0; atts[i] != NULL; i += 2)
    {
    char *name = atts[i], *val = atts[i+1];
    field = hashFindVal(table->fieldHash, name);
    if (field == NULL)
        errAbort("Attribute %s of tag %s not in dtd", name, tagName);
    dyStringAppendEscapedForTabFile(field->dy, val);
    }
return table;
}

struct assoc
/* List of associations we can't write until we know
 * our key. */
    {
    struct assoc *next;
    FILE *f;		/* File to write to. */
    char *childKey;	/* Key in child (allocated here). */
    };

struct assoc *assocNew(FILE *f, char *childKey)
/* Create new association. */
{
struct assoc *assoc;
AllocVar(assoc);
assoc->f = f;
assoc->childKey = cloneString(childKey);
return assoc;
}

void assocFreeList(struct assoc **pList)
/* Free up list of associations. */
{
struct assoc *el, *next;
for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    freeMem(el->childKey);
    freeMem(el);
    }
*pList = NULL;
}

void endHandler(struct xap *xap, char *name)
/* Called at end of a tag */
{
struct table *table = xap->stack->object;
struct table *parentTable = xap->stack[1].object;
struct field *field;
struct fieldRef *fieldRef;
struct assocRef *assocRef;
struct dyString *dy = table->uniqString;
char *text = skipLeadingSpaces(xap->stack->text->string);
char *primaryKeyVal = NULL;
struct assoc *assoc;

if (text[0] != 0)
    {
    field = hashFindVal(table->fieldHash, textField);
    if (field == NULL)
        errAbort("No text for %s expected in dtd", table->name);
    dyStringAppendEscapedForTabFile(field->dy, text);
    }

/* Construct uniq string from fields, etc. */
dyStringClear(dy);
for (field = table->fieldList; field != NULL; field = field->next)
    {
    if (!(field->isPrimaryKey  && field->isMadeUpKey))
	{
	if (field->dy->stringSize == 0 && !field->isString)
	    {
	    dyStringAppendC(field->dy, '0');
	    }
	dyStringAppendN(dy, field->dy->string, field->dy->stringSize);
	if (field->next != NULL)
	    dyStringAppendC(dy, '\t');
	}
    }
for (assoc = table->assocList; assoc != NULL; assoc = assoc->next)
    {
    dyStringPrintf(dy, "%p\t%s\t", assoc->f, assoc->childKey);
    }

primaryKeyVal = hashFindVal(table->uniqHash, dy->string);
if (primaryKeyVal == NULL)
    {
    if (table->madeUpPrimary)
	{
	table->lastId += 1;
	dyStringPrintf(table->primaryKey->dy, "%d", table->lastId);
	}
    primaryKeyVal = table->primaryKey->dy->string;
    for (field = table->fieldList; field != NULL; field = field->next)
        {
	fprintf(table->tabFile, "%s", field->dy->string);
	if (field->next != NULL)
	   fprintf(table->tabFile, "\t");
	}
    fprintf(table->tabFile, "\n");
    hashAdd(table->uniqHash, dy->string, cloneString(primaryKeyVal));
    }
for (fieldRef = table->parentKeys; fieldRef != NULL; fieldRef = fieldRef->next)
    {
    field = fieldRef->field;
    if (field->table == parentTable)
        {
	dyStringAppend(field->dy, primaryKeyVal);
	break;
	}
    }

for (assocRef = table->parentAssocs; assocRef != NULL; 
	assocRef = assocRef->next)
    {
    if (assocRef->parent == parentTable)
        {
	assoc = assocNew(assocRef->assoc->tabFile,
	    primaryKeyVal);
	slAddHead(&parentTable->assocList, assoc);
	}
    }

slReverse(&table->assocList);
for (assoc = table->assocList; assoc != NULL; assoc = assoc->next)
    fprintf(assoc->f, "%s\t%s\n", primaryKeyVal, assoc->childKey);
assocFreeList(&table->assocList);
}

void printType(FILE *f, struct attStat *att)
/* Print out a good SQL type for attribute */
{
char *type = att->type;
int len = att->maxLen;
if (sameString(type, "int"))
    {
    if (len <= 1)
        fprintf(f, "tinyint");
    else if (len <= 3)
        fprintf(f, "smallint");
    else if (len <= 9)
        fprintf(f, "int");
    else
        fprintf(f, "bigint");
    }
else if (sameString(type, "string"))
    {
    if (len <= 64)
        fprintf(f, "varchar(255)");
    else
        fprintf(f, "longtext");
    }
else
    fprintf(f, "%s", type);
}

void writeCreateSql(char *fileName, struct table *table)
/* Write out table definition. */
{
FILE *f = mustOpen(fileName, "w");
struct field *field;

fprintf(f, "CREATE TABLE %s (\n", table->name);
for (field = table->fieldList; field != NULL; field = field->next)
    {
    struct attStat *att = field->attStat;
    fprintf(f, "    %s ", field->mixedCaseName);
    if (att == NULL)
        fprintf(f, "int");
    else
	printType(f, att);
    fprintf(f, " not null,");
    fprintf(f, "\n");
    }
if (table->isAssoc)
    {
    for (field = table->fieldList; field != NULL; field = field->next)
	{
	if (field->isString)
	    fprintf(f, "    INDEX(%s(12))", field->mixedCaseName);
	else
	    fprintf(f, "    INDEX(%s)", field->mixedCaseName);
	if (field->next != NULL)
	    fprintf(f, ",");
	fprintf(f, "\n");
	}
    }
else
    {
    struct field *primaryKey = table->primaryKey;
    char *keyName = primaryKey->name;
    if (primaryKey->isString)
	fprintf(f, "    PRIMARY KEY(%s(12))\n", table->primaryKey->mixedCaseName);
    else
	fprintf(f, "    PRIMARY KEY(%s)\n", table->primaryKey->mixedCaseName);
    }

fprintf(f, ");\n");

carefulClose(&f);
}

void dtdRenameMixedCase(struct dtdElement *dtdList)
/* Rename mixed case names in dtd to avoid conflicts with
 * C and SQL.  Likely will migrate this into dtdParse soon. */
{
struct hash *elHash = newHash(0);
struct dtdElement *el;
struct dtdElChild *child;
renameAddSqlWords(elHash);
renameAddCWords(elHash);

/* First rename tables if need be. */
for (el = dtdList; el != NULL; el = el->next)
    {
    el->mixedCaseName = renameUnique(elHash, el->mixedCaseName);
    hashAdd(elHash, el->mixedCaseName, NULL);
    }

/* Now rename fields in tables if need be. */
for (el = dtdList; el != NULL; el = el->next)
    {
    struct dtdAttribute *att;
    struct dtdElChild *child;
    struct hash *attHash = hashNew(8);
    renameAddSqlWords(attHash);
    renameAddCWords(attHash);

    /* Don't want attributes to conflict with child elements. */
    for (child = el->children; child != NULL; child = child->next)
        hashAdd(attHash, child->el->mixedCaseName, NULL);
    for (att = el->attributes; att != NULL; att = att->next)
	{
        att->mixedCaseName = renameUnique(attHash, att->mixedCaseName);
	hashAdd(attHash, att->mixedCaseName, NULL);
	}
    hashFree(&attHash);
    }
hashFree(&elHash);
}

void xmlToSql(char *xmlFileName, char *dtdFileName, char *statsFileName,
	char *outDir)
/* xmlToSql - Convert XML dump into a fairly normalized relational database. */
{
struct elStat *elStatList = NULL;
struct dtdElement *dtdList, *dtdEl;
struct hash *dtdHash, *dtdMixedHash = hashNew(0);
struct table *tableList = NULL, *table;
struct hash *tableHash = hashNew(0);
struct xap *xap = xapNew(startHandler, endHandler, xmlFileName);
char outFile[PATH_LEN];

/* Load up dtd and stats file. */
elStatList = elStatLoadAll(statsFileName);
verbose(2, "%d elements in %s\n", slCount(elStatList), statsFileName);
dtdParse(dtdFileName, globalPrefix, textField,
	&dtdList, &dtdHash);
dtdRenameMixedCase(dtdList);
verbose(1, "%d elements in %s\n", dtdHash->elCount, dtdFileName);

/* Build up hash of dtdElements keyed by mixed name rather
 * than tag name. */
for (dtdEl = dtdList; dtdEl != NULL; dtdEl = dtdEl->next)
    hashAdd(dtdMixedHash, dtdEl->mixedCaseName, dtdEl);

/* Create list of tables that correspond to tag types. 
 * This doesn't include any association tables we create
 * to handle lists of child elements. */
tableList = elsIntoTables(elStatList, dtdHash);
verbose(2, "Made tableList\n");

/* Create hashes of the table lists - one keyed by the
 * table name, and one keyed by the tag name. */
xmlTableHash = hashNew(0);
for (table = tableList; table != NULL; table = table->next)
    {
    hashAdd(tableHash, table->name, table);
    hashAdd(xmlTableHash, table->dtdElement->name, table);
    }
verbose(2, "Made table hashes\n");

/* Find top level tag (which we won't actually output). */
countUsesAsChild(dtdList, tableHash);
verbose(2, "Past countUsesAsChild\n");
rootTable = findRootTable(tableList);
verbose(2, "Root table is %s\n", rootTable->name);

/* Add stuff to support parent-child relationships. */
addParentKeys(rootTable->dtdElement, tableHash, &tableList);
verbose(2, "Added parent keys\n");

/* Make output directory. */
makeDir(outDir);

/* Make table creation SQL files. */
for (table = tableList; table != NULL; table = table->next)
    {
    safef(outFile, sizeof(outFile), "%s/%s.sql", 
      outDir, table->name);
    writeCreateSql(outFile, table);
    }
verbose(2, "Made sql table creation files\n");

/* Set up output directory and open tab-separated files. */
for (table = tableList; table != NULL; table = table->next)
    {
    safef(outFile, sizeof(outFile), "%s/%s.tab", 
      outDir, table->name);
    table->tabFile = mustOpen(outFile, "w");
    }
verbose(2, "Created output files.\n");

/* Stream through XML adding to tab-separated files.. */
xapParseFile(xap, xmlFileName);
verbose(2, "Streamed through XML\n");

/* Close down files */
for (table = tableList; table != NULL; table = table->next)
    carefulClose(&table->tabFile);
verbose(2, "Closed tab files\n");

verbose(1, "All done\n");
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
globalPrefix = optionVal("prefix", globalPrefix);
textField = optionVal("textField", textField);
if (argc != 5)
    usage();
xmlToSql(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
