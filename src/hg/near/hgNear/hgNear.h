/* hgNear.h - interfaces to plug columns into hgNear.  The must important
 * thing here is the column structure. */

#ifndef HGNEAR_H
#define HGNEAR_H

#ifndef CART_H
#include "cart.h"
#endif 

#ifndef JKSQL_H
#include "jksql.h"
#endif

struct genePos
/* A gene and optionally a position. */
    {
    struct genePos *next;
    char *name;		/* Gene (transcript) ID. */
    char *chrom;	/* Optional chromosome location. NULL ok. */
    int start;		/* Chromosome start. Disregarded if chrom == NULL. */
    int end;		/* End in chromosome. Disregarded if chrom == NULL. */
    char *protein;	/* Protein ID. */
    float distance;	/* Something to help sort on. */
    int count;		/* Something to count with. Not always set. */
    };
#define genePosTooFar 1000000000	/* Definitely not in neighborhood. */

int genePosCmpName(const void *va, const void *vb);
/* Sort function to compare two genePos by name. */

int genePosCmpPos(const void *va, const void *vb);
/* Sort function to compare two genePos by chrom,start. */

void genePosFillFrom5(struct genePos *gp, char **row);
/* Fill in genePos from row containing ascii version of
 * name/chrom/start/end/protein. */

struct searchResult
/* A result from simple search - includes short and long names as well
 * as genePos. */
    {
    struct searchResult *next;
    char *shortLabel;		/* Short name (gene name) */
    char *longLabel;		/* Long name (gene description) */
    struct genePos gp;		/* Gene id. */
    };

int searchResultCmpShortLabel(const void *va, const void *vb);
/* Compare to sort based on short label. */

struct column
/* A column in the big table. The central data structure for
 * hgNear. */
   {
   /* Data set during initialization that is guaranteed to be in each track.  */
   struct column *next;		/* Next column. */
   char *name;			/* Column name, not allocated here. */
   char *shortLabel;		/* Column label. */
   char *longLabel;		/* Column description. */
   boolean on;			/* True if turned on. */
   boolean defaultOn;		/* On by default? */
   float priority;		/* Order displayed. */
   char *type;			/* Type - encodes which methods to used etc. */
   char *itemUrl;		/* printf formatted URL string for linking.
                                 * May be NULL.  Should contain one %s, which
				 * get's filled in with whatever cellVal
				 * return. */
   struct hash *settings;	/* Settings from ra file. */

   /* -- Methods -- */
   boolean (*exists)(struct column *col, struct sqlConnection *conn);
   /* Return TRUE if column exists in database. */

   char *(*cellVal)(struct column *col, struct genePos *gp, 
   	struct sqlConnection *conn);
   /* Get value of one cell as string.  FreeMem this when done.  Note that
    * gp->chrom may be NULL legitimately. */

   void (*cellPrint)(struct column *col, struct genePos *gp, 
   	struct sqlConnection *conn);
   /* Print one cell of this column.  Note that gp->chrom may be 
    * NULL legitimately. */

   void (*labelPrint)(struct column *col);
   /* Print the label in the label row. */

   int (*tableColumns)(struct column *col);
   /* How many html columns this uses. */

   void (*configControls)(struct column *col);
   /* Print out configuration controls. */

   struct searchResult *(*simpleSearch)(struct column *col, 
   	struct sqlConnection *conn, char *search);
   /* Return list of genes with descriptions that match search. */

   void (*filterControls)(struct column *col, struct sqlConnection *conn);
   /* Print out controls for advanced filter. */

   struct genePos *(*advFilter)(struct column *col, struct sqlConnection *conn,
   	struct genePos *inputList);
   /* Return list of positions for advanced filter. */

   /* -- Data that may be column-specific. -- */
      /* Most columns that need any extra data at all use the next few fields. */
   char *table;			/* Name of associated table. */
   char *keyField;		/* GeneId field in associated table. */
   char *valField;		/* Value field in associated table. */
   boolean selfLink;		/* Put in link to self. */

      /* The distance type columns like homology need this field too. */
   char *curGeneField;		/* curGeneId field in associated table.  Used by distance columns*/

      /* The expression ratio type columns use the next bunch of fields as well as
       * the table/key/val fields above. */
   char *experimentTable;	/* Experiment table in hgFixed if any. */
   char *posTable;		/* Positional (bed12) for expression experiments. */
   double expScale;		/* What to scale by to get expression val from -1 to 1. */
   int representativeCount;	/* Count of representative experiments. */
   int *representatives;	/* Array (may be null) of representatives. */
   boolean expRatioUseBlue;	/* Use blue rather than red in expRatio. */

       /* The expMulti (absolute and ratio) columns use these fields. */
   struct expMultiData *emdList;	/* List of all experiment display types. */
   struct expMultiData *emd;		/* Currently selected display type. */
   double expRatioScale;		/* 1/Maximum non-clipped expression ratio. */
   double expAbsScale;			/* 1/Maximum non-clipped absolute expression. */
   boolean expShowAbs;			/* Show absolute rather than ratio. */

      /* The GO column uses this. */
   struct sqlConnection *goConn;  /* Connection to go database. */

   /* Association tables use this. */
   char *tablesUsed;	/* Space delimited list of tables. */
   char *queryFull;	/* Query that returns 2 columns key/value. */
   char *queryOne;	/* Query that returns value given key. */
   char *invQueryOne;	/* Query that returns key given value. */
   boolean protKey;	/* Use protein rather than geneId for key. */

   /* Pfam uses this. */
   char *protDb; /* Which protein database pfam tables are in. */
   };

