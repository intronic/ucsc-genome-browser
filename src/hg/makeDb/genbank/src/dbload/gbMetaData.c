/* Parsing and output of mrna metadata in ra file into seq, mrna and
 * associated unique string tables.  This module functions as a singlton
 * object, with global state that can be reset.  Refer to the
 * doc/database-update-step.html before modifying.
 */

#include "gbMetaData.h"
#include "common.h"
#include "hash.h"
#include "portable.h"
#include "linefile.h"
#include "localmem.h"
#include "dystring.h"
#include "jksql.h"
#include "sqlUpdater.h"
#include "seqTbl.h"
#include "imageCloneTbl.h"
#include "dbLoadOptions.h"
#include "gbDefs.h"
#include "gbVerb.h"
#include "gbIndex.h"
#include "gbEntry.h"
#include "gbRelease.h"
#include "gbProcessed.h"
#include "extFileTbl.h"
#include "gbFileOps.h"
#include "gbStatusTbl.h"
#include "uniqueStrTbl.h"
#include "gbBuildState.h"
#include "sqlDeleter.h"
#include "genbank.h"
#include "gbSql.h"

static char const rcsid[] = "$Id: gbMetaData.c,v 1.15 2003/10/16 20:27:17 markd Exp $";

// FIXME: move mrna, otherse to objects.

static char* mrnaCreate =
/* This keeps track of mRNA. */
"create table mrna ("
  "id int unsigned not null primary key,"         /* Id, same as seq ID. */
  "acc char(12) not null,"                        /* Genbank accession. */
  "version smallint unsigned not null,"           /* Genbank version. */
  "moddate date not null,"                        /* last modified date. */
  "type enum('EST','mRNA') not null,"             /* Full length or EST. */
  "direction enum('5','3','0') not null,"         /* Read direction. */
  "source int unsigned not null,"                 /* Ref in source table. */
  "organism int unsigned not null,"               /* Ref in organism table. */
  "library int unsigned not null,"                /* Ref in library table. */
  "mrnaClone int unsigned not null,"              /* Ref in clone table. */
  "sex int unsigned not null,"                    /* Ref in sex table. */
  "tissue int unsigned not null,"                 /* Ref in tissue table. */
  "development int unsigned not null,"            /* Ref in development table. */
  "cell int unsigned not null,"                   /* Ref in cell table. */
  "cds int unsigned not null,"                    /* Ref in CDS table. */
  "keyword int unsigned not null,"                /* Ref in key table. */
  "description int unsigned not null,"            /* Ref in description table. */
  "geneName int unsigned not null,"               /* Ref in geneName table. */
  "productName int unsigned not null,"            /* Ref in productName table. */
  "author int unsigned not null,"                 /* Ref in author table. */
           /* Extra indices. */
  "unique(acc),"
  "unique(acc, version),"
  "index(type),"
  "index(library),"
  "index(mrnaClone),"
  "index(tissue),"
  "index(development),"
  "index(cell),"
  "index(keyword),"
  "index(description),"
  "index(geneName),"
  "index(productName),"
  "index(author))";

static char* refSeqStatusCreate = 
"CREATE TABLE refSeqStatus ("
"  mrnaAcc varchar(255) not null,"
"  status enum('Unknown', 'Reviewed', "
"    'Validated', 'Provisional', 'Predicted', 'Inferred') not null,"
"  PRIMARY KEY(mrnaAcc))";

static char* refLinkCreate = 
"CREATE TABLE refLink (\n"
"    name varchar(255) not null,        # Name displayed in UI\n"
"    product varchar(255) not null,     # Name of protein product\n"
"    mrnaAcc varchar(255) not null,     # mRNA accession\n"
"    protAcc varchar(255) not null,     # protein accession\n"
"    geneName int unsigned not null,    # pointer to geneName table\n"
"    prodName int unsigned not null,    # pointer to product name table\n"
"    locusLinkId int unsigned not null, # Locus Link ID\n"
"    omimId int unsigned not null,      # Locus Link ID\n"
"              #Indices\n"
"    PRIMARY KEY(mrnaAcc),\n"
"    index(name(10)),\n"
"    index(protAcc(10)),\n"
"    index(locusLinkId),\n"
"    index(omimId),\n"
"    index(prodName),\n"
"    index(geneName)\n"
")";

/*
 * Summary table is sparse, only created for refSeqs that have Summary:
 * or COMPLETENESS: in comment.
 * Mapping of COMPLETENESS:
 *    Complete5End       = complete on the 5' end.
 *    Complete3End       = complete on the 3' end.
 *    FullLength         = full length.
 *    IncompleteBothEnds = incomplete on both ends.
 *    Incomplete5End     = incomplete on the 5' end.
 *    Incomplete3End     = incomplete on the 3' end.
 *    Partial            = not full length.
 *    Unknown            = unknown
 */
