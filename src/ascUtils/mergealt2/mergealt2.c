/* mergealt2 - looking at newalt raw file decide what should be taken out
 * of oldalt and what maybe needs to go in.  NewAlt was programatically
 * generated by an improved database searcher. OldAlt was hand edited from
 * a previous database searcher.  If somethings not in newAlt it shouldn't
 * be in oldAlt.  Only some of the new stuff in newalt really belongs. */
#include "common.h"
#include "hash.h"


 struct altSpec
    {
    struct altSpec *next;
    char *hrefTag;
    char *orfName;
    boolean skipExon, skipIntron, nonGenomic;
    int alt3, alt5;
    int ieOverlap, iiOverlap;
    struct slName *cdna;
    };

int cmpAlts(const void *va, const void *vb)
/* Compare two slNames. */
{
const struct altSpec *a = *((struct altSpec **)va);
const struct altSpec *b = *((struct altSpec **)vb);
return strcmp(a->orfName, b->orfName);
}

char *eatHrefTag(char *line)
{
char *startTag = strchr(line, '<');
char *endTag = strchr(startTag, '>');
char *hrefTag = cloneStringZ(startTag, endTag-startTag+1);
strcpy(startTag, endTag+1);
startTag = strchr(startTag, '<');
endTag = strchr(startTag, '>');
strcpy(startTag, endTag+1);
return hrefTag;
}

struct altSpec *readAlts(char *fileName)
{
FILE *f = mustOpen(fileName, "r");
char lineBuf[2048];
int lineLen;
int lineCount = 0;
struct altSpec *list = NULL, *el;
char *words[512];
int wordCount;
char *linePt;
int i;

while (fgets(lineBuf, sizeof(lineBuf), f) != NULL)
    {
    ++lineCount;
    lineLen = strlen(lineBuf);
    if (lineLen < 3)
        continue;
    if (lineLen < 56)
        errAbort("Short line %d of %s\n", lineCount, fileName);
    if (lineLen == sizeof(lineBuf)-1)
        errAbort("Line longer than %d chars line %d of %s, oops.", lineLen, lineCount, fileName);
    AllocVar(el);
    el->hrefTag = eatHrefTag(lineBuf);
    el->skipExon =  (lineBuf[17] != ' ');
    el->skipIntron = (lineBuf[22] != ' ');
    el->nonGenomic = (lineBuf[27] != ' ');
    chopByWhite(lineBuf, words, 1);
    el->orfName = cloneString(words[0]);
    linePt = lineBuf + 29;
    wordCount = chopLine(linePt, words);
    if (wordCount == ArraySize(words))
        errAbort("At least %d words on line %d of %s, oops.", wordCount, lineCount, fileName);
    assert(isdigit(words[0][0]));
    el->alt3 = atoi(words[0]);
    el->alt5 = atoi(words[1]);
    el->ieOverlap = atoi(words[2]);
    el->iiOverlap = atoi(words[3]);
    assert(wordCount >= 5);
    for (i=4; i<wordCount; ++i)
        slAddTail(&el->cdna, newSlName(words[i]));
    slAddHead(&list, el);
    }
slReverse(&list);
fclose(f);
return list;
}

char *boos(boolean boo, char *trues, char *falses)
/* Return string reflecting boolean value. */
{
return boo ? trues : falses;
}

void writeAlts(char *fileName, struct altSpec *altList)
{
FILE *f = mustOpen(fileName, "w");
struct altSpec *alt;
struct slName *cdna;

fprintf(f, "<PRE>\n");
for (alt = altList; alt != NULL; alt = alt->next)
    {
    fprintf(f, "%s%-13s</A>   %s   %s   %s   %5d %5d %5d %5d  ",
        alt->hrefTag, alt->orfName, 
        boos(alt->skipExon, "EX", "  "), boos(alt->skipIntron, "IN", "  "),
        boos(alt->nonGenomic, "NG", "  "), alt->alt3, alt->alt5,
        alt->ieOverlap, alt->iiOverlap);
    for (cdna = alt->cdna; cdna != NULL; cdna = cdna->next)
        fprintf(f, " %s", cdna->name);
    fputc('\n', f);
    }
fclose(f);
}

char *altBase(char *name)
/* Chop off isoform letter at the end of an alt-spliced ORF name. */
{
int len = strlen(name);
if (len > 2 && isalpha(name[len-1]) && isdigit(name[len-2]))
    {
    static char nbuf[64];
    strcpy(nbuf, name);
    nbuf[len-1] = 0;
    return nbuf;
    }
else
    return name;
}

void weedAlts(struct altSpec *garden, struct altSpec *protect,
    struct altSpec **retGood, struct altSpec **retBad)
/* Remove weeds from garden. */
{
struct altSpec *goodList = NULL, *badList = NULL, *spec, *next;
struct hash *weedHash = newHash(14);

for (spec = protect; spec != NULL; spec = spec->next)
    hashAdd(weedHash, altBase(spec->orfName), NULL);

for (spec = garden; spec != NULL; spec = next)
    {
    next = spec->next;
    if (hashLookup(weedHash, altBase(spec->orfName)))
        {
        slAddHead(&goodList, spec);
        }
    else
        {
        slAddHead(&badList, spec);
        }
    }
slReverse(&goodList);
slReverse(&badList);
*retGood = goodList;
*retBad = badList;
}

struct altSpec *filterBorderline(struct altSpec *fullList)
/* Remove some trashy looking cases. */
{
struct altSpec *newList = NULL, *spec, *next;

for (spec = fullList; spec != NULL; spec = next)
    {
    next = spec->next;
    if (spec->skipExon || spec->skipIntron || (spec->alt3 == 0 && spec->alt5 > 0)
        || (spec->alt5 == 0 && spec->alt3 > 0) || spec->iiOverlap > 3 || spec->ieOverlap > 8)
        {
        slAddHead(&newList, spec);
        }
    }
slReverse(&newList);
return newList;
}

int main(int argc, char *argv[])
{
struct altSpec *oldAlts, *newAlts;
struct altSpec *goodOld, *badOld;
struct altSpec *repeatAlts, *reallyNewAlts;

newAlts = readAlts("newalt.html");
oldAlts = readAlts("oldalt.html");
uglyf("Got %d in newAlts, %d in oldAlts\n", slCount(newAlts), slCount(oldAlts));
weedAlts(oldAlts, newAlts, &goodOld, &badOld);
uglyf("Got %d in goodOld %d in badOld after weeding\n", slCount(goodOld), slCount(badOld));
slSort(&oldAlts, cmpAlts);
writeAlts("\\inetpub\\wwwroot\\test\\goodOld.html", goodOld);
writeAlts("\\inetpub\\wwwroot\\test\\badOld.html", badOld);
oldAlts = readAlts("oldalt.html");
weedAlts(newAlts, oldAlts, &repeatAlts, &reallyNewAlts);
uglyf("Got %d 'unfiltered new' alts\n", slCount(reallyNewAlts));
reallyNewAlts = filterBorderline(reallyNewAlts);
slSort(&reallyNewAlts, cmpAlts);
writeAlts("\\inetpub\\wwwroot\\test\\okNew.html", reallyNewAlts);
uglyf("Got %d 'really new' alts\n", slCount(reallyNewAlts));
return 0;
}
