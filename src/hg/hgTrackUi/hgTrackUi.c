/* hgTrackUi - Display track-specific user interface.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "htmshell.h"
#include "web.h"
#include "cart.h"
#include "jksql.h"
#include "trackDb.h"
#include "hdb.h"
#include "hCommon.h"
#include "hui.h"
#include "sample.h"
#include "cdsColors.h"
#include "wiggle.h"
#include "hgMaf.h"
#include "obscure.h"
#include "chainCart.h"
#include "chainDb.h"

#define CDS_HELP_PAGE "../goldenPath/help/hgCodonColoring.html"
#define CDS_MRNA_HELP_PAGE "../goldenPath/help/hgCodonColoringMrna.html"

static char const rcsid[] = "$Id: hgTrackUi.c,v 1.147 2004/11/02 10:07:15 daryl Exp $";

struct cart *cart = NULL;	/* Cookie cart with UI settings */
char *database = NULL;		/* Current database. */
char *chromosome = NULL;	/* Chromosome. */

void radioButton(char *var, char *val, char *ourVal)
/* Print one radio button */
{
cgiMakeRadioButton(var, ourVal, sameString(ourVal, val));
printf("%s ", ourVal);
}

void filterButtons(char *filterTypeVar, char *filterTypeVal, boolean none)
/* Put up some filter buttons. */
{
printf("<B>Filter:</B> ");
radioButton(filterTypeVar, filterTypeVal, "red");
radioButton(filterTypeVar, filterTypeVal, "green");
radioButton(filterTypeVar, filterTypeVal, "blue");
radioButton(filterTypeVar, filterTypeVal, "exclude");
radioButton(filterTypeVar, filterTypeVal, "include");
if (none)
    radioButton(filterTypeVar, filterTypeVal, "none");
}


void stsMapUi(struct trackDb *tdb)
/* Put up UI stsMarkers. */
{
char *stsMapFilter = cartUsualString(cart, "stsMap.filter", "blue");
char *stsMapMap = cartUsualString(cart, "stsMap.type", smoeEnumToString(0));
filterButtons("stsMap.filter", stsMapFilter, TRUE);
printf(" ");
smoeDropDown("stsMap.type", stsMapMap);
}

void stsMapMouseUi(struct trackDb *tdb)
/* Put up UI stsMapMouse. */
{
char *stsMapMouseFilter = cartUsualString(cart, "stsMapMouse.filter", "blue");
char *stsMapMouseMap = cartUsualString(cart, "stsMapMouse.type", smmoeEnumToString(0));
filterButtons("stsMapMouse.filter", stsMapMouseFilter, TRUE);
printf(" ");
smmoeDropDown("stsMapMouse.type", stsMapMouseMap);
}

void stsMapRatUi(struct trackDb *tdb)
/* Put up UI stsMapRat. */
{
char *stsMapRatFilter = cartUsualString(cart, "stsMapRat.filter", "blue");
char *stsMapRatMap = cartUsualString(cart, "stsMapRat.type", smroeEnumToString(0));
filterButtons("stsMapRat.filter", stsMapRatFilter, TRUE);
printf(" ");
smroeDropDown("stsMapRat.type", stsMapRatMap);
}

void snpMapTypeFilterButtons(char *filterTypeVar, char *filterTypeVal)
/* Put up some filter buttons. */
{
radioButton(filterTypeVar, filterTypeVal, "include");
radioButton(filterTypeVar, filterTypeVal, "exclude");
}

void snpColorFilterButtons(char *filterTypeVar, char *filterTypeVal)
/* Put up some filter buttons. */
{
radioButton(filterTypeVar, filterTypeVal, "Source");
radioButton(filterTypeVar, filterTypeVal, "MolType");
radioButton(filterTypeVar, filterTypeVal, "Class");
radioButton(filterTypeVar, filterTypeVal, "Valid");
radioButton(filterTypeVar, filterTypeVal, "Func");
radioButton(filterTypeVar, filterTypeVal, "Black");
}

void snpFilterButtons(char *filterTypeVar, char *filterTypeVal)
/* Put up some filter buttons. */
{
radioButton(filterTypeVar, filterTypeVal, "red");
radioButton(filterTypeVar, filterTypeVal, "green");
radioButton(filterTypeVar, filterTypeVal, "blue");
radioButton(filterTypeVar, filterTypeVal, "black");
radioButton(filterTypeVar, filterTypeVal, "exclude");
}

void snpMapUi(struct trackDb *tdb)
/* Put up UI snpMap data. */
{
int   snpMapSource = 0;
int   snpMapType = 0;

if (strncmp(database,"hg",2))
    return;
printf("<BR><B>Variant Sources:</B><BR>\n");
for (snpMapSource=0; snpMapSource<ArraySize(snpMapSourceCart); snpMapSource++)
    {
    snpMapSourceCart[snpMapSource] = 
	cartUsualString(cart, snpMapSourceString(snpMapSource),	snpMapSourceDefaultString(snpMapSource));
    snpFilterButtons(snpMapSourceString(snpMapSource), snpMapSourceCart[snpMapSource]);
    printf(" - <B>%s</B><BR>\n",snpMapSourceLabel(snpMapSource));
    }
printf("<BR><B>Variant Types:</B><BR>\n");
for (snpMapType=0; snpMapType<3; snpMapType++)
    {
    snpMapTypeCart[snpMapType] = 
	cartUsualString(cart, snpMapTypeString(snpMapType), snpMapTypeDefaultString(snpMapType));
    snpMapTypeFilterButtons(snpMapTypeString(snpMapType), snpMapTypeCart[snpMapType]);
    printf(" - <B>%s</B><BR>\n",snpMapTypeLabelStr(snpMapType));
    }
}

void snpUi(struct trackDb *tdb)
/* Put up UI snp data. */
{
int   snpSourceCount  = ArraySize(snpSourceCart);
int   snpMolTypeCount = ArraySize(snpMolTypeCart);
int   snpClassCount   = ArraySize(snpClassCart);
int   snpValidCount   = ArraySize(snpValidCart);
int   snpFuncCount    = ArraySize(snpFuncCart);
int   snpSource  = 0;
int   snpMolType = 0;
int   snpClass   = 0;
int   snpValid   = 0;
int   snpFunc    = 0;

if (strncmp(database,"hg",2))
    return;
printf("<BR><B>Color Specification:</B><BR>\n");
    snpColorSourceCart[0] = cartUsualString(cart, "snpColor", "Source" );
    snpColorFilterButtons("snpColor", snpColorSourceCart[0]);
printf("<BR><BR><B>Sources:</B><BR>\n");
for (snpSource=0; snpSource < snpSourceCount; snpSource++)
    {
    snpSourceCart[snpSource] = 
	cartUsualString(cart, snpSourceString(snpSource), snpSourceDefaultString(snpSource));
    snpFilterButtons(snpSourceString(snpSource), snpSourceCart[snpSource]);
    printf(" - <B>%s</B><BR>\n",snpSourceLabel(snpSource));
    }
printf("<BR><B>Molecule Types:</B><BR>\n");
for (snpMolType=0; snpMolType<snpMolTypeCount; snpMolType++)
    {
    snpMolTypeCart[snpMolType] = 
	cartUsualString(cart, snpMolTypeString(snpMolType), snpMolTypeDefaultString(snpMolType));
    snpFilterButtons(snpMolTypeString(snpMolType), snpMolTypeCart[snpMolType]);
    printf(" - <B>%s</B><BR>\n",snpMolTypeLabel(snpMolType));
    }
printf("<BR><B>Classes:</B><BR>\n");
for (snpClass=0; snpClass<snpClassCount; snpClass++)
    {
    snpClassCart[snpClass] = 
	cartUsualString(cart, snpClassString(snpClass), snpClassDefaultString(snpClass));
    snpFilterButtons(snpClassString(snpClass), snpClassCart[snpClass]);
    printf(" - <B>%s</B><BR>\n",snpClassLabel(snpClass));
    }
printf("<BR><B>Validation Status:</B><BR>\n");
for (snpValid=0; snpValid<snpValidCount; snpValid++)
    {
    snpValidCart[snpValid] = 
	cartUsualString(cart, snpValidString(snpValid), snpValidDefaultString(snpValid));
    snpFilterButtons(snpValidString(snpValid), snpValidCart[snpValid]);
    printf(" - <B>%s</B><BR>\n",snpValidLabel(snpValid));
    }
printf("<BR><B>Functional classification:</B><BR>\n");
for (snpFunc=0; snpFunc<snpFuncCount; snpFunc++)
    {
    snpFuncCart[snpFunc] = 
	cartUsualString(cart, snpFuncString(snpFunc), snpFuncDefaultString(snpFunc));
    snpFilterButtons(snpFuncString(snpFunc), snpFuncCart[snpFunc]);
    printf(" - <B>%s</B><BR>\n",snpFuncLabel(snpFunc));
    }
}

