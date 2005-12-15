#include "common.h"
#include "memgfx.h"
#include "gemfont.h"

/*	formerly known as sailfnt.c mgSmallFont() now mgSmallFont()	*/
static char const rcsid[] = "$Id: mgSmallFont.c,v 1.4 2005/12/15 20:33:55 hiram Exp $";

/* static char sail_name[] = "SAIL.FNT"; */

static UBYTE sail_data[] = {
	0x00,0xc5, 0x14,0x23, 0xe6,0x04, 0x11,0x00, 
	0x00,0x00, 0x00,0x02, 0x30,0x87, 0x1c,0x13, 
	0xe7,0x3e, 0x71,0xc0, 0x00,0x00, 0x00,0x1c, 
	0x71,0xcf, 0x1c,0xf3, 0xef,0x9c, 0x89,0xc0, 
	0xa2,0x82, 0x28,0x9c, 0xf1,0xcf, 0x1c,0xfa, 
	0x28,0xa2, 0x8a,0x27, 0x9c,0x81, 0xc2,0x00, 
	0x20,0x08, 0x00,0x08, 0x03,0x80, 0x80,0x41, 
	0x10,0x30, 0x00,0x00, 0x00,0x00, 0x00,0x40, 
	0x00,0x00, 0x00,0x00, 0x0c,0x21, 0x80,0x00, 
	0x00,0xc5, 0x3e,0x7b, 0x2a,0x04, 0x20,0x8a, 
	0x88,0x00, 0x00,0x04, 0x49,0x88, 0xa2,0x32, 
	0x08,0x02, 0x8a,0x22, 0x04,0x10, 0x04,0x22, 
	0x8a,0x28, 0xa2,0x8a, 0x08,0x22, 0x88,0x80, 
	0xa4,0x83, 0x6c,0xa2, 0x8a,0x28, 0xa2,0x22, 
	0x28,0xa2, 0x52,0x20, 0x90,0x40, 0x47,0x00, 
	0x21,0xcb, 0x0e,0x69, 0xc4,0x1e, 0xb0,0x00, 
	0x12,0x13, 0x4b,0x1c, 0xb1,0xab, 0x1e,0xfa, 
	0x28,0xa2, 0x8a,0x2f, 0x90,0x20, 0x44,0x00, 
	0x00,0xc5, 0x14,0xa0, 0x44,0x08, 0x40,0x47, 
	0x08,0x00, 0x00,0x04, 0x58,0x80, 0x8c,0x53, 
	0xcf,0x04, 0x72,0x22, 0x04,0x23, 0xe2,0x02, 
	0xbb,0xef, 0x20,0x8b, 0xcf,0x20, 0xf8,0x80, 
	0xa8,0x82, 0xaa,0xa2, 0xf2,0x2f, 0x18,0x22, 
	0x28,0xaa, 0x21,0xc1, 0x10,0x20, 0x4a,0x80, 
	0x10,0x2c, 0x90,0x9a, 0x2f,0x22, 0xc8,0xc3, 
	0x14,0x12, 0xac,0xa2, 0xca,0x6c, 0xa0,0x42, 
	0x28,0xa2, 0x52,0x21, 0x10,0x20, 0x4a,0x80, 
	0x00,0xc0, 0x14,0x70, 0x8a,0x80, 0x40,0x4d, 
	0xbe,0x03, 0xe0,0x08, 0x68,0x87, 0x02,0xf8, 
	0x28,0x84, 0x89,0xe0, 0x00,0x40, 0x01,0x0c, 
	0xba,0x28, 0xa0,0x8a, 0x08,0x26, 0x88,0x80, 
	0xb8,0x82, 0xa9,0xa2, 0x82,0xaa, 0x04,0x22, 
	0x25,0x2a, 0x50,0x82, 0x10,0x20, 0x42,0x00, 
	0x01,0xe8, 0x90,0x8b, 0xe4,0x22, 0x88,0x41, 
	0x18,0x12, 0xa8,0xa2, 0x8a,0x28, 0x1c,0x42, 
	0x25,0x2a, 0x22,0x22, 0x20,0x20, 0x21,0x00, 
	0x00,0x00, 0x3e,0x29, 0x69,0x00, 0x40,0x47, 
	0x08,0x20, 0x02,0x08, 0x48,0x88, 0x22,0x12, 
	0x28,0x88, 0x88,0x22, 0x04,0x23, 0xe2,0x00, 
	0x82,0x28, 0xa2,0x8a, 0x08,0x22, 0x88,0x88, 
	0xa4,0x82, 0x28,0xa2, 0x82,0x69, 0x22,0x22, 
	0x25,0x36, 0x88,0x84, 0x10,0x10, 0x42,0x00, 
	0x02,0x2c, 0x90,0x9a, 0x04,0x1c, 0x88,0x41, 
	0x14,0x12, 0xa8,0xa2, 0xca,0x68, 0x02,0x42, 
	0x65,0x2a, 0x51,0xe4, 0x10,0x20, 0x40,0x00, 
	0x00,0xc0, 0x14,0xf2, 0x6e,0x80, 0x20,0x8a, 
	0x88,0x20, 0x02,0x10, 0x31,0xcf, 0x9c,0x11, 
	0xc7,0x08, 0x71,0xc2, 0x04,0x10, 0x04,0x08, 
	0x72,0x2f, 0x1c,0xf3, 0xe8,0x1c, 0x89,0xc7, 
	0x22,0xfa, 0x28,0x9c, 0x81,0xc8, 0x9c,0x21, 
	0xc2,0x22, 0x88,0x87, 0x90,0x08, 0x42,0x00, 
	0x01,0xab, 0x0e,0x69, 0xc4,0x02, 0x88,0x41, 
	0x12,0x12, 0xa8,0x9c, 0xb1,0xa8, 0x3c,0x31, 
	0xa2,0x14, 0x88,0x2f, 0x90,0x20, 0x40,0x00, 
	0x00,0x00, 0x00,0x20, 0x00,0x00, 0x11,0x00, 
	0x00,0x40, 0x00,0x00, 0x00,0x00, 0x00,0x00, 
	0x00,0x00, 0x00,0x00, 0x08,0x00, 0x00,0x00, 
	0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 
	0x00,0x00, 0x00,0x00, 0x00,0x20, 0x00,0x00, 
	0x00,0x00, 0x00,0x00, 0x1c,0x01, 0xc0,0x3e, 
	0x00,0x00, 0x00,0x00, 0x00,0x3c, 0x00,0x06, 
	0x00,0x00, 0x00,0x00, 0x80,0x20, 0x00,0x00, 
	0x00,0x00, 0x01,0xc0, 0x0c,0x21, 0x80,0x00, 
	};

