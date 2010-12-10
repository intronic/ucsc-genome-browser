/* fileUi.c - human genome file downloads common controls. */

#include "common.h"
#include "hash.h"
#include "cheapcgi.h"
#include "jsHelper.h"
#include "cart.h"
#include "fileUi.h"
#include "hui.h"
#include "obscure.h"
#include "mdb.h"

// FIXME: Move to hui.h since hui.c also needs this
#define ENCODE_DCC_DOWNLOADS "encodeDCC"

void fileDbFree(struct fileDb **pFileList)
// free one or more fileDb objects
{
while (pFileList && *pFileList)
    {
    struct fileDb *oneFile = slPopHead(pFileList);

    freeMem(oneFile->fileName);
    freeMem(oneFile->fileDate);
    freeMem(oneFile->sortFields);
    freeMem(oneFile->reverse);
    mdbObjsFree(&(oneFile->mdb));
    freeMem(oneFile);
    }
}

struct fileDb *fileDbGet(char *db, char *dir, char *subDir, char *fileName)
// Returns NULL or if found a fileDb struct with name, size and date filled in.
{
static char *savedDb     = NULL;
static char *savedDir    = NULL;
static char *savedSubDir = NULL;
static struct fileDb *foundFiles = NULL;// Use static list to save excess IO
struct fileDb *oneFile = NULL;
if (foundFiles == NULL
||  savedDb  == NULL    || differentString(savedDb, db)
||  savedDir == NULL    || differentString(savedDir,dir)
||  savedSubDir == NULL || differentString(savedSubDir,subDir))
    {
    // free up any static mem
    freeMem(savedDb);
    freeMem(savedDir);
    freeMem(savedSubDir);
    fileDbFree(&foundFiles);

    FILE *scriptOutput = NULL;
    char buf[1024];
    char cmd[512];
    char *words[10];
    char *server = hDownloadsServer();
    if (sameString(server,"hgdownload-test.cse.ucsc.edu")) // genome-test is different
        {
        // Does not work: rsync -avn rsync://hgdownload-test.cse.ucsc.edu/goldenPath/hg19/encodeDCC/wgEncodeBroadHistone
        // Use ls -log --time=ctime --time-style=long-iso /usr/local/apache/htdocs-hgdownload/goldenPath/hg19/encodeDCC/wgEncodeBroadHistone
        safef(cmd,sizeof(cmd),"ls -log --time-style=long-iso /usr/local/apache/htdocs-hgdownload/goldenPath/%s/%s/%s/", db,dir,subDir);
        }
    else  // genome and hgwbeta can use rsync
        {
        // Works:         rsync -avn rsync://hgdownload.cse.ucsc.edu/goldenPath/hg18/encodeDCC/wgEncodeBroadChipSeq
        safef(cmd,sizeof(cmd),"rsync -avn rsync://%s/goldenPath/%s/%s/%s/%s | grep %s", server, db, subDir,dir,fileName,fileName);
        }
    //warn("cmd: %s",cmd);
    scriptOutput = popen(cmd, "r");
    while(fgets(buf, sizeof(buf), scriptOutput))
        {
        eraseTrailingSpaces(buf);
        if (!endsWith(buf,".md5sum")) // Just ignore these
            {
            int count = chopLine(buf, words);
            if (count == 6 && sameString(server,"hgdownload-test.cse.ucsc.edu")) // genome-test is different
                {
                //-rw-rw-r-- 5  502826550 2010-10-22 16:51 /usr/local/apache/htdocs-hgdownload/goldenPath/hg19/encodeDCC/wgEncodeBroadHistone/wgEncodeBroadHistoneGm12878ControlStdRawDataRep1.fastq.gz
                AllocVar(oneFile);
                oneFile->fileSize = atoi(words[2]);
                oneFile->fileDate = cloneString(words[3]);
                char *atSlash = strrchr(words[5], '/');
                if (atSlash != NULL)
                    oneFile->fileName = cloneString(atSlash + 1);
                else
                    oneFile->fileName = cloneString(words[5]);
                slAddHead(&foundFiles,oneFile);
                }
            else if (count == 5 && differentString(server,"hgdownload-test.cse.ucsc.edu"))// genome and hgwbeta can use rsync
                {
                //-rw-rw-r--    26420982 2009/09/29 14:53:30 wgEncodeBroadChipSeq/wgEncodeBroadChipSeqSignalNhlfH4k20me1.wig.gz
                AllocVar(oneFile);
                oneFile->fileSize = atoi(words[1]);
                oneFile->fileDate = cloneString(words[2]);
                strSwapChar(oneFile->fileDate,'/','-');// Standardize YYYY-MM-DD, no time
                oneFile->fileName = cloneString(words[4]);
                slAddHead(&foundFiles,oneFile);
                }
            //warn("File:%s  size:%ld",foundFiles->fileName,foundFiles->fileSize);
            }
        }
    pclose(scriptOutput);
    //warn("found %d files",slCount(foundFiles));

    if (foundFiles == NULL)
        {
        AllocVar(oneFile);
        oneFile->fileName = cloneString("No files found!");
        oneFile->fileDate = cloneString(cmd);
        slAddHead(&foundFiles,oneFile);
        warn("No files found for command:\n%s",cmd);
        return NULL;
        }

    // mark this as done to avoid excessive io
    savedDb     = cloneString(db);
    savedDir    = cloneString(dir);
    savedSubDir = cloneString(subDir);
    }

// special code that only gets called in debug mode
if (sameString(fileName,"listAll"))
    {
    for(oneFile=foundFiles;oneFile;oneFile=oneFile->next)
        warn("%s",oneFile->fileName);
    return NULL;
    }
// Look up the file and return it
struct fileDb *newList = NULL;
while (foundFiles)
    {
    oneFile = slPopHead(&foundFiles);
    if (sameString(fileName,oneFile->fileName))
        break;

    slAddHead(&newList,oneFile);
    oneFile = NULL;
    }
if (newList)
    foundFiles = slCat(newList,foundFiles);

return oneFile;
}