void cbrWabaUi(struct trackDb *tdb)
/* Put up UI cbrWaba. */
{
/*   This link is disabled in the external browser
char *cbrWabaFilter = cartUsualString(cart, "cbrWaba.filter", "red");
char *cbrWabaMap = cartUsualString(cart, "cbrWaba.type", fcoeEnumToString(0));
int start = cartInt(cart, "cbrWaba.start");
int end = cartInt(cart, "cbrWaba.end");
printf(
"<P><A HREF=\"http://genome-test.cse.ucsc.edu/cgi-bin/tracks.exe?where=%s%%3A%d-%d\"> Temporary Intronerator link: %s:%d-%d</A> <I>for testing purposes only</I> \n</P>", chromosome+3, start, end, chromosome+3, start, end );
*/
}

void fishClonesUi(struct trackDb *tdb)
/* Put up UI fishClones. */
{
char *fishClonesFilter = cartUsualString(cart, "fishClones.filter", "red");
char *fishClonesMap = cartUsualString(cart, "fishClones.type", fcoeEnumToString(0));
filterButtons("fishClones.filter", fishClonesFilter, TRUE);
printf(" ");
fcoeDropDown("fishClones.type", fishClonesMap);
}

void recombRateUi(struct trackDb *tdb)
/* Put up UI recombRate. */
{
char *recombRateMap = cartUsualString(cart, "recombRate.type", rroeEnumToString(0));
printf("<b>Map Distances: </b>");
rroeDropDown("recombRate.type", recombRateMap);
}

void recombRateRatUi(struct trackDb *tdb)
/* Put up UI recombRateRat. */
{
char *recombRateRatMap = cartUsualString(cart, "recombRateRat.type", rrroeEnumToString(0));
printf("<b>Map Distances: </b>");
rrroeDropDown("recombRateRat.type", recombRateRatMap);
}

void recombRateMouseUi(struct trackDb *tdb)
/* Put up UI recombRateMouse. */
{
char *recombRateMouseMap = cartUsualString(cart, "recombRateMouse.type", rrmoeEnumToString(0));
printf("<b>Map Distances: </b>");
rrmoeDropDown("recombRateMouse.type", recombRateMouseMap);
}

void cghNci60Ui(struct trackDb *tdb)
/* Put up UI cghNci60. */
{
char *cghNci60Map = cartUsualString(cart, "cghNci60.type", cghoeEnumToString(0));
char *col = cartUsualString(cart, "cghNci60.color", "gr");
printf(" <b>Cell Lines: </b> ");
cghoeDropDown("cghNci60.type", cghNci60Map);
printf(" ");
printf(" <b>Color Scheme</b>: ");
cgiMakeRadioButton("cghNci60.color", "gr", sameString(col, "gr"));
printf(" green/red ");
cgiMakeRadioButton("cghNci60.color", "rg", sameString(col, "rg"));
printf(" red/green ");
cgiMakeRadioButton("cghNci60.color", "rb", sameString(col, "rb"));
printf(" red/blue ");
}

void nci60Ui(struct trackDb *tdb)
/* put up UI for the nci60 track from stanford track */
{
char *nci60Map = cartUsualString(cart, "nci60.type", nci60EnumToString(0));
char *col = cartUsualString(cart, "exprssn.color", "rg");
printf("<p><b>Cell Lines: </b> ");
nci60DropDown("nci60.type", nci60Map);
printf(" ");
printf(" <b>Color Scheme</b>: ");
cgiMakeRadioButton("exprssn.color", "rg", sameString(col, "rg"));
printf(" red/green ");
cgiMakeRadioButton("exprssn.color", "rb", sameString(col, "rb"));
printf(" red/blue ");
}

void affyUi(struct trackDb *tdb)
/* put up UI for the affy track from stanford track */
{
char *affyMap;
char *col;
char varName[128];

safef(varName, sizeof(varName), "%s.%s", tdb->tableName, "type");
affyMap = cartUsualString(cart, varName, affyEnumToString(affyTissue));
col = cartUsualString(cart, "exprssn.color", "rg");
printf("<p><b>Experiment Display: </b> ");
affyDropDown(varName, affyMap);
printf(" <b>Color Scheme</b>: ");
cgiMakeRadioButton("exprssn.color", "rg", sameString(col, "rg"));
printf(" red/green ");
cgiMakeRadioButton("exprssn.color", "rb", sameString(col, "rb"));
printf(" red/blue ");
}

void rosettaUi(struct trackDb *tdb)
/* put up UI for the rosetta track */
{
char *rosettaMap = cartUsualString(cart, "rosetta.type", rosettaEnumToString(0));
char *col = cartUsualString(cart, "exprssn.color", "rg");
char *exonTypes = cartUsualString(cart, "rosetta.et",  rosettaExonEnumToString(0));

printf("<p><b>Reference Sample: </b> ");
rosettaDropDown("rosetta.type", rosettaMap);
printf("  ");
printf("<b>Exons Shown:</b> ");
rosettaExonDropDown("rosetta.et", exonTypes);
printf(" <b>Color Scheme</b>: ");
cgiMakeRadioButton("exprssn.color", "rg", sameString(col, "rg"));
printf(" red/green ");
cgiMakeRadioButton("exprssn.color", "rb", sameString(col, "rb"));
printf(" red/blue ");
}