static char* refSeqSummaryCreate = 
"CREATE TABLE refSeqSummary ("
"  mrnaAcc varchar(255) not null,"
"  completeness enum('Unknown', 'Complete5End', "
"    'Complete3End', 'FullLength', 'IncompleteBothEnds',"
"    'Incomplete5End', 'Incomplete3End', 'Partial') not null,"
"  summary text not null,"
"  PRIMARY KEY(mrnaAcc))";


/* list of the names of the id to string tables */
static char *raFieldTables[] =
    {
    "development", "cell", "cds", "geneName", "productName",
    "source", "organism", "library", "mrnaClone", "sex", "tissue",
    "author", "keyword", "description", NULL
    };

/* global configuration */
static struct dbLoadOptions* gOptions; /* options from cmdline and conf */
static char gTmpDir[PATH_LEN];      /* tmp dir for load file */
static unsigned gSrcDb = 0;         /* source database */
static char gGbdbGenBank[PATH_LEN];  /* root dir to store in database */

/* FIXME: maybe should drop gSrcDb ??? */

/* Update objects for each table */
static struct seqTbl* seqTbl = NULL;
static struct imageCloneTbl* imageCloneTbl = NULL;
static struct sqlUpdater* allUpdaters = NULL; /* list of tab files */
static struct sqlUpdater* mrnaUpd = NULL;
static struct sqlUpdater* refSeqStatusUpd = NULL;
static struct sqlUpdater* refSeqSummaryUpd = NULL;
static struct sqlUpdater* refLinkUpd = NULL;

/* other state objects */
static struct extFileTbl* extFiles = NULL;


/* Globals containing ra field info for current record that are not stored
 * in unique string tables.  */
static char raAcc[GB_ACC_BUFSZ];
static char raDir;  /* direction */
static unsigned raDnaSize;
static off_t raFaOff;
static unsigned raFaSize;
static time_t raModDate;
static short raVersion;
static char *raRefSeqStatus;
static char *raRefSeqCompleteness;
static struct dyString* raRefSeqSummary = NULL;
static unsigned raLocusLinkId;
static unsigned raOmimId;
static char raCds[8192];  /* Big due to join() specification.  FIXME: make dstring */
static char raProtAcc[GB_ACC_BUFSZ];
static short raProtVersion;
static unsigned raProtSize;
static off_t raProtFaOff;
static unsigned raProtFaSize;
static struct dyString* raLocusTag = NULL;

struct raField
/* Entry for a ra field.  New values are buffered until we decide that
 * the entry is going to be kept, then they are added to a unique string
 * table. */
{
    struct raField *next;
    char *raName;                 /* field name */
    HGID curId;                   /* current id  */
    char *curVal;                 /* current val, or null if not set. */
    struct dyString* valBuf;      /* buffer for values */
    struct uniqueStrTbl *ust;     /* table of strings to ids */
};
static struct raField *gRaFieldTableList = NULL;
static struct hash *gRaFields = NULL;

static void raFieldDefine(struct sqlConnection *conn, char *table,
                           char *raName, int hashPow2Size)
/* Define a ra field, setting the unique table associated with it */
{
struct hashEl *hel;
struct raField *raf;
AllocVar(raf);

if (hashPow2Size == 0)
    hashPow2Size = 14; /* 16kb */

hel = hashAdd(gRaFields, raName, raf);
raf->raName = hel->name;
raf->valBuf = dyStringNew(0);
raf->ust = uniqueStrTblNew(conn, table, hashPow2Size,
                           ((gOptions->flags & DBLOAD_GO_FASTER) != 0),
                           gTmpDir, (verbose >= 2));
raf->next = gRaFieldTableList;
gRaFieldTableList = raf;
}

static void raFieldFree(struct raField *raf)
/* Free data associated with a ra field */
{
dyStringFree(&raf->valBuf);
uniqueStrTblFree(&raf->ust);
freeMem(raf);
}

static void raFieldsInit(struct sqlConnection *conn)
/* initialize global table of ra fields */
{
assert(gRaFieldTableList == NULL);

gRaFields = hashNew(8);
/* default is 16kb */
raFieldDefine(conn, "source", "src", 0);
raFieldDefine(conn, "organism", "org", 0);
raFieldDefine(conn, "library", "lib", 0);
raFieldDefine(conn, "mrnaClone", "clo", 21);    /* 2mb */
raFieldDefine(conn, "sex", "sex", 9);           /* 512k */
raFieldDefine(conn, "tissue", "tis", 0);
raFieldDefine(conn, "development", "dev", 0);
raFieldDefine(conn, "cell", "cel", 14);
raFieldDefine(conn, "cds", "cds", 0);
raFieldDefine(conn, "geneName", "gen", 0);
raFieldDefine(conn, "productName", "pro", 19);   /* 256k */
raFieldDefine(conn, "author", "aut", 0);
raFieldDefine(conn, "keyword", "key", 0);
raFieldDefine(conn, "description", "def", 19);   /* 512k */
}

static void raFieldsFree()
/* Free ra field data */
{
struct raField* raf;
while ((raf = gRaFieldTableList) != NULL)
    {
    gRaFieldTableList = gRaFieldTableList->next;
    raFieldFree(raf);
    }
hashFree(&gRaFields);
}

