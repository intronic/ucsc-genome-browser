/* hgGenome - Full genome (as opposed to chromosome) view of data. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "cheapcgi.h"
#include "htmshell.h"
#include "cart.h"
#include "hui.h"
#include "dbDb.h"
#include "hdb.h"
#include "web.h"
#include "portable.h"
#include "hgColors.h"
#include "trackLayout.h"
#include "chromInfo.h"
#include "vGfx.h"
#include "genoLay.h"
#include "cytoBand.h"
#include "hCytoBand.h"
#include "hgGenome.h"

static char const rcsid[] = "$Id: hgGenome.c,v 1.8 2006/06/23 23:45:05 kent Exp $";

/* ---- Global variables. ---- */
struct cart *cart;	/* This holds cgi and other variables between clicks. */
struct hash *oldCart;	/* Old cart hash. */
char *database;		/* Name of genome database - hg15, mm3, or the like. */
char *genome;		/* Name of genome - mouse, human, etc. */
struct trackLayout tl;  /* Dimensions of things, fonts, etc. */


void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgGenome - Full genome (as opposed to chromosome) view of data\n"
  "This is a cgi script, but it takes the following parameters:\n"
  "   db=<genome database>\n"
  "   hggt_table=on where table is name of chromGraph table\n"
  );
}

int genoLayChromCmpName(const void *va, const void *vb)
/* Compare two chromosome names so as to sort numerical part
 * by number.. */
{
const struct genoLayChrom *a = *((struct genoLayChrom **)va);
const struct genoLayChrom *b = *((struct genoLayChrom **)vb);
char *aName = a->shortName, *bName = b->shortName;
if (isdigit(aName[0]))
    {
    if (isdigit(bName[0]))
	{
	int diff = atoi(aName) - atoi(bName);
	if (diff == 0)
	    diff = strcmp(skipNumeric(aName), skipNumeric(bName));
	return diff;
	}
    else
        return -1;
    }
else if (isdigit(bName[0]))
    return 1;
else
    return strcmp(aName, bName);
}

struct genoLayChrom *genoLayDbChroms(struct sqlConnection *conn, 
	boolean withRandom)
/* Get chrom info list. */
{
struct sqlResult *sr;
char **row;
struct genoLayChrom *chrom, *chromList = NULL;
int count = sqlQuickNum(conn, "select count(*) from chromInfo");
if (count > 500)
    errAbort("Sorry, hgGenome only works on assemblies mapped to chromosomes.");
sr = sqlGetResult(conn, "select chrom,size from chromInfo");
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *name = row[0];
    if (withRandom || (startsWith("chr", name) && 
    	!strchr(name, '_') && !sameString("chrM", name) 
	&& !sameString("chrUn", name)))
        {
	AllocVar(chrom);
	chrom->fullName = cloneString(name);
	chrom->shortName = chrom->fullName+3;
	chrom->size = sqlUnsigned(row[1]);
	slAddHead(&chromList, chrom);
	}
    }
if (chromList == NULL)
    errAbort("No chromosomes starting with chr in chromInfo for %s.", database);
slReverse(&chromList);
slSort(&chromList, genoLayChromCmpName);
return chromList;
}

static void separateSexChroms(struct slRef *in,
	struct slRef **retAutoList, struct slRef **retSexList)
/* Separate input chromosome list into sex and non-sex chromosomes. */
{
struct slRef *autoList = NULL, *sexList = NULL, *ref, *next;

for (ref = in; ref != NULL; ref = next)
    {
    struct genoLayChrom *chrom = ref->val;
    char *name = chrom->shortName;
    next = ref->next;
    if (sameWord(name, "X") || sameWord(name, "Y") || sameWord(name, "Z")
    	|| sameWord(name, "W"))
	{
	slAddHead(&sexList, ref);
	}
    else
        {
	slAddHead(&autoList, ref);
	}
    }
slReverse(&sexList);
slReverse(&autoList);
*retAutoList = autoList;
*retSexList = sexList;
}

struct genoLay *genoLayNew(struct genoLayChrom *chromList,
	MgFont *font, int picWidth, int lineHeight,
	int minLeftLabelWidth, int minRightLabelWidth)