struct expMultiData 
/* Info on how to display expression data that can come in many forms. */
    {
    struct expMultiData *next;
    char *name;		/* Symbolic name. */
    char *shortLabel;	/* UI short label. */
    char *experimentTable;	/* Table of sample descriptions. */
    char *ratioTable;	/* Table of ratios. */
    char *absoluteTable;	/* Table of absolute expression values. */
    int representativeCount;	/* Count of representative experiments. */
    int *representatives;	/* Array of representatives. */
    };

struct order
/* A row order of the big table. */
    {
    struct order *next;	/* Next in list. */
    char *name;			/* Symbolic name, not allocated here. */
    char *shortLabel;		/* Short readable label. */
    char *longLabel;		/* Longer description. */
    char *type;			/* Type - encodes which methods to used etc. */
    float priority;		/* Order to display in. */
    struct hash *settings;	/* Settings from ra file. */

    boolean (*exists)(struct order *ord, struct sqlConnection *conn);
    /* Return TRUE if this ordering can be computed from available data. */

    void (*calcDistances)(struct order *ord, struct sqlConnection *conn, 
    	struct genePos **pGeneList, struct hash *geneHash, int maxCount);
    /* Fill in distance fields of first maxCount members of *pGeneList.
     * GeneHash and *pGeneList contain the same genes.  GeneHash is
     * keyed by gene->name. This function may reorder *pGeneList, 
     * though it will ultimately be sorted by distance. */

    /* -- Data that may be order-specific. -- */
    char *table;			/* Name of associated table. */
    char *keyField;		/* GeneId field in associated table. */
    char *valField;		/* Value field in associated table. */
    char *curGeneField;		/* curGeneId field in associated table. */
    float distanceMultiplier;	/* What to multiply valField by for distance. */
    };

struct order *orderGetAll(struct sqlConnection *conn);
/* Return list of row orders available. */


/* ---- global variables ---- */

extern struct cart *cart; /* This holds variables between clicks. */
extern char *database;	  /* Name of genome database - hg15, or the like. */
extern char *organism;	  /* Name of organism - mouse, human, etc. */
extern char *groupOn;	  /* Current grouping strategy. */
extern int displayCount;  /* Number of items to display. */

extern struct genePos *curGeneId;	  /* Identity of current gene. */

/* ---- Cart Variables ---- */

#define dbVarName "db"      /* Which assembly to use. */
#define orgVarName "org"      /* Which organism to use. */
#define confVarName "near.do.configure"	/* Configuration button */
#define detailsVarName "near.do.details" /* Do details page. */
#define countVarName "near.count"	/* How many items to display. */
#define colInfoVarName "near.do.colInfo"	/* Display column info. */
#define searchVarName "near_search"	
	/* Search term - underbar for Javascript.  Hardcoded in Javascript. */
#define idVarName "near.do.id"         	
	/* Overrides searchVarName if it exists */
#define idPosVarName "near.idPos"      	
	/* chrN:X-Y position of id, may be empty. */
#define groupVarName "near.group"	/* Grouping scheme. */
#define orderVarName "near.order"	/* Ordering scheme. */
#define getSeqVarName "near.getSeq"	/* Button to get sequence. */
#define getGenomicSeqVarName "near.getGenomicSeq"	
	/* Button to fetch genomic sequence. */