void cdsColorOptions(struct trackDb *tdb, int value)
/*Codon coloring options*/
{
    char *drawOption;
    char *drawOptionsDefault;
    char *cdsDrawDefault;
    char cdsColorVar[128];
    boolean isGenePred = (value >= 0);
    
    if (isGenePred)
        drawOptionsDefault = "enabled";
    else
        drawOptionsDefault = "disabled";
    if(sameString(trackDbSettingOrDefault(tdb,
                  "cdsDrawOptions", drawOptionsDefault), "disabled"))
        return;
    
    printf("<p>Color track by codons:</b>");
    safef(cdsColorVar, 128, "%s.cds.draw", tdb->tableName );
    cdsDrawDefault = trackDbSettingOrDefault(tdb, "cdsDrawDefault", CDS_DRAW_DEFAULT);
    drawOption = cartUsualString(cart, cdsColorVar, cdsDrawDefault);
    cdsColorDropDown(cdsColorVar, drawOption, value);
    if(value>0)
        printf("(<a href=%s>Codon coloring help</a>)<br>",CDS_HELP_PAGE);
    else
        printf("(<a href=%s>mRNA coloring help</a>)<br>",CDS_MRNA_HELP_PAGE);
}

void blastSGUi(struct trackDb *tdb)
{
char geneName[64];
char accName[64];
char sprotName[64];
char posName[64];
char cModeStr[64];
boolean useGene, useAcc, useSprot, usePos;
int cMode;
char *cModes[3] = {"0", "1", "2"};

safef(geneName, sizeof(geneName), "%s.geneLabel", tdb->tableName);
safef(accName, sizeof(accName), "%s.accLabel", tdb->tableName);
safef(sprotName, sizeof(sprotName), "%s.sprotLabel", tdb->tableName);
safef(posName, sizeof(posName), "%s.posLabel", tdb->tableName);
useGene= cartUsualBoolean(cart, geneName, TRUE);
useAcc= cartUsualBoolean(cart, accName, FALSE);
useSprot= cartUsualBoolean(cart, sprotName, FALSE);
usePos= cartUsualBoolean(cart, posName, FALSE);

safef(cModeStr, sizeof(cModeStr), "%s.cmode", tdb->tableName);
cMode = cartUsualInt(cart, cModeStr, 0);

printf("<P><B>Color elements: </B> ");
cgiMakeRadioButton(cModeStr, cModes[0], cMode == 0);
printf("by score ");
cgiMakeRadioButton(cModeStr, cModes[1], cMode == 1);
printf("by yeast chromosome ");
cgiMakeRadioButton(cModeStr, cModes[2], cMode == 2);
printf("black");

printf("<P><B>Label elements by: </B> ");
cgiMakeCheckBox(geneName, useGene);
printf("Yeast Gene ");
cgiMakeCheckBox(accName, useAcc);
printf("Yeast mRNA ");
cgiMakeCheckBox(sprotName, useSprot);
printf("SwissProt ID ");
cgiMakeCheckBox(posName, usePos);
printf("Yeast Position");

cdsColorOptions(tdb, 2);
}
void blastFBUi(struct trackDb *tdb)
{
char geneName[64];
char accName[64];
char sprotName[64];
char posName[64];
char cModeStr[64];
boolean useGene, useAcc, useSprot, usePos;
int cMode;
char *cModes[3] = {"0", "1", "2"};

safef(geneName, sizeof(geneName), "%s.geneLabel", tdb->tableName);
safef(accName, sizeof(accName), "%s.accLabel", tdb->tableName);
safef(sprotName, sizeof(sprotName), "%s.sprotLabel", tdb->tableName);
safef(posName, sizeof(posName), "%s.posLabel", tdb->tableName);
useGene= cartUsualBoolean(cart, geneName, TRUE);
useAcc= cartUsualBoolean(cart, accName, FALSE);
usePos= cartUsualBoolean(cart, posName, FALSE);

safef(cModeStr, sizeof(cModeStr), "%s.cmode", tdb->tableName);
cMode = cartUsualInt(cart, cModeStr, 0);

printf("<P><B>Color elements: </B> ");
cgiMakeRadioButton(cModeStr, cModes[0], cMode == 0);
printf("by score ");
cgiMakeRadioButton(cModeStr, cModes[1], cMode == 1);
printf("by D. mel. chromosome ");
cgiMakeRadioButton(cModeStr, cModes[2], cMode == 2);
printf("black");
printf("<P><B>Label elements by: </B> ");
cgiMakeCheckBox(geneName, useGene);
printf("FlyBase Gene ");
cgiMakeCheckBox(accName, useAcc);
printf("D. melanogaster mRNA ");
cgiMakeCheckBox(posName, usePos);
printf("D. melanogaster Position");

cdsColorOptions(tdb, 2);
}
void blastUi(struct trackDb *tdb)
{
char geneName[64];
char accName[64];
char sprotName[64];
char posName[64];
char cModeStr[64];
boolean useGene, useAcc, useSprot, usePos;
int cMode;
char *cModes[3] = {"0", "1", "2"};

safef(geneName, sizeof(geneName), "%s.geneLabel", tdb->tableName);
safef(accName, sizeof(accName), "%s.accLabel", tdb->tableName);
safef(sprotName, sizeof(sprotName), "%s.sprotLabel", tdb->tableName);
safef(posName, sizeof(posName), "%s.posLabel", tdb->tableName);
useGene= cartUsualBoolean(cart, geneName, TRUE);
useAcc= cartUsualBoolean(cart, accName, FALSE);
useSprot= cartUsualBoolean(cart, sprotName, FALSE);
usePos= cartUsualBoolean(cart, posName, FALSE);

safef(cModeStr, sizeof(cModeStr), "%s.cmode", tdb->tableName);
cMode = cartUsualInt(cart, cModeStr, 0);

printf("<P><B>Color elements: </B> ");
cgiMakeRadioButton(cModeStr, cModes[0], cMode == 0);
printf("by score ");
cgiMakeRadioButton(cModeStr, cModes[1], cMode == 1);
printf("by human chromosome ");
cgiMakeRadioButton(cModeStr, cModes[2], cMode == 2);
printf("black");

printf("<P><B>Label elements by: </B> ");
cgiMakeCheckBox(geneName, useGene);
printf("Human Gene ");
cgiMakeCheckBox(accName, useAcc);
printf("Human mRNA ");
cgiMakeCheckBox(sprotName, useSprot);
printf("SwissProt ID ");
cgiMakeCheckBox(posName, usePos);
printf("Human Position");

cdsColorOptions(tdb, 2);
}

void refGeneUI(struct trackDb *tdb)
/* Put up refGene-specifc controls */
{
char varName[64];
char *refGeneLabel;
safef(varName, sizeof(varName), "%s.label", tdb->tableName);
refGeneLabel = cartUsualString(cart, varName, "gene");
printf("<B>Label:</B> ");
radioButton(varName, refGeneLabel, "gene");
radioButton(varName, refGeneLabel, "accession");
radioButton(varName, refGeneLabel, "both");
radioButton(varName, refGeneLabel, "none");

cdsColorOptions(tdb, 2);
}

void oneMrnaFilterUi(struct controlGrid *cg, char *text, char *var)
/* Print out user interface for one type of mrna filter. */
{
controlGridStartCell(cg);
printf("%s:<BR>", text);
cgiMakeTextVar(var, cartUsualString(cart, var, ""), 19);
controlGridEndCell(cg);
}