static void raFieldClearLastIds()
/* Clear lastId from all ra field tables, so that undefined fields
 * in the next record parsed get a zero id */
{
struct raField *raf;
for (raf = gRaFieldTableList; raf != NULL; raf = raf->next)
    {
    raf->curId = 0;
    raf->curVal = NULL;
    dyStringClear(raf->valBuf);
    }
}

static void raFieldSet(char *raField, char *val)
/* Set a RA field value for later use.  This does not load the unique string
 * table. A value of NULL clears the value */
{
/* ignore if not tracking field */
struct raField *raf = hashFindVal(gRaFields, raField);
if (raf != NULL)
    {
    /* never append to an existing value, replace or clear */
    dyStringClear(raf->valBuf);
    raf->curId = 0;
    raf->curVal = NULL;
    if (val != NULL)
        {
        dyStringAppend(raf->valBuf, val);
        raf->curVal = raf->valBuf->string;
        }
    }
}

static void raFieldStore(struct raField *raf, struct sqlConnection *conn)
/* store a RA field value in the unique string table */
{
assert(raf->curId == 0);
raf->curId = uniqueStrTblGet(raf->ust, conn, raf->curVal, NULL);
}

static HGID raFieldCurId(char *raField, struct sqlConnection *conn)
/* get the last id that was stored for a raField */
{
struct raField *raf = hashFindVal(gRaFields, raField);
if (raf == NULL)
    return 0;  /* not tracking field */
if ((raf->curId == 0) && (raf->curVal != NULL))
    raFieldStore(raf, conn);
return raf->curId;
}

static char* raFieldCurVal(char *raField)
/* get the last string that was stored for a raField */
{
struct raField *raf = hashFindVal(gRaFields, raField);
if (raf == NULL)
    return NULL;  /* not tracking field */
else
    return raf->curVal;
}

static void gbWarn(char *format, ...)
/* issue a warning */
{
va_list args;
va_start(args, format);
fflush(stdout);
fprintf(stderr, "Warning: ");
vfprintf(stderr, format, args);
fprintf(stderr, "\n");
va_end(args);
}

static char *emptyForNull(char *s)
/* Return "" if s is empty, else return s. */
{
return ((s == NULL) ? "" : s);
}

void gbMetaDataInit(struct sqlConnection *conn, unsigned srcDb,
                    struct dbLoadOptions* options, char *gbdbGenBank,
                    char *tmpDir)
/* initialize for parsing metadata */
{
gOptions = options;
gSrcDb = srcDb;
gGbdbGenBank[0] = '\0';
if (gbdbGenBank != NULL)
    strcpy(gGbdbGenBank, gbdbGenBank);
strcpy(gTmpDir, tmpDir);

/* field table is cached in goFaster mode */
if (gRaFieldTableList == NULL)
    raFieldsInit(conn);
if (seqTbl == NULL)
    seqTbl = seqTblNew(conn, gTmpDir, (verbose >= 2));
if (imageCloneTbl == NULL)
    imageCloneTbl = imageCloneTblNew(conn, gTmpDir, (verbose >= 2));

if (!sqlTableExists(conn, "mrna"))
    sqlUpdate(conn, mrnaCreate);
if (mrnaUpd == NULL)
    mrnaUpd = sqlUpdaterNew("mrna", gTmpDir, (verbose >= 2), &allUpdaters);

if (gSrcDb == GB_REFSEQ)
    {
    if (!sqlTableExists(conn, "refSeqStatus"))
        sqlUpdate(conn, refSeqStatusCreate);
    if (refSeqStatusUpd == NULL)
        refSeqStatusUpd = sqlUpdaterNew("refSeqStatus", gTmpDir, (verbose >= 2),
                                        &allUpdaters);
    if (!sqlTableExists(conn, "refSeqSummary"))
        sqlUpdate(conn, refSeqSummaryCreate);
    if (refSeqSummaryUpd == NULL)
        refSeqSummaryUpd = sqlUpdaterNew("refSeqSummary", gTmpDir, (verbose >= 2),
                                         &allUpdaters);
    if (!sqlTableExists(conn, "refLink"))
        sqlUpdate(conn, refLinkCreate);
    if (refLinkUpd == NULL)
        refLinkUpd = sqlUpdaterNew("refLink", gTmpDir, (verbose >= 2),
                                   &allUpdaters);
    }
}

static HGID getExtFileId(struct sqlConnection *conn, char* relPath)
/* get the extFile id for a file, prepending the gbdb root dir */
{
char path[PATH_LEN];
path[0] = '\0';
if (gGbdbGenBank[0] != '\0')
    {
    strcpy(path, gGbdbGenBank);
    strcat(path, "/");
    }
strcat(path, relPath);

if (extFiles == NULL)
    {
    gbVerbMsg(1, "loading extFile table");
    extFiles = extFileTblLoad(conn);
    }
return extFileTblGet(extFiles, conn, path);
}

