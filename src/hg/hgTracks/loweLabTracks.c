#include "common.h"
#include "hash.h"
#include "localmem.h"
#include "linefile.h"
#include "jksql.h"
#include "hdb.h"
#include "bed.h"
#include "psl.h"
#include "codeBlast.h"
#include "cogs.h"
#include "rnaGenes.h"
#include "hgTracks.h"
#include "expRatioTracks.h"
#include "loweLabTracks.h"


/* Declare our color gradients and the the number of colors in them */
#define LL_EXPR_DATA_SHADES 16
Color LLshadesOfGreen[LL_EXPR_DATA_SHADES];
Color LLshadesOfRed[LL_EXPR_DATA_SHADES];
boolean LLexprBedColorsMade = FALSE; /* Have the shades of Green, Red, and Blue been allocated? */
int LLmaxRGBShade = LL_EXPR_DATA_SHADES - 1;

#define LL_COG_SHADES 26
Color LLshadesOfCOGS[LL_COG_SHADES];
/**** Lowe lab additions ***/

void initializeColors(struct vGfx *vg)
    {
    LLshadesOfCOGS['J'-'A']=vgFindColorIx(vg, 252, 204,252);
    LLshadesOfCOGS['A'-'A']=vgFindColorIx(vg, 252, 220,252);
    LLshadesOfCOGS['K'-'A']=vgFindColorIx(vg, 252, 220,236);
    LLshadesOfCOGS['L'-'A']=vgFindColorIx(vg, 252, 220,220);
    LLshadesOfCOGS['B'-'A']=vgFindColorIx(vg, 252, 220,204);
    LLshadesOfCOGS['D'-'A']=vgFindColorIx(vg, 252, 252,220);
    LLshadesOfCOGS['Y'-'A']=vgFindColorIx(vg, 252, 252,204);
    LLshadesOfCOGS['V'-'A']=vgFindColorIx(vg, 252, 252,188);
    LLshadesOfCOGS['T'-'A']=vgFindColorIx(vg, 252, 252,172);
    LLshadesOfCOGS['M'-'A']=vgFindColorIx(vg, 236, 252,172);
    LLshadesOfCOGS['N'-'A']=vgFindColorIx(vg, 220, 252,172);
    LLshadesOfCOGS['Z'-'A']=vgFindColorIx(vg, 204, 252,172);
    LLshadesOfCOGS['W'-'A']=vgFindColorIx(vg, 188, 252,172);
    LLshadesOfCOGS['U'-'A']=vgFindColorIx(vg, 172, 252,172);
    LLshadesOfCOGS['O'-'A']=vgFindColorIx(vg, 156, 252,172);
    LLshadesOfCOGS['C'-'A']=vgFindColorIx(vg, 188, 252,252);
    LLshadesOfCOGS['G'-'A']=vgFindColorIx(vg, 204, 252,252);
    LLshadesOfCOGS['E'-'A']=vgFindColorIx(vg, 220, 252,252);
    LLshadesOfCOGS['F'-'A']=vgFindColorIx(vg, 220, 236,252);
    LLshadesOfCOGS['H'-'A']=vgFindColorIx(vg, 220, 220,252);
    LLshadesOfCOGS['I'-'A']=vgFindColorIx(vg, 220, 204,252);
    LLshadesOfCOGS['P'-'A']=vgFindColorIx(vg, 204, 204,252);
    LLshadesOfCOGS['Q'-'A']=vgFindColorIx(vg, 188, 204,252);
    LLshadesOfCOGS['R'-'A']=vgFindColorIx(vg, 224, 224,224);
    LLshadesOfCOGS['S'-'A']=vgFindColorIx(vg, 204, 204,204);
    }

void loadBed6(struct track *tg)
/* Load the items in one custom track - just move beds in
 * window... */
{
struct bed *bed, *list = NULL;
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
int rowOffset;

sr = hRangeQuery(conn, tg->mapName, chromName, winStart, winEnd, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    bed = bedLoadN(row+rowOffset, 6);
    slAddHead(&list, bed);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&list);
tg->items = list;
}