/* Figure out layout.  For human and most mammals this will be
 * two columns with sex chromosomes on bottom.  This is complicated
 * by the platypus having a bunch of sex chromosomes. */
{
int margin = 2;
struct slRef *refList = NULL, *ref, *left, *right;
struct genoLayChrom *chrom;
struct genoLay *gl;
int autoCount, halfCount, bases, chromInLine;
int leftLabelWidth=0, rightLabelWidth=0, labelWidth;
int spaceWidth = mgFontCharWidth(font, ' ');
int extraLabelPadding = 0;
int autosomeOtherPixels=0, sexOtherPixels=0;
int autosomeBasesInLine=0;	/* Maximum bases in a line for autosome. */
int sexBasesInLine=0;		/* Bases in line for sex chromsome. */
double sexBasesPerPixel, autosomeBasesPerPixel, basesPerPixel;
int pos = margin;
int y = 0;

refList = refListFromSlList(chromList);

/* Allocate genoLay object and fill in simple fields. */
AllocVar(gl);
gl->chromList = chromList;
gl->chromHash = hashNew(0);
gl->font = font;
gl->picWidth = picWidth;
gl->margin = margin;
gl->spaceWidth = spaceWidth;

/* Save chromosomes in hash too, for easy access */
for (chrom = chromList; chrom != NULL; chrom = chrom->next)
    hashAdd(gl->chromHash, chrom->fullName, chrom);

/* Put sex chromosomes on bottom, and rest on left. */
separateSexChroms(refList, &refList, &gl->bottomList);
autoCount = slCount(refList);
gl->leftList = refList;

/* If there are a lot of chromosomes, then move later
 * (and smaller) chromosomes to a new right column */
if (autoCount > 12)
    {
    halfCount = (autoCount+1)/2;
    ref = slElementFromIx(refList, halfCount-1);
    gl->rightList = ref->next;
    ref->next = NULL;
    slReverse(&gl->rightList);
    }

/* Figure out space needed for autosomes. */
left = gl->leftList;
right = gl->rightList;
while (left || right)
    {
    bases = 0;
    chromInLine = 0;
    if (left)
        {
	chrom = left->val;
	labelWidth = mgFontStringWidth(font, chrom->shortName) + spaceWidth;
	if (leftLabelWidth < labelWidth)
	    leftLabelWidth = labelWidth;
	bases = chrom->size;
	left = left->next;
	}
    if (right)
        {
	chrom = right->val;
	labelWidth = mgFontStringWidth(font, chrom->shortName) + spaceWidth;
	if (rightLabelWidth < labelWidth)
	    rightLabelWidth = labelWidth;
	bases += chrom->size;
	right = right->next;
	}
    if (autosomeBasesInLine < bases)
        autosomeBasesInLine = bases;
    gl->lineCount += 1;
    }

/* Figure out space needed for sex chromosomes. */
if (gl->bottomList)
    {
    gl->lineCount += 1;
    bases = 0;
    sexOtherPixels = spaceWidth + 2*margin;
    for (ref = gl->bottomList; ref != NULL; ref = ref->next)
	{
	chrom = ref->val;
	sexBasesInLine += chrom->size;
	labelWidth = mgFontStringWidth(font, chrom->shortName) + spaceWidth;
	if (ref == gl->bottomList )
	    {
	    if (leftLabelWidth < labelWidth)
		leftLabelWidth  = labelWidth;
	    sexOtherPixels = leftLabelWidth;
	    }
	else if (ref->next == NULL)
	    {
	    if (rightLabelWidth < labelWidth)
		rightLabelWidth  = labelWidth;
	    sexOtherPixels += rightLabelWidth + spaceWidth;
	    }
	else
	    {
	    sexOtherPixels += labelWidth + spaceWidth;
	    }
	}
    }

/* Do some adjustments if side labels are bigger than needed for
 * chromosome names. */
if (leftLabelWidth < minLeftLabelWidth)
    {
    extraLabelPadding += (minLeftLabelWidth - leftLabelWidth);
    leftLabelWidth = minLeftLabelWidth;
    }
if (rightLabelWidth < minRightLabelWidth)
    {
    extraLabelPadding += (minRightLabelWidth - rightLabelWidth);
    rightLabelWidth = minRightLabelWidth;
    }
sexOtherPixels += extraLabelPadding;

/* Figure out the number of bases needed per pixel. */
autosomeOtherPixels = 2*margin + spaceWidth + leftLabelWidth + rightLabelWidth;
basesPerPixel = autosomeBasesPerPixel 
	= autosomeBasesInLine/(picWidth-autosomeOtherPixels);
if (gl->bottomList)
    {
    sexBasesPerPixel = sexBasesInLine/(picWidth-sexOtherPixels);
    if (sexBasesPerPixel > basesPerPixel)
        basesPerPixel = sexBasesPerPixel;
    }

/* Save positions and sizes of some things in layout structure. */
gl->leftLabelWidth = leftLabelWidth;
gl->rightLabelWidth = rightLabelWidth;
gl->basesPerPixel = basesPerPixel;
uglyf("autosomeOtherPixels %d, sexOtherPixels %d, picWidth %d<BR>\n", autosomeOtherPixels, sexOtherPixels, picWidth);

/* Set pixel positions for left autosomes */
for (ref = gl->leftList; ref != NULL; ref = ref->next)
    {
    chrom = ref->val;
    chrom->x = leftLabelWidth + margin;
    chrom->y = y;
    chrom->width = round(chrom->size/basesPerPixel);
    chrom->height = lineHeight;
    y += lineHeight;
    }

/* Set pixel positions for right autosomes */
y = 0;
for (ref = gl->rightList; ref != NULL; ref = ref->next)
    {
    chrom = ref->val;
    chrom->width = round(chrom->size/basesPerPixel);
    chrom->height = lineHeight;
    chrom->x = picWidth - margin - rightLabelWidth - chrom->width;
    chrom->y = y;
    y += lineHeight;
    }
gl->picHeight = 2*margin + lineHeight * gl->lineCount;
y = gl->picHeight - margin - lineHeight;

/* Set pixel positions for sex chromosomes */
for (ref = gl->bottomList; ref != NULL; ref = ref->next)
    {
    chrom = ref->val;
    chrom->y = y;
    chrom->width = round(chrom->size/basesPerPixel);
    chrom->height = lineHeight;
    if (ref == gl->bottomList)
	chrom->x = leftLabelWidth + margin;
    else if (ref->next == NULL)
        chrom->x = picWidth - margin - rightLabelWidth - chrom->width;
    else
	chrom->x = 2*spaceWidth+mgFontStringWidth(font,chrom->shortName) + pos;
    pos = chrom->x + chrom->width;
    }
return gl;
}

