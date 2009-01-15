/* Handle variable substitutions in strings from trackDb and other
 * labels. See trackDb/README for descriptions of values that can be
 * substitute.  This code needs to do a special handle to remain compatibility
 * with behavior of the old substitution mechanism. */
#include "common.h"
#include "trackDb.h"
#include "hdb.h"
#include "sqlNum.h"
#include "hVarSubst.h"

static boolean isVarEnd(boolean inBraces, char c)
/** does the character end a variable reference. */
{
if (inBraces)
    return (c == '}');
else
    return !((c == '_') || isalnum(c));
}

static char *parseVarName(char *desc, char *varStart, char *varName, int varNameSize)
/* parse substitution variable name out of a string, returning next position
 * after name. */
{
char *p = varStart+1;
if (*p == '\0')
    errAbort("trailing `$' while doing variable substitution in %s", desc);
boolean inBraces = (*p == '{');
if (inBraces)
    p++;
int nameIdx = 0;
while ((nameIdx < varNameSize-1) && !isVarEnd(inBraces, *p))
    varName[nameIdx++] = *p++;
if (nameIdx == 0)
    errAbort("empty variable name in %s", desc);
if (nameIdx == varNameSize)
    errAbort("variable name in desc %s exceeds maximum length of %d, starting with: \"%.*s\"",
             desc, varNameSize-1, varNameSize-1, varName);
varName[nameIdx] = '\0';
if (inBraces)
    p++;
return p;
}

static char *lookupTrackDbSubVar(char *desc, struct trackDb *tdb, char *settingName, char *varName)
/* get the specified track setting to substitute or die trying; more useful
 * message than trackDbRequiredSetting when doing substitution */
{
char *val = trackDbSetting(tdb, settingName);
if (val == NULL)
   errAbort("trackDb setting \"%s\" not found for variable substitution of \"$%s\" in %s",
            settingName, varName, desc);
return val;
}

static char *lookupOtherDb(char *desc, struct trackDb *tdb, char *varName)
/* look up the otherDb variable, which is needed for substituting varName */
{
return lookupTrackDbSubVar(desc, tdb, "otherDb", varName);
}

static void insertMatrixHtml(struct trackDb *tdb, char *matrix,
                             char *matrixHeader, struct dyString *dest)
/* Generate HTML table from matrix setting in trackDb. matrixHeader is
 * optional. */
{
char *words[100];
char *headerWords[10];
int size;
int i, j, k;
int wordCount = 0, headerCount = 0;

wordCount = chopString(cloneString(matrix), ", \t", words, ArraySize(words));
if (matrixHeader != NULL)
    headerCount = chopString(cloneString(matrixHeader), 
                    ", \t", headerWords, ArraySize(headerWords));
errno = 0;
size = sqrt(sqlDouble(words[0]));
if (errno)
    errAbort("Invalid matrix size in for track %s: %s\n", tdb->tableName,
             words[0]);
dyStringAppend(dest, "The following matrix was used:<P>\n");
k = 1;
dyStringAppend(dest, "<BLOCKQUOTE><TABLE BORDER=1 CELLPADDING=4 BORDERCOLOR=\"#aaaaaa\">\n");
if (matrixHeader)
    {
    dyStringAppend(dest, "<TR ALIGN=right><TD></TD>");
    for (i = 0; i < size && i < headerCount; i++)
        dyStringPrintf(dest, "<TD><B>%s</B></TD>", headerWords[i]);
    dyStringAppend(dest, "</TR>\n");
    }
for (i = 0; i < size; i++)
    {
    dyStringAppend(dest, "<TR ALIGN=right>");
    if (matrixHeader)
        dyStringPrintf(dest, "<TD><B>%s<B></TD>", headerWords[i]);
    for (j = 0; j < size && k < wordCount ; j++)
        dyStringPrintf(dest, "<TD>%s</TD>", words[k++]);
    dyStringAppend(dest, "</TR>\n");
    }
dyStringAppend(dest, "</TABLE></BLOCKQUOTE></P>\n");
}

static void substMatrixHtml(struct trackDb *tdb, struct dyString *dest)
/* Generate HTML table from matrix setting in trackDb.  Note: for
 * compatibility, substitutes and empty string if matrix setting not found in
 * trackDb. */
{
char *matrix = trackDbSetting(tdb, "matrix");
if (matrix != NULL)
    insertMatrixHtml(tdb, matrix, trackDbSetting(tdb, "matrixHeader"), dest);
}

static boolean isAbbrevScientificName(char *name)
/* Return true if name looks like an abbreviated scientific name 
* (e.g. D. yakuba). */
{
return (name != NULL && strlen(name) > 4 &&
	isalpha(name[0]) &&
	name[1] == '.' && name[2] == ' ' &&
	isalpha(name[3]));
}