static char *parseRefSeqStatus(char *rss)
/* parse the refseq status field */
{
if ((rss == NULL) || sameString(rss, "unk"))
    return "Unknown";
else if (sameString(rss, "rev"))
    return "Reviewed";
else if (sameString(rss, "val"))
    return "Validated";
else if (sameString(rss, "pro"))
    return "Provisional";
else if (sameString(rss, "pre"))
    return "Predicted";
else if (sameString(rss, "inf"))
    return "Inferred";
else
    errAbort("invalid value for ra rss field \"%s\"", rss);
return NULL; /* don't make it here */
}

static char *parseRefSeqCompletness(char *rsc)
/* parse the refseq completeness field */
{
if ((rsc == NULL) || sameString(rsc, "unk"))
    return "Unknown";
else if (sameString(rsc, "cmpl5"))
    return "Complete5End";
else if (sameString(rsc, "cmpl3"))
    return "Complete3End";
else if (sameString(rsc, "full"))
    return "FullLength";
else if (sameString(rsc, "incmpl"))
    return "IncompleteBothEnds";
else if (sameString(rsc, "incmpl5"))
    return "Incomplete5End";
else if (sameString(rsc, "incmpl3"))
    return "Incomplete3End";
else if (sameString(rsc, "part"))
    return "Partial";
else
    errAbort("invalid value for ra rsc field \"%s\"", rsc);
return NULL; /* don't make it here */
}

static char* parseEntry(struct sqlConnection *conn,
                        unsigned type, struct lineFile *raLf)
/* Parse the next record from a ra file into current metadata state.
 * Returns accession or NULL on EOF. */
{
int lineCnt = 0;
char *tag, *val;

/* reset globals for new record */
raFieldClearLastIds();
raAcc[0] = '\0';
raDir = '0';
raDnaSize = 0;
raFaOff = NULL_OFFSET;
raFaSize = 0;
raModDate = NULL_DATE;
raVersion = NULL_VERSION;
raRefSeqStatus = NULL;
raRefSeqCompleteness = NULL;
if (raRefSeqSummary == NULL)
    raRefSeqSummary = dyStringNew(1024);
dyStringClear(raRefSeqSummary);
raLocusLinkId = 0;
raOmimId = 0;
raCds[0] = '\0';
raProtAcc[0] = '\0';
raProtVersion = -1;
raProtSize = 0;
raProtFaOff = -1;
raProtFaSize = 0;
if (raLocusTag == NULL)
    raLocusTag = dyStringNew(128);
dyStringClear(raLocusTag);

for (;;)
    {
    if (!lineFileNext(raLf, &tag, NULL))
        {
        if (lineCnt > 0)
            errAbort("Unexpected eof in %s", raLf->fileName);
        return NULL;
        }
    if (tag[0] == 0)
        break;
    val = strchr(tag, ' ');
    if (val == NULL)
        errAbort("Badly formatted tag %s:%d", raLf->fileName, raLf->lineIx);
    *val++ = 0;
    if (sameString(tag, "acc"))
        {
        char *s = firstWordInLine(val);
        strncpy(raAcc, s, GB_ACC_BUFSZ);
        }
    else if (sameString(tag, "dir"))
        raDir = val[0];
    else if (sameString(tag, "dat"))
        raModDate = gbParseDate(raLf, val);
    else if (sameString(tag, "ver"))
        raVersion = gbParseInt(raLf, firstWordInLine(val));
    else if (sameString(tag, "siz"))
        raDnaSize = gbParseUnsigned(raLf, val);
    else if (sameString(tag, "fao"))
        raFaOff = gbParseFileOff(raLf, val);
    else if (sameString(tag, "fas"))
        raFaSize = gbParseUnsigned(raLf, val);
    else if (sameString(tag, "prt"))
        {
        /* version is optional, remove it if it exists  */
        if (strchr(val, '.') != NULL)
            raProtVersion = gbSplitAccVer(val, raProtAcc);
        else
            safef(raProtAcc, sizeof(raProtAcc), "%s", val);
        }
    else if (sameString(tag, "prs"))
        raProtSize = gbParseUnsigned(raLf, val);
    else if (sameString(tag, "pfo"))
        raProtFaOff = gbParseFileOff(raLf, val);
    else if (sameString(tag, "pfs"))
        raProtFaSize = gbParseUnsigned(raLf, val);
    else if (sameString(tag, "rss"))
        raRefSeqStatus = parseRefSeqStatus(val);
    else if (sameString(tag, "rsc"))
        raRefSeqCompleteness = parseRefSeqCompletness(val);
    else if (sameString(tag, "rsu"))
        dyStringAppend(raRefSeqSummary, val);
    else if (sameString(tag, "loc"))
        raLocusLinkId = gbParseUnsigned(raLf, val);
    else if (sameString(tag, "lot"))
        dyStringAppend(raLocusTag, val);
    else if (sameString(tag, "mim"))
        {
        /* might have multiple values, just use first */
        raOmimId = gbParseUnsigned(raLf, firstWordInLine(val));
        }
    else
        {
        /* save under hashed name */
        if (sameString(tag, "cds"))
            safef(raCds, sizeof(raCds), "%s", val);
        raFieldSet(tag, val);
        }
    }

/* If we didn't get the gene name, substitute the locus tag if available.
 * This is needed by Drosophila, others */
if ((raFieldCurId("gen", conn) == 0) && (raLocusTag->stringSize > 0))
    raFieldSet("gen", raLocusTag->string);

/* do a little error checking. */
if (strlen(raAcc) == 0)
    errAbort("No accession in %s\n", raLf->fileName);
if (raModDate == NULL_DATE)
    errAbort("No date for %s in %s\n", raAcc, raLf->fileName);
if (raVersion == NULL_VERSION)
    errAbort("No version for %s in %s\n", raAcc, raLf->fileName);
if (raDnaSize == 0)
    errAbort("No size for %s in %s\n", raAcc, raLf->fileName);
if (raFaOff == NULL_OFFSET)
    errAbort("No fasta offset for %s in %s\n", raAcc, raLf->fileName);
if (raFaSize == 0)
    errAbort("No fasta size for %s in %s\n", raAcc, raLf->fileName);

return raAcc;
}