void mrnaUi(struct trackDb *tdb, boolean isXeno)
/* Put up UI for an mRNA (or EST) track. */
{
struct mrnaUiData *mud = newMrnaUiData(tdb->tableName, isXeno);
struct mrnaFilter *fil;
struct controlGrid *cg = NULL;
char *filterTypeVar = mud->filterTypeVar;
char *filterTypeVal = cartUsualString(cart, filterTypeVar, "red");
char *logicTypeVar = mud->logicTypeVar;
char *logicTypeVal = cartUsualString(cart, logicTypeVar, "and");

/* Define type of filter. */
filterButtons(filterTypeVar, filterTypeVal, FALSE);
printf("  <B>Combination Logic:</B> ");
radioButton(logicTypeVar, logicTypeVal, "and");
radioButton(logicTypeVar, logicTypeVal, "or");
printf("<BR>\n");

/* List various fields you can filter on. */
printf("<table border=0 cellspacing=1 cellpadding=1 width=%d>\n", CONTROL_TABLE_WIDTH);
cg = startControlGrid(4, NULL);
for (fil = mud->filterList; fil != NULL; fil = fil->next)
     oneMrnaFilterUi(cg, fil->label, fil->key);
endControlGrid(&cg);
cdsColorOptions(tdb, -1);
}

void bedUi(struct trackDb *tdb)
/* Put up UI for an mRNA (or EST) track. */
{
struct mrnaUiData *mud = newBedUiData(tdb->tableName);
struct mrnaFilter *fil;
struct controlGrid *cg = NULL;
char *filterTypeVar = mud->filterTypeVar;
char *filterTypeVal = cartUsualString(cart, filterTypeVar, "red");
char *logicTypeVar = mud->logicTypeVar;
char *logicTypeVal = cartUsualString(cart, logicTypeVar, "and");

/* Define type of filter. */
filterButtons(filterTypeVar, filterTypeVal, FALSE);
printf("  <B>Combination Logic:</B> ");
radioButton(logicTypeVar, logicTypeVal, "and");
radioButton(logicTypeVar, logicTypeVal, "or");
printf("<BR>\n");

/* List various fields you can filter on. */
printf("<table border=0 cellspacing=1 cellpadding=1 width=%d>\n", CONTROL_TABLE_WIDTH);
cg = startControlGrid(4, NULL);
for (fil = mud->filterList; fil != NULL; fil = fil->next)
     oneMrnaFilterUi(cg, fil->label, fil->key);
endControlGrid(&cg);
}

static void filterByChrom(struct trackDb *tdb)
{
char *filterSetting;
char filterVar[256];
char *filterVal = "";

printf("<p><b>Filter by chromosome (eg. chr10) </b>: ");
snprintf(filterVar, sizeof(filterVar), "%s.chromFilter", tdb->tableName);
filterSetting = cartUsualString(cart, filterVar, filterVal);
cgiMakeTextVar(filterVar, cartUsualString(cart, filterVar, ""), 5);
}

void crossSpeciesUi(struct trackDb *tdb)
/* Put up UI for selecting rainbow chromosome color or intensity score. */
{
char colorVar[256];
char *colorSetting;
/* initial value of chromosome coloring option is "on", unless
 * overridden by the colorChromDefault setting in the track */
char *colorDefault = trackDbSettingOrDefault(tdb, "colorChromDefault", "on");

printf("<p><b>Color track based on chromosome</b>: ");
snprintf(colorVar, sizeof(colorVar), "%s.color", tdb->tableName);
colorSetting = cartUsualString(cart, colorVar, colorDefault);
cgiMakeRadioButton(colorVar, "on", sameString(colorSetting, "on"));
printf(" on ");
cgiMakeRadioButton(colorVar, "off", sameString(colorSetting, "off"));
printf(" off ");
printf("<br><br>");
filterByChrom(tdb);
}

void scoreUi(struct trackDb *tdb)
/* Put up UI for filtering bed track based on a score threshold */
{
char scoreVar[256];
int scoreSetting;
int scoreVal = 0;
char *scoreValString = trackDbSetting(tdb, "scoreFilter");
char tempScore[256];

/* initial value of score theshold is 0, unless
 * overridden by the scoreFilter setting in the track */
if (scoreValString != NULL)
    scoreVal = atoi(scoreValString);
printf("<p><b>Only Show items that score at or above </b>: ");
snprintf(scoreVar, sizeof(scoreVar), "%s.scoreFilter", tdb->tableName);
scoreSetting = cartUsualInt(cart,  scoreVar,  scoreVal);
safef(tempScore, sizeof(tempScore), "%d",scoreSetting);
cgiMakeTextVar( scoreVar, tempScore, 4);
}

void transRegCodeUi(struct trackDb *tdb)
/* Put upt UI for transcriptional regulatory code - not
 * much more than score UI. */
{
scoreUi(tdb);
printf("%s",
	"<BR>The scoring ranges from 0 to 1000, and is based on the number of lines "
	"of evidence that support the motif being active.  Each of the two "
	"<I>sensu stricto</I> species that the motif was conserved in counts "
	"as a line of evidence.  If the CHIP/CHIP data showed good (P 0.001) "
	"evidence of binding to the transcription factor associated with the "
	"motif, that counts as two lines of evidence.  If the CHIP/CHIP data "
	"showed weaker (P 0.005) evidence of binding, that counts as just one line "
	"of evidence.  The following table shows the relationship between lines "
	"of evidence and score.");
printf("<P>");
hTableStart();
printf("%s",
   "<TR><TH>Evidence</TH><TH>Score</TH></TR>\n"
   "<TR><TD>4</TD><TD>1000</TD></TR>\n"
   "<TR><TD>3</TD><TD>500</TD></TR>\n"
   "<TR><TD>2</TD><TD>333</TD></TR>\n"
   "<TR><TD>1</TD><TD>250</TD></TR>\n"
   "<TR><TD>0</TD><TD>200</TD></TR>\n"
   );
hTableEnd();
}


void zooWiggleUi(struct trackDb *tdb )
/* put up UI for zoo track with one species on each line
 * and checkboxes to toggle each of them on/off*/
{

char options[7][256];
int thisHeightPer, thisLineGap;
float thisMinYRange, thisMaxYRange;
char *interpolate, *fill;

char **row;
int rowOffset;
struct sample *sample;
struct sqlResult *sr;

char option[64];
struct sqlConnection *conn = hAllocConn();

char newRow = 0;

snprintf( &options[0][0], 256, "%s.heightPer", tdb->tableName );
snprintf( &options[1][0], 256, "%s.linear.interp", tdb->tableName );
snprintf( &options[3][0], 256, "%s.fill", tdb->tableName );
snprintf( &options[4][0], 256, "%s.min.cutoff", tdb->tableName );
snprintf( &options[5][0], 256, "%s.max.cutoff", tdb->tableName );
snprintf( &options[6][0], 256, "%s.interp.gap", tdb->tableName );

thisHeightPer = atoi(cartUsualString(cart, &options[0][0], "50"));
interpolate = cartUsualString(cart, &options[1][0], "Linear Interpolation");
fill = cartUsualString(cart, &options[3][0], "1");
thisMinYRange = atof(cartUsualString(cart, &options[4][0], "0.0"));
thisMaxYRange = atof(cartUsualString(cart, &options[5][0], "1000.0"));
thisLineGap = atoi(cartUsualString(cart, &options[6][0], "200"));

printf("<p><b>Interpolation: </b> ");
wiggleDropDown(&options[1][0], interpolate );
printf(" ");

printf("<br><br>");
printf(" <b>Fill Blocks</b>: ");
cgiMakeRadioButton(&options[3][0], "1", sameString(fill, "1"));
printf(" on ");

cgiMakeRadioButton(&options[3][0], "0", sameString(fill, "0"));
printf(" off ");

printf("<p><b>Track Height</b>:&nbsp;&nbsp;");
cgiMakeIntVar(&options[0][0], thisHeightPer, 5 );
printf("&nbsp;pixels");

printf("<p><b>Vertical Range</b>:&nbsp;&nbsp;\nmin:");
cgiMakeDoubleVar(&options[4][0], thisMinYRange, 6 );
printf("&nbsp;&nbsp;&nbsp;&nbsp;max:");
cgiMakeDoubleVar(&options[5][0], thisMaxYRange, 6 );

printf("<p><b>Toggle Species on/off</b><br>" );
sr = hRangeQuery(conn, tdb->tableName, chromosome, 0, 1877426, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    sample = sampleLoad(row + rowOffset);
    snprintf( option, sizeof(option), "zooSpecies.%s", sample->name );
    if( cartUsualBoolean(cart, option, TRUE ) )
	cgiMakeCheckBox(option, TRUE );
    else 
	cgiMakeCheckBox(option, FALSE );
    printf("%s&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;", sample->name );

    newRow++;
    if( newRow % 6 == 0 ) printf("<br>");
		    
    sampleFree(&sample);
    }

}