#define getSeqHowVarName "near.getSeqHow" /* How to get sequence. */
#define getSeqPageVarName "near.do.getSeqPage" /* Button go to getSequence page. */
#define proUpSizeVarName "near.proUpSize" /* Promoter upstream size. */
#define proDownSizeVarName "near.proDownSize" /* Promoter downstream size. */
#define proIncludeFiveOnly "near.proIncludeFiveOnly" 
	/* Include without 5' UTR? */
#define getTextVarName "near.getText"	/* Button to get as text. */

#define advFilterPrefix "near.as."      
	/* Prefix for advanced filter variables. */
#define advFilterPrefixI "near.asi."    
	/* Prefix for advanced filter variables not forcing search. */
#define advFilterVarName "near.do.advFilter"      /* Advanced filter */
#define advFilterClearVarName "near.do.advFilterClear" 
	/* Advanced filter clear all button. */
#define advFilterListVarName "near.do.advFilterList" 
	/* Advanced filter submit list. */

#define filSaveSettingsPrefix "near_filUserSet_" /* Prefix for filter sets. */
    /* Underbars on this one for sake of javascript. */
#define filSaveCurrentVarName "near.do.filUserSet.save"   /* Save filter set. */
#define filSavedCurrentVarName "near.do.filUserSet.saved" /* Saved filter set. */
#define filUseSavedVarName "near.do.filUserSet.used"      /* Use filter set. */

#define colConfigPrefix "near.col."     
	/* Prefix for stuff set in configuration pages. */
#define colOrderVar "near.colOrder"     /* Order of columns. */
#define defaultConfName "near.do.colDefault"  /* Restore to default settings. */
#define hideAllConfName "near.do.colHideAll"  /* Hide all columns. */
#define showAllConfName "near.do.colShowAll"  /* Show all columns. */

#define colSaveSettingsPrefix "near_colUserSet_"  /* Prefix for column sets. */
    /* Underbars on this one for sake of javascript. */
#define saveCurrentConfName "near.do.colUserSet.save"   /* Save column set. */
#define savedCurrentConfName "near.do.colUserSet.saved" /* Saved column set. */
#define useSavedConfName "near.do.colUserSet.used"      /* Use column set. */

#define showAllSpliceVarName "near.showAllSplice" 
	/* Show all splice varients. */
#define expRatioColorVarName "near.expRatioColors" 
	/* Color scheme for expression ratios. */
#define keyWordUploadPrefix "near.do.keyUp." /* Prefix for keyword uploads. */
#define keyWordPastePrefix "near.do.keyPaste." /* Prefix for keyword paste-ins. */
#define keyWordPastedPrefix "near.do.keyPasted." 
	/* Prefix for keyword paste-ins. */
#define keyWordClearPrefix "near.do.keyClear." /* Prefix for keyword paste-ins. */

/* ---- General purpose helper routines. ---- */

int genePosCmpName(const void *va, const void *vb);
/* Sort function to compare two genePos by name. */

boolean anyWild(char *s);
/* Return TRUE if there are '?' or '*' characters in s. */

boolean wildMatchAny(char *word, struct slName *wildList);
/* Return TRUE if word matches any thing in wildList. */

boolean wildMatchAll(char *word, struct slName *wildList);
/* Return TRUE if word matches all things in wildList. */

boolean wildMatchList(char *word, struct slName *wildList, boolean orLogic);
/* Return TRUE if word matches things in wildList. */

struct hash *readRas(char *rootName);
/* Read in ra in root, root/org, and root/org/database. */

char *mustFindInRaHash(char *fileName, struct hash *raHash, char *name);
/* Look up in ra hash or die trying. */

char *genomeSetting(char *name);
/* Return genome setting value.   Aborts if setting not found. */

/* ---- Some html helper routines. ---- */

void hvPrintf(char *format, va_list args);
/* Print out some html. */

void hPrintf(char *format, ...);
/* Print out some html. */

void hPrintNonBreak(char *s);
/* Print out string but replace spaces with &nbsp; */

void makeTitle(char *title, char *helpName);
/* Make title bar. */

void selfAnchorId(struct genePos *gp);
/* Print self anchor to given id. */

void colInfoAnchor(struct column *col);
/* Print anchor tag that leads to column info page. */

void cellSelfLinkPrint(struct column *col, struct genePos *gp,
	struct sqlConnection *conn);
/* Print self and hyperlink to make this the search term. */

