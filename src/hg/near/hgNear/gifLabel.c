/* gifLabel - create labels as GIF files. */

#include "common.h"
#include "memgfx.h"
#include "hgNear.h"

static char const rcsid[] = "$Id: gifLabel.c,v 1.1 2003/08/30 00:22:22 kent Exp $";

int gifLabelMaxWidth(char **labels, int labelCount)
/* Return maximum pixel width of labels.  It's ok to have
 * NULLs in labels array. */
{
int width = 0, w, i;
MgFont *font = mgSmallishFont();
for (i=0; i<labelCount; ++i)
    {
    char *label = labels[i];
    if (label != NULL)
	{
	w = mgFontStringWidth(font, labels[i]);
	if (w > width)
	    width = w;
	}
    }
width += 2;
return width;
}

static struct memGfx *altColorLabels(char **labels, int labelCount, int width)
/* Return a memory image with alternating colors. */
{
struct memGfx *mg = NULL;
Color c1,c2;
MgFont *font = mgSmallishFont();
int lineHeight = mgFontLineHeight(font)-1;
int height = lineHeight * labelCount, w, i;
int y = 0;

/* Allocate picture and set up colors. */
mg = mgNew(width, height);
c1 = mgFindColor(mg, 0xE0, 0xE0, 0xFF);
c2 = mgFindColor(mg, 0xFF, 0xC8, 0xC8);

/* Draw text. */
for (i=labelCount-1; i >= 0; --i)
    {
    Color c = ((i&1) ? c2 : c1);
    mgDrawBox(mg, 0, y, width, lineHeight, c);
    mgTextRight(mg, 0+1, y+1, width-1, lineHeight, MG_BLACK, font, labels[i]);
    y += lineHeight;
    }

return mg;
}

struct memGfx *mgRotate90(struct memGfx *in)
/* Create a copy of input that is rotated 90 degrees clockwise. */
{
int iWidth = in->width, iHeight = in->height;
struct memGfx *out = mgNew(iHeight, iWidth);
Color *inCol, *outRow, *outRowStart;
int i,j;

memcpy(out->colorMap, in->colorMap, sizeof(out->colorMap));
outRowStart = out->pixels;
for (i=0; i<iWidth; ++i)
    {
    inCol = in->pixels + i;
    outRow = outRowStart;
    outRowStart += _mgBpr(out);
    j = iHeight;
    while (--j >= 0)
        {
	outRow[j] = *inCol;
	inCol += _mgBpr(in);
	}
    }
return out;
}

void gifLabelVerticalText(char *fileName, char **labels, int labelCount, 
	int height)
/* Make a gif file with given labels. */
{
if (!fileExists(fileName))
    {
    struct memGfx *straight = altColorLabels(labels, labelCount, height);
    struct memGfx *rotated = mgRotate90(straight);
    mgSaveGif(rotated, fileName);
    mgFree(&straight);
    mgFree(&rotated);
    }
}


#ifdef DEBUG
void gifTest()
{
static char *labels[] = {"cerebellum", "thymus", "breast", "heart",
			 "stomach", "cartilage", "kidney", "liver",
			 "lung", "testis", "black hole" };
int size = gifLabelMaxWidth(labels, ArraySize(labels));
int gifLabelMaxWidth(char **labels, int labelCount)
gifLabelVerticalText("../trash/foo.gif", labels, ArraySize(labels), size);
uglyf("<IMG SRC=\"../trash/foo.gif\">");
}
#endif /* DEBUG */
