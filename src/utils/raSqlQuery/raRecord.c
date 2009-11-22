#include "common.h"
#include "localmem.h"
#include "linefile.h"
#include "raRecord.h"

struct raFilePos *raFilePosNew(struct lm *lm, char *fileName, int lineIx)
/* Create new raFilePos record. */
{
struct raFilePos *fp;
lmAllocVar(lm, fp);
fp->fileName = fileName;
fp->lineIx = lineIx;
return fp;
}

struct raField *raRecordField(struct raRecord *ra, char *fieldName)
/* Return named field if it exists, otherwise NULL */
{
struct raField *field;
for (field = ra->fieldList; field != NULL; field = field->next)
    {
    if (sameString(field->name, fieldName))
        return field;
    }
return NULL;
}

struct raField *raFieldFromLine(char *line, struct lm *lm)
/* Parse out line and convert it to a raField.  Will return NULL on empty lines. 
 * Will insert some zeroes into the input line as well. */
{
char *word = nextWord(&line);
if (word == NULL)
    return NULL;
struct raField *field;
lmAllocVar(lm, field);
field->name = lmCloneString(lm, word);
char *val = emptyForNull(skipLeadingSpaces(line));
field->val = lmCloneString(lm, val);
return field;
}

char *lmCloneFirstWord(struct lm *lm, char *line)
/* Clone first word in line */
{
char *startFirstWord = skipLeadingSpaces(line);
if (startFirstWord == NULL)
    return NULL;
char *endFirstWord = skipToSpaces(startFirstWord);
if (endFirstWord == NULL)
    return lmCloneString(lm, startFirstWord);
else
    return lmCloneStringZ(lm, startFirstWord, endFirstWord - startFirstWord);
}
    
struct raRecord *raRecordReadOne(struct lineFile *lf, char *key, struct lm *lm)
/* Read next record from file. Returns NULL at end of file. */
{
struct raField *field, *fieldList = NULL;
char *line;
char *keyVal = NULL;
boolean override = FALSE;

/* Skip over blank initial lines. */
for (;;)
    {
    if (!lineFileNext(lf, &line, NULL))
        return NULL;
    line = skipLeadingSpaces(line);
    if (line != NULL && (line[0] != 0 && line[0] != '#'))
         {
	 lineFileReuse(lf);
	 break;
	 }
    }

/* Keep going until get a blank line. */
for (;;)
    {
    if (!lineFileNext(lf, &line, NULL))
        break;
    line = skipLeadingSpaces(line);
    if (line[0] == '#')
        continue;
    field = raFieldFromLine(line, lm);
    if (field == NULL)
        break;
    if (sameString(field->name, key))
	{
	keyVal = lmCloneFirstWord(lm, field->val);
	if (endsWith(field->val, "override") && !sameString("override", field->val))
	    override = TRUE;
	}
    slAddHead(&fieldList, field);
    }

if (fieldList == NULL)
    return NULL;
slReverse(&fieldList);
struct raRecord *record;
lmAllocVar(lm, record);
record->fieldList = fieldList;
record->key = keyVal;
record->override = override;
return record;
}