void leftLabel(struct vGfx *vg, struct genoLay *gl,
	struct genoLayChrom *chrom, int fontHeight,
	int color)
/* Draw a chromosome with label on left. */
{
vgTextRight(vg, gl->margin, chrom->y + chrom->height - fontHeight, 
    chrom->x - gl->margin - gl->spaceWidth, fontHeight, color,
    gl->font, chrom->shortName);
}

void rightLabel(struct vGfx *vg, struct genoLay *gl,
	struct genoLayChrom *chrom, int fontHeight, 
	int color)
/* Draw a chromosome with label on left. */
{
vgText(vg, chrom->x + chrom->width + gl->spaceWidth,
	chrom->y + chrom->height - fontHeight, 
	color, gl->font, chrom->shortName);
}

void midLabel(struct vGfx *vg, struct genoLay *gl,
	struct genoLayChrom *chrom, int fontHeight, 
	int color)
/* Draw a chromosome with label on left. */
{
MgFont *font = gl->font;
int textWidth = mgFontStringWidth(font, chrom->shortName);
vgTextRight(vg, chrom->x - textWidth - gl->spaceWidth, 
    chrom->y + chrom->height - fontHeight, 
    textWidth, fontHeight, color,
    font, chrom->shortName);
}

void genoLayDrawChromLabels(struct genoLay *gl, struct vGfx *vg, int color)
/* Draw chromosomes labels in image */
{
struct slRef *ref;
struct genoLayChrom *chrom;
int pixelHeight = mgFontPixelHeight(gl->font);

/* Draw chromosome labels. */
for (ref = gl->leftList; ref != NULL; ref = ref->next)
    leftLabel(vg, gl, ref->val, pixelHeight, color);
for (ref = gl->rightList; ref != NULL; ref = ref->next)
    rightLabel(vg, gl, ref->val, pixelHeight, color);
for (ref = gl->bottomList; ref != NULL; ref = ref->next)
    {
    chrom = ref->val;
    if (ref == gl->bottomList)
	leftLabel(vg, gl, chrom, pixelHeight, color);
    else if (ref->next == NULL)
	rightLabel(vg, gl, chrom, pixelHeight, color);
    else
        midLabel(vg, gl, chrom, pixelHeight, color);
    }
}

void genoLayDrawSimpleChroms(struct genoLay *gl,
	struct vGfx *vg, int color)