void chainColorUi(struct trackDb *tdb)
/* UI for the chain tracks */
{
if (chainDbNormScoreAvailable(chromosome, tdb->tableName, NULL))
    {
    char options[1][256];	/*	our option strings here	*/
    char *colorOpt;
    (void) chainFetchColorOption(tdb, &colorOpt);
    snprintf( &options[0][0], 256, "%s.%s", tdb->tableName, OPT_CHROM_COLORS );
    printf("<p><b>Color chains by:&nbsp;</b>");
    chainColorDropDown(&options[0][0], colorOpt);

    freeMem (colorOpt);
    filterByChrom(tdb);
    }
else
    crossSpeciesUi(tdb);

}

void wigUi(struct trackDb *tdb)
/* UI for the wiggle track */
{
char *typeLine = NULL;	/*	to parse the trackDb type line	*/
char *words[8];		/*	to parse the trackDb type line	*/
int wordCount = 0;	/*	to parse the trackDb type line	*/
char options[14][256];	/*	our option strings here	*/
double minY;		/*	from trackDb or cart	*/
double maxY;		/*	from trackDb or cart	*/
double tDbMinY;		/*	data range limits from trackDb type line */
double tDbMaxY;		/*	data range limits from trackDb type line */
int defaultHeight;	/*	pixels per item	*/
char *horizontalGrid = NULL;	/*	Grid lines, off by default */
char *lineBar;	/*	Line or Bar graph */
char *autoScale;	/*	Auto scaling on or off */
char *windowingFunction;	/*	Maximum, Mean, or Minimum */
char *smoothingWindow;	/*	OFF or [2:16] */
char *yLineMarkOnOff;	/*	user defined Y marker line to draw */
double yLineMark;		/*	from trackDb or cart	*/
int maxHeightPixels = atoi(DEFAULT_HEIGHT_PER);
int minHeightPixels = MIN_HEIGHT_PER;

typeLine = cloneString(tdb->type);
wordCount = chopLine(typeLine,words);

snprintf( &options[0][0], 256, "%s.%s", tdb->tableName, HEIGHTPER );
snprintf( &options[4][0], 256, "%s.%s", tdb->tableName, MIN_Y );
snprintf( &options[5][0], 256, "%s.%s", tdb->tableName, MAX_Y );
snprintf( &options[7][0], 256, "%s.%s", tdb->tableName, HORIZGRID );
snprintf( &options[8][0], 256, "%s.%s", tdb->tableName, LINEBAR );
snprintf( &options[9][0], 256, "%s.%s", tdb->tableName, AUTOSCALE );
snprintf( &options[10][0], 256, "%s.%s", tdb->tableName, WINDOWINGFUNCTION );
snprintf( &options[11][0], 256, "%s.%s", tdb->tableName, SMOOTHINGWINDOW );
snprintf( &options[12][0], 256, "%s.%s", tdb->tableName, YLINEMARK );
snprintf( &options[13][0], 256, "%s.%s", tdb->tableName, YLINEONOFF );

wigFetchMinMaxPixels(tdb, &minHeightPixels, &maxHeightPixels, &defaultHeight);
wigFetchMinMaxY(tdb, &minY, &maxY, &tDbMinY, &tDbMaxY, wordCount, words);
(void) wigFetchHorizontalGrid(tdb, &horizontalGrid);
(void) wigFetchAutoScale(tdb, &autoScale);
(void) wigFetchGraphType(tdb, &lineBar);
(void) wigFetchWindowingFunction(tdb, &windowingFunction);
(void) wigFetchSmoothingWindow(tdb, &smoothingWindow);
(void) wigFetchYLineMark(tdb, &yLineMarkOnOff);
wigFetchYLineMarkValue(tdb, &yLineMark);

puts("<A HREF=\"/goldenPath/help/hgWiggleTrackHelp.html\" TARGET=_blank>"
     "Graph configuration help</A>");
printf("<TABLE BORDER=0><TR><TD ALIGN=LEFT>\n");

printf("<b>Type of graph:&nbsp;</b>");
wiggleGraphDropDown(&options[8][0], lineBar);
printf("</TD><TD ALIGN=LEFT>\n");

printf("<b>y&nbsp;=&nbsp;0.0 line:&nbsp;</b>");
wiggleGridDropDown(&options[7][0], horizontalGrid);
printf(" </TD></TR><TR><TD ALIGN=LEFT COLSPAN=2>\n");

printf("<b>Track height:&nbsp;</b>");
cgiMakeIntVar(&options[0][0], defaultHeight, 5);
printf("&nbsp;pixels&nbsp;(range:&nbsp;%d-%d)",
	minHeightPixels, maxHeightPixels);
printf("</TD></TR><TR><TD ALIGN=LEFT COLSPAN=2>\n");

printf("<b>Vertical viewing range</b>:&nbsp;&nbsp;\n<b>min:&nbsp;</b>");
cgiMakeDoubleVar(&options[4][0], minY, 6);
printf("&nbsp;&nbsp;&nbsp;&nbsp;<b>max:&nbsp;</b>");
cgiMakeDoubleVar(&options[5][0], maxY, 6);
printf("<BR>\n&nbsp; &nbsp;(viewing range limits:&nbsp;min:&nbsp;%g&nbsp;&nbsp;max:&nbsp;%g)",
    tDbMinY, tDbMaxY);
printf("<BR>\n<b>Data view scaling:&nbsp;</b>");
wiggleScaleDropDown(&options[9][0], autoScale);
printf("<BR>\n<b>Windowing function:&nbsp;</b>");
wiggleWindowingDropDown(&options[10][0], windowingFunction);
printf("<BR>\n<b>Smoothing window:&nbsp;</b>");
wiggleSmoothingDropDown(&options[11][0], smoothingWindow);
printf("&nbsp;pixels");
printf("<BR>\n<b>Draw indicator line at y&nbsp;=&nbsp;</b>&nbsp;\n");
cgiMakeDoubleVar(&options[12][0], yLineMark, 6);
printf("&nbsp;&nbsp;");
wiggleYLineMarkDropDown(&options[13][0], yLineMarkOnOff);
printf("</TD></TR></TABLE>\n");

freeMem(typeLine);

}