static WORD sail_ch_ofst[] = {
0, 6, 12, 18, 24, 30, 36, 42,
48, 54, 60, 66, 72, 78, 84, 90,
96, 102, 108, 114, 120, 126, 132, 138,
144, 150, 156, 162, 168, 174, 180, 186,
192, 198, 204, 210, 216, 222, 228, 234,
240, 246, 252, 258, 264, 270, 276, 282,
288, 294, 300, 306, 312, 318, 324, 330,
336, 342, 348, 354, 360, 366, 372, 378,
384, 390, 396, 402, 408, 414, 420, 426,
432, 438, 444, 450, 456, 462, 468, 474,
480, 486, 492, 498, 504, 510, 516, 522,
528, 534, 540, 546, 552, 558, 564, 570,
576, 582, 588,
};

static struct font_hdr sail_font =
	{
	STPROP, 0, "SAIL.FNT", ' ',127,  
	0,0,0,0,0,	/* *_dist */
	6, 6,  /*widths... */
	0,0,0,0,0,0,	/* through skew_m */
	0, /*flags */
	NULL, /* hz_ofst */
	sail_ch_ofst,
	sail_data,
	72, 7,	/* frm_wdt,frm_hgt */
	NULL,	/* next font */
	0, -1,	/* x/y offset */
	};

MgFont *mgSmallFont()
{
return &sail_font;
}

/* SmallBold.c - compiled data for font AdobeHelv-B-R-N--10-100-75-75-P */
/* generated source code by utils/bdfToGem, do not edit */
/* BDF data file input: 75dpi/helvB10-L1.bdf */