/* Draw boxes for all chromosomes in given color */
{
int lineHeight = mgFontLineHeight(gl->font);
struct genoLayChrom *chrom;
for (chrom = gl->chromList; chrom != NULL; chrom = chrom->next)
    vgBox(vg, chrom->x, chrom->y + chrom->height - lineHeight, 
	chrom->width, lineHeight, color);

}

void genoLayDrawBandedChroms(struct genoLay *gl, struct vGfx *vg,
	struct sqlConnection *conn, Color *shadesOfGray, int maxShade, 
	int defaultColor)
/* Draw chromosomes with centromere and band glyphs. 
 * Get the band data from the database.  If the data isn't
 * there then draw simple chroms in default color instead */
{
char *bandTable = "cytoBandIdeo";
if (sqlTableExists(conn, bandTable))
    {
    int centromereColor = hCytoBandCentromereColor(vg);
    double pixelsPerBase = 1.0/gl->basesPerPixel;
    int height = mgFontLineHeight(gl->font);
    struct genoLayChrom *chrom;
    boolean isDmel = hCytoBandIsDmel();
    boolean bColor = vgFindColorIx(vg, 200, 150, 150);
    int fontPixelHeight = mgFontPixelHeight(gl->font);
    for (chrom = gl->chromList; chrom != NULL; chrom = chrom->next)
	{
	struct sqlResult *sr;
	char **row;
	char query[256];
	int cenX1=BIGNUM, cenX2=0;
	int y = chrom->y + chrom->height - height;
	safef(query, sizeof(query), "select * from %s where chrom='%s'",
		bandTable, chrom->fullName);
	sr = sqlGetResult(conn, query);
	while ((row = sqlNextRow(sr)) != NULL)
	    {
	    struct cytoBand band;
	    int x1, x2;
	    cytoBandStaticLoad(row, &band);
	    x1 = pixelsPerBase*band.chromStart;
	    x2 = pixelsPerBase*band.chromEnd;
	    if (sameString(band.gieStain, "acen"))
		{
		if (x1 < cenX1)
		    cenX1 = x1;
		if (x2 > cenX2)
		    cenX2 = x2;
		}
	    else
		{
		hCytoBandDrawAt(&band, vg, x1+chrom->x, y, x2-x1, height, 
			isDmel, gl->font, fontPixelHeight, MG_BLACK, bColor,
		    shadesOfGray, maxShade);
		}
	    }
	sqlFreeResult(&sr);
	if (cenX2 > cenX1)
	    {
	    hCytoBandDrawCentromere(vg, cenX1+chrom->x, y, cenX2-cenX1, height,
	       MG_WHITE, centromereColor);
	    }
	}
    }
else
    genoLayDrawSimpleChroms(gl, vg, defaultColor);
}

void genomeGif(struct sqlConnection *conn, struct genoLay *gl)
/* Create genome GIF file and HTML that includes it. */
{
struct vGfx *vg;
struct tempName gifTn;
Color shadesOfGray[10];
int maxShade = ArraySize(shadesOfGray)-1;

makeTempName(&gifTn, "hgtIdeo", ".gif");
vg = vgOpenGif(gl->picWidth, gl->picHeight, gifTn.forCgi);
hMakeGrayShades(vg, shadesOfGray, maxShade);
printf("<IMG SRC = \"%s\" BORDER=1 WIDTH=%d HEIGHT=%d>",
	    gifTn.forHtml, gl->picWidth, gl->picHeight);

genoLayDrawChromLabels(gl, vg, MG_BLACK);
genoLayDrawSimpleChroms(gl, vg, MG_BLUE);
genoLayDrawBandedChroms(gl, vg, conn, shadesOfGray, maxShade, MG_GREEN);



#ifdef SOMEDAY
if(doIdeo)
    {
    char startBand[16];
    char endBand[16];
    char title[32];
    startBand[0] = endBand[0] = '\0';
    fillInStartEndBands(ideoTrack, startBand, endBand, sizeof(startBand)); 
    /* Draw the ideogram. */
    makeTempName(&gifTn, "hgtIdeo", ".gif");
    /* Start up client side map. */
    hPrintf("<MAP Name=%s>\n", mapName);
    ideoHeight = gfxBorder + ideoTrack->height;
    vg = vgOpenGif(ideoWidth, ideoHeight, gifTn.forCgi);
    makeGrayShades(vg);
    makeBrownShades(vg);
    makeSeaShades(vg);
    ideoTrack->ixColor = vgFindRgb(vg, &ideoTrack->color);
    ideoTrack->ixAltColor = vgFindRgb(vg, &ideoTrack->altColor);
    vgSetClip(vg, 0, gfxBorder, ideoWidth, ideoTrack->height);
    if(sameString(startBand, endBand)) 
	safef(title, sizeof(title), "%s (%s)", chromName, startBand);
    else
	safef(title, sizeof(title), "%s (%s-%s)", chromName, startBand, endBand);
    textWidth = mgFontStringWidth(font, title);
    vgTextCentered(vg, 2, gfxBorder, textWidth, ideoTrack->height, MG_BLACK, font, title);
    ideoTrack->drawItems(ideoTrack, winStart, winEnd, vg, textWidth+4, gfxBorder, ideoWidth-textWidth-4,
			 font, ideoTrack->ixColor, ideoTrack->limitedVis);
    vgUnclip(vg);
    /* Save out picture and tell html file about it. */
    vgClose(&vg);
    /* Finish map. */
    hPrintf("</MAP>\n");
    }
hPrintf("<TABLE BORDER=0 CELLPADDING=0>");
if(doIdeo)
    {
    hPrintf("<TR><TD HEIGHT=5></TD></TR>");
    hPrintf("<TR><TD><IMG SRC = \"%s\" BORDER=1 WIDTH=%d HEIGHT=%d USEMAP=#%s >",
	    gifTn.forHtml, ideoWidth, ideoHeight, mapName);
    hPrintf("</TD></TR>");
    hPrintf("<TR><TD HEIGHT=5></TD></TR></TABLE>");
    }
else
    hPrintf("<TR><TD HEIGHT=10></TD></TR></TABLE>");
if(ideoTrack != NULL)
    {
    ideoTrack->limitedVisSet = TRUE;
    ideoTrack->limitedVis = tvHide; /* Don't draw in main gif. */
    }
#endif /* SOMEDAY */
vgClose(&vg);
}