void rulerUi(struct trackDb *tdb)
/* UI for base position (ruler) */
{
/* Configure zoom when click occurs */
char *currentZoom = cartCgiUsualString(cart, RULER_BASE_ZOOM_VAR, ZOOM_3X);
char *motifString = cartCgiUsualString(cart, "hgt.motifs", "");
puts("<P><B>Zoom in:&nbsp;</B>");
zoomRadioButtons(RULER_BASE_ZOOM_VAR, currentZoom);
puts("<P><B>Motifs to highlight:&nbsp;</B>");
cgiMakeTextVar("hgt.motifs", motifString, 20);
puts("&nbsp;(Comma separated list, i.e.: GT,AG for splice sites)");
}

void oligoMatchUi(struct trackDb *tdb)
/* UI for oligo match track */
{
char *oligo = cartUsualString(cart, oligoMatchVar, oligoMatchDefault);
puts("<P><B>Short (2-30 base) sequence:</B>");
cgiMakeTextVar(oligoMatchVar, oligo, 45);
}

struct wigMafSpecies 
    {
    struct wigMafSpecies *next;
    char *name;
    int group;
    };

void wigMafUi(struct trackDb *tdb)
/* UI for maf/wiggle track
 * NOTE: calls wigUi */
{
char *speciesOrder = trackDbSetting(tdb, SPECIES_ORDER_VAR);
char *speciesGroup = trackDbSetting(tdb, SPECIES_GROUP_VAR);
char *species[100];
char *groups[20];
char sGroup[24];
char *treeImage = NULL;
struct wigMafSpecies *wmSpecies, *wmSpeciesList = NULL;
int group, prevGroup;
int speciesCt = 0, groupCt = 1;
int i;
char option[64];

if (speciesOrder == NULL && speciesGroup == NULL)
    errAbort(
      "Track %s missing required trackDb setting: speciesOrder or speciesGroup",
                tdb->tableName);
if (speciesGroup)
    groupCt = chopLine(speciesGroup, groups);

for (group = 0; group < groupCt; group++)
    {
    if (groupCt != 1 || !speciesOrder)
        {
        safef(sGroup, sizeof sGroup, "%s%s", 
                                SPECIES_GROUP_PREFIX, groups[group]);
        speciesOrder = trackDbRequiredSetting(tdb, sGroup);
        }
    speciesCt = chopLine(speciesOrder, species);
    for (i = 0; i < speciesCt; i++)
        {
        AllocVar(wmSpecies);
        wmSpecies->name = cloneString(species[i]);
        wmSpecies->group = group;
        slAddHead(&wmSpeciesList, wmSpecies);
        }
    }
slReverse(&wmSpeciesList);

//#define CODON_HIGHLIGHT
#ifdef CODON_HIGHLIGHT
char *currentCodonMode;
#endif

puts("<P><B>Pairwise alignments:</B><BR>" );
treeImage = trackDbSetting(tdb, "treeImage");
if (treeImage)
    printf("<IMG ALIGN=right SRC=\"/images/%s\">", treeImage);
if (groupCt == 1)
    puts("<TABLE><TR>");
group = -1;
for (wmSpecies = wmSpeciesList, i = 0; wmSpecies != NULL; 
                wmSpecies = wmSpecies->next, i++)
    {
    char *label;
    prevGroup = group;
    group = wmSpecies->group;
    if (groupCt != 1 && group != prevGroup)
        {
        i = 0;
        if (group != 0)
            puts("</TR></TABLE>");
        printf("<P>&nbsp;&nbsp;<B><EM>%s</EM></B>", groups[group]);
        puts("<TABLE><TR>");
        }
    if (i != 0 && (i % 6) == 0)
        puts("</TR><TR>");
    puts("<TD>");
    safef(option, sizeof(option), "%s.%s", tdb->tableName, wmSpecies->name);
    cgiMakeCheckBox(option, cartUsualBoolean(cart, option, TRUE));
    label = hOrganism(wmSpecies->name);
    if (label == NULL)
        label = wmSpecies->name;
    *label = tolower(*label);
    printf ("%s<BR>", label);
    puts("</TD>");
    }
puts("</TR></TABLE><BR>");

puts("<P><B>Multiple alignment base-level:</B><BR>" );
safef(option, sizeof option, "%s.%s", tdb->tableName, MAF_DOT_VAR);
cgiMakeCheckBox(option, cartCgiUsualBoolean(cart, option, FALSE));
puts("Display bases identical to reference as dots<BR>" );
safef(option, sizeof option, "%s.%s", tdb->tableName, MAF_CHAIN_VAR);
cgiMakeCheckBox(option, cartCgiUsualBoolean(cart, option, FALSE));
puts("Display unaligned bases with spanning chain as o's" );

puts("<P><B>Codon highlighting:</B></P>" );

#ifdef GENE_FRAMING

safef(option, sizeof(option), "%s.%s", tdb->tableName, MAF_FRAME_VAR);
currentCodonMode = cartCgiUsualString(cart, option, MAF_FRAME_GENE);

/* Disable codon highlighting */
cgiMakeRadioButton(option, MAF_FRAME_NONE, 
                sameString(MAF_FRAME_NONE, currentCodonMode));
puts("None &nbsp;");

/* Use gene pred */
cgiMakeRadioButton(option, MAF_FRAME_GENE, 
                        sameString(MAF_FRAME_GENE, currentCodonMode));
puts("CDS-annotated frame based on");
safef(option, sizeof(option), "%s.%s", tdb->tableName, MAF_GENEPRED_VAR);
genePredDropDown(cart, makeTrackHash(database, chromosome), NULL, option);

#else
snprintf(option, sizeof(option), "%s.%s", tdb->tableName, BASE_COLORS_VAR);
puts ("&nbsp; Alternate colors every");
cgiMakeIntVar(option, cartCgiUsualInt(cart, option, 0), 1);
puts ("bases<BR>");
snprintf(option, sizeof(option), "%s.%s", tdb->tableName, 
                        BASE_COLORS_OFFSET_VAR);
puts ("&nbsp; Offset alternate colors by");
cgiMakeIntVar(option, cartCgiUsualInt(cart, option, 0), 1);
puts ("bases<BR>");
#endif


puts("<P><B>Conservation graph:</B>" );
wigUi(tdb);
}