void selfAnchorSearch(struct genePos *gp);
/* Print self anchor to given search term. */

/* -- Other helper routines. -- */

int columnCmpPriority(const void *va, const void *vb);
/* Compare to sort columns based on priority. */

void refinePriorities(struct column *colList);
/* Consult colOrderVar if it exists to reorder priorities. */

struct hash *hashColumns(struct column *colList);
/* Return a hash of columns keyed by name. */

struct column *findNamedColumn(struct column *colList, char *name);
/* Return column of given name from list or NULL if none. */

/* ---- Some helper routines for column methods. ---- */

char *columnSetting(struct column *col, char *name, char *defaultVal);
/* Return value of named setting in column, or default if it doesn't exist. */

int columnIntSetting(struct column *col, char *name, int defaultVal);
/* Return value of named integer setting or default if it doesn't exist. */

boolean columnSettingExists(struct column *col, char *name);
/* Return TRUE if setting exists in column. */

char *colVarName(struct column *col, char *prefix);
/* Return variable name prefix.col->name. This is just a static
 * variable, so don't nest these calls. */

void colButton(struct column *col, char *prefix, char *label);
/* Make a button named prefix.col->name with given label. */

struct column *colButtonPressed(struct column *colList, char *prefix);
/* See if a button named prefix.column is pressed for some
 * column, and if so return the column, else NULL. */

char *cellLookupVal(struct column *col, struct genePos *gp, struct sqlConnection *conn);
/* Get a field in a table defined by col->table, col->keyField, col->valField. */

void cellSimplePrint(struct column *col, struct genePos *gp, struct sqlConnection *conn);
/* This just prints cellSimpleVal. */

void labelSimplePrint(struct column *col);
/* This just prints cell->shortLabel. */

boolean simpleTableExists(struct column *col, struct sqlConnection *conn);
/* This returns true if col->table exists. */

void lookupAdvFilterControls(struct column *col, struct sqlConnection *conn);
/* Print out controls for advanced filter on lookup column. */

void minMaxAdvFilterControls(struct column *col, struct sqlConnection *conn);
/* Print out controls for min/max advanced filter. */

struct searchResult *lookupTypeSimpleSearch(struct column *col, 
    struct sqlConnection *conn, char *search);
/* Search lookup type column. */

struct searchResult *knownGeneSearchResult(struct sqlConnection *conn, 
	char *kgID, char *alias);
/* Return a searchResult for a known gene. */

struct genePos *knownPosAll(struct sqlConnection *conn);
/* Get all positions in knownGene table. */

struct genePos *knownPosOne(struct sqlConnection *conn, char *name);
/* Get all positions of named gene. */

void fillInKnownPos(struct genePos *gp, struct sqlConnection *conn);
/* If gp->chrom is not filled in go look it up. */

struct hash *keyFileHash(struct column *col);
/* Make up a hash from key file for this column. 
 * Return NULL if no key file. */

struct slName *keyFileList(struct column *col);
/* Make up list from key file for this column.
 * return NULL if no key file. */

char *configVarName(struct column *col, char *varName);
/* Return variable name for configuration. */

char *configVarVal(struct column *col, char *varName);
/* Return value for configuration variable.  Return NULL if it
 * doesn't exist or if it is "" */

struct genePos *advFilterResults(struct column *colList, 
	struct sqlConnection *conn);
/* Get list of genes that pass all advanced filter filters.  
 * If no filters are on this returns all genes. */

char *advFilterName(struct column *col, char *varName);
/* Return variable name for advanced filter. */

char *advFilterVal(struct column *col, char *varName);
/* Return value for advanced filter variable.  Return NULL if it
 * doesn't exist or if it is "" */

char *advFilterNameI(struct column *col, char *varName);
/* Return name for advanced filter that doesn't force search. */

void advFilterRemakeTextVar(struct column *col, char *varName, int size);
/* Make a text field of given name and size filling it in with
 * the existing value if any. */

void advFilterAnyAllMenu(struct column *col, char *varName, 
	boolean defaultAny);
/* Make a drop-down menu with value all/any. */

boolean advFilterOrLogic(struct column *col, char *varName, 
	boolean defaultOr);
/* Return TRUE if user has selected 'all' from any/all menu */

boolean gotAdvFilter();
/* Return TRUE if advanced filter variables are set. */

boolean advFilterColAnySet(struct column *col);
/* Return TRUE if any of the advanced filter variables
 * for this col are set. */