Color gbGeneColor(struct track *tg, void *item, struct vGfx *vg)
/* Return color to draw gene in. */
{
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char query[512];
struct bed *bed = item;
struct COG *COG=NULL;
char *temparray[160];
char **row;

if(hTableExists("COG")){
    sprintf(query, "select * from COG where name = '%s'", bed->name);
    sr = sqlGetResult(conn, query);
    if ((row = sqlNextRow(sr)) != NULL)
    	{
   	    COG = COGLoad(row);
  	}
    sqlFreeResult(&sr);
    hFreeConn(&conn);
    initializeColors(vg);
    if(COG!=NULL){
        chopString(COG->code, "," , temparray, 9999);
        return LLshadesOfCOGS[(temparray[0][0]-'A')];
    }
    else
        {
        return shadesOfGray[9];
        }
    }
else
    {
    return shadesOfGray[9];
    }
}

void gbGeneMethods(struct track *tg)
/* Track group for genbank gene tracks */
{
tg->loadItems = loadBed6;
tg->itemColor = gbGeneColor;
}

Color sargassoSeaGeneColor(struct track *tg, void *item, struct vGfx *vg)
/* Return color to draw gene in. */
{
struct bed *lf=item;
if (lf->score > 990)
    return shadesOfGray[10];
else if (lf->score > 850)
    return shadesOfGray[9];
else if (lf->score > 700)
    return shadesOfGray[8];
else if (lf->score > 550)
    return shadesOfGray[7];
else if (lf->score > 450)
    return shadesOfGray[6];
else if (lf->score > 300)
    return shadesOfGray[5];
else if (lf->score > 200)
    return shadesOfGray[4];
else if (lf->score > 100)
    return shadesOfGray[3];
else return shadesOfGray[2];

}


void sargassoSeaMethods(struct track *tg)
/* Track group for genbank gene tracks */
{
tg->loadItems = loadBed6;
tg->itemColor = sargassoSeaGeneColor;
}

Color tigrGeneColor(struct track *tg, void *item, struct vGfx *vg)
/* Return color to draw gene in. */
{
struct bed *bed = item;
if (bed->strand[0] == '+')
    return tg->ixColor;
return tg->ixAltColor;
}

void tigrGeneMethods(struct track *tg)
/* Track group for genbank gene tracks */
{
tg->loadItems = loadBed6;
tg->itemColor = tigrGeneColor;
}

char *llBlastPName(struct track *tg, void *item)
{
struct bed *bed = item;
char *itemName = cloneString(bed->name);
static char buf[256];
char *nameParts[2];
chopByChar(itemName,'|',nameParts,ArraySize(nameParts));
sprintf(buf, "%s", nameParts[0]);
chopByChar(buf,'$',nameParts,ArraySize(nameParts));
sprintf(buf,"%s",nameParts[1]);
freeMem(itemName);
return buf;
}

void llBlastPMethods(struct track *tg)
{
tg->itemName = llBlastPName;
}

struct linkedFeatures *lfsToLf(struct linkedFeaturesSeries *lfs)
/* convert a linked feature into a linked feature series */
{
struct linkedFeatures *lf = NULL;
struct simpleFeature *sf = NULL;
AllocVar(lf);
safef(lf->name,64,"%s",lfs->name);
lf->start = lfs->start;
lf->end = lfs->end;
lf->tallStart = lfs->start;
lf->tallEnd = lfs->end;
lf->grayIx = lfs->grayIx;
lf->orientation = lfs->orientation;
lf->extra = cloneString(lfs->features->extra);
AllocVar(sf);
sf->start = lfs->start;
sf->end = lfs->end;
sf->grayIx = lfs->grayIx;
lf->components = sf;
return lf;
}

struct linkedFeatures *lfFromBed6(struct codeBlast *bed, int scoreMin, 
	int scoreMax)
/* Return a linked feature from a (full) bed. */
{
struct linkedFeatures *lf;
struct simpleFeature *sf, *sfList = NULL;
int grayIx = grayInRange(bed->score, scoreMin, scoreMax);
AllocVar(lf);
lf->grayIx = grayIx;
strncpy(lf->name, bed->name, sizeof(lf->name));
lf->orientation = orientFromChar(bed->strand[0]);
AllocVar(sf);
sf->start = bed->chromStart;
sf->end = bed->chromEnd;
sf->grayIx = grayIx;
lf->components = sf;
linkedFeaturesBoundsAndGrays(lf);
lf->tallStart = bed->chromStart;
lf->tallEnd = bed->chromEnd;
return lf;
}