static UBYTE SmallBold_data[2821] = {
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0x20,0x8,0x10,0x14,0,0x10,
0,0,0x10,0x10,0x80,0x22,0x40,0,0xa1,0,
0x20,0x80,0xa0,0,0,0x4,0x2,0x8,0,0x8,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0x10,0x10,0x28,0x28,0x28,0x28,0,0,0x8,
0x21,0x45,0x14,0xb4,0x1,0x40,0x80,0x41,0x41,0x41,
0x40,0,0x2,0x4,0x14,0x28,0x10,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x8,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x10,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0x2,0x1,0x18,0x52,0x84,
0,0,0x80,0x84,0x29,0x4,0xaa,0x14,0x80,0x84,
0x28,0xa0,0,0x20,0x21,0xa,0x8,0x2,0x80,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0xc,0xa0,0x8,0x62,0x70,0x45,0x14,0,0,
0x17,0xc,0x71,0xc1,0xbe,0x73,0xe7,0x1c,0,0,
0x1,0xc0,0,0x71,0xf0,0xf3,0xc7,0xdf,0x1e,0x66,
0xc3,0x66,0xc3,0x6,0xc6,0x38,0xf8,0x71,0xf1,0xe7,
0xec,0xd8,0xd9,0x9b,0x1b,0xd,0xfb,0xa3,0x88,0x4,
0x3,0,0xc,0x3,0x81,0x86,0xd8,0x60,0,0,
0,0,0x1,0x80,0,0,0,0,0xc,0xb0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0x1,0xc0,0x42,0x4e,0x51,0xe1,
0xc0,0,0,0x78,0xe0,0,0x10,0x7,0xc0,0xe,
0,0x84,0x42,0x71,0,0x38,0x38,0x38,0x38,0x38,
0x38,0x3f,0x8f,0x3e,0xfb,0xef,0xb6,0xdb,0xc6,0x31,
0xc1,0xc1,0xc1,0xc1,0xc0,0x7,0x59,0xb3,0x66,0xcd,
0x86,0xc0,0xe1,0x2,0x34,0xa2,0x8a,0,0,0x41,
0xa,0x28,0x9a,0xac,0x28,0x41,0xa,0x50,0xa0,0,
0x10,0x42,0x8a,0x11,0x82,0x80,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0xc,0xa5,
0x1c,0xb4,0xd8,0x4d,0x88,0,0,0x1d,0xbc,0xdb,
0x63,0xb0,0xd8,0x6d,0xb6,0,0,0x3,0x61,0xf0,
0x71,0x99,0x9b,0x66,0x18,0x33,0x66,0xc3,0x6c,0xc3,
0x8e,0xe6,0x6c,0xcc,0xd9,0x9b,0x31,0x8c,0xd8,0xd9,
0x9b,0x1b,0xc,0x1b,0x21,0x9c,0x2,0x3,0,0xc,
0x6,0x1,0x80,0x18,0x60,0,0,0,0,0x1,
0x80,0,0,0,0,0x18,0x98,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0x23,0x5b,0x66,0x59,0x2,0x10,0x40,0,0,
0x84,0x6,0x63,0xe0,0xe,0x80,0x4a,0x1,0x88,0xc4,
0x22,0,0x38,0x38,0x38,0x38,0x38,0x38,0x3c,0x19,
0xb0,0xc3,0xc,0x36,0xdb,0x66,0x33,0x63,0x63,0x63,
0x63,0x60,0xd,0x99,0xb3,0x66,0xcd,0x86,0xf9,0x90,
0,0,0,0x4,0,0,0,0,0,0,
0x14,0,0,0,0,0,0,0,0,0,
0x1,0x80,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0xc,0xa5,0x2a,0x68,0xd8,
0x48,0x94,0x60,0,0x2d,0x8c,0x18,0x65,0xb0,0xc0,
0x6d,0xb6,0x48,0xc0,0x60,0x66,0x8,0xd9,0x9b,0xb,
0x36,0x18,0x61,0x66,0xc3,0x78,0xc3,0x8e,0xe6,0xc6,
0xcd,0x8d,0x9b,0x81,0x8c,0xcd,0x99,0x99,0xb1,0x98,
0x33,0x11,0xb6,0,0x73,0xc7,0x3c,0xe7,0x35,0xe6,
0xdb,0x6b,0x65,0x8e,0x58,0xd5,0xb9,0xdb,0x6d,0xad,
0x9b,0x6f,0x98,0x98,0,0,0,0,0,0,
0,0,0,0,0,0,0,0xc,0xe3,0xe,
0x24,0x5c,0x4,0xc9,0x40,0,0x1,0x7a,0xa,0x65,
0x43,0x6e,0x80,0xce,0,0x90,0x48,0x14,0xc,0x6c,
0x6c,0x6c,0x6c,0x6c,0x6c,0x6c,0x30,0xb0,0xc3,0xc,
0x36,0xdb,0x37,0x36,0x36,0x36,0x36,0x36,0x36,0x79,
0xd9,0xb3,0x66,0xcc,0xcc,0xcd,0x93,0x8e,0x38,0xe3,
0x8e,0x3f,0x1c,0xe3,0x8e,0x39,0xb6,0xce,0x58,0xe3,
0x8e,0x38,0xe1,0x8e,0xed,0xb6,0xdb,0x6d,0xe6,0xc0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0xc,0xf,0xf8,0x10,0x70,0x18,0xc0,0x60,
0,0x2d,0x8c,0x18,0xc5,0xbc,0xf0,0xc7,0x36,0x49,
0x9f,0x30,0xc4,0xd4,0xd9,0xf3,0x3,0x37,0xde,0x60,
0x7e,0xc3,0x70,0xc3,0xde,0xd6,0xc6,0xcd,0x8d,0x99,
0xe1,0x8c,0xcd,0x8d,0xb0,0xe1,0x98,0x63,0x11,0xa2,
0,0x9b,0x6d,0x6d,0xb6,0x6d,0xb6,0xde,0x6d,0xb6,
0xdb,0x6d,0xb7,0x6d,0x9b,0x6d,0xac,0xf3,0x61,0x98,
0x98,0,0,0,0,0,0,0,0,0,
0,0,0,0,0x1,0xb7,0x9b,0x3c,0x4e,0x5,
0x28,0,0x1f,0x1,0x4a,0xd,0xfa,0x23,0x6e,0x80,
0x40,0,0x90,0x48,0x64,0,0x6c,0x6c,0x6c,0x6c,
0x6c,0x6c,0x6f,0xb0,0x3e,0xfb,0xef,0xb6,0xdb,0xb6,
0xb6,0x36,0x36,0x36,0x36,0x33,0xda,0xd9,0xb3,0x66,
0xcc,0xcc,0xcd,0xa4,0xd3,0x4d,0x34,0xd3,0x4d,0xb5,
0xb6,0xdb,0x6d,0xb6,0xdb,0x6d,0xb6,0xdb,0x6d,0xb0,
0x1b,0x6d,0xb6,0xdb,0x6d,0xb6,0xc0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x8,
0x5,0x1c,0x10,0xde,0x18,0xc1,0xf8,0xf0,0x4d,0x8c,
0x30,0x69,0x86,0xd8,0xcd,0x9e,0x3,0,0x19,0x89,
0x24,0xd9,0x9b,0x3,0x36,0x18,0x67,0x66,0xc3,0x78,
0xc3,0x56,0xd6,0xc6,0xf9,0x8d,0xf0,0x71,0x8c,0xcd,
0x8d,0xb0,0xe0,0xf0,0xe3,0x9,0x80,0,0x7b,0x2c,
0x4d,0xf6,0x4d,0xb6,0xdc,0x6d,0xb6,0xdb,0x65,0x36,
0x39,0x9b,0x6d,0xac,0x63,0x63,0x30,0x8c,0xd8,0,
0,0,0,0,0,0,0,0,0,0,
0,0x5,0x43,0x1b,0x7e,0x13,0x5,0x9,0xcd,0x81,
0x79,0x72,0,0x67,0xc3,0x66,0xb0,0x4e,0xd8,0x24,
0x16,0x9,0xc,0x6c,0x6c,0x6c,0x6c,0x6c,0x6c,0x6c,
0x30,0x30,0xc3,0xc,0x36,0xdb,0x36,0xb6,0x36,0x36,
0x36,0x36,0x31,0x9a,0xd9,0xb3,0x66,0xcc,0x78,0xcd,
0x93,0xcf,0x3c,0xf3,0xcf,0x3f,0xb1,0xf7,0xdf,0x7d,
0xb6,0xdb,0x6d,0xb6,0xdb,0x6d,0xb7,0xff,0x6d,0xb6,
0xdb,0x6d,0x96,0xc0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0x8,0xf,0x8e,0x2c,
0xcc,0x18,0xc0,0x60,0,0x4d,0x8c,0x60,0x6f,0xe6,
0xd8,0xcd,0x86,0x1,0x9f,0x31,0x8a,0x25,0xfd,0x9b,
0xb,0x36,0x18,0x63,0x66,0xc3,0x6c,0xc3,0x76,0xce,
0xc6,0xc1,0xad,0x9a,0x31,0x8c,0xc7,0xf,0xf1,0xb0,
0x60,0xc3,0x9,0x80,0,0xdb,0x2c,0x4d,0x86,0x4d,
0xb6,0xde,0x6d,0xb6,0xdb,0x65,0x36,0xd,0x9b,0x28,
0xd8,0xf3,0x66,0x18,0x99,0xb0,0,0,0,0,
0,0,0,0,0,0,0,0,0x5,0x43,
0xe,0x18,0x19,0x4,0xe8,0x1b,0x1,0x1,0x4a,0,
0x60,0x3,0x62,0x80,0,0x6c,0x4c,0x2a,0x13,0xc,
0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfc,0x30,0xb0,0xc3,
0xc,0x36,0xdb,0x36,0x76,0x36,0x36,0x36,0x36,0x33,
0xdc,0xd9,0xb3,0x66,0xcc,0x30,0xf9,0x96,0xdb,0x6d,
0xb6,0xdb,0x6c,0x31,0x86,0x18,0x61,0xb6,0xdb,0x6d,
0xb6,0xdb,0x6d,0xb0,0x1b,0x6d,0xb6,0xdb,0x6d,0x96,
0xc0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0xa,0x2a,0x56,0xdc,0x18,0xc0,
0x62,0x2,0x8d,0x8c,0xc3,0x61,0xb6,0xd9,0x8d,0xb6,
0x48,0xc0,0x60,0xa,0x49,0x8d,0x99,0x9b,0x66,0x18,
0x33,0x66,0xdb,0x66,0xc3,0x26,0xce,0x6c,0xc0,0xd9,
0x9b,0x31,0x8c,0xc7,0x6,0x63,0x18,0x61,0x83,0x5,
0x80,0,0xdb,0x6d,0x6d,0xb6,0x6d,0xb6,0xdb,0x6d,
0xb6,0xdb,0x6d,0xb6,0x6d,0x9b,0x38,0xd9,0x99,0xec,
0x18,0x98,0,0,0,0,0,0,0,0,
0,0,0,0,0,0xd,0xb3,0x5b,0x7e,0x4e,
0x2,0x10,0xd,0x80,0,0x84,0,0,0x3,0x62,
0x80,0,0xd8,0x5e,0x24,0x17,0x98,0xc6,0xc6,0xc6,
0xc6,0xc6,0xc6,0xcc,0x19,0xb0,0xc3,0xc,0x36,0xdb,
0x66,0x73,0x63,0x63,0x63,0x63,0x66,0x6d,0x99,0xb3,
0x66,0xcc,0x30,0xc1,0x96,0xdb,0x6d,0xb6,0xdb,0x6d,
0xb5,0xb6,0xdb,0x6d,0xb6,0xdb,0x6d,0xb6,0xdb,0x6d,
0xb1,0x9b,0x6d,0xb6,0xdb,0x3d,0xb3,0xc0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0xc,0xa,0x1c,0x8c,0x76,0x8,0x80,0x2,0x2,0x87,
0xc,0xf9,0xc1,0x9c,0x71,0x87,0x1c,0x48,0,0x1,
0x89,0xb1,0x8d,0xf0,0xf3,0xc7,0xd8,0x1d,0x66,0xce,
0x63,0xfb,0x26,0xc6,0x38,0xc0,0x79,0x99,0xe1,0x87,
0x82,0x4,0x23,0x18,0x61,0xfb,0x5,0x80,0,0x6f,
0xc7,0x3c,0xe6,0x3d,0xb6,0xd9,0xed,0xb6,0xce,0x78,
0xf6,0x38,0xcd,0x10,0xd9,0x98,0xcf,0x98,0x98,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0xc,0xe6,0xc0,0x18,0x47,0x1,0xe0,0,
0,0,0x78,0x1,0xf8,0x3,0xa2,0x80,0,0,
0x84,0x4e,0x21,0x30,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,
0xcf,0x8f,0x3e,0xfb,0xef,0xb6,0xdb,0xc6,0x31,0xc1,
0xc1,0xc1,0xc1,0xc0,0x17,0xf,0x1e,0x3c,0x78,0x30,
0xc1,0xa3,0x6d,0xb6,0xdb,0x6d,0xb7,0x1c,0xe3,0x8e,
0x39,0xb6,0xce,0x6c,0xe3,0x8e,0x38,0xe0,0xe,0x34,
0xd3,0x4d,0x19,0xe1,0x80,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x8,
0,0,0xd,0x80,0x2,0,0,0,0,0,
0,0,0,0,0x8,0,0,0x4,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x4,0,0,0,0,0,0,
0,0,0x3,0x1,0x80,0,0,0,0,0,
0xc,0,0xc0,0,0,0,0x60,0x30,0,0,
0,0,0,0xc0,0x18,0x98,0,0,0,0,
0,0,0,0,0,0,0,0,0,0xc,
0x80,0,0,0x53,0,0,0,0,0,0,
0,0,0x3,0x2,0x82,0,0,0,0,0,
0x36,0,0,0,0,0,0,0,0x4,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0x8,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x19,
0x81,0x80,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x5,
0,0x4,0,0,0,0,0,0,0,0,
0,0x10,0,0,0x3,0xe0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x3,
0x83,0x81,0xf8,0,0,0,0,0x38,0,0x80,
0,0,0,0x60,0x30,0,0,0,0,0x1,
0x80,0xc,0xb0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0xc,0,0,0,
0x4e,0,0,0,0,0,0,0,0,0x3,
0x2,0x86,0,0,0,0,0,0x1c,0,0,
0,0,0,0,0,0xc,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0x18,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0x31,0x83,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,
};