void advFilterKeyUploadButton(struct column *col);
/* Make a button for uploading keywords. */

struct column *advFilterKeyUploadPressed(struct column *colList);
/* Return column where an key upload button was pressed, or
 * NULL if none. */

void doAdvFilterKeyUpload(struct sqlConnection *conn, struct column *colList, 
    struct column *col);
/* Handle upload keyword list button press in advanced filter form. */

void advFilterKeyPasteButton(struct column *col);
/* Make a button for uploading keywords. */

struct column *advFilterKeyPastePressed(struct column *colList);
/* Return column where an key upload button was pressed, or
 * NULL if none. */

void doAdvFilterKeyPaste(struct sqlConnection *conn, struct column *colList, 
    struct column *col);
/* Handle search keyword list button press in advanced filter form. */

struct column *advFilterKeyPastedPressed(struct column *colList);
/* Return column where an key upload button was pressed, or
 * NULL if none. */

void doAdvFilterKeyPasted(struct sqlConnection *conn, struct column *colList, 
    struct column *col);
/* Handle search keyword list button press in advanced filter form. */

void advFilterKeyClearButton(struct column *col);
/* Make a button for uploading keywords. */

struct column *advFilterKeyClearPressed(struct column *colList);
/* Return column where an key upload button was pressed, or
 * NULL if none. */

void doAdvFilterKeyClear(struct sqlConnection *conn, struct column *colList, 
    struct column *col);
/* Handle clear keyword list button press in advanced filter form. */

void doNameCurrentFilters();
/* Put up page to save current filter settings. */

void doSaveCurrentFilters(struct sqlConnection *conn, struct column *colList);
/* Handle save current filters form result. */

void doUseSavedFilters(struct sqlConnection *conn, struct column *colList);
/* Use indicated filter settings. */

struct genePos *weedUnlessInHash(struct genePos *inList, struct hash *hash);
/* Return input list with stuff not in hash removed. */

void gifLabelVerticalText(char *fileName, char **labels, int labelCount,
	int height);
/* Make a gif file with given labels. */

int gifLabelMaxWidth(char **labels, int labelCount);
/* Return maximum pixel width of labels.  It's ok to have
 * NULLs in labels array. */

struct sqlConnection *hgFixedConn();
/* Return connection to hgFixed database. 
 * This is effectively a global, but not
 * opened until needed. */

/* ---- Column method setters. ---- */

void columnDefaultMethods(struct column *col);
/* Set up default methods. */

void setupColumnNum(struct column *col, char *parameters);
/* Set up column that displays index in displayed list. */

void setupColumnLookup(struct column *col, char *parameters);
/* Set up column that just looks up one field in a table
 * keyed by the geneId. */

void setupColumnAssociation(struct column *col, char *parameters);
/* Set up a column that looks for an association table 
 * keyed by the geneId. */

void setupColumnAcc(struct column *col, char *parameters);
/* Set up a column that displays the geneId (accession) */

void setupColumnDistance(struct column *col, char *parameters);
/* Set up a column that looks up a field in a distance matrix
 * type table such as the expression or homology tables. */

void setupColumnKnownPos(struct column *col, char *parameters);
/* Set up column that links to genome browser based on known gene
 * position. */

void setupColumnKnownDetails(struct column *col, char *parameters);
/* Set up a column that links to details page for known genes. */

void setupColumnKnownName(struct column *col, char *parameters);
/* Set up a column that looks up name to display, and selects on a click. */

void setupColumnSwissProt(struct column *col, char *parameters);
/* Set up a column that protein name and links to SwissProt. */

void setupColumnExpRatio(struct column *col, char *parameters);
/* Set up expression ratio type column. */

void setupColumnExpMulti(struct column *col, char *parameters);
/* Set up expression type column that can be ratio or absolute,
 * short or long. */

void setupColumnExpMax(struct column *col, char *parameters);
/* Set up maximum expression value column. */

void setupColumnGo(struct column *col, char *parameters);
/* Set up gene ontology column. */

void setupColumnPfam(struct column *col, char *parameters);
/* Setup Pfam column. */

void goSimilarityMethods(struct order *ord, char *parameters);
/* Set up go similarity ordering. */

/* ---- Get config options ---- */
boolean showOnlyCanonical();
/* Return TRUE if we only show canonical splicing varients. */

struct hash *canonicalHash();
/* Get canonicalHash if necessary, otherwise return NULL. */

boolean expRatioUseBlue();
/* Return TRUE if should use blue instead of red
 * in the expression ratios. */