void genericWiggleUi(struct trackDb *tdb, int optionNum )
/* put up UI for any standard wiggle track (a.k.a. sample track)*/
{

	char options[7][256];
	int thisHeightPer, thisLineGap;
	float thisMinYRange, thisMaxYRange;
	char *interpolate, *fill;

snprintf( &options[0][0], 256, "%s.heightPer", tdb->tableName );
snprintf( &options[1][0], 256, "%s.linear.interp", tdb->tableName );
snprintf( &options[3][0], 256, "%s.fill", tdb->tableName );
snprintf( &options[4][0], 256, "%s.min.cutoff", tdb->tableName );
snprintf( &options[5][0], 256, "%s.max.cutoff", tdb->tableName );
snprintf( &options[6][0], 256, "%s.interp.gap", tdb->tableName );

thisHeightPer = atoi(cartUsualString(cart, &options[0][0], "50"));
interpolate = cartUsualString(cart, &options[1][0], "Linear Interpolation");
fill = cartUsualString(cart, &options[3][0], "1");
thisMinYRange = atof(cartUsualString(cart, &options[4][0], "0.0"));
thisMaxYRange = atof(cartUsualString(cart, &options[5][0], "1000.0"));
thisLineGap = atoi(cartUsualString(cart, &options[6][0], "200"));

printf("<p><b>Interpolation: </b> ");
wiggleDropDown(&options[1][0], interpolate );
printf(" ");

printf("<br><br>");
printf(" <b>Fill Blocks</b>: ");
cgiMakeRadioButton(&options[3][0], "1", sameString(fill, "1"));
printf(" on ");

cgiMakeRadioButton(&options[3][0], "0", sameString(fill, "0"));
printf(" off ");

printf("<p><b>Track Height</b>:&nbsp;&nbsp;");
cgiMakeIntVar(&options[0][0], thisHeightPer, 5 );
printf("&nbsp;pixels");

printf("<p><b>Vertical Range</b>:&nbsp;&nbsp;\nmin:");
cgiMakeDoubleVar(&options[4][0], thisMinYRange, 6 );
printf("&nbsp;&nbsp;&nbsp;&nbsp;max:");
cgiMakeDoubleVar(&options[5][0], thisMaxYRange, 6 );

if( optionNum >= 7 )
    {
    printf("<p><b>Maximum Interval to Interpolate Across</b>:&nbsp;&nbsp;");
    cgiMakeIntVar(&options[6][0], thisLineGap, 10 );
    printf("&nbsp;bases");
    }
}



void humMusUi(struct trackDb *tdb, int optionNum )
/* put up UI for human/mouse conservation sample tracks (humMusL and musHumL)*/
{

	char options[7][256];
	int thisHeightPer, thisLineGap;
	float thisMinYRange, thisMaxYRange;
	char *interpolate, *fill;

snprintf( &options[0][0], 256, "%s.heightPer", tdb->tableName );
snprintf( &options[1][0], 256, "%s.linear.interp", tdb->tableName );
snprintf( &options[3][0], 256, "%s.fill", tdb->tableName );
snprintf( &options[4][0], 256, "%s.min.cutoff", tdb->tableName );
snprintf( &options[5][0], 256, "%s.max.cutoff", tdb->tableName );
snprintf( &options[6][0], 256, "%s.interp.gap", tdb->tableName );

thisHeightPer = atoi(cartUsualString(cart, &options[0][0], "50"));
interpolate = cartUsualString(cart, &options[1][0], "Linear Interpolation");
fill = cartUsualString(cart, &options[3][0], "1");
thisMinYRange = atof(cartUsualString(cart, &options[4][0], "0.0"));
thisMaxYRange = atof(cartUsualString(cart, &options[5][0], "8.0"));
thisLineGap = atoi(cartUsualString(cart, &options[6][0], "200"));

printf("<p><b>Interpolation: </b> ");
wiggleDropDown(&options[1][0], interpolate );
printf(" ");

printf("<br><br>");
printf(" <b>Fill Blocks</b>: ");
cgiMakeRadioButton(&options[3][0], "1", sameString(fill, "1"));
printf(" on ");

cgiMakeRadioButton(&options[3][0], "0", sameString(fill, "0"));
printf(" off ");

printf("<p><b>Track Height</b>:&nbsp;&nbsp;");
cgiMakeIntVar(&options[0][0], thisHeightPer, 5 );
printf("&nbsp;pixels");

printf("<p><b>Vertical Range</b>:&nbsp;&nbsp;\nmin:");
cgiMakeDoubleVar(&options[4][0], thisMinYRange, 6 );
printf("&nbsp;&nbsp;&nbsp;&nbsp;max:");
cgiMakeDoubleVar(&options[5][0], thisMaxYRange, 6 );

if( optionNum >= 7 )
{
	printf("<p><b>Maximum Interval to Interpolate Across</b>:&nbsp;&nbsp;");
	cgiMakeIntVar(&options[6][0], thisLineGap, 10 );
	printf("&nbsp;bases");
}
}




void affyTranscriptomeUi(struct trackDb *tdb)
	/* put up UI for the GC percent track (a sample track)*/
{
	int affyTranscriptomeHeightPer = atoi(cartUsualString(cart, "affyTranscriptome.heightPer", "100"));
	char *fill = cartUsualString(cart, "affyTranscriptome.fill", "1");

	printf("<br><br>");
	printf(" <b>Fill Blocks</b>: ");
	cgiMakeRadioButton("affyTranscriptome.fill", "1", sameString(fill, "1"));
	printf(" on ");

	cgiMakeRadioButton("affyTranscriptome.fill", "0", sameString(fill, "0"));
	printf(" off ");

	printf("<p><b>Track Height</b>:&nbsp;&nbsp;");
	cgiMakeIntVar("affyTranscriptome.heightPer", affyTranscriptomeHeightPer, 5 );
	printf("&nbsp;pixels");

}


void ancientRUi(struct trackDb *tdb)
	/* put up UI for the ancient repeats track to let user enter an
	 * integer to filter out those repeats with less aligned bases.*/
{
	int ancientRMinLength = atoi(cartUsualString(cart, "ancientR.minLength", "50"));
	printf("<p><b>Length Filter</b><br>Exclude aligned repeats with less than ");
	cgiMakeIntVar("ancientR.minLength", ancientRMinLength, 4 );
	printf("aligned bases (not necessarily identical). Enter 0 for no filtering.");
}


void affyTransfragUi(struct trackDb *tdb) 
/* Options for filtering affymetrix transfrag track based on score. */
{
boolean skipPseudos = cartUsualBoolean(cart, "affyTransfrags.skipPseudos", TRUE);
boolean skipDups = cartUsualBoolean(cart, "affyTransfrags.skipDups", FALSE);
printf("<br>");
cgiMakeCheckBox("affyTransfrags.skipPseudos", skipPseudos);
printf(" Remove transfrags that overlap pseudogenes from display.<br>");
cgiMakeCheckBox("affyTransfrags.skipDups", skipDups);
printf(" Remove transfrags that have a BLAT match elsewhere in the genome from display.<br>");
}