static WORD SmallBold_ch_ofst[225] = {
0,3,7,12,18,24,32,40,43,47,
51,55,61,64,69,72,76,82,88,94,
100,106,112,118,124,130,136,139,142,147,
153,158,164,175,183,190,198,205,211,217,
225,232,235,241,248,254,264,272,280,287,
295,302,309,316,323,331,342,350,359,366,
370,374,378,383,389,392,398,404,409,415,
421,425,431,437,440,443,449,452,461,467,
473,479,485,489,495,499,505,511,519,526,
532,538,543,546,551,557,560,563,566,569,
572,575,578,581,584,587,590,593,596,599,
602,605,608,611,614,617,620,623,626,629,
632,635,638,641,644,647,650,653,656,659,
663,669,675,681,688,691,697,700,710,715,
722,729,734,744,747,751,757,760,763,766,
772,778,781,784,787,792,799,808,817,826,
832,840,848,856,864,872,880,890,898,904,
910,916,922,925,928,931,934,941,949,957,
965,973,981,989,995,1003,1010,1017,1024,1031,
1040,1047,1053,1059,1065,1071,1077,1083,1089,1098,
1103,1109,1115,1121,1127,1130,1133,1136,1139,1145,
1151,1157,1163,1169,1175,1181,1187,1193,1199,1205,
1211,1217,1223,1229,1235,
};