static int fileDbSortCmp(const void *va, const void *vb)
// Compare two sortable tdb items based upon sort columns.
{
const struct fileDb *a = *((struct fileDb **)va);
const struct fileDb *b = *((struct fileDb **)vb);
char **fieldsA = a->sortFields;
char **fieldsB = b->sortFields;
int ix=0;
int compared = 0;
while(fieldsA[ix] != NULL && fieldsB[ix] != NULL && compared == 0)
    {
    compared = strcmp(fieldsA[ix], fieldsB[ix]) * (a->reverse[ix]? -1: 1);
    ix++;
    }
return compared;
}

static void fileDbSortList(struct fileDb **fileList, sortOrder_t *sortOrder)
{// If sortOrder struct provided, will update sortFields in fileList, then sort it
if (sortOrder && fileList)
    {
    struct fileDb *oneFile = NULL;
    for(oneFile = *fileList;oneFile != NULL;oneFile=oneFile->next)
        {
        oneFile->sortFields = needMem(sizeof(char *)    * (sortOrder->count + 1)); // +1 Null terminated
        oneFile->reverse    = needMem(sizeof(boolean *) *  sortOrder->count);
        int ix;
        for(ix=0;ix<sortOrder->count;ix++)
            {
            char *field = mdbObjFindValue(oneFile->mdb,sortOrder->column[ix]);
            if (field)
                {
                oneFile->sortFields[sortOrder->order[ix] - 1] = mdbObjFindValue(oneFile->mdb,sortOrder->column[ix]);
                oneFile->reverse[   sortOrder->order[ix] - 1] = (!sortOrder->forward[ix]);
                }
            else
                {
                oneFile->sortFields[sortOrder->order[ix] - 1] = NULL;
                oneFile->reverse[   sortOrder->order[ix] - 1] = FALSE;
                }
            oneFile->sortFields[sortOrder->count] = NULL;
            }
        }
    slSort(fileList,fileDbSortCmp);
    }
}