static void seqUpdate(struct gbStatus* status, HGID faFileId)
/* Update the seq table for the current entry */
{
assert((status->stateChg & GB_NEW) || (status->gbSeqId != 0));
if (status->stateChg & GB_NEW)
    {
    char* seqType = (status->type == GB_MRNA) ?  SEQ_MRNA : SEQ_EST;
    char* seqSrcDb = (gSrcDb == GB_GENBANK) ? SEQ_GENBANK : SEQ_REFSEQ;
    assert(status->gbSeqId == 0);

    status->gbSeqId = seqTblAdd(seqTbl, raAcc, raVersion, seqType, seqSrcDb,
                                faFileId, raDnaSize, raFaOff, raFaSize);
    }
else if (status->stateChg & (GB_SEQ_CHG|GB_EXT_CHG))
    {
    /* Update only the columns that need to change */
    HGID fileId = (status->stateChg & (GB_SEQ_CHG|GB_EXT_CHG))
        ? faFileId : 0;
    short version =  (status->stateChg & GB_SEQ_CHG)
        ? raVersion : -1;
    seqTblMod(seqTbl, status->gbSeqId, version, fileId, raDnaSize, raFaOff,
              raFaSize);
    }
}

static void mrnaUpdate(struct gbStatus* status, struct sqlConnection *conn)
/* Update the mrna table for the current entry */
{
if (status->stateChg & GB_NEW)
    {
    sqlUpdaterAddRow(mrnaUpd, "%u\t%s\t%u\t%s\t%s\t%c\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u",
                     status->gbSeqId, raAcc, raVersion, gbFormatDate(raModDate),
                     ((status->type == GB_MRNA) ? "mRNA" : "EST"), raDir,
                     raFieldCurId("src", conn), raFieldCurId("org", conn),
                     raFieldCurId("lib", conn), raFieldCurId("clo", conn),
                     raFieldCurId("sex", conn), raFieldCurId("tis", conn),
                     raFieldCurId("dev", conn), raFieldCurId("cel", conn),
                     raFieldCurId("cds", conn), raFieldCurId("key", conn),
                     raFieldCurId("def", conn), raFieldCurId("gen", conn),
                     raFieldCurId("pro", conn), raFieldCurId("aut", conn));
    }
else if (status->stateChg & GB_META_CHG)
    {
    sqlUpdaterModRow(mrnaUpd, 1, "version='%u', moddate='%s', direction='%c', "
                     "source=%u, organism=%u, library=%u, mrnaClone=%u, sex=%u, "
                     "tissue=%u, development=%u, cell=%u, cds=%u, keyword=%u, "
                     "description=%u, geneName=%u, productName=%u, author=%u "
                     "WHERE id=%u",
                     raVersion, gbFormatDate(raModDate), raDir,
                     raFieldCurId("src", conn), raFieldCurId("org", conn),
                     raFieldCurId("lib", conn), raFieldCurId("clo", conn),
                     raFieldCurId("sex", conn), raFieldCurId("tis", conn),
                     raFieldCurId("dev", conn), raFieldCurId("cel", conn),
                     raFieldCurId("cds", conn), raFieldCurId("key", conn),
                     raFieldCurId("def", conn), raFieldCurId("gen", conn),
                     raFieldCurId("pro", conn), raFieldCurId("aut", conn),
                     status->gbSeqId);
    }
}

static void imageCloneUpdate(struct gbStatus* status, struct sqlConnection *conn)
/* update image clone table */
{
/* assumes image id is never removed; most like true, would only
 * be changed on a mistake */
if (status->stateChg & (GB_NEW|GB_META_CHG))
    {
    unsigned imageId = imageCloneGBParse(raFieldCurVal("clo"));
    if (imageId != 0)
        {
        if (status->stateChg & GB_NEW)
            {
            imageCloneTblAdd(imageCloneTbl, imageId, status->acc, status->type,
                             raDir);
            }
        else if (status->stateChg & GB_META_CHG)
            {
            unsigned oldImageId = imageCloneTblGetId(conn, status->acc);
            if (oldImageId == 0)
                imageCloneTblAdd(imageCloneTbl, imageId, status->acc, status->type,
                                 raDir);
            else if (imageId != oldImageId)
                imageCloneTblMod(imageCloneTbl, imageId, status->acc, raDir);
            }
        }
    }
}