static struct font_hdr SmallBold_font = {
STPROP, 10, "AdobeHelv-B-R-N--10-100-75-75-P", 32, 255,
13, 11, 6, 2, 2,
11, 11, 0, 0,
0, 0, 0x5555, (WORD)0xaaaa,
0x0, NULL,
SmallBold_ch_ofst, SmallBold_data,
217, 13,
NULL,
0, 0, /* x/y offset */
};

MgFont *mgSmallBoldFont()
{
return &SmallBold_font;
}

/* SmallFixed.c - compiled data for font -Misc-Fixed-M-R-N--10-100-75-75 */
/* generated source code by utils/bdfToGem, do not edit */
/* BDF data file input: misc/6x10-L1.bdf */

static UBYTE SmallFixed_data[2160] = {
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0x20,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0x50,0,0,0,0,0x3e,0,0x3,
0x1c,0x10,0,0,0,0x80,0,0x41,0xc,0,
0x40,0x42,0x12,0x50,0x80,0,0x40,0x42,0x14,0x40,
0x42,0x14,0,0xa4,0x4,0x20,0xa5,0,0x1,0x1,
0x8,0x50,0x40,0,0x40,0x42,0xa,0,0x80,0,
0x40,0x42,0,0x40,0x82,0,0,0xa4,0x4,0x20,
0xa0,0,0x1,0x1,0x8,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0x85,0x14,0x21,
0x24,0x8,0x11,0,0,0,0,0x2,0x20,0x87,
0x3e,0x13,0xe3,0x3e,0x71,0xc0,0,0x8,0x4,0x1c,
0x70,0x8f,0x1c,0xf3,0xef,0x9c,0x89,0xc3,0xa2,0x82,
0x28,0x9c,0xf1,0xcf,0x1c,0xfa,0x28,0xa2,0x8a,0x2f,
0x9c,0x81,0xc2,0,0x10,0x8,0,0x8,0x3,0,
0x80,0x80,0xa0,0x60,0,0,0,0,0,0x40,
0,0,0,0,0x6,0x21,0x84,0x80,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x80,0xc,0x2,0x22,0x1c,0x1,0xc3,
0x80,0,0x7,0,0x20,0x4,0x82,0x20,0x7,0x80,
0x1,0x83,0,0xc3,0x2,0x8,0x20,0x85,0x2c,0x1,
0x43,0xdc,0xfb,0xef,0xbe,0x20,0x85,0,0xf1,0x42,
0x8,0x51,0x40,0,0x70,0x82,0x14,0,0x88,0x1c,
0x20,0x85,0x14,0x51,0x40,0,0x20,0x85,0x14,0x21,
0x5,0x14,0xc1,0x42,0x8,0x51,0x45,0,0,0x82,
0x14,0x50,0x40,0x14,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x85,0x14,0x72,0xaa,0x8,0x20,0x88,
0x88,0,0,0x2,0x51,0x88,0x82,0x32,0x4,0x2,
0x8a,0x22,0x8,0x10,0x2,0x22,0x89,0x44,0xa2,0x4a,
0x8,0x22,0x88,0x81,0x24,0x82,0x28,0xa2,0x8a,0x28,
0xa2,0x22,0x28,0xa2,0x8a,0x20,0x90,0x80,0x45,0,
0,0x8,0,0x8,0x4,0x80,0x80,0,0x20,0x20,
0,0,0,0,0,0x40,0,0,0,0,
0x8,0x20,0x4a,0x80,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x2,
0x12,0x2,0x22,0x20,0x2,0x24,0x80,0,0x8,0x80,
0x50,0x81,0xc,0,0xe,0x80,0,0x84,0x80,0x41,
0x4,0,0x71,0xc7,0x1c,0x71,0xc5,0x22,0x82,0x8,
0x20,0x71,0xc7,0x1c,0x4a,0x27,0x1c,0x71,0xc7,0,
0x9a,0x28,0x80,0x8a,0x2f,0x22,0,0,0,0,
0x80,0,0,0,0,0,0,0,0x30,0,
0,0,0,0x8,0,0,0,0,0x88,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x85,
0x3e,0xa1,0x4a,0x8,0x40,0x45,0x8,0,0,0x4,
0x8a,0x80,0x84,0x52,0xc8,0x4,0x8a,0x67,0x1c,0x23,
0xe1,0x4,0x9a,0x24,0xa0,0x4a,0x8,0x20,0x88,0x81,
0x28,0x83,0x6c,0xa2,0x8a,0x28,0xa0,0x22,0x28,0xa2,
0x51,0x41,0x10,0x40,0x48,0x80,0x1,0xcb,0x1c,0x69,
0xc4,0x1e,0xb1,0x81,0xa2,0x23,0x4b,0x1c,0xb1,0xab,
0x1c,0xf2,0x28,0xa2,0x8a,0x2f,0x84,0x20,0x89,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0x87,0x90,0x89,0x42,0x38,
0x2,0xa5,0x89,0,0xe,0x80,0x20,0x82,0x2,0x2,
0x2e,0x80,0,0x84,0xa4,0x41,0x2,0x8,0x8a,0x28,
0xa2,0x8a,0x29,0x20,0x82,0x8,0x20,0x20,0x82,0x8,
0x4b,0x28,0xa2,0x8a,0x28,0xa2,0x9a,0x28,0xa2,0x8a,
0x28,0xa4,0x71,0xc7,0x1c,0x71,0xc7,0x9c,0x71,0xc7,
0x1c,0x61,0x86,0x18,0x72,0xc7,0x1c,0x71,0xc7,0,
0x7a,0x28,0xa2,0x8a,0x2f,0x22,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0x80,0x14,0x70,0x84,0,
0x40,0x4f,0xbe,0x3,0xe0,0x8,0x88,0x83,0xc,0x93,
0x2b,0x4,0x71,0xa2,0x8,0x40,0,0x88,0xaa,0x27,
0x20,0x4b,0xcf,0x20,0xf8,0x81,0x30,0x82,0xaa,0xa2,
0xf2,0x2f,0x1c,0x22,0x25,0x2a,0x20,0x82,0x10,0x20,
0x40,0,0,0x2c,0xa2,0x9a,0x2f,0x22,0xc8,0x80,
0xa4,0x22,0xac,0xa2,0xca,0x6c,0xa0,0x42,0x28,0xa2,
0x52,0x21,0x18,0x20,0x60,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0x8a,0x38,0x70,0x80,0x24,0x3,0x22,0x92,0x79,
0xec,0x80,0x3,0xe7,0x9c,0x2,0x26,0x88,0x1,0xc3,
0x12,0xe7,0xac,0x88,0x8a,0x28,0xa2,0x8a,0x29,0xe0,
0xf3,0xcf,0x3c,0x20,0x82,0x8,0xea,0xa8,0xa2,0x8a,
0x28,0x94,0xaa,0x28,0xa2,0x89,0x4f,0x28,0x8,0x20,
0x82,0x8,0x21,0x62,0x8a,0x28,0xa2,0x20,0x82,0x8,
0x8b,0x28,0xa2,0x8a,0x28,0xbe,0x9a,0x28,0xa2,0x8a,
0x28,0xa2,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0x80,0x3e,0x29,0x4a,0x80,0x40,0x45,0x8,0,
0,0x10,0x88,0x84,0x2,0xf8,0x2c,0x88,0x88,0x20,
0,0x23,0xe1,0x8,0xb3,0xe4,0xa0,0x4a,0x8,0x26,
0x88,0x81,0x28,0x82,0x29,0xa2,0x82,0x2a,0x2,0x22,
0x25,0x2a,0x50,0x84,0x10,0x10,0x40,0,0x1,0xe8,
0xa0,0x8b,0xe4,0x22,0x88,0x80,0xb8,0x22,0xa8,0xa2,
0x8a,0x28,0x1c,0x42,0x25,0x2a,0x22,0x62,0x4,0x20,
0x80,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0x8a,0x10,0x53,
0xe2,0x12,0x2,0xa0,0x24,0x8,0xc,0x80,0,0x80,
0,0x2,0x22,0x80,0,0,0x9,0xc,0x51,0x90,
0xfb,0xef,0xbe,0xfb,0xef,0x20,0x82,0x8,0x20,0x20,
0x82,0x8,0x4a,0x68,0xa2,0x8a,0x28,0x88,0xca,0x28,
0xa2,0x88,0x88,0x24,0x79,0xe7,0x9e,0x79,0xe7,0xe0,
0xfb,0xef,0xbe,0x20,0x82,0x8,0x8a,0x28,0xa2,0x8a,
0x28,0x80,0xaa,0x28,0xa2,0x8a,0x68,0xa6,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0x14,0x72,
0xa9,0,0x20,0x88,0x88,0x30,0x2,0x20,0x50,0x88,
0x22,0x12,0x28,0x90,0x88,0x42,0xc,0x10,0x2,0,
0x82,0x24,0xa2,0x4a,0x8,0x22,0x88,0x89,0x24,0x82,
0x28,0xa2,0x82,0xa9,0x22,0x22,0x25,0x36,0x88,0x88,
0x10,0x8,0x40,0,0x2,0x2c,0xa2,0x9a,0x4,0x1e,
0x88,0x80,0xa4,0x22,0xa8,0xa2,0xca,0x68,0x2,0x4a,
0x65,0x2a,0x51,0xa4,0x8,0x20,0x40,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x8a,0x12,0x70,0x82,0xe,0x2,0x27,
0x92,0,0x8,0x80,0,0x80,0,0x3,0x22,0x80,
0,0x7,0x92,0x14,0x12,0xa2,0x8a,0x28,0xa2,0x8a,
0x29,0x22,0x82,0x8,0x20,0x20,0x82,0x8,0x4a,0x28,
0xa2,0x8a,0x28,0x94,0xca,0x28,0xa2,0x88,0x88,0x22,
0x8a,0x28,0xa2,0x8a,0x29,0x22,0x82,0x8,0x20,0x20,
0x82,0x8,0x8a,0x28,0xa2,0x8a,0x28,0x88,0xca,0x69,
0xa6,0x99,0xa8,0x9a,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x80,0x14,0x22,0x46,0x80,0x11,0,
0,0x20,0x7,0x20,0x23,0xef,0x9c,0x11,0xc7,0x10,
0x71,0x87,0x8,0x8,0x4,0x8,0x72,0x2f,0x1c,0xf3,
0xe8,0x1c,0x89,0xc6,0x22,0xfa,0x28,0x9c,0x81,0xc8,
0x9c,0x21,0xc2,0x22,0x88,0x8f,0x9c,0x9,0xc0,0,
0x1,0xeb,0x1c,0x69,0xc4,0x2,0x89,0xc4,0xa2,0x72,
0x28,0x9c,0xb1,0xa8,0x3c,0x31,0xa2,0x14,0x88,0x2f,
0x86,0x21,0x80,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x87,
0xac,0x88,0x82,0x2,0x1,0xc0,0x9,0,0x7,0,
0x3,0xe0,0,0x2,0xc2,0x80,0,0,0x24,0x3c,
0x27,0x9c,0x8a,0x28,0xa2,0x8a,0x29,0xdc,0xfb,0xef,
0xbe,0x71,0xc7,0x1c,0xf2,0x27,0x1c,0x71,0xc7,0x22,
0x71,0xc7,0x1c,0x70,0x88,0x2c,0x79,0xe7,0x9e,0x79,
0xe7,0xdc,0x71,0xc7,0x1c,0x71,0xc7,0x1c,0x72,0x27,
0x1c,0x71,0xc7,0,0xf1,0xa6,0x9a,0x68,0x2f,0x2,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0x40,0x2,0,
0,0,0,0,0,0,0,0x2,0x10,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0x20,0,0,0,0,
0,0,0,0,0,0x3e,0,0,0,0,
0,0x22,0,0x4,0x80,0,0,0,0x80,0x20,
0,0,0,0,0x2,0x20,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0x2,0,0,0x80,0x1c,
0,0,0,0,0,0,0,0,0,0x2,
0,0,0x10,0,0,0x4,0x70,0x80,0,0,
0,0,0,0x8,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0x8,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x2,0x28,0x22,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0x1c,0,0x3,
0,0,0,0,0x80,0x20,0,0,0,0,
0x1,0xc0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0x20,0,
0,0,0,0,0,0,0,0,0,0x10,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0x10,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0x1,
0xc8,0x1c,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
};