void filesDownloadUi(char *db, struct cart *cart, struct trackDb *tdb)
// UI for a "composite like" track: This will list downloadable files associated with
// a single trackDb entry (composite or of type "downloadsOnly". The list of files
// will have links to their download and have metadata information associated.
// The list will be a sortable table and there may be filtering controls.
{
    // The basic idea:
    // 1) tdb of composite or type=downloadsOnly tableless track
    // 2) All mdb Objs associated with "composite=tdb->track" and having fileName
    // 3) Verification of each file in its discovered location
    // 4) Lookup of filters (tdb dimensions?) and filterComposite style controls
    // 5) Presort of files list
    // 6) make table class=sortable
    // 7) Final file count
    // Get preamble from dir ??
    // Use trackDb settings to get at html description, long and short labels
    // Recommend different color background to get the point across that these are files, not tracks

// FIXME: Trick while developing:
if (tdb->table != NULL)
    tdb->track = tdb->table;

boolean debug = cartUsualBoolean(cart,"debug",FALSE);
int ix;

struct sqlConnection *conn = sqlConnect(db);
char *mdbTable = mdbTableName(conn,TRUE); // Look for sandBox name first
if(mdbTable == NULL)
    errAbort("TABLE NOT FOUND: '%s.%s'.\n",db,MDB_DEFAULT_NAME);

struct fileDb *fileList = NULL, *oneFile = NULL;

// Get an mdbObj list of all that belong to this track and have a fileName
char buf[256];
safef(buf,sizeof(buf),"composite=%s fileName=?",tdb->track);
struct mdbByVar *mdbVars = mdbByVarsLineParse(buf);
struct mdbObj *mdbList = mdbObjsQueryByVars(conn,mdbTable,mdbVars);

// Now get Indexes  But be sure not to duplicate entries in the list!!!
safef(buf,sizeof(buf),"composite=%s fileIndex= fileName!=",tdb->track);
mdbVars = mdbByVarsLineParse(buf);
mdbList = slCat(mdbList, mdbObjsQueryByVars(conn,mdbTable,mdbVars));
sqlDisconnect(&conn);

if (slCount(mdbList) == 0)
    {
    warn("No files specified in memtadata for: %s\n%s",tdb->track,tdb->longLabel);
    return;
    }

// Remove common vars from mdbs grant=Bernstein; lab=Broad; dataType=ChipSeq; setType=exp; control=std;
char *commonTerms[] = { "grant", "lab", "dataType", "control", "setType" };
struct dyString *dyCommon = dyStringNew(256);
for(ix=0;ix<ArraySize(commonTerms);ix++)
    {
    char *val = mdbRemoveCommonVar(mdbList, commonTerms[ix]);
    if (val)
        dyStringPrintf(dyCommon,"%s=%s ",commonTerms[ix],val);
    }
if (debug && dyStringLen(dyCommon))
    warn("These terms are common:%s",dyStringContents(dyCommon));
dyStringFree(&dyCommon);

// Verify file existance and make fileList of those found
while(mdbList)
    {
    boolean found = FALSE;
    struct mdbObj *mdbFile = slPopHead(&mdbList);
    // First for FileName
    char *fileName = mdbObjFindValue(mdbFile,"fileName");
    if (fileName != NULL)
        {
        oneFile = fileDbGet(db, ENCODE_DCC_DOWNLOADS, tdb->track, fileName);
        if (oneFile)
            {
            slAddHead(&fileList,oneFile);
            oneFile->mdb = mdbFile;
            found = TRUE;
            }
        else if (debug)
            warn("goldenPath/%s/%s/%s/%s    in mdb but not found in directory",db,ENCODE_DCC_DOWNLOADS, tdb->track,fileName);
        }
    // Now for FileIndexes
    fileName = mdbObjFindValue(mdbFile,"fileIndex");
    if (fileName != NULL)
        {
        // Verify existance first
        oneFile = fileDbGet(db, ENCODE_DCC_DOWNLOADS, tdb->track, fileName);
        if (oneFile)
            {
            slAddHead(&fileList,oneFile);
            if (found) // if already found then need two mdbObjs (assertable but then this is metadata)
                oneFile->mdb = mdbObjClone(mdbFile);  // Do we really need to clone this?
            else
                oneFile->mdb = mdbFile;
            found = TRUE;
            }
        else if (debug)
            warn("goldenPath/%s/%s/%s/%s    in mdb but not found in directory",db,ENCODE_DCC_DOWNLOADS, tdb->track,fileName);
        }
    if (!found)
        mdbObjsFree(&mdbFile);
    }

if (slCount(fileList) == 0)
    {
    warn("No downloadable files currently available for: %s\n%s",tdb->track,tdb->longLabel);
    return;  // No files so nothing to do.
    }
if (debug)
    {
    warn("The following files are in goldenPath/%s/%s/%s/ but NOT in the mdb:",db,ENCODE_DCC_DOWNLOADS, tdb->track);
    fileDbGet(db, ENCODE_DCC_DOWNLOADS, tdb->track, "listAll");
    }

// FilterBoxes ?
    // Dimensions
    // cart contents
    //boolean filterAble = dimensionsExist(tdb);
    //membersForAll_t* membersForAll = membersForAllSubGroupsGet(tdb,cart);

// Now update all files with their sortable fields and sort the list
sortOrder_t *sortOrder = sortOrderGet(cart,tdb);
if (sortOrder != NULL)
    {
    // FIXME: This should be done elsewhere
    // FIXME: We should probably have a fielSortOrder trackDb setting for this!
    // sortOrder->column needs "tag" to be replaced with mdb Var  Should be abole to use:
    // controlledVocabulary encode/cv.ra cellType=cell factor=antibody
    char *vocab = trackDbSetting(tdb, "controlledVocabulary");
    char *words[15];
    int count,cix;
    if((count = chopByWhite(cloneString(vocab), words,15)) > 2)
        {
        for(cix=1;cix<count;cix++)
            {
            if(countChars(words[cix],'=') == 1)  // cellType=cell
                {
                strSwapChar(words[cix],'=',0);
                for(ix=0;ix<sortOrder->count;ix++)
                    {
                    if(sameString(sortOrder->column[ix],words[cix]))  // tags match, but need mdb var
                        sortOrder->column[ix] = cloneString(words[cix]+strlen(words[cix])+1); // skip past tag=
                    }
                }
            }
        }
    // FIXME: More mess until there is a fileSortOrder
    boolean hasRep=FALSE;
    for(ix=0;ix<sortOrder->count;ix++)
        {
        if(sameString(sortOrder->column[ix],"rep"))  // tags match, but need mdb var
            hasRep=TRUE;
        }
    if (!hasRep)
        {
        char **  column  = needMem(sizeof(char *)  * (sortOrder->count + 1));
        char **  title   = needMem(sizeof(char *)  * (sortOrder->count + 1));
        boolean *forward = needMem(sizeof(boolean) * (sortOrder->count + 1));
        int *    order   = needMem(sizeof(int)     * (sortOrder->count + 1));
        memcpy(column, sortOrder->column, (sizeof(char *)  * sortOrder->count));
        memcpy(title,  sortOrder->title,  (sizeof(char *)  * sortOrder->count));
        memcpy(forward,sortOrder->forward,(sizeof(boolean) * sortOrder->count));
        memcpy(order,  sortOrder->order,  (sizeof(int)     * sortOrder->count));
        freeMem(sortOrder->column);
        freeMem(sortOrder->title);
        freeMem(sortOrder->forward);
        freeMem(sortOrder->order);
        sortOrder->column  = column ;
        sortOrder->title   = title  ;
        sortOrder->forward = forward;
        sortOrder->order   = order  ;
        sortOrder->column[ sortOrder->count] = cloneString("replicate");
        sortOrder->title[  sortOrder->count] = cloneString("Rep");
        sortOrder->forward[sortOrder->count] = TRUE;
        sortOrder->order[  sortOrder->count] = ++sortOrder->count;
        }


    // Fill in and sort fileList
    fileDbSortList(&fileList,sortOrder);
    }

// Total (if long enough or if filetBoxes: 12 of 147 files)

// Table class=sortable
    // Columns: Restricted Until | Name? | size | date | dimension columns | ...
jsIncludeFile("hui.js",NULL);
jsIncludeFile("ajax.js",NULL);

printf("<TABLE class='sortable' style='border: 2px outset #006600;'>\n");
printf("<THEAD class='sortable'>\n");
// Column: Restricted Until | sortColumns... | name{...} | size | date
printf("<TR class='sortable'>\n");
printf("<TD valign='center' align='center'>&nbsp;");
int filesCount = slCount(fileList);
if (filesCount > 5)
    printf("<i>%d files</i>",filesCount);    //puts("<FONT class='subCBcount'></font>"); // Use this style when filterboxes are up and running
//if (sortOrder)
//  printf("<INPUT TYPE=HIDDEN NAME='%s' class='sortOrder' VALUE=\"%s\">",sortOrder->htmlId, sortOrder->sortOrder);
printf("</TD>\n");

// Now the columns
int curOrder = 0;
if (sortOrder)
    {
    curOrder = sortOrder->count;
    for(ix=0;ix<sortOrder->count;ix++)
        {
        printf("<TH class='sortable sort%d%s' valign='bottom' nowrap>%s</TH>\n",
            sortOrder->order[ix],(sortOrder->forward[ix]?"":" sortRev"), sortOrder->title[ix]); // keeing track of sortOrder
        }
    }
//#define INCLUDE_FILENAMES
#ifndef INCLUDE_FILENAMES
else
#endif///defn INCLUDE_FILENAMES
    printf("<TH class='sortable sort%d' valign='bottom' nowrap>File Name</TH>\n",                  ++curOrder);
printf("<TH class='sortable sort%d' abbr=use valign='bottom' align='right' nowrap>Size</TH>\n",    ++curOrder);
printf("<TH class='sortable sort%d' valign='bottom' nowrap>Type</TH>\n",                           ++curOrder);
printf("<TH class='sortable sort%d' valign='bottom' nowrap>Submitted</TH>\n",                      ++curOrder);
printf("<TH class='sortable sort%d' valign='bottom' nowrap>RESTRICTED<BR>until</TH>\n",            ++curOrder);
printf("<TH class='sortable sort%d' valign='bottom' align='left' nowrap>Additional Details</TH>\n",++curOrder);
printf("</TR></THEAD>\n");

// Now the files...
printf("<TBODY class='sortable'>\n");
for(oneFile = fileList;oneFile!= NULL;oneFile=oneFile->next)
    {
    char *field = NULL;

    printf("<TR valign='top'>");   // TODO: BUILD IN THE CLASSES TO ALLOW FILTERBOXES TO WORK!!!

    // Download button
    printf("<TD>");
    printf("<A HREF='http://%s/goldenPath/%s/%s/%s/%s' title='Download %s ...' TARGET=ucscDownloads>",
                hDownloadsServer(),db,ENCODE_DCC_DOWNLOADS, tdb->track, oneFile->fileName, oneFile->fileName);
    printf("<input type='button' value='Download'>");
    printf("</a></TD>\n");

    // Each of the pulled out mdb vars
    if (sortOrder)
        {
        for(ix=0;ix<sortOrder->count;ix++)
            {
            field = oneFile->sortFields[sortOrder->order[ix] - 1];
            printf("<TD class='' align='center' nowrap>%s</td>",field?field:" &nbsp;");
            mdbObjRemoveVars(oneFile->mdb,sortOrder->column[ix]); // Remove this from mdb now so that it isn't displayed in "extras'
            }
        }
#ifndef INCLUDE_FILENAMES
    else
#endif///ndef INCLUDE_FILENAMES
        { // fileName
        printf("<TD nowrap>%s",oneFile->fileName);
        //// FIXME: " The "..." encapsulation could be rebuilt so it could be called here
        //printf("&nbsp;<A HREF='#a_meta_%s' onclick='return metadataShowHide(\"%s\",true,true);' title='Show metadata details...'>...</A>",
        //    oneFile->mdb->obj,oneFile->mdb->obj);
        //printf("<DIV id='div_%s_meta' style='display:none;'></div></td>",oneFile->mdb->obj);
        }


    // Size
    char niceNumber[128];
    sprintWithGreekByte(niceNumber, sizeof(niceNumber), oneFile->fileSize);
    printf("<TD abbr='%ld' align='right' nowrap>%s</td>",oneFile->fileSize,niceNumber);

    // Type
    field = cloneString(oneFile->fileName + strlen(oneFile->mdb->obj) + 1);
    if (endsWith(field,".gz"))
        chopSuffix(field);
    printf("<TD nowrap>%s</td>",field?field:" &nbsp;");

    // dateSubmitted
    field = mdbObjFindValue(oneFile->mdb,"dateSubmitted");
    printf("<TD nowrap>%s</td>",field?field:" &nbsp;");

    // Restricted Until
    field = mdbObjFindValue(oneFile->mdb,"dateUnrestricted");
    printf("<TD nowrap align='center'>%s</td>",field?field:" &nbsp;");

    // Extras  grant=Bernstein; lab=Broad; dataType=ChipSeq; setType=exp; control=std;
    mdbObjRemoveVars(oneFile->mdb,"dateUnrestricted dateSubmitted fileName fileIndex composite project"); // Remove this from mdb now so that it isn't displayed in "extras'
    mdbObjReorderVars(oneFile->mdb,"grant lab dataType cell treatment antibody protocol replicate view",FALSE); // Bring to front
    mdbObjReorderVars(oneFile->mdb,"subId submittedDataVersion dateResubmitted dataVersion setType inputType controlId tableName",TRUE); // Send to back
    field = mdbObjVarValPairsAsLine(oneFile->mdb,TRUE);
    printf("<TD nowrap>%s</td>",field?field:" &nbsp;");

    printf("</TR>\n");
    }

printf("</TBODY></TABLE><BR>\n");
printf("<script type='text/javascript'>{$(document).ready(function() {sortTableInitialize($('table.sortable')[0],true,true);});}</script>\n");

// Total
if (filesCount > 5)
    printf("&nbsp;&nbsp;&nbsp;&nbsp;<i>%d files</i>\n",filesCount);    //puts("<FONT class='subCBcount'></font>"); // Use this style when filterboxes are up and running

// Free mem?
}