static boolean isDatabaseVar(char *varBase)
/* Is this a variable that can be resolved only from the database name?
 * Specify the base name, excluding the o_ prefix. */
{
return (strcasecmp(varBase, "organism") == 0)
    || (strcasecmp(varBase, "date") == 0)
    || (strcasecmp(varBase, "db") == 0);
}

static void substDatabaseVar(char *database, char *varBase,
                             struct dyString *dest)
/* substitute a variable resolved from the database name.
 * Specify the base name, excluding the o_ prefix. If database
 * can be looked up, just substitute the database name. */
{
if (sameString(varBase, "Organism"))
    {
    char *org = hOrganism(database);
    if (org == NULL)
        dyStringAppend(dest, database);
    else
        {
        dyStringAppend(dest, org);
        freeMem(org);
        }
    }
else if (sameString(varBase, "ORGANISM"))
    {
    char *org = hOrganism(database);
    if (org == NULL)
        dyStringAppend(dest, database);
    else
        {
        touppers(org);
        dyStringAppend(dest, org);
        freeMem(org);
        }
    }
else if (sameString(varBase, "organism"))
    {
    char *org = hOrganism(database);
    if (org == NULL)
        dyStringAppend(dest, database);
    else
        {
        if (!isAbbrevScientificName(org))
            tolowers(org);
        dyStringAppend(dest, org);
        freeMem(org);
        }
    }
else if (sameString(varBase, "date"))
    {
    char *date = hFreezeDateOpt(database);
    if (date == NULL)
        dyStringAppend(dest, database);
    else
        {
        dyStringAppend(dest, date);
        freeMem(date);
        }
    }
else if (sameString(varBase, "db"))
    dyStringAppend(dest, database);
}

static void substTrackDbVar(char *desc, struct trackDb *tdb, char *database,
                            char *varName, struct dyString *dest)
/* substitute a variable value obtained from trackDb */
{
if (sameString(varName, "matrix"))
    substMatrixHtml(tdb, dest);
else
    dyStringAppend(dest, lookupTrackDbSubVar(desc, tdb, varName, varName));
}

static void substVar(char *desc, struct trackDb *tdb, char *database,
                     char *varName, struct dyString *dest)
/* look up varName and insert value in output string.  Error if variable
 * can't be found */
{
if (isDatabaseVar(varName))
    substDatabaseVar(database, varName, dest);
else if (tdb == NULL)
    errAbort("invalid variable \"%s\" to substitute in %s",
             varName, desc);
else if (startsWith("o_", varName) && isDatabaseVar(varName+2))
    substDatabaseVar(lookupOtherDb(desc, tdb, varName), varName+2, dest);
else
    substTrackDbVar(desc, tdb, database, varName, dest);
}

char *hVarSubst(char *desc, struct trackDb *tdb, char *database, char *src)
/* Parse a string and substitute variable references.  Return NULL if
 * no variable references were found.  Error on missing variables (except
 * $matrix).  desc is a brief description to print on error to help with
 * debugging. tdb maybe NULL to only do substitutions based on database
 * and organism. See trackDb/README for more information.*/
{
struct dyString *dest = NULL;
char *start = src;  // start of current static string in src
char *next = src;   // cursor
char varName[65];

while ((next = strchr(next, '$')) != NULL)
    {
    if (*(next+1) == '$')
        next +=2;  // $$ is a literal
    else
        {
        if (dest == NULL)
            dest = dyStringNew(strlen(src));
        dyStringAppendN(dest, start, next-start);
        start = next = parseVarName(desc, next, varName, sizeof(varName));
        substVar(desc, tdb, database, varName, dest);
        }
    }
if (dest != NULL)
    {
    dyStringAppend(dest, start);
    return dyStringCannibalize(&dest);
    }
else
    return NULL; // no substitutions
}

void hVarSubstInVar(char *desc, struct trackDb *tdb, char *database, char **varPtr)
/* hVarSubst on a dynamically allocated string, replacing string in substitutions
 * occur, freeing the old memory if necessary.  See hVarSubst for details.
 */
{
char *dest = hVarSubst(desc, tdb, database, *varPtr);
if (dest != NULL)
    {
    freez(varPtr);
    *varPtr = dest;
    }
}
                       
void hVarSubstTrackDb(struct trackDb *tdb, char *database)
/* Substitute variables in trackDb shortLabel, longLabel, and html fields. */
{
hVarSubstInVar(tdb->tableName, tdb, database, &tdb->shortLabel);
hVarSubstInVar(tdb->tableName, tdb, database, &tdb->longLabel);
hVarSubstInVar(tdb->tableName, tdb, database, &tdb->html);
}