void loadCodeBlast(struct track *tg)
/* from the bed 6+1 codeBlast table, make a linkedFeaturesSeries and load it.  */
{
struct linkedFeaturesSeries *lfs = NULL, *originalLfs, *codeLfs, *lfsList = NULL;
struct linkedFeatures *lf;
struct slName *codes = NULL, *code, *track=NULL, *tracks, *scores=NULL, *otherscores;
struct codeBlast *bed,  *bedList;
struct codeBlast *cb, *list=NULL;
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char  *score;

char **temparray3;
char *temparray[16];
char *temparray2;
char query[256];
char *temp="a";
char **row;
char *tempstring;
int rowOffset;
int length, x, y, z;
char codeList[18] = {'g', 'z', 'c','e','o','b','v','u', 'y', 't','h','d','k','l', 'm','a','n','r'};
char *codeNames[18] = {"within genus", "\t", "crenarchaea","euryarchaea","\t","bacteria", "\t", "eukarya","\t","thermophile","hyperthermophile","acidophile","alkaliphile", "halophile","methanogen","strict aerobe","strict anaerobe", "anerobe or aerobe"}; int i;
 
sprintf(query, "select * from %s where chromStart > %i AND chromEnd < %i", tg->mapName, winStart,winEnd);
sr = sqlGetResult(conn, query);

while ((row = sqlNextRow(sr)) != NULL)
    {
    char *thecode = NULL;
    struct slName *tmp = NULL;
    cb = codeBlastLoad(row);
    slAddHead(&list, cb);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&list);

if(list == NULL)
    return;

for(cb = list; cb != NULL; cb = cb->next)
    {
    AllocVar(lfs);
    AllocVar(lf);
    lfs->name = cloneString(cb->name);
    lf = lfFromBed6(cb,0,1000);
   
    lf->score = cb->score;
    tempstring=cloneString(cb->code);
 
    chopString(tempstring, "," , temparray, ArraySize(temparray));
    temparray3=(char**)calloc(18*8,sizeof(char**));
    for(x=0; x<18; x++){
        temparray3[x]=(char *)calloc(256, sizeof(char*));
	//Fix to cloneString problem when both patricia and my track
	//was showing at the same time	
	if(temparray[x]!=NULL){
	    if(atoi(temparray[x])==1000){
		temparray3[x]="1000";
	    }
	    else if(atoi(temparray[x])==900){
		temparray3[x]="900";
	    }
	    else if(atoi(temparray[x])==800){
		temparray3[x]="800";
            }
	    else if(atoi(temparray[x])==700){
		temparray3[x]="700";
	    }
	    else if(atoi(temparray[x])==600){
		temparray3[x]="600";
	    }
	    else if(atoi(temparray[x])==500){
		temparray3[x]="500";
	    }
	    else if(atoi(temparray[x])==400){
		temparray3[x]="400";
	    }
	    else if(atoi(temparray[x])==300){
		temparray3[x]="300";
	    }
	    else if(atoi(temparray[x])==200){
		temparray3[x]="200";
	    }
	    else if(atoi(temparray[x])==100){
		temparray3[x]="100";
	    }
	    else{
		temparray3[x]="0";
 	    }
        }
	
    }
    lf->extra = temparray3;
    
    lfs->start = lf->start;
    lfs->end = lf->end;
    lfs->features= lf;  
    slAddHead(&lfsList, lfs);
   
 
   }

tg->items=lfsList;
bedList=tg->items;
lfsList=NULL;

if(tg->limitedVis != tvDense)
    {

    originalLfs = tg->items;
    for (i = 0; i < 18; i++)
        {
	struct linkedFeatures *lfList = NULL;
        AllocVar(codeLfs);
        codeLfs->name = cloneString(codeNames[i]);
        codeLfs->noLine = TRUE;
	for (lfs = originalLfs; lfs != NULL; lfs = lfs->next)
            {
	    lf = lfsToLf(lfs);
            temparray2=((char**)(lfs->features->extra))[i];
	    if (i!=1 && i!=4 && i!=6 && i!=8 && atoi(temparray2)!=-9999)
                {
		lf->score=atoi(temparray2);
                slAddHead(&lfList,lf);

		}
            }
        slReverse(&lfList);
        codeLfs->features = lfList;   
        slAddHead(&lfsList,codeLfs);
        }
    freeLinkedFeaturesSeries(&originalLfs);
    slReverse(&lfsList);
    tg->items=lfsList;
    }

slFreeList(&track);

slFreeList(&scores);
//slFreeList(&tg);

slFreeList(&codes);
codeBlastFree(&list);
}