static WORD SmallFixed_ch_ofst[225] = {
0,6,12,18,24,30,36,42,48,54,
60,66,72,78,84,90,96,102,108,114,
120,126,132,138,144,150,156,162,168,174,
180,186,192,198,204,210,216,222,228,234,
240,246,252,258,264,270,276,282,288,294,
300,306,312,318,324,330,336,342,348,354,
360,366,372,378,384,390,396,402,408,414,
420,426,432,438,444,450,456,462,468,474,
480,486,492,498,504,510,516,522,528,534,
540,546,552,558,564,570,576,582,588,594,
600,606,612,618,624,630,636,642,648,654,
660,666,672,678,684,690,696,702,708,714,
720,726,732,738,744,750,756,762,768,774,
780,786,792,798,804,810,816,822,828,834,
840,846,852,858,864,870,876,882,888,894,
900,906,912,918,924,930,936,942,948,954,
960,966,972,978,984,990,996,1002,1008,1014,
1020,1026,1032,1038,1044,1050,1056,1062,1068,1074,
1080,1086,1092,1098,1104,1110,1116,1122,1128,1134,
1140,1146,1152,1158,1164,1170,1176,1182,1188,1194,
1200,1206,1212,1218,1224,1230,1236,1242,1248,1254,
1260,1266,1272,1278,1284,1290,1296,1302,1308,1314,
1320,1326,1332,1338,1344,
};

static struct font_hdr SmallFixed_font = {
STPROP, 10, "-Misc-Fixed-M-R-N--10-100-75-75", 32, 255,
10, 8, 5, 2, 2,
6, 6, 0, 0,
0, 0, 0x5555, (WORD)0xaaaa,
0x0, NULL,
SmallFixed_ch_ofst, SmallFixed_data,
216, 10,
NULL,
0, 0, /* x/y offset */
};

MgFont *mgSmallFixedFont()
{
return &SmallFixed_font;
}