void webMain(struct sqlConnection *conn)
/* Set up fancy web page with hotlinks bar and
 * sections. */
{
struct genoLayChrom *chromList, *chrom;
struct slRef *left, *right, *ref;
struct genoLay *gl;
int total;
int fontHeight, lineHeight;
trackLayoutInit(&tl, cart);
fontHeight = mgFontLineHeight(tl.font);
lineHeight = fontHeight*3;
chromList = genoLayDbChroms(conn, FALSE);
gl = genoLayNew(chromList, tl.font, tl.picWidth, lineHeight, 
	3*tl.nWidth, 4*tl.nWidth);
uglyf("gl: lineCount %d, leftLabelWidth %d, rightLabelWidth %d, basesPerPixel %f<BR>\n",
	gl->lineCount, gl->leftLabelWidth, gl->rightLabelWidth, gl->basesPerPixel);
for (left = gl->leftList, right = gl->rightList; left != NULL || right != NULL;)
    {
    total=0;
    if (left != NULL)
	{
	chrom = left->val;
	uglyf("%s@%d,%d[%d] %d ----  ", chrom->fullName, chrom->x, chrom->y, chrom->width, chrom->size);
	total += chrom->size;
        left = left->next;
	}
    if (right != NULL)
	{
	chrom = right->val;
	uglyf("%d  %s@%d,%d[%d]", chrom->size, chrom->fullName, chrom->x, chrom->y, chrom->width);
	total += chrom->size;
        right = right->next;
	}
    uglyf(" : %d<BR>", total);
    }
total=0;
for (ref = gl->bottomList; ref != NULL; ref = ref->next)
    {
    chrom = ref->val;
    total += chrom->size;
    uglyf("%s@%d,%d[%d] %d ...  ", chrom->fullName, chrom->x, chrom->y, chrom->width, chrom->size);
    }
uglyf(" : %d<BR>", total);
genomeGif(conn, gl);
}

void cartMain(struct cart *theCart)
/* We got the persistent/CGI variable cart.  Now
 * set up the globals and make a web page. */
{
struct sqlConnection *conn = NULL;
cart = theCart;
getDbAndGenome(cart, &database, &genome);
hSetDb(database);
conn = hAllocConn();

    {
    /* Default case - start fancy web page. */
    cartWebStart(cart, "%s Genome View", genome);
    webMain(conn);
    cartWebEnd();
    }
}

char *excludeVars[] = {"Submit", "submit", NULL};

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
htmlSetStyle(htmlStyleUndecoratedLink);
if (argc != 1)
    usage();
oldCart = hashNew(12);
cartEmptyShell(cartMain, hUserCookie(), excludeVars, oldCart);
return 0;
return 0;
}