Color cbGeneColor(struct track *tg, void *item, struct vGfx *vg)
/* Return color to draw gene in. */
{
struct linkedFeatures *lf=item;
if (lf->score > 990)
    return shadesOfGray[9];
else if (lf->score > 850)
    return shadesOfGray[8];
else if (lf->score > 700)
    return shadesOfGray[7];
else if (lf->score > 550)
    return shadesOfGray[6];
else if (lf->score > 450)
    return shadesOfGray[5];
else if (lf->score > 300)
    return shadesOfGray[4];
else if (lf->score > 200)
    return shadesOfGray[3];
else if (lf->score > 100)
    return shadesOfGray[3];
else return shadesOfGray[2];

}

void codeBlastMethods(struct track *tg)
{

linkedFeaturesSeriesMethods(tg);
tg->loadItems = loadCodeBlast;
tg->mapItem = lfsMapItemName;
tg->itemColor=cbGeneColor;
tg->mapsSelf = TRUE;
}

Color rgGeneColor(struct track *tg, void *item, struct vGfx *vg)
/* Return color to draw gene in. */
{
struct rnaGenes *lf=item;
makeRedGreenShades(vg);
if (lf->score ==100) {return shadesOfGreen[15];}
if (lf->score == 300) {return shadesOfRed[15];}
if (lf->score == 200){return shadesOfBlue[15];}
else {return shadesOfGray[9];}
}
void rnaGenesMethods(struct track *tg)
{
tg->itemColor=rgGeneColor;
}

void loadOperon(struct track *tg)
/* Load the items in one custom track - just move beds in
 * window... */
{
struct linkedFeatures *lfList = NULL, *lf;
struct bed *bed, *list = NULL;
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
int rowOffset;

sr = hRangeQuery(conn, tg->mapName, chromName, winStart, winEnd, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    bed = bedLoadN(row+rowOffset, 15);
    slAddHead(&list, bed);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&list);

for (bed = list; bed != NULL; bed = bed->next)
    {
    struct simpleFeature *sf;
    int i;
    lf = lfFromBed(bed);
    for (sf = lf->components, i = 0; sf != NULL, i < bed->expCount; sf = sf->next, i++)
	sf->grayIx = grayInRange((int)(bed->expScores[i]),0,1000);
    slAddHead(&lfList,lf);
    }
tg->items = lfList;
}

void tigrOperonDrawAt(struct track *tg, void *item,
	struct vGfx *vg, int xOff, int y, double scale, 
	MgFont *font, Color color, enum trackVisibility vis)
/* Draw the operon at position. */
{
struct linkedFeatures *lf = item; 
struct simpleFeature *sf;
int heightPer = tg->heightPer;
int x1,x2;
int s, e, e2, s2;
Color *shades = tg->colorShades;
int midY = y + (heightPer>>1);
int midY1 = midY - (heightPer>>2);
int midY2 = midY + (heightPer>>2);
int w;

color = tg->ixColor;
x1 = round((double)((int)lf->start-winStart)*scale) + xOff;
x2 = round((double)((int)lf->end-winStart)*scale) + xOff;
w = x2-x1;
innerLine(vg, x1, midY, w, color);
if (vis == tvFull || vis == tvPack)
    {
    clippedBarbs(vg, x1, midY, w, 2, 5, 
		 lf->orientation, color, FALSE);
    }
for (sf = lf->components; sf != NULL; sf = sf->next)
    {
    s = sf->start; e = sf->end;
    /* shade ORF (exon) based on the grayIx value of the sf */
    color = shades[sf->grayIx];
    drawScaledBox(vg, s, e, scale, xOff, y, heightPer,
			color );
    }
}

void tigrOperonMethods(struct track *tg)
{
linkedFeaturesMethods(tg);
tg->loadItems = loadOperon;
tg->colorShades = shadesOfGray;
tg->drawItemAt = tigrOperonDrawAt;
}

/**** End of Lowe lab additions ****/