void specificUi(struct trackDb *tdb)
	/* Draw track specific parts of UI. */
{
char *track = tdb->tableName;

if (sameString(track, "stsMap"))
        stsMapUi(tdb);
else if (sameString(track, "stsMapMouseNew"))
        stsMapMouseUi(tdb);
else if (sameString(track, "stsMapRat"))
        stsMapRatUi(tdb);
else if (sameString(track, "snpMap"))
        snpMapUi(tdb);
else if (sameString(track, "snp"))
        snpUi(tdb);
else if (sameString(track, "cbr_waba"))
        cbrWabaUi(tdb);
else if (sameString(track, "fishClones"))
        fishClonesUi(tdb);
else if (sameString(track, "recombRate"))
        recombRateUi(tdb);
else if (sameString(track, "recombRateRat"))
        recombRateRatUi(tdb);
else if (sameString(track, "recombRateMouse"))
        recombRateMouseUi(tdb);
else if (sameString(track, "nci60"))
        nci60Ui(tdb);
else if (sameString(track, "cghNci60"))
        cghNci60Ui(tdb);
else if (sameString(track, "xenoRefGene"))
        refGeneUI(tdb);
else if (sameString(track, "refGene"))
        refGeneUI(tdb);
else if (sameString(track, "all_mrna"))
    mrnaUi(tdb, FALSE);
else if (sameString(track, "mrna"))
    mrnaUi(tdb, FALSE);
else if (sameString(track, "splicesP"))
    bedUi(tdb);
else if (sameString(track, "all_est"))
        mrnaUi(tdb, FALSE);
else if (sameString(track, "est"))
        mrnaUi(tdb, FALSE);
else if (sameString(track, "tightMrna"))
        mrnaUi(tdb, FALSE);
else if (sameString(track, "tightEst"))
        mrnaUi(tdb, FALSE);
else if (sameString(track, "intronEst"))
        mrnaUi(tdb, FALSE);
else if (sameString(track, "xenoMrna"))
        mrnaUi(tdb, TRUE);
else if (sameString(track, "xenoEst"))
        mrnaUi(tdb, TRUE);
else if (sameString(track, "rosetta"))
        rosettaUi(tdb);
else if (sameString(track, "blastDm1FB"))
        blastFBUi(tdb);
else if (sameString(track, "blastSacCer1SG"))
        blastSGUi(tdb);
else if (sameString(track, "blastHg17KG") || sameString(track, "blastHg16KG") || sameString(track, "tblastnHg16KGPep"))
        blastUi(tdb);
else if (startsWith("wig", tdb->type))
        {
        if (startsWith("wigMaf", tdb->type))
            wigMafUi(tdb);
        else
            wigUi(tdb);
        }
else if (sameString(track, "affyRatio"))
        affyUi(tdb);
else if (sameString(track, "ancientR"))
        ancientRUi(tdb);
else if (sameString(track, "zoo") || sameString(track, "zooNew" ))
         zooWiggleUi(tdb);
else if (sameString(track, "humMusL") ||
         sameString( track, "musHumL") ||
         sameString( track, "regpotent") ||
         sameString( track, "mm3Rn2L" )	 ||
         sameString( track, "mm3Hg15L" ) ||
         sameString( track, "hg15Mm3L" ))
            humMusUi(tdb,7);
/* NOTE: type psl xeno <otherDb> tracks use crossSpeciesUi, so
 * add explicitly here only if track has another type (bed, chain).
 * For crossSpeciesUi, the
 * default for chrom coloring is "on", unless track setting
 * colorChromDefault is set to "off" */
else if (endsWith("chainSelf", track))
    chainColorUi(tdb);
else if (sameString(track, "orthoTop4"))
    /* still used ?? */
    crossSpeciesUi(tdb);
else if (sameString(track, "mouseOrtho"))
    crossSpeciesUi(tdb);
else if (sameString(track, "mouseSyn"))
    crossSpeciesUi(tdb);
else if (sameString(track, "affyTranscriptome"))
    affyTranscriptomeUi(tdb);
else if (startsWith("sample", tdb->type))
    genericWiggleUi(tdb,7);
else if (sameString(track, RULER_TRACK_NAME))
    rulerUi(tdb);
else if (sameString(track, OLIGO_MATCH_TRACK_NAME))
    oligoMatchUi(tdb);
else if(sameString(track, "affyTransfrags"))
    affyTransfragUi(tdb);
else if (sameString(track, "transRegCode"))
    transRegCodeUi(tdb);
else if (tdb->type != NULL)
    {
    /* handle all tracks with type genePred or bed or "psl xeno <otherDb>" */
    char *typeLine = cloneString(tdb->type);
    char *words[8];
    int wordCount = 0;
    wordCount = chopLine(typeLine, words);
    if (wordCount > 0)
	{
	if (sameWord(words[0], "genePred"))
	    cdsColorOptions(tdb, 2);
	/* if bed has score then show optional filter based on score */
	if (sameWord(words[0], "bed") && wordCount == 3)
	    {
	    if (atoi(words[1]) > 4)
		scoreUi(tdb);
	    }
	else if (sameWord(words[0], "psl"))
	    {
	    if (wordCount == 3)
		if (sameWord(words[1], "xeno"))
		    crossSpeciesUi(tdb);
	    cdsColorOptions(tdb, -1);
	    }
	}
    freeMem(typeLine);
    }
}

void trackUi(struct trackDb *tdb)
/* Put up track-specific user interface. */
{
char *vis = hStringFromTv(tdb->visibility);
printf("<H1>%s</H1>\n", tdb->longLabel);
printf("<B>Display mode:</B>");
hTvDropDown(tdb->tableName, 
    hTvFromString(cartUsualString(cart,tdb->tableName, vis)), tdb->canPack);
printf(" ");
cgiMakeButton("Submit", "Submit");
printf("<BR>\n");

specificUi(tdb);

if (tdb->html != NULL && tdb->html[0] != 0)
    {
    htmlHorizontalLine();
    puts(tdb->html);
    }
}

struct trackDb *trackDbForPseudoTrack(char *tableName, char *shortLabel, 
	char *longLabel, int defaultVis, boolean canPack)
/* Create trackDb for a track without a corresponding table. */
{
struct trackDb *tdb;
char htmlFile[256];
char *buf;
size_t size;

AllocVar(tdb);
tdb->tableName = tableName;
tdb->shortLabel = shortLabel;
tdb->longLabel = longLabel;
tdb->visibility = defaultVis;
tdb->priority = 1.0;
safef(htmlFile, 256, "%s/%s.html", HELP_DIR, tableName);
readInGulp(htmlFile, &buf, &size);
tdb->html = buf;
tdb->type = "none";
tdb->grp = "map";
tdb->canPack = canPack;
return tdb;
}

struct trackDb *trackDbForRuler()
/* Create a trackDb entry for the base position ruler.
   It is not (yet?) a real track, so doesn't appear in trackDb */
{
return trackDbForPseudoTrack(RULER_TRACK_NAME,
	RULER_TRACK_LABEL, RULER_TRACK_LONGLABEL, tvPack, FALSE);
}

struct trackDb *trackDbForOligoMatch()
/* Create a trackDb entry for the oligo matcher pseudo-track. */
{
return trackDbForPseudoTrack(OLIGO_MATCH_TRACK_NAME,
	OLIGO_MATCH_TRACK_LABEL, OLIGO_MATCH_TRACK_LONGLABEL, tvHide, TRUE);
}

void doMiddle(struct cart *theCart)
/* Write body of web page. */
{
struct trackDb *tdb;
char *track;
cart = theCart;
track = cartString(cart, "g");
database = cartUsualString(cart, "db", hGetDb());
hSetDb(database);
chromosome = cartUsualString(cart, "c", hDefaultChrom());
if (sameWord(track, RULER_TRACK_NAME))
    /* special handling -- it's not a full-fledged track */
    tdb = trackDbForRuler();
else if (sameWord(track, OLIGO_MATCH_TRACK_NAME))
    tdb = trackDbForOligoMatch();
else
    tdb = hTrackDbForTrack(track);
if (tdb == NULL)
   errAbort("Can't find %s in track database %s chromosome %s",
	    track, database, chromosome);
cartWebStart(cart, "%s Track Settings", tdb->shortLabel);
printf("<FORM ACTION=\"%s\">\n\n", hgTracksName());
cartSaveSession(cart);
trackUi(tdb);
printf("</FORM>");
webEnd();
}

char *excludeVars[] = { "submit", "Submit", "g", NULL,};

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
htmlSetBackground("../images/floret.jpg");
cartEmptyShell(doMiddle, hUserCookie(), excludeVars, NULL);
return 0;
}
