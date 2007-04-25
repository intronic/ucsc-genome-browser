/* variation.h - hgTracks routines that are specific to the SNP and haplotype tracks */

#ifndef VARIATION_H
#define VARIATION_H

#include "common.h"
#include "hCommon.h"
#include "hdb.h"
#include "hgTracks.h"
#include "snp.h"
#include "snpMap.h"
#include "hui.h"
#include "snpUi.h"
#include "snp125.h"
#include "snp125Ui.h"
#include "spaceSaver.h"
#include "ld.h"
#include "hash.h"
#include "ldUi.h"
#include "gfxPoly.h"
#include "memgfx.h"
#include "cnpIafrate.h"
#include "cnpIafrate2.h"
#include "cnpSebat.h"
#include "cnpSebat2.h"
#include "cnpSharp.h"
#include "cnpSharp2.h"
#include "cnpRedon.h"
#include "cnpLocke.h"
#include "cnpSharpSample.h"
#include "cnpSharpCutoff.h"
#include "hgTracks.h"

/****** snpMap *******/

boolean snpMapSourceFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

boolean snpMapTypeFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

void loadSnpMap(struct track *tg);
/* Load up snpMarkers from database table to track items. */

void freeSnpMap(struct track *tg);
/* Free up snpMap items. */

Color snpMapColor(struct track *tg, void *item, struct vGfx *vg);
/* Return color of snpMap track item. */

void snpMapDrawItemAt(struct track *tg, void *item, struct vGfx *vg, int xOff, int y, 
		      double scale, MgFont *font, Color color, enum trackVisibility vis);
/* Draw a single snpMap item at position. */

void drawDiamond(struct vGfx *vg, 
		 int xl, int yl, int xt, int yt, int xr, int yr, int xb, int yb, 
		 Color fillColor, Color outlineColor);
/* Draw diamond shape. */

void mapDiamondUi(int xl, int yl, int xt, int yt, 
			 int xr, int yr, int xb, int yb, 
			 char *name, char *shortLabel);
/* Print out image map rectangle that invokes hgTrackUi. */

Color getOutlineColor(struct track *tg, int itemCount);
/* get outline color from cart and set outlineColor*/

void mapTrackBackground(struct track *tg, int xOff, int yOff);
/* Print out image map rectangle that invokes hgTrackUi. */

void initColorLookup(struct track *tg, struct vGfx *vg, boolean isDprime);

void ldAddToDenseValueHash(struct hash *ldHash, unsigned a, char charValue);
/* Add new values to LD hash or update existing values.
   Values are averaged along the diagonals. */

void ldDrawDenseValueHash(struct vGfx *vg, struct track *tg, int xOff, int yOff, 
			  double scale, Color outlineColor, struct hash *ldHash);
/* Draw all dense LD values */


void snpMapMethods(struct track *tg);
/* Make track for snps. */

/****** snp ******/

void filterSnpItems(struct track *tg, boolean (*filter)(struct track *tg, void *item));
/* Filter out items from track->itemList. */

boolean snpSourceFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

boolean snpMolTypeFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

boolean snpClassFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

boolean snpValidFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

boolean snpFuncFilterItem(struct track *tg, void *item);
/* Return TRUE if item passes filter. */

void loadSnp(struct track *tg);
void loadSnp125(struct track *tg);
/* Load up snps from database table to track items. */

void freeSnp(struct track *tg);
/* Free up snp items. */

Color snpColor(struct track *tg, void *item, struct vGfx *vg);
Color snp125Color(struct track *tg, void *item, struct vGfx *vg);
/* Return color of snp track item. */

void snpDrawItemAt(struct track *tg, void *item, struct vGfx *vg, int xOff, int y, 
		   double scale, MgFont *font, Color color, enum trackVisibility vis);
/* Draw a single snp item at position. */

void snpMethods(struct track *tg);
void snp125Methods(struct track *tg);
/* Make track for snps. */

struct orthoBed
/* Browser extensible data - first four fields plus a chimp allele */
    {
    struct orthoBed *next;       /* Next in singly linked list. */
    char            *chrom;      /* Human chromosome or FPC contig */
    unsigned         chromStart; /* Start position in chromosome */
    unsigned         chromEnd;   /* End position in chromosome */
    char            *name;       /* Name of item */
    char            *chimp;      /* Chimp allele */
    };

/***** haplotypes *****/

char *perlegenName(struct track *tg, void *item);
/* return the actual perlegen name, in form xx/yyyy cut off xx/ return yyyy */

int haplotypeHeight(struct track *tg, struct linkedFeatures *lf, struct simpleFeature *sf);
/* if the item isn't the first or the last make it smaller */

void haplotypeMethods(struct track *tg);
/* setup special methods for haplotype track */

void perlegenMethods(struct track *tg);
/* setup special methods for Perlegen haplotype track */


/****** LD *****/

/* 10 shades from black to fully saturated of red/green/blue */
#define LD_DATA_SHADES 10
extern Color ldShadesPos[LD_DATA_SHADES];
extern Color ldHighLodLowDprime;
extern Color ldHighDprimeLowLod;
extern int colorLookup[256];

