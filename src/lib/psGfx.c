/* PostScript graphics - 
 * This provides a bit of a shell around writing graphics to
 * a postScript file.  Perhaps the most important thing it
 * does is convert 0,0 from being at the bottom left to
 * being at the top left. */
#include "common.h"
#include "psGfx.h"

static void psFloatOut(FILE *f, double x)
/* Write out a floating point number, but not in too much
 * precision. */
{
int i = round(x);
if (i == x)
   fprintf(f, "%d ", i);
else
   fprintf(f, "%0.4f ", x);
}

static void psWriteHeader(FILE *f, double width, double height)
/* Write postScript header.  It's encapsulated PostScript
 * actually, so you need to supply image width and height
 * in points. */
{
char *s =
#include "common.pss"
;

fprintf(f, "%%!PS-Adobe-3.1 EPSF-3.0\n");
fprintf(f, "%%%%BoundingBox: 0 0 ");
psFloatOut(f, width);
psFloatOut(f, height);
fprintf(f, "\n\n");
fprintf(f, "%s", s);
}

struct psGfx *psOpen(char *fileName, 
	int pixWidth, int pixHeight, 	 /* Dimension of image in pixels. */
	double ptWidth, double ptHeight, /* Dimension of image in points. */
	double ptMargin)                 /* Image margin in points. */
/* Open up a new postscript file.  If ptHeight is 0, it will be
 * calculated to keep pixels square. */
{
struct psGfx *ps;

/* Allocate structure and open file. */
AllocVar(ps);
ps->f = mustOpen(fileName, "w");
psWriteHeader(ps->f, ptWidth, ptHeight);

/* Save page dimensions and calculate scaling factors. */
ps->pixWidth = pixWidth;
ps->pixHeight = pixHeight;
ps->ptWidth = ptWidth;
ps->xScale = (ptWidth - 2*ptMargin)/pixWidth;
if (ptHeight != 0.0)
   {
   ps->ptHeight = ptHeight;
   ps->yScale = (ptHeight - 2*ptMargin) / pixHeight;
   }
else
   {
   ps->yScale = ps->xScale;
   ps->ptHeight = pixHeight * ps->yScale + 2*ptMargin;
   }
ps->xOff = ptMargin;
ps->yOff = ptMargin;
ps->fontHeight = 10;

/* Cope with fact y coordinates are bottom to top rather
 * than top to bottom. */
ps->yScale = -ps->yScale;
ps->yOff = ps->ptHeight - ps->yOff;

return ps;
}

void psClose(struct psGfx **pPs)
/* Close out postScript file. */
{
struct psGfx *ps = *pPs;
if (ps != NULL)
    {
    carefulClose(&ps->f);
    freez(pPs);
    }
}

void psXyOut(struct psGfx *ps, int x, int y)
/* Output x,y position transformed into PostScript space. */
{
FILE *f = ps->f;
psFloatOut(f, x * ps->xScale + ps->xOff);
psFloatOut(f, y * ps->yScale + ps->yOff);
}

void psWhOut(struct psGfx *ps, int width, int height)
/* Output width/height transformed into PostScript space. */
{
FILE *f = ps->f;
psFloatOut(f, width * ps->xScale);
psFloatOut(f, height * -ps->yScale);
}

void psMoveTo(struct psGfx *ps, int x, int y)
/* Move PostScript position to given point. */
{
psXyOut(ps, x, y);
fprintf(ps->f, "moveto\n");
}

void psDrawBox(struct psGfx *ps, int x, int y, int width, int height)
/* Draw a filled box in current color. */
{
psWhOut(ps, width, height);
psXyOut(ps, x, y+height);
fprintf(ps->f, "fillBox\n");
}

void psDrawLine(struct psGfx *ps, int x1, int y1, int x2, int y2)
/* Draw a line from x1/y1 to x2/y2 */
{
FILE *f = ps->f;
fprintf(f, "newpath\n");
psMoveTo(ps, x1, y1);
psXyOut(ps, x2, y2);
fprintf(ps->f, "lineto\n");
fprintf(f, "stroke\n");
}

void psTimesFont(struct psGfx *ps, double size)
/* Set font to times of a certain size. */
{
FILE *f = ps->f;
fprintf(f, "/Times-Roman findfont ");

/* Note the 1.2 and the 0.95 below seem to get it to 
 * position about where the stuff developed for pixel
 * based systems expects it.  It is all a kludge though! */
fprintf(f, "%f scalefont setfont\n", -size*ps->yScale*1.2);
ps->fontHeight = size*0.95;
}


void psTextAt(struct psGfx *ps, int x, int y, char *text)
/* Output text in current font at given position. */
{
psMoveTo(ps, x, y + ps->fontHeight);
fprintf(ps->f, "(%s) show\n", text);
}

void psTextDown(struct psGfx *ps, int x, int y, char *text)
/* Output text going downwards rather than across at position. */
{
FILE *f = ps->f;
psMoveTo(ps, x, y);
fprintf(ps->f, "gsave\n");
fprintf(ps->f, "-90 rotate\n");
fprintf(ps->f, "(%s) show\n", text);
fprintf(ps->f, "grestore\n");
}

void psSetColor(struct psGfx *ps, int r, int g, int b)
/* Set current color. */
{
FILE *f = ps->f;
double scale = 1.0/255;
if (r == g && g == b)
    {
    psFloatOut(f, scale * r);
    fprintf(f, "setgray\n");
    }
else
    {
    psFloatOut(f, scale * r);
    psFloatOut(f, scale * g);
    psFloatOut(f, scale * b);
    fprintf(f, "setrgbcolor\n");
    }
}

void psSetGray(struct psGfx *ps, double grayVal)
/* Set gray value. */
{
FILE *f = ps->f;
if (grayVal < 0) grayVal = 0;
if (grayVal > 1) grayVal = 1;
psFloatOut(f, grayVal);
fprintf(f, "setgray\n");
}

void psPushG(struct psGfx *ps)
/* Save graphics state on stack. */
{
fprintf(ps->f, "gsave\n");
}

void psPopG(struct psGfx *ps)
/* Pop off saved graphics state. */
{
fprintf(ps->f, "grestore\n");
}

void psPushClipRect(struct psGfx *ps, int x, int y, int width, int height)
/* Push clipping rectangle onto graphics stack. */
{
FILE *f = ps->f;
fprintf(f, "gsave\n");
fprintf(f, "newpath\n");
psWhOut(ps, width, height);
psXyOut(ps, x, y+height);
fprintf(f, "boxAt\n");
fprintf(f, "clip\n");
}