static void refSeqStatusUpdate(struct gbStatus* status)
/* Update the refSeqStatus table for the current entry */
{
if (status->stateChg & GB_NEW)
    sqlUpdaterAddRow(refSeqStatusUpd, "%s\t%s", raAcc, raRefSeqStatus);
else if (status->stateChg & GB_META_CHG)
    sqlUpdaterModRow(refSeqStatusUpd, 1, "status='%s' WHERE mrnaAcc='%s'",
                     raRefSeqStatus, raAcc);
}

static void refSeqSummaryUpdate(struct sqlConnection *conn, struct gbStatus* status)
/* Update the refSeqSummary table for the current entry */
{
/* only add to table if we actually have something */
if (((raRefSeqSummary->stringSize > 0) || (raRefSeqCompleteness != NULL))
    && (status->stateChg & (GB_NEW|GB_META_CHG)))
    {
    char query[256];
    unsigned sumNew;
    char* summary = sqlEscapeString2(alloca(2*raRefSeqSummary->stringSize+1), 
                                     raRefSeqSummary->string);
    if (raRefSeqCompleteness == NULL)
        raRefSeqCompleteness = "Unknown";
    
    /* If sequence may not be new, but summary could be.  This table is
     * sparse, so must check it */
    if (status->stateChg & GB_NEW)
        sumNew = TRUE;
    else
        {
        safef(query, sizeof(query), "SELECT count(*) from refSeqSummary "
              "WHERE mrnaAcc = '%s'", raAcc);
        sumNew = (sqlQuickNum(conn, query) == 0);
        }

    if (sumNew)
        sqlUpdaterAddRow(refSeqSummaryUpd, "%s\t%s\t%s", raAcc, raRefSeqCompleteness,
                         summary);
    else
        sqlUpdaterModRow(refSeqSummaryUpd, 1, "completeness='%s', summary='%s'"
                         " WHERE mrnaAcc='%s'", raRefSeqCompleteness,
                         summary, raAcc);
    }
}

static void refLinkUpdate(struct sqlConnection *conn, struct gbStatus* status)
/* Update the refLink table for the current entry */
{
char *gen = emptyForNull(raFieldCurVal("gen"));
char *pro = emptyForNull(raFieldCurVal("pro"));

gen = sqlEscapeString2(alloca(2*strlen(gen)+1), gen);
pro = sqlEscapeString2(alloca(2*strlen(pro)+1), pro);

if (status->stateChg & GB_NEW)
    sqlUpdaterAddRow(refLinkUpd, "%s\t%s\t%s\t%s\t%u\t%u\t%u\t%u",
                     gen, pro, raAcc, raProtAcc, raFieldCurId("gen", conn),
                     raFieldCurId("pro", conn), raLocusLinkId, raOmimId);
else if (status->stateChg & GB_META_CHG)
    sqlUpdaterModRow(refLinkUpd, 1, "name='%s', product='%s', protAcc='%s', "
                     "geneName=%u, prodName=%u, locusLinkId=%u, "
                     "omimId=%u where mrnaAcc='%s'",
                     gen, pro, raProtAcc, raFieldCurId("gen", conn),
                     raFieldCurId("pro", conn), raLocusLinkId, raOmimId, raAcc);
}

static void refSeqPepUpdate(struct sqlConnection *conn, HGID pepFaId)
/* Make or update a seq table entry for a RefSeq peptide sequence
 * referenced a refLink table. */
{
/* done for all stateChg values */
if (raProtFaOff >= 0)
    {
    HGID gbSeqId = seqTblGetId(seqTbl, conn, raProtAcc);
    if (gbSeqId == 0)
        seqTblAdd(seqTbl, raProtAcc, raProtVersion, SEQ_PEP, SEQ_REFSEQ,
                  pepFaId, raProtSize, raProtFaOff, raProtFaSize);
    else
        seqTblMod(seqTbl, gbSeqId, raProtVersion, pepFaId, raProtSize,
                  raProtFaOff, raProtFaSize);
    }
}

static boolean keepDesc(struct gbStatus* status)
/* Check if description should be stored. */
{
return dbLoadOptionsGetAttr(gOptions, status->srcDb, status->type, status->orgCat)->loadDesc;
}

static void updateMetaData(struct sqlConnection *conn, struct gbStatus* status,
                           struct gbStatusTbl* statusTbl, HGID faFileId,
                           HGID pepFaId)