struct ldStats 
/* collect stats for drawing LD values in dense mode */
{
    struct ldStats *next;
    char           *name;      /* chromStart as a string */
    unsigned        n;         /* count of snps with valid LD values */
    unsigned        sumValues; /* sum of valid LD values */
};

void ldLoadItems(struct track *tg);
/* loadItems loads up items for the chromosome range indicated.   */

int ldTotalHeight(struct track *tg, enum trackVisibility vis);
/* Return total height. Called before and after drawItems. 
 * Must set height, lineHeight, heightPer */ 

void ldDrawItems(struct track *tg, int seqStart, int seqEnd,
		  struct vGfx *vg, int xOff, int yOff, int width, 
		  MgFont *font, Color color, enum trackVisibility vis);
/* Draw item list, one per track. */

void ldDrawItemAt(struct track *tg, void *item, struct vGfx *vg, 
		  int xOff, int yOff, double scale, 
		  MgFont *font, Color color, enum trackVisibility vis);
/* Draw a single item.  Required for genericDrawItems */

void ldFreeItems(struct track *tg);
/* Free item list. */

void ldMethods(struct track *tg);
/* setup special methods for Linkage Disequilibrium track */

/****** CNP / Structural Variants ******/

void cnpIafrateLoadItems(struct track *tg);
/* loader for cnpIafrate table */

void cnpIafrateFreeItems(struct track *tg);

Color cnpIafrateColor(struct track *tg, void *item, struct vGfx *vg);
/* green for GAIN, red for LOSS, blue for both */

void cnpIafrateMethods(struct track *tg);
/* methods for cnpIafrate */

void cnpIafrate2LoadItems(struct track *tg);
/* loader for cnpIafrate2 table */

void cnpIafrate2FreeItems(struct track *tg);

Color cnpIafrate2Color(struct track *tg, void *item, struct vGfx *vg);
/* green for GAIN, red for LOSS, blue for both */

void cnpIafrate2Methods(struct track *tg);
/* methods for cnpIafrate2 */

void cnpSebatLoadItems(struct track *tg);
/* loader for cnpSebat table */

void cnpSebatFreeItems(struct track *tg);

Color cnpSebatColor(struct track *tg, void *item, struct vGfx *vg);
/* green for GAIN, red for LOSS, blue for both */

void cnpSebatMethods(struct track *tg);
/* methods for cnpSebat */

void cnpSebat2LoadItems(struct track *tg);
/* loader for cnpSebat2 table */

void cnpSebat2FreeItems(struct track *tg);

Color cnpSebat2Color(struct track *tg, void *item, struct vGfx *vg);
/* green for GAIN, red for LOSS */

void cnpSebat2Methods(struct track *tg);
/* methods for cnpSebat2 */

void cnpSharpLoadItems(struct track *tg);
/* loader for cnpSharp table */

void cnpSharpFreeItems(struct track *tg);

Color cnpSharpColor(struct track *tg, void *item, struct vGfx *vg);
/* green for GAIN, red for LOSS, blue for both */

void cnpSharpMethods(struct track *tg);
/* methods for cnpSharp */

void cnpSharp2LoadItems(struct track *tg);
/* loader for cnpSharp2 table */

void cnpSharp2FreeItems(struct track *tg);

Color cnpSharp2Color(struct track *tg, void *item, struct vGfx *vg);
/* green for GAIN, red for LOSS, blue for both */

void cnpSharp2Methods(struct track *tg);
/* methods for cnpSharp2 */

void cnpFosmidLoadItems(struct track *tg);
/* loader for cnpFosmid table */

void cnpFosmidFreeItems(struct track *tg);

Color cnpFosmidColor(struct track *tg, void *item, struct vGfx *vg);
/* green for I, red for D */

void cnpFosmidMethods(struct track *tg);
/* methods for cnpFosmid */

void cnpRedonLoadItems(struct track *tg);
/* loader for cnpRedon table */

void cnpRedonFreeItems(struct track *tg);

Color cnpRedonColor(struct track *tg, void *item, struct vGfx *vg);
/* always gray */

void cnpRedonMethods(struct track *tg);
/* methods for cnpRedon */

void cnpLockeLoadItems(struct track *tg);
/* loader for cnpLocke table */

void cnpLockeFreeItems(struct track *tg);

Color cnpLockeColor(struct track *tg, void *item, struct vGfx *vg);

void cnpLockeMethods(struct track *tg);
/* methods for cnpLocke */

Color delConradColor(struct track *tg, void *item, struct vGfx *vg);
/* always red */

void delConradMethods(struct track *tg);
/* methods for delConrad */

Color delConrad2Color(struct track *tg, void *item, struct vGfx *vg);
/* always red */

void delConrad2Methods(struct track *tg);
/* methods for delConrad2 */

Color delMccarrollColor(struct track *tg, void *item, struct vGfx *vg);
/* always red */

void delMccarrollMethods(struct track *tg);
/* methods for delMccarroll */

Color delHindsColor(struct track *tg, void *item, struct vGfx *vg);
/* always red */

void delHindsMethods(struct track *tg);
/* methods for delHinds */

#endif