/* ---- Create high level pages. ---- */
void displayData(struct sqlConnection *conn, struct column *colList, struct genePos *gp);
/* Display data in neighborhood of gene. */

void doSearch(struct sqlConnection *conn, struct column *colList);
/* Search.  If result is unambiguous call doMain, otherwise
 * put up a page of choices. */

void doAdvFilter(struct sqlConnection *conn, struct column *colList);
/* Put up advanced filter page. */

void doAdvFilterClear(struct sqlConnection *conn, struct column *colList);
/* Clear variables in advanced filter page. */

void doAdvFilterList(struct sqlConnection *conn, struct column *colList);
/* List gene names matching advanced filter. */

void doConfigure(struct sqlConnection *conn, struct column *colList, 
	char *bumpVar);
/* Configuration page. */

void doDetails(struct sqlConnection *conn, struct column *colList,  char *gene);
/* Put up details page on gene. */

void doDefaultConfigure(struct sqlConnection *conn, struct column *colList );
/* Do configuration starting with defaults. */

void doConfigHideAll(struct sqlConnection *conn, struct column *colList);
/* Respond to hide all button in configuration page. */

void doConfigShowAll(struct sqlConnection *conn, struct column *colList);
/* Respond to show all button in configuration page. */

void doNameCurrentColumns();
/* Put up page to save current column configuration. */

void doSaveCurrentColumns(struct sqlConnection *conn, struct column *colList);
/* Save the current columns, and then go on. */

void doConfigUseSaved(struct sqlConnection *conn, struct column *colList);
/* Respond to Use Saved Settings buttin in configuration page. */

void doGetSeqPage(struct sqlConnection *conn, struct column *colList);
/* Put up the get sequence page. */

void doGetSeq(struct sqlConnection *conn, struct column *colList, 
	struct genePos *geneList, char *how);
/* Show sequence. */

void doGetGenomicSeq(struct sqlConnection *conn, struct column *colList,
	struct genePos *geneList);
/* Retrieve genomic sequence sequence according to options. */

void doExamples(struct sqlConnection *conn, struct column *colList);
/* Put up controls and then some helpful text and examples.
 * Called when search box is empty. */

/* ---- User settings stuff - soon to be moved to library I hope. */

struct userSettings 
/* This helps us the user a set of cart variables, defined
 * by settingsPrefix.  It allows the user to save different
 * sets of settings under different names. */
     {
     struct userSettings *next;	/* Next in list. */
     struct cart *cart;		/* Associated cart. */
     char *formTitle;		/* Heading of settings save form. */
     char *nameVar;		/* Variable for name on give-it-a-name page. */
     char *savePrefix;		/* Prefix for where we store settings.
                                 * We store at savePrefix.name where name
				 * is taken from listVar above. */
     char *formVar;		/* Submit button on give-it-a-name page. */
     char *listDisplayVar;	/* Variable name for list display control. */
     struct slName *saveList;	/* List of variables to save. */
     };

struct userSettings *userSettingsNew(
	struct cart *cart,	/* Persistent variable cart. */
	char *formTitle,	/* Heading of settings save form. */
	char *formVar,		/* Name of button variable on save form. */
	char *localVarPrefix);  /* Prefix to use for internal cart variables. 
		                 * No periods allowed because of javascript. */
/* Make new object to help manage sets of user settings. */

void userSettingsCaptureVar(struct userSettings *us, char *varName);
/* Add a single variable to list of variables to capture. */

void userSettingsCapturePrefix(struct userSettings *us, char *prefix);
/* Capture all variables that start with prefix. */

boolean userSettingsAnySaved(struct userSettings *us);
/* Return TRUE if any user settings are saved. */

void userSettingsUseNamed(struct userSettings *us, char *setName);
/* Use named collection of settings. */

void userSettingsUseSelected(struct userSettings *us);
/* Use currently selected user settings. */

void userSettingsSaveForm(struct userSettings *us);
/* Put up controls that let user name and save the current
 * set. */

void userSettingsLoadForm(struct userSettings *us);
/* Put up controls that let user name and save the current
 * set. */

boolean userSettingsProcessForm(struct userSettings *us);
/* Handle button press in userSettings form. 
 * If this returns TRUE then form is finished processing
 * and you can call something to make the next page. */

void userSettingsDropDown(struct userSettings *us);
/* Display list of available saved settings . */

#endif /* HGNEAR_H */