/* update the database tables for the current entry based on the stateChg
 * flags */
{
char *geneName;

/* clear description if we are not keeping it */
if (!keepDesc(status))
    raFieldSet("def", NULL);

seqUpdate(status, faFileId);  /* must be first to get status->gbSeqId */
mrnaUpdate(status, conn);
imageCloneUpdate(status, conn);

if (gSrcDb == GB_REFSEQ)
    {
    refSeqStatusUpdate(status);
    refSeqSummaryUpdate(conn, status);
    refLinkUpdate(conn, status);
    refSeqPepUpdate(conn, pepFaId);
    }

/* update in-memory gbStatus entry  */
status->modDate = raModDate;

/* save CDS for use by the alignments */
if (!genbankParseCds(raCds, &status->cdsStart, &status->cdsEnd))
    {
    /* not valid CDS, only warn if RefSeq, where we expect to be better */
    if (gSrcDb == GB_REFSEQ)
        gbWarn("%s: malformed RefSeq CDS: %s", status->acc, raCds);
    }

/* geneName for refFlat, if not available, try locus_tag  */
geneName = raFieldCurVal("gen");
if (geneName == NULL)
    geneName = raFieldCurVal("lot");
if (geneName != NULL)
    status->geneName = lmCloneString(statusTbl->accHash->lm, geneName);

/* mark as done so dups in other updates don't get added to the database */
status->metaDone = TRUE;
}

static boolean processEntry(struct sqlConnection *conn,
                            struct gbStatusTbl* statusTbl,
                            unsigned type, struct lineFile *raLf,
                            HGID faFileId, HGID pepFaId)
/* Parse and process the next ra entry, check to see if it is selected
 * for update. */
{
char* acc;
struct gbStatus* status;

if ((acc = parseEntry(conn, type, raLf)) == NULL)
    return FALSE;

status = gbStatusTblFind(statusTbl, acc);
if ((status != NULL) && (status->selectProc != NULL) && (!status->metaDone)
    && (status->selectProc->modDate == raModDate))
    updateMetaData(conn, status, statusTbl, faFileId, pepFaId);
return TRUE;
}

static void metaDataProcess(struct sqlConnection *conn,
                            struct gbStatusTbl* statusTbl,
                            char* raPath, unsigned type,
                            HGID faFileId, HGID pepFaId)
/* Parse a ra file looking for accessions to add to the database.  If the
 * ra entry matches the status->selectProc object, it will be saved for
 * loading. */
{
struct lineFile *raLf;

/* parse and save all ra records, in assending file offset order */
raLf = gzLineFileOpen(raPath);
while (processEntry(conn, statusTbl, type, raLf, faFileId, pepFaId))
    continue; /* loop filler */
gzLineFileClose(&raLf);
}

static HGID getPeptideFaId(struct sqlConnection *conn, struct gbSelect* select)
/* Get the extFile id for a refseq peptide fasta file */
{
HGID pepFaId = 0;
char pepFaPath[PATH_LEN];

gbProcessedGetDir(select, pepFaPath);
strcat(pepFaPath, "/pep");
if (select->accPrefix != NULL)
    {
    strcat(pepFaPath, ".");
    strcat(pepFaPath, select->accPrefix);
    }
strcat(pepFaPath, ".fa");
if (fileExists(pepFaPath))
    pepFaId = getExtFileId(conn, pepFaPath);
return pepFaId;
}

void gbMetaDataProcess(struct sqlConnection *conn,
                       struct gbStatusTbl* statusTbl,
                       struct gbSelect* select)
/* process metadata for an update */
{
HGID faFileId, pepFaId = 0;
char raPath[PATH_LEN], faPath[PATH_LEN];

/* need fasta file id */
gbProcessedGetPath(select, "fa", faPath);
faFileId = getExtFileId(conn, faPath);

if (select->release->srcDb == GB_REFSEQ)
    pepFaId = getPeptideFaId(conn, select);

/* parse and save for loading metadata from this update */
gbProcessedGetPath(select, "ra.gz", raPath);
metaDataProcess(conn, statusTbl, raPath, select->type, faFileId, pepFaId);
}

void gbMetaDataDbLoad(struct sqlConnection *conn)
/* load the metadata changes into the database */
{
struct raField *nextRa;
struct sqlUpdater *nextUpd;

/* this should never have been called if these tables not set up */
assert(imageCloneTbl != NULL);
assert(seqTbl != NULL);

/* seq must be first */
seqTblCommit(seqTbl, conn);
seqTblFree(&seqTbl);

/* unique string tables next, before mrna */
for (nextRa = gRaFieldTableList; nextRa != NULL; nextRa = nextRa->next)
    uniqueStrTblCommit(nextRa->ust, conn);

/* image ids are loaded next */
imageCloneTblCommit(imageCloneTbl, conn);

/* other metadata */
while ((nextUpd = slPopHead(&allUpdaters)) != NULL)
    {
    sqlUpdaterCommit(nextUpd, conn);
    sqlUpdaterFree(&nextUpd);
    }
mrnaUpd = NULL;
refSeqStatusUpd = NULL;
refLinkUpd = NULL;
/* cache unique string tables in goFaster mode */
if ((gOptions->flags & DBLOAD_GO_FASTER) == 0)
    raFieldsFree();
}

void gbMetaDataFree()
/* Free data structures */
{
raFieldsFree();
extFileTblFree(&extFiles);
}

void gbMetaDataDeleteFromIdTables(struct sqlConnection *conn,
                                  struct sqlDeleter* deleter)
/* delete sequence from metadata tables with ids.  These are always
 * deleted and rebuilt even for modification */
{
sqlDeleterDel(deleter, conn, IMAGE_CLONE_TBL, "acc");
}

void gbMetaDataDeleteFromTables(struct sqlConnection *conn, unsigned srcDb,
                                struct sqlDeleter* deleter)
/* delete sequence from metadata tables */
{
if (srcDb == GB_REFSEQ)
    {
    sqlDeleterDel(deleter, conn, "refSeqStatus", "mrnaAcc");
    sqlDeleterDel(deleter, conn, "refLink", "mrnaAcc");
    }
sqlDeleterDel(deleter, conn, IMAGE_CLONE_TBL, "acc");
sqlDeleterDel(deleter, conn, "mrna", "acc");
/* seq must be last */
sqlDeleterDel(deleter, conn, SEQ_TBL, "acc");
}

static void refSeqPepClean(struct sqlConnection *conn)
/* Delete all refseq peptides that are in gbSeq but no longer
 * referenced by refLink.  */
{
char query[1024];
struct sqlResult* sr;
char **row;
struct sqlDeleter* deleter = sqlDeleterNew(gTmpDir, (verbose >= 2));

/* Use a join to get list of acc, which proved reasonable fastly because
* the the list is small */
safef(query, sizeof(query), "SELECT acc FROM gbSeq LEFT JOIN refLink ON (refLink.protAcc = gbSeq.acc) "
      "WHERE (acc LIKE 'NP_%%') AND (refLink.protAcc IS NULL)");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    sqlDeleterAddAcc(deleter, row[0]);
sqlFreeResult(&sr);

sqlDeleterDel(deleter, conn, SEQ_TBL, "acc");
sqlDeleterFree(&deleter);
}

void gbMetaDataDeleteOutdated(struct sqlConnection *conn,
                              struct gbSelect* select,
                              struct gbStatusTbl* statusTbl,
                              struct dbLoadOptions* options,
                              char *tmpDir)
/* delete outdated metadata */
{
struct sqlDeleter* deleter = sqlDeleterNew(tmpDir, (verbose >= 2));
struct gbStatus* status;
gSrcDb = select->release->srcDb;
gOptions = options;
strcpy(gTmpDir, tmpDir);

/* Delete any meta modified from id tables */
deleter = sqlDeleterNew(tmpDir, (verbose >= 2));
for (status = statusTbl->seqChgList; status != NULL; status = status->next)
    {
    if (status->stateChg & GB_META_CHG)
        sqlDeleterAddAcc(deleter, status->acc);
    }
for (status = statusTbl->metaChgList; status != NULL; status = status->next)
    sqlDeleterAddAcc(deleter, status->acc);
for (status = statusTbl->deleteList; status != NULL; status = status->next)
    sqlDeleterAddAcc(deleter, status->acc);
for (status = statusTbl->orphanList; status != NULL; status = status->next)
    sqlDeleterAddAcc(deleter, status->acc);
gbMetaDataDeleteFromIdTables(conn, deleter);
sqlDeleterFree(&deleter);

/* remove deleted and orphans from metadata. */
deleter = sqlDeleterNew(tmpDir, (verbose >= 2));
for (status = statusTbl->deleteList; status != NULL; status = status->next)
    sqlDeleterAddAcc(deleter, status->acc);
for (status = statusTbl->orphanList; status != NULL; status = status->next)
    sqlDeleterAddAcc(deleter, status->acc);

gbMetaDataDeleteFromTables(conn, select->release->srcDb, deleter);

sqlDeleterFree(&deleter);

/* if we are cleaning out the ext table, we need to get rid of any
 * refseq peptides in gbSeq that are no longer referenced.  We don't
 * do it other times as these are not reachable directly.
 */
if ((select->release->srcDb == GB_REFSEQ)
    && (gOptions->flags & DBLOAD_EXT_FILE_UPDATE))
    refSeqPepClean(conn);
} 

struct slName* gbMetaDataListTables(struct sqlConnection *conn)
/* Get a list of metadata tables that exist in the database */
{
static char* TABLE_NAMES[] = {
    "gbSeq", "gbExtFile", "mrna", "refSeqStatus", "refSeqSummary", "refLink",
    "imageClone", NULL
};
struct slName* tables = NULL;
int i;

for (i = 0; TABLE_NAMES[i] != NULL; i++)
    gbAddTableIfExists(conn, TABLE_NAMES[i], &tables);
for (i = 0; raFieldTables[i] != NULL; ++i)
    gbAddTableIfExists(conn, raFieldTables[i], &tables);
return tables;
}

/*
 * Local Variables:
 * c-file-style: "jkent-c"
 * End:
 */

