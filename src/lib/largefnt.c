#include "common.h"
#include "memgfx.h"
#include "gemfont.h"

static char const rcsid[] = "$Id: largefnt.c,v 1.4 2003/05/06 07:33:43 kate Exp $";

static UBYTE largefnt_data[] = {
0x0,0x0,0x0,0x0,0x0,0x0,0x40,0x0,
0x0,0x0,0x0,0x0,0x0,0xa0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0xf,0xc0,
0x1,0xf8,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x18,0x18,0x0,0xe,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x83,0x0,0x30,0x60,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x70,0x40,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x7,0xe0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x0,
0x1,0xb0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0xf,0xc0,0x1,0xf8,0x40,0x0,
0x18,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x38,0x70,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x18,0x18,0x0,
0x11,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x83,
0x0,0x30,0x60,0x0,0x0,0x0,0x0,0x0,
0x60,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0xff,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x1f,0xf0,0xf,
0x0,0x31,0x8c,0x4,0x8,0x0,0xf0,0x0,
0x0,0x3,0xc0,0x1,0x83,0x18,0x4,0x0,
0x0,0x0,0x0,0x0,0x1,0x3,0x80,0x2,
0x0,0xfc,0x1,0xf0,0x0,0xf0,0x1f,0xf0,
0x1e,0x7,0xff,0x3,0xc0,0xc,0x0,0x0,
0x0,0x0,0x0,0x0,0xfc,0x0,0x7e,0x0,
0x1,0x80,0x1f,0xfe,0x0,0x1f,0x23,0xff,
0x80,0x7f,0xff,0x1f,0xff,0xc0,0x3f,0x90,
0x7f,0x87,0xfb,0xfc,0xf,0xf7,0xf8,0xfc,
0x3f,0xc0,0xf,0xe0,0x3,0xf7,0xe0,0x3f,
0x80,0xfe,0x3,0xff,0xc0,0xf,0xe0,0x3f,
0xfc,0x0,0x1e,0x13,0xff,0xfc,0xff,0x3,
0xfb,0xfc,0xf,0xef,0xf3,0xfc,0x7f,0x7e,
0x7,0xe7,0xf0,0x3f,0x8f,0xff,0x8c,0x10,
0x0,0x18,0xe0,0x0,0x30,0x0,0x0,0x80,
0x0,0x0,0x0,0x10,0x0,0x3,0xe0,0x0,
0x4,0x0,0x20,0x8,0x40,0x0,0x80,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x63,0x18,0x7,0x4,0x7,0x90,0xc1,
0x80,0x38,0x4,0x0,0xc1,0x80,0x70,0x0,
0x70,0x0,0x0,0x30,0x30,0xc1,0xc1,0x98,
0x40,0x1,0x80,0x0,0x11,0x0,0x0,0x0,
0x0,0x3,0xff,0xf0,0x8,0x1,0x8c,0x2,
0x0,0x70,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x1f,0x0,0x1f,0x60,0x7,0x0,0xe0,
0x38,0x1,0xc0,0xe0,0x81,0x7,0x0,0xc,
0x3,0x0,0x0,0x0,0x0,0x40,0x0,0x0,
0xfc,0x0,0x1f,0x80,0xff,0x60,0x30,0x78,
0xf,0xf8,0x0,0x0,0x0,0x33,0xde,0x4,
0x8,0x3,0xde,0x1e,0x4,0x6,0x60,0x3,
0xc6,0xc,0x24,0x80,0x0,0x0,0x0,0x0,
0x3,0xe,0xe0,0x6,0x3,0xff,0x7,0x1c,
0x0,0xf0,0x1f,0xe0,0x70,0xf,0xfe,0xe,
0x70,0x3f,0x0,0x0,0x0,0x0,0x0,0x3,
0xff,0x1,0xc3,0x80,0x1,0x80,0x7,0x83,
0x80,0x70,0xe0,0xf0,0x60,0x1e,0x1,0x7,
0x80,0x40,0xe0,0x70,0x1e,0x1,0xe0,0xf0,
0x3,0xc1,0xe0,0x30,0xf,0x0,0x3,0xe0,
0x3,0xc1,0xe0,0xe,0x3,0x83,0x80,0xf0,
0x70,0x38,0x38,0xf,0x7,0x0,0x31,0xf2,
0x6,0x4,0x3c,0x0,0xe0,0xf0,0x3,0x83,
0xc0,0xf0,0x1c,0x1c,0x1,0x81,0xc0,0xe,
0x8,0x3,0xc,0x18,0x0,0x19,0xb0,0x0,
0x60,0x0,0x1,0x80,0x0,0x0,0x0,0x30,
0x0,0x6,0x30,0x0,0xc,0x0,0x70,0x1c,
0xc0,0x1,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0xc3,0xc,0xf,
0xf8,0x1c,0x70,0xc1,0x80,0x60,0xe,0x0,
0xc1,0x80,0x18,0x0,0x88,0x0,0x0,0x78,
0x30,0xc0,0x61,0x98,0xe0,0x1,0x80,0x0,
0x11,0x0,0x0,0x0,0x0,0x1,0xe0,0x10,
0x1c,0x1,0x8c,0x7,0x0,0x18,0x0,0x0,
0x7,0xe0,0x3e,0x0,0x0,0x3f,0xc0,0x71,
0xc0,0xc,0x1,0x80,0x60,0x3,0x1,0xff,
0x0,0x0,0x0,0xc,0x7,0x80,0x0,0x0,
0x0,0x40,0x18,0x3,0x87,0x0,0x70,0xe0,
0xff,0x70,0x71,0xfc,0x0,0x3,0xe0,0x1f,
0x0,0x7b,0x9c,0x4,0x8,0xe,0x43,0xb3,
0x4,0xc,0x30,0x1,0xc6,0xc,0x15,0x0,
0x0,0x0,0x0,0x0,0x2,0x18,0x30,0xe,
0x6,0x3,0xc,0x6,0x1,0xb0,0x20,0x0,
0xc0,0x10,0x6,0x18,0x18,0x61,0x80,0x0,
0x0,0x0,0x0,0x7,0x3,0x83,0x0,0xc0,
0x1,0x80,0x3,0x1,0xc0,0xc0,0x30,0x60,
0x18,0xc,0x0,0x83,0x0,0x21,0x80,0x18,
0xc,0x0,0xc0,0x60,0x1,0x80,0xc0,0x60,
0x6,0x0,0x1,0xe0,0x3,0x80,0xf0,0x4,
0x6,0x0,0xc0,0x60,0x30,0x60,0xc,0x6,
0x3,0x0,0x60,0x74,0x6,0x2,0x18,0x0,
0x40,0x60,0x1,0x1,0x80,0x60,0xc,0xe,
0x1,0x0,0xe0,0x4,0x10,0x7,0xc,0x8,
0x0,0x1b,0x18,0x0,0x60,0x0,0x3,0x80,
0x0,0x0,0x0,0x70,0x0,0xc,0x30,0x0,
0x1c,0x0,0x20,0x9,0xc0,0x3,0x80,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x20,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0xc3,0xc,0x10,0x70,0x30,0x18,0x0,
0x0,0x80,0x1b,0x0,0x0,0x0,0x4,0x0,
0x88,0x0,0x0,0xcc,0x0,0x0,0x10,0x1,
0xb0,0x1,0x80,0x0,0xe,0x0,0x0,0x0,
0x0,0x3,0x60,0x8,0x36,0x0,0x0,0xd,
0x80,0x4,0x0,0x7c,0x3,0xc0,0x1c,0x0,
0x0,0x30,0xc0,0xc0,0xe0,0x10,0x2,0x0,
0x80,0x4,0x2,0xe,0x1f,0x80,0xf8,0x0,
0x7,0x80,0x0,0x0,0x0,0x40,0x18,0x6,
0x1,0x80,0xc0,0x30,0x18,0x78,0xf1,0x8e,
0x0,0x0,0x0,0x0,0x0,0x7b,0x18,0x4,
0x8,0x1c,0x41,0xa1,0x8,0x18,0x18,0x0,
0xcc,0x6,0xe,0x0,0x0,0x0,0x0,0x0,
0x6,0x30,0x18,0x6,0x4,0x1,0x98,0x3,
0x1,0x30,0x20,0x1,0x80,0x0,0x4,0x18,
0x18,0xc0,0xc0,0x0,0x0,0x0,0x0,0xc,
0x0,0xc6,0x0,0x60,0x2,0xc0,0x3,0x0,
0xc1,0x80,0x10,0x60,0xc,0xc,0x0,0x83,
0x0,0x23,0x0,0x8,0xc,0x0,0xc0,0x60,
0x1,0x80,0xc0,0xc0,0x6,0x0,0x1,0x70,
0x5,0x80,0xb8,0x4,0xc,0x0,0x60,0x60,
0x18,0xc0,0x6,0x6,0x1,0x80,0xc0,0x34,
0x6,0x2,0x18,0x0,0x40,0x60,0x1,0x1,
0x80,0x60,0x8,0x7,0x2,0x0,0x70,0x8,
0x10,0x6,0xc,0xc,0x0,0x18,0x0,0x0,
0x70,0x0,0x1,0x80,0x0,0x0,0x0,0x30,
0x0,0xc,0x0,0x0,0xc,0x0,0x0,0x0,
0xc0,0x1,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x20,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0xc3,0xc,0x0,
0x0,0x60,0x8,0x0,0x0,0x0,0x31,0x80,
0x0,0x0,0x0,0x0,0x88,0x0,0x1,0x86,
0x0,0x0,0x0,0x3,0x18,0x2,0xc0,0x0,
0x16,0x0,0x0,0x0,0x0,0x2,0x60,0x8,
0x63,0x0,0x0,0x18,0xc0,0x0,0x1,0xc7,
0x1,0x80,0x8,0x0,0x0,0x60,0x61,0x81,
0xb0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0xf,0xc0,0x70,0x0,0x3,0x0,0x0,0x0,
0x0,0x60,0x18,0xc,0x0,0xc1,0x80,0x18,
0x18,0x7d,0xf3,0x6,0x0,0x0,0x0,0x0,
0x0,0x7b,0x18,0x4,0x8,0x18,0x40,0xa1,
0x8,0x18,0x18,0x0,0xcc,0x6,0x7f,0xc0,
0x0,0x0,0x0,0x0,0x4,0x30,0x18,0x6,
0x0,0x1,0x80,0x3,0x3,0x30,0x40,0x3,
0x80,0x0,0x4,0x30,0xc,0xc0,0xe3,0x18,
0x6,0x0,0x18,0xc,0x0,0xce,0x0,0x70,
0x2,0xc0,0x3,0x0,0xe3,0x80,0x8,0x60,
0x6,0xc,0x0,0x3,0x0,0x7,0x0,0x4,
0xc,0x0,0xc0,0x60,0x1,0x80,0xc1,0x80,
0x6,0x0,0x1,0x70,0x5,0x80,0xb8,0x4,
0x1c,0x0,0x70,0x60,0x19,0xc0,0x7,0x6,
0x1,0x80,0xc0,0x10,0x6,0x0,0x18,0x0,
0x40,0x30,0x2,0x1,0x80,0x60,0x8,0x7,
0x4,0x0,0x70,0x8,0x0,0xe,0xc,0x4,
0x0,0x18,0x0,0x0,0x78,0x0,0x1,0x80,
0x0,0x0,0x0,0x30,0x0,0xc,0x0,0x0,
0xc,0x0,0x0,0x0,0xc0,0x1,0x80,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x60,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x83,0x6,0x0,0x0,0xe0,0x4,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x70,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x2,0xc0,0x0,0x16,0x0,0x0,0x0,
0x0,0x6,0x60,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x3,0x1,0x81,0x80,0x8,0x0,
0x0,0x60,0x3,0x81,0xb8,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x5,0xc0,0x20,0x0,
0x0,0x0,0x0,0x0,0x1,0xb8,0x18,0x1c,
0x0,0xe3,0x80,0x1c,0x18,0x6f,0xb3,0x6,
0x0,0x0,0x0,0x0,0x0,0x79,0x8c,0xff,
0xff,0xd8,0x40,0x21,0x10,0x18,0x18,0x1,
0x8c,0x6,0xe,0x0,0xc0,0x0,0x0,0x0,
0xc,0x60,0xc,0x6,0x0,0x0,0xc0,0x3,
0x2,0x30,0x40,0x3,0x0,0x0,0x8,0x30,
0xd,0x80,0x67,0xbc,0xc,0x0,0xc,0x0,
0x0,0xcc,0x2,0x30,0x4,0x60,0x3,0x0,
0xe3,0x0,0x0,0x60,0x6,0xc,0x0,0x3,
0x0,0x6,0x0,0x0,0xc,0x0,0xc0,0x60,
0x1,0x80,0xc3,0x0,0x6,0x0,0x1,0x70,
0x5,0x80,0x9c,0x4,0x18,0x0,0x30,0x60,
0x19,0x80,0x3,0x6,0x1,0x80,0xc0,0x10,
0x6,0x0,0x18,0x0,0x40,0x30,0x2,0x0,
0xc0,0x30,0x8,0x3,0x84,0x0,0x38,0x10,
0x0,0x1c,0xc,0x6,0x0,0x18,0x0,0x0,
0x30,0x0,0x1,0x80,0x0,0x0,0x0,0x30,
0x0,0xc,0x0,0x0,0xc,0x0,0x0,0x0,
0xc0,0x1,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x1,0x83,0x6,0x0,
0x0,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x4,0x60,0x0,
0x23,0x0,0x0,0x0,0x0,0x4,0x60,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x0,
0xc1,0x80,0x8,0x0,0x20,0x60,0x3,0x3,
0x18,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x4,0xe0,0x20,0xc,0x0,0x1,0x8d,0x8c,
0x2,0x19,0xff,0x98,0x0,0x63,0x0,0xc,
0x18,0x67,0x33,0x4,0x70,0x0,0x0,0x7,
0x0,0x78,0xc6,0x4,0x8,0x1c,0x40,0x33,
0x10,0xc,0x30,0x3,0x18,0x3,0x15,0x0,
0xc0,0x0,0x0,0x0,0x8,0x60,0xc,0x6,
0x0,0x0,0xc0,0x6,0x6,0x30,0x80,0x7,
0x0,0x0,0x8,0x18,0x19,0x80,0x77,0xbc,
0x18,0x0,0x6,0x0,0x0,0xdc,0x3e,0x18,
0x4,0x60,0x3,0x0,0xc7,0x0,0x0,0x60,
0x3,0xc,0x0,0x3,0x0,0xe,0x0,0x0,
0xc,0x0,0xc0,0x60,0x1,0x80,0xc6,0x0,
0x6,0x0,0x1,0x38,0x9,0x80,0x8e,0x4,
0x38,0x0,0x38,0x60,0x1b,0x80,0x3,0x86,
0x1,0x80,0xc0,0x0,0x6,0x0,0x18,0x0,
0x40,0x18,0x4,0x0,0xc0,0x70,0x10,0x1,
0xc8,0x0,0x1c,0x20,0x0,0x18,0xc,0x2,
0x0,0x18,0x0,0x0,0x0,0x3e,0x1,0x9e,
0x0,0x70,0xf,0xb0,0x1c,0x3f,0x0,0x3c,
0xc,0xf0,0x10,0x4,0xc3,0xc1,0x84,0xf0,
0x78,0x9,0xe0,0x7,0x0,0xbe,0x0,0xf9,
0x9,0xc3,0xcb,0xfc,0x70,0x73,0xf0,0x7b,
0xfb,0xf8,0xfb,0xf1,0xf3,0xf0,0x3c,0xff,
0xe1,0x83,0x6,0x0,0x1,0xc0,0x1,0xc1,
0xc0,0x60,0x1f,0x0,0x3e,0x0,0x7c,0x0,
0xf8,0x0,0xc0,0x18,0x3,0x0,0x60,0x20,
0x20,0x4,0x60,0x0,0x23,0x0,0xf,0x83,
0x0,0xc,0x60,0x0,0xc,0x0,0x60,0x70,
0x73,0x83,0x8e,0x0,0xe1,0x80,0x8,0x6,
0x20,0x60,0x7,0x3,0x1c,0x1f,0x0,0x80,
0x60,0x70,0x70,0x9c,0x4,0x70,0x20,0xc,
0x0,0x3,0x18,0xc6,0x6,0x1d,0xff,0xb8,
0x78,0x77,0x1f,0xe,0x18,0x62,0x33,0xc,
0x0,0x0,0x0,0x0,0x0,0x78,0x0,0x4,
0x8,0xf,0x40,0x1e,0x20,0x6,0x60,0x0,
0x18,0x3,0x24,0x80,0xc0,0x0,0x0,0x0,
0x18,0x60,0xc,0x6,0x0,0x1,0x80,0xe,
0x4,0x30,0x80,0x6,0x0,0x0,0x10,0x1c,
0x39,0x80,0x73,0x18,0x30,0x0,0x3,0x0,
0x3,0x98,0x42,0x18,0x4,0x60,0x3,0x1,
0xc6,0x0,0x0,0x60,0x3,0xc,0x0,0x3,
0x0,0xc,0x0,0x0,0xc,0x0,0xc0,0x60,
0x1,0x80,0xcc,0x0,0x6,0x0,0x1,0x38,
0x9,0x80,0x8e,0x4,0x30,0x0,0x18,0x60,
0x1b,0x0,0x1,0x86,0x1,0x80,0xe0,0x0,
0x6,0x0,0x18,0x0,0x40,0x18,0x4,0x0,
0x60,0x58,0x10,0x1,0xd0,0x0,0x1c,0x20,
0x0,0x38,0xc,0x3,0x0,0x18,0x0,0x0,
0x0,0xe3,0x1,0xa7,0x81,0x9c,0x38,0xf0,
0x7f,0x3f,0x0,0xcf,0xcd,0xfc,0x30,0xc,
0xc1,0x1,0x8d,0xfc,0xfe,0x1b,0xf8,0x1f,
0xc1,0xe7,0x83,0x8f,0x1a,0xe6,0x38,0x60,
0x30,0x30,0xe0,0x20,0xe0,0xe0,0x20,0xe0,
0x40,0xe0,0x10,0xc1,0xc3,0x3,0x3,0x0,
0x1,0x80,0x0,0xc0,0xc1,0xf8,0x71,0x80,
0xe3,0x1,0xc6,0x3,0x8c,0x3,0x70,0x7e,
0xf,0xc1,0xf8,0x60,0x60,0x4,0x60,0x0,
0x23,0x0,0x38,0xcf,0xc0,0x8,0x60,0x0,
0x3f,0x1,0xf8,0x30,0x31,0x81,0x8c,0x0,
0x61,0x80,0x8,0x1f,0xc3,0xfc,0x6,0x6,
0xc,0x71,0x81,0x81,0xf8,0x30,0x31,0xbf,
0x4,0x70,0x20,0xc,0x0,0x6,0x30,0x63,
0x4,0x1c,0x18,0x30,0x80,0x36,0x10,0x86,
0x0,0x0,0x3,0x18,0x0,0x0,0x0,0x0,
0x0,0x30,0x0,0x4,0x8,0x7,0xc0,0x0,
0x20,0x3,0xc0,0x0,0x18,0x3,0x4,0x0,
0xc0,0x0,0x0,0x0,0x10,0x60,0xc,0x6,
0x0,0x1,0x80,0x1c,0xc,0x31,0xfe,0x6,
0x18,0x0,0x10,0xf,0xf0,0xc0,0xf0,0x0,
0x60,0xff,0xc1,0x80,0x7,0x18,0x42,0x18,
0x8,0x30,0x3,0x3,0x86,0x0,0x0,0x60,
0x3,0xc,0x2,0x3,0x1,0xc,0x0,0x0,
0xc,0x0,0xc0,0x60,0x1,0x80,0xd8,0x0,
0x6,0x0,0x1,0x1c,0x11,0x80,0x87,0x4,
0x30,0x0,0x18,0x60,0x33,0x0,0x1,0x86,
0x3,0x0,0x78,0x0,0x6,0x0,0x18,0x0,
0x40,0x18,0x4,0x0,0x60,0x58,0x10,0x0,
0xf0,0x0,0xe,0x40,0x0,0x30,0xc,0x1,
0x0,0x18,0x0,0x0,0x1,0xc1,0x81,0xc1,
0xc2,0xc,0x60,0x30,0xc3,0xc,0x1,0x87,
0xe,0x1c,0x70,0x1c,0xc2,0x1,0x9e,0x1f,
0xe,0x3c,0x38,0x30,0x63,0x81,0xc6,0x3,
0x3c,0x4c,0x8,0x60,0x30,0x30,0xe0,0x20,
0xe0,0xe0,0x20,0x70,0x80,0xe0,0x10,0x81,
0x83,0x3,0x3,0x0,0x1,0x80,0x0,0xc0,
0xc3,0x18,0xe0,0xc1,0xc1,0x83,0x83,0x7,
0x6,0x4,0x30,0xc6,0x18,0xc3,0x18,0xe0,
0xe0,0x8,0x30,0x0,0x41,0x80,0x70,0x78,
0xc0,0x18,0x60,0x20,0x61,0x83,0xc,0x30,
0x31,0x81,0x9c,0x0,0x71,0x80,0x8,0x30,
0xc0,0x60,0x6,0x6,0xc,0xe0,0xc3,0x83,
0xc,0x30,0x33,0xc7,0x4,0x38,0x20,0xc,
0x3,0xc,0x60,0x31,0x8c,0x8,0x18,0x30,
0x80,0x36,0x10,0x86,0x0,0x0,0x3,0x3c,
0x0,0x7,0xf7,0xf8,0x0,0x30,0x0,0x4,
0x8,0x1,0xf0,0x0,0x40,0x7,0xc0,0x0,
0x18,0x3,0x0,0x0,0xc0,0x0,0x0,0x0,
0x30,0x60,0xc,0x6,0x0,0x3,0x0,0x78,
0x8,0x31,0xff,0x86,0x7e,0x0,0x20,0x7,
0xe0,0xc0,0xf0,0x0,0xc0,0xff,0xc0,0xc0,
0xc,0x18,0x82,0x18,0x8,0x30,0x3,0xff,
0x6,0x0,0x0,0x60,0x3,0xc,0x2,0x3,
0x1,0xc,0x0,0x0,0xc,0x0,0xc0,0x60,
0x1,0x80,0xf0,0x0,0x6,0x0,0x1,0x1c,
0x11,0x80,0x83,0x84,0x30,0x0,0x18,0x70,
0x73,0x0,0x1,0x87,0x7,0x0,0x3e,0x0,
0x6,0x0,0x18,0x0,0x40,0xc,0x8,0x0,
0x60,0x98,0x20,0x0,0x70,0x0,0x6,0x80,
0x0,0x70,0xc,0x1,0x80,0x18,0x0,0x0,
0x1,0xc1,0x81,0x80,0xc6,0xe,0x60,0x31,
0x81,0x8c,0x3,0x83,0x8c,0xe,0x30,0xc,
0xc4,0x1,0x8c,0xe,0x7,0x18,0x1c,0x60,
0x31,0x80,0xc6,0x3,0x18,0xc,0x8,0x60,
0x30,0x30,0x70,0x40,0x70,0x70,0x40,0x70,
0x80,0x70,0x20,0x83,0x86,0x3,0x1,0x80,
0x1,0x80,0x0,0xc0,0xc6,0xc,0xe0,0xc1,
0xc1,0x83,0x83,0x7,0x6,0xc,0x39,0x83,
0x30,0x66,0xc,0x60,0x60,0x8,0x30,0x0,
0x41,0x80,0x70,0x70,0x60,0x10,0x60,0x20,
0xc0,0xc6,0x6,0x30,0x31,0x81,0x98,0x0,
0x31,0x80,0x8,0x60,0xe0,0x60,0x6,0xc,
0xc,0xe0,0xc1,0x86,0x6,0x30,0x31,0x83,
0x84,0x1c,0x20,0xc,0x3,0x18,0xc0,0x18,
0xcc,0x0,0x18,0x30,0x80,0x36,0x1f,0x6,
0x0,0x0,0x3,0x3e,0x3,0xe0,0x0,0xf,
0x0,0x30,0x0,0x4,0x8,0x0,0x7c,0x0,
0x40,0xc,0xc1,0xe0,0x18,0x3,0x0,0x1f,
0xfe,0x7,0xfe,0x0,0x20,0x60,0xc,0x6,
0x0,0x6,0x0,0x18,0x18,0x30,0x1,0xc6,
0xc3,0x0,0x20,0xe,0xf0,0x61,0xb0,0x1,
0x80,0x0,0x0,0x60,0x18,0x18,0x82,0x10,
0x10,0x18,0x3,0x3,0x86,0x0,0x0,0x60,
0x3,0xf,0xfe,0x3,0xff,0xc,0x0,0xff,
0xf,0xff,0xc0,0x60,0x1,0x80,0xf8,0x0,
0x6,0x0,0x1,0x1c,0x11,0x80,0x83,0x84,
0x30,0x0,0x18,0x6f,0xc3,0x0,0x1,0x86,
0xfc,0x0,0xf,0x80,0x6,0x0,0x18,0x0,
0x40,0xc,0x8,0x0,0x30,0x8c,0x20,0x0,
0x70,0x0,0x7,0x80,0x0,0xe0,0xc,0x0,
0x80,0x18,0x0,0x0,0x0,0x81,0x81,0x80,
0xe4,0xe,0xc0,0x31,0x1,0x8c,0x3,0x83,
0x8c,0x6,0x30,0xc,0xc8,0x1,0x8c,0x6,
0x3,0x18,0xc,0x60,0x31,0x80,0xec,0x3,
0x18,0xc,0x0,0x60,0x30,0x30,0x70,0x40,
0x70,0x70,0x40,0x39,0x0,0x70,0x20,0x7,
0xc,0x3,0x0,0xc0,0x1,0x80,0x0,0xc0,
0xc4,0xc,0x40,0xc0,0x81,0x81,0x3,0x2,
0x6,0x8,0x39,0x3,0x20,0x64,0xc,0x60,
0x60,0x10,0x18,0x0,0x80,0xc0,0x20,0x60,
0x60,0x30,0x7f,0xe0,0xc0,0xc6,0x6,0x30,
0x31,0x81,0x98,0x0,0x31,0x80,0x8,0x61,
0x60,0x60,0x6,0xc,0xc,0x40,0xc1,0x86,
0x6,0x30,0x31,0x81,0x84,0x1c,0x20,0x18,
0x3,0x31,0x80,0xc,0x6c,0x0,0x18,0x30,
0x80,0x36,0x12,0x6,0x0,0x0,0x3,0x7,
0x7f,0xfc,0x0,0x0,0x0,0x30,0x0,0x4,
0x8,0x0,0x5e,0x0,0x80,0x18,0x60,0xc0,
0x18,0x3,0x0,0x1f,0xfe,0x7,0xfe,0x0,
0x60,0x60,0xc,0x6,0x0,0xc,0x0,0xc,
0x10,0x30,0x0,0xe7,0x81,0x80,0x40,0x1c,
0x38,0x3f,0x30,0x0,0xc0,0x0,0x0,0xc0,
0x30,0x18,0x82,0x30,0x10,0x18,0x3,0x0,
0xc6,0x0,0x0,0x60,0x3,0xc,0x2,0x3,
0x1,0xc,0x0,0x3c,0xc,0x0,0xc0,0x60,
0x1,0x80,0xdc,0x0,0x6,0x0,0x1,0xe,
0x21,0x80,0x81,0xc4,0x30,0x0,0x18,0x60,
0x3,0x0,0x1,0x86,0x1c,0x0,0x3,0xc0,
0x6,0x0,0x18,0x0,0x40,0x6,0x10,0x0,
0x30,0x8c,0x20,0x0,0x70,0x0,0x3,0x0,
0x0,0xc0,0xc,0x0,0xc0,0x18,0x0,0x0,
0x0,0x3,0x81,0x80,0x6c,0x4,0xc0,0x33,
0xff,0x8c,0x3,0x83,0xc,0x6,0x30,0xc,
0xd0,0x1,0x8c,0x6,0x3,0x18,0xc,0xc0,
0x19,0x80,0x6c,0x3,0x18,0x6,0x0,0x60,
0x30,0x30,0x70,0x40,0x70,0x70,0x40,0x1e,
0x0,0x70,0x20,0x7,0x6,0x3,0x1,0x80,
0x1,0x80,0x0,0xc0,0xcf,0xfc,0x1,0xc0,
0x3,0x80,0x7,0x0,0xe,0x18,0x13,0xff,
0x7f,0xef,0xfc,0x60,0x60,0x10,0x18,0x0,
0x80,0xc0,0x0,0xff,0xe0,0x20,0x60,0x21,
0x80,0x6c,0x3,0x30,0x31,0x81,0x98,0x0,
0x31,0x80,0x8,0xc2,0x30,0x60,0x6,0x18,
0xc,0x1,0xc1,0x8c,0x3,0x30,0x31,0x81,
0x84,0xe,0x20,0x30,0x3,0x18,0xc0,0x18,
0xcc,0x0,0x18,0x30,0x80,0x36,0x11,0x6,
0x0,0x0,0x3,0x3,0x7,0xf8,0x7,0xf0,
0x0,0x30,0x0,0xff,0xff,0xc0,0x47,0x0,
0x80,0x18,0x31,0x80,0x18,0x3,0x0,0x0,
0xc0,0x0,0x0,0x0,0x40,0x60,0xc,0x6,
0x0,0x18,0x0,0x6,0x30,0x30,0x0,0x67,
0x81,0x80,0x40,0x18,0x18,0xc,0x30,0x0,
0x60,0xff,0xc1,0x80,0x30,0x18,0x42,0x60,
0x10,0x18,0x3,0x0,0xe6,0x0,0x0,0x60,
0x3,0xc,0x2,0x3,0x1,0xc,0x0,0x18,
0xc,0x0,0xc0,0x60,0x1,0x80,0xce,0x0,
0x6,0x0,0x1,0xe,0x21,0x80,0x80,0xe4,
0x30,0x0,0x18,0x60,0x3,0x0,0x1,0x86,
0xe,0x0,0x0,0xe0,0x6,0x0,0x18,0x0,
0x40,0x6,0x10,0x0,0x31,0xc,0x40,0x0,
0xb8,0x0,0x3,0x0,0x1,0xc0,0xc,0x0,
0x40,0x18,0x0,0x0,0x0,0x1f,0x81,0x80,
0x6c,0x0,0xc0,0x33,0x0,0xc,0x3,0xc3,
0xc,0x6,0x30,0xc,0xe0,0x1,0x8c,0x6,
0x3,0x18,0xc,0xc0,0x19,0x80,0x6c,0x3,
0x18,0x7,0x80,0x60,0x30,0x30,0x38,0x80,
0x38,0xb8,0x80,0x1e,0x0,0x38,0x40,0xe,
0x3,0x3,0x3,0x0,0x1,0x80,0x0,0xc0,
0xcc,0x0,0xf,0xc0,0x1f,0x80,0x3f,0x0,
0x7e,0x18,0x3,0x0,0x60,0xc,0x0,0x60,
0x60,0x10,0x18,0x0,0x80,0xc0,0x7,0xe0,
0x0,0x60,0x60,0x21,0x80,0x6c,0x3,0x30,
0x31,0x81,0x98,0x0,0x31,0x80,0x8,0xc2,
0x30,0x60,0x6,0x18,0xc,0xf,0xc1,0x8c,
0x3,0x30,0x31,0x81,0x84,0x7,0x20,0xe0,
0x3,0xc,0x60,0x31,0x8e,0x0,0x18,0x30,
0x80,0x36,0x10,0x86,0x0,0x0,0x3,0x3,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4,
0x8,0x0,0x43,0x81,0x0,0x30,0x1b,0x0,
0x18,0x3,0x0,0x0,0xc0,0x0,0x0,0x0,
0xc0,0x60,0xc,0x6,0x0,0x30,0x0,0x6,
0x3f,0xfc,0x0,0x37,0x0,0xc0,0x80,0x30,
0xc,0x0,0x33,0x0,0x30,0xff,0xc3,0x0,
0x30,0x18,0x43,0x80,0x3f,0xfc,0x3,0x0,
0x66,0x0,0x0,0x60,0x3,0xc,0x0,0x3,
0x0,0xc,0x0,0x18,0xc,0x0,0xc0,0x60,
0x1,0x80,0xc7,0x0,0x6,0x0,0x1,0x7,
0x41,0x80,0x80,0xe4,0x30,0x0,0x18,0x60,
0x3,0x0,0x1,0x86,0x7,0x0,0x0,0x70,
0x6,0x0,0x18,0x0,0x40,0x6,0x30,0x0,
0x19,0x6,0x40,0x1,0x9c,0x0,0x3,0x0,
0x1,0x80,0xc,0x0,0x60,0x18,0x0,0x0,
0x0,0x79,0x81,0x80,0x6c,0x0,0xc0,0x33,
0x0,0xc,0x1,0xc6,0xc,0x6,0x30,0xc,
0xf0,0x1,0x8c,0x6,0x3,0x18,0xc,0xc0,
0x19,0x80,0x6c,0x3,0x18,0x1,0xe0,0x60,
0x30,0x30,0x38,0x80,0x38,0xb8,0x80,0xe,
0x0,0x38,0x40,0xc,0x3,0x3,0x3,0x0,
0x1,0x80,0x0,0xc0,0xcc,0x0,0x3c,0xc0,
0x79,0x80,0xf3,0x1,0xe6,0x18,0x3,0x0,
0x60,0xc,0x0,0x60,0x60,0x3f,0xfc,0x1,
0xff,0xe0,0x1e,0x60,0x0,0x7f,0xe0,0x1,
0x80,0x6c,0x3,0x30,0x31,0x81,0x98,0x0,
0x31,0x80,0x8,0xc4,0x30,0x60,0x6,0x30,
0xc,0x3c,0xc1,0x8c,0x3,0x30,0x31,0x81,
0x84,0x7,0x21,0x80,0x7,0x86,0x30,0x63,
0xe,0x0,0x18,0x30,0x78,0x36,0x10,0x46,
0x0,0x0,0x3,0x3,0x0,0x0,0x0,0x3,
0x0,0x0,0x0,0x4,0x8,0x0,0x41,0x81,
0xf,0x30,0xe,0x0,0x18,0x3,0x0,0x0,
0xc0,0x0,0x0,0x0,0x80,0x60,0xc,0x6,
0x0,0x60,0x0,0x3,0x7f,0xfc,0x0,0x37,
0x0,0xc0,0x80,0x30,0xc,0x0,0x37,0x98,
0x18,0x0,0x6,0x0,0x30,0x1c,0x3e,0x0,
0x20,0xc,0x3,0x0,0x77,0x0,0x0,0x60,
0x3,0xc,0x0,0x3,0x0,0xc,0x0,0x18,
0xc,0x0,0xc0,0x60,0x1,0x80,0xc3,0x80,
0x6,0x0,0x1,0x7,0x41,0x80,0x80,0x74,
0x38,0x0,0x38,0x60,0x3,0x80,0x3,0x86,
0x7,0x0,0x0,0x30,0x6,0x0,0x18,0x0,
0x40,0x3,0x20,0x0,0x19,0x6,0x40,0x1,
0x1c,0x0,0x3,0x0,0x3,0x80,0xc,0x0,
0x20,0x18,0x0,0x0,0x0,0xe1,0x81,0x80,
0x6c,0x0,0xc0,0x33,0x0,0xc,0x0,0xfc,
0xc,0x6,0x30,0xc,0xd8,0x1,0x8c,0x6,
0x3,0x18,0xc,0xc0,0x19,0x80,0x6c,0x3,
0x18,0x0,0x70,0x60,0x30,0x30,0x1d,0x0,
0x1d,0x1d,0x0,0x17,0x0,0x1c,0x80,0x1c,
0x1,0x83,0x6,0x0,0x1,0xc0,0x0,0xc0,
0xcc,0x0,0x70,0xc0,0xe1,0x81,0xc3,0x3,
0x86,0x18,0x3,0x0,0x60,0xc,0x0,0x60,
0x60,0x20,0xc,0x1,0x0,0x60,0x38,0x60,
0x0,0xc0,0x60,0x1,0x80,0x6c,0x3,0x30,
0x31,0x81,0x9c,0x0,0x71,0x80,0x8,0xc8,
0x30,0x60,0x7,0x30,0x1c,0x70,0xc1,0x8c,
0x3,0x30,0x31,0x81,0x84,0x3,0xa3,0x0,
0x7,0x83,0x18,0xc6,0x7,0x4,0x18,0x38,
0x0,0x77,0x0,0xe,0x0,0x0,0x3,0x3,
0x0,0x1f,0xc0,0x3f,0x0,0x0,0x0,0x4,
0x8,0x0,0x41,0x82,0x19,0xb0,0xe,0x0,
0xc,0x6,0x0,0x0,0xc0,0x0,0x0,0x1,
0x80,0x60,0xc,0x6,0x0,0xc0,0x0,0x3,
0x0,0x30,0x0,0x33,0x0,0xc1,0x0,0x30,
0xc,0x0,0x67,0xbc,0xc,0x0,0xc,0x0,
0x30,0xc,0x0,0x0,0x40,0x6,0x3,0x0,
0x73,0x0,0x0,0x60,0x6,0xc,0x0,0x3,
0x0,0x6,0x0,0x18,0xc,0x0,0xc0,0x60,
0x1,0x80,0xc1,0xc0,0x6,0x0,0x1,0x7,
0x41,0x80,0x80,0x3c,0x18,0x0,0x30,0x60,
0x1,0x80,0x3,0x6,0x3,0x80,0x80,0x30,
0x6,0x0,0x18,0x0,0x40,0x3,0x20,0x0,
0xe,0x3,0x80,0x2,0xe,0x0,0x3,0x0,
0x7,0x0,0xc,0x0,0x30,0x18,0x0,0x0,
0x1,0xc1,0x81,0x80,0x6e,0x0,0xe0,0x33,
0x80,0xc,0x0,0x80,0xc,0x6,0x30,0xc,
0xcc,0x1,0x8c,0x6,0x3,0x18,0xc,0xc0,
0x19,0x80,0x6e,0x3,0x18,0x0,0x30,0x60,
0x30,0x30,0x1d,0x0,0x1d,0x1d,0x0,0x17,
0x0,0x1c,0x80,0x38,0x1,0x83,0x6,0x0,
0x0,0xc0,0x0,0xc0,0xce,0x0,0xe0,0xc1,
0xc1,0x83,0x83,0x7,0x6,0x1c,0x3,0x80,
0x70,0xe,0x0,0x60,0x60,0x40,0x6,0x2,
0x0,0x30,0x70,0x70,0x0,0x80,0x60,0x1,
0x80,0x6c,0x3,0x30,0x31,0x81,0x8c,0x0,
0x61,0x80,0x8,0xc8,0x30,0x60,0x3,0x60,
0x18,0xe0,0xc1,0x8c,0x3,0x30,0x31,0x81,
0x84,0x1,0xe3,0x0,0x7,0x81,0x8d,0x8c,
0x7,0x88,0x18,0x18,0x0,0x63,0x0,0xc,
0x0,0x0,0x3,0x3,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x4,0x8,0x10,0x41,0x82,
0x10,0xb0,0xe,0x0,0xc,0x6,0x0,0x0,
0xc0,0x0,0x0,0x1,0x0,0x30,0x18,0x6,
0x1,0x80,0x18,0x3,0x0,0x31,0x80,0x63,
0x81,0x81,0x0,0x38,0x1c,0x0,0x63,0x1c,
0x6,0x0,0x18,0x0,0x0,0xe,0x0,0x0,
0x40,0x6,0x3,0x0,0x73,0x0,0x0,0x60,
0x6,0xc,0x0,0x3,0x0,0x6,0x0,0x18,
0xc,0x0,0xc0,0x60,0x41,0x80,0xc0,0xe0,
0x6,0x0,0x1,0x3,0x81,0x80,0x80,0x3c,
0x1c,0x0,0x70,0x60,0x1,0xc0,0x7,0x6,
0x1,0xc0,0x80,0x30,0x6,0x0,0x18,0x0,
0x40,0x1,0xc0,0x0,0xe,0x3,0x80,0x6,
0x7,0x0,0x3,0x0,0x6,0x0,0xc,0x0,
0x10,0x18,0x0,0x0,0x3,0x81,0x81,0x80,
0xce,0x0,0xe0,0x33,0x80,0xc,0x1,0x0,
0xc,0x6,0x30,0xc,0xc6,0x1,0x8c,0x6,
0x3,0x18,0xc,0x60,0x31,0x80,0xce,0x3,
0x18,0x0,0x18,0x60,0x30,0x30,0x1d,0x0,
0x1d,0x1d,0x0,0x23,0x80,0x1c,0x80,0x38,
0x1,0x83,0x6,0x0,0x0,0xc0,0x0,0xc0,
0xce,0x1,0xc0,0xc3,0x81,0x87,0x3,0xe,
0x6,0x1c,0x3,0x80,0x70,0xe,0x0,0x60,
0x60,0x40,0x6,0x2,0x0,0x30,0xe0,0x70,
0x1,0x80,0x60,0x0,0xc0,0xc6,0x6,0x30,
0x31,0x81,0x8e,0x0,0xe1,0x80,0x8,0x70,
0x60,0x60,0x3,0xe0,0x39,0xc0,0xc1,0x86,
0x6,0x30,0x31,0x81,0x84,0x1,0xe3,0x0,
0x37,0x80,0x0,0x0,0x3,0xf0,0x18,0x1c,
0x0,0xe3,0x80,0x1c,0x0,0x0,0x3,0x3,
0x0,0x1,0xfc,0x3f,0x0,0x30,0x0,0x4,
0x8,0x18,0x43,0x4,0x10,0x98,0x1b,0x0,
0xc,0x6,0x0,0x0,0x0,0x60,0x0,0x63,
0x0,0x30,0x18,0x6,0x3,0x0,0x5c,0x7,
0x0,0x31,0xc0,0xe1,0x81,0x82,0x0,0x18,
0x18,0x0,0xc0,0xc,0x0,0x0,0x0,0x0,
0x0,0x6,0x0,0x0,0x40,0x6,0x3,0x0,
0xe1,0x80,0x8,0x60,0xc,0xc,0x0,0x43,
0x0,0x3,0x0,0x10,0xc,0x0,0xc0,0x60,
0xe1,0x80,0xc0,0x70,0x6,0x0,0x21,0x3,
0x81,0x80,0x80,0x1c,0xc,0x0,0x60,0x60,
0x0,0xc0,0x6,0x6,0x1,0xc0,0xc0,0x30,
0x6,0x0,0xc,0x0,0x80,0x1,0xc0,0x0,
0xe,0x3,0x80,0x4,0x7,0x0,0x3,0x0,
0xe,0x0,0x2c,0x0,0x18,0x18,0x0,0x0,
0x3,0x81,0x81,0x80,0xc7,0x2,0x70,0x31,
0xc0,0x8c,0x3,0x80,0xc,0x6,0x30,0xc,
0xc3,0x1,0x8c,0x6,0x3,0x18,0xc,0x60,
0x31,0x80,0xc7,0x3,0x18,0x8,0x18,0x60,
0x38,0x30,0xe,0x0,0xe,0xe,0x0,0x41,
0xc0,0xf,0x0,0x70,0x20,0xc3,0xc,0x0,
0x0,0x60,0x4,0xe0,0xc7,0x5,0xc0,0xc3,
0x81,0x87,0x3,0xe,0x6,0xe,0x9,0xc1,
0x38,0x27,0x4,0x60,0x60,0x40,0x6,0x2,
0x0,0x30,0xe0,0x78,0x21,0x0,0x60,0x4,
0xc0,0xc6,0x6,0x38,0x31,0xc1,0x86,0x0,
0xc0,0xc0,0x10,0x60,0x63,0x60,0x1,0xc0,
0x31,0xc0,0xc1,0x86,0x6,0x38,0x31,0x81,
0x84,0x0,0xe3,0x0,0x37,0x80,0x0,0x0,
0x0,0xe0,0x18,0xc,0x0,0xc1,0x80,0x18,
0x0,0x0,0x3,0x3,0x0,0xff,0xf8,0x0,
0x0,0x78,0x0,0x0,0x0,0x1e,0x4e,0x4,
0x10,0x98,0x31,0x88,0x6,0xc,0x0,0x0,
0x0,0xf0,0x0,0xf2,0x0,0x18,0x30,0x6,
0x6,0x0,0x5e,0xe,0x0,0x31,0xe1,0xc0,
0xc3,0x2,0x0,0x1c,0x38,0x1,0x80,0xc,
0x0,0x0,0x0,0x0,0x0,0x3,0x0,0xc0,
0x80,0x3,0x3,0x0,0xe0,0xc0,0x10,0x60,
0x18,0xc,0x0,0x43,0x0,0x1,0x80,0x20,
0xc,0x0,0xc0,0x60,0xf1,0x80,0xc0,0x38,
0x6,0x0,0x21,0x1,0x1,0x80,0x80,0xc,
0x6,0x0,0xc0,0x60,0x0,0x60,0xc,0x6,
0x0,0xe0,0xe0,0x60,0x6,0x0,0xc,0x0,
0x80,0x1,0xc0,0x0,0x4,0x1,0x0,0x8,
0x3,0x80,0x3,0x0,0xc,0x0,0x2c,0x0,
0x8,0x18,0x0,0x0,0x1,0xc3,0x89,0x81,
0x87,0x84,0x7c,0x79,0xe1,0xc,0x3,0xfe,
0xc,0x6,0x30,0xc,0xc1,0x81,0x8c,0x6,
0x3,0x18,0xc,0x30,0x61,0x81,0x87,0xc7,
0x18,0x8,0x18,0x71,0x1c,0x78,0xe,0x0,
0xe,0xe,0x0,0x41,0xc0,0xf,0x0,0x60,
0x20,0xc3,0xc,0x0,0x0,0x30,0x8,0x71,
0xe7,0x88,0xe1,0xc9,0xc3,0x93,0x87,0x27,
0xe,0x4f,0x11,0xe2,0x3c,0x47,0x88,0x60,
0x60,0x80,0x3,0x4,0x0,0x18,0x70,0xfc,
0x43,0x0,0x60,0x4,0x61,0x83,0xc,0x1c,
0x78,0xe3,0xc3,0x1,0x80,0xc0,0x10,0x30,
0xc4,0xf0,0x10,0xc0,0x60,0xe1,0xc9,0x83,
0xc,0x1c,0x79,0x81,0x84,0x0,0x61,0xc0,
0xe7,0x80,0x0,0x0,0x0,0x40,0x18,0x6,
0x1,0x80,0xc0,0x30,0x0,0x0,0x3,0x26,
0x0,0x0,0x0,0x0,0x0,0x78,0x0,0x0,
0x0,0x3,0xd8,0x8,0x19,0x8c,0xe0,0xf0,
0x6,0xc,0x0,0x0,0x0,0x70,0x0,0xf6,
0x0,0xe,0xe0,0xf,0x7,0xff,0xc7,0xfc,
0x0,0x30,0x7f,0x80,0x7e,0x4,0x0,0xf,
0xf0,0x7,0x0,0x18,0x0,0x0,0x0,0x0,
0x30,0x1,0xc3,0x80,0x80,0x3,0x7,0x83,
0xc0,0x70,0x60,0xf0,0x60,0x1e,0x0,0x87,
0x80,0x0,0xe0,0xc0,0x1e,0x1,0xe0,0xf0,
0x73,0x1,0xe0,0x3e,0xf,0x0,0x43,0x81,
0x3,0xc1,0xc0,0xc,0x3,0x83,0x80,0xf0,
0x0,0x38,0x38,0xf,0x0,0x78,0xf8,0xc0,
0xf,0x0,0x7,0x3,0x0,0x0,0x80,0x0,
0x4,0x1,0x0,0x18,0x1,0xc0,0x7,0x80,
0x1c,0x0,0x4c,0x0,0xc,0x18,0x0,0x0,
0x1,0xfd,0xd0,0xc7,0x3,0xf8,0x3f,0xb0,
0xfe,0x1e,0x1,0xff,0x8c,0x6,0x30,0xc,
0xc0,0xe1,0x8c,0x6,0x3,0x18,0xc,0x1f,
0xc1,0xc7,0x3,0xfb,0x18,0xe,0x30,0x3e,
0x1f,0xb0,0x4,0x0,0x4,0x4,0x0,0x80,
0xe0,0x6,0x0,0xe0,0x40,0xc3,0xc,0x0,
0x0,0x1c,0x30,0x7e,0xc3,0xf0,0xfe,0xf1,
0xfd,0xe3,0xfb,0xc7,0xf7,0x87,0xe0,0xfc,
0x1f,0x83,0xf0,0x60,0x60,0x80,0x3,0x4,
0x0,0x18,0x7f,0x1f,0x87,0x0,0xe0,0x8,
0x3f,0x1,0xf8,0x1f,0xb0,0xfd,0x81,0xc7,
0x0,0x70,0x60,0x5f,0x84,0xdf,0xe1,0xf1,
0xc0,0xfe,0xf1,0x81,0xf8,0x1f,0xb1,0x81,
0x8e,0x0,0x60,0xff,0xc3,0x0,0x0,0x0,
0x0,0x40,0x18,0x3,0x87,0x0,0x70,0xe0,
0x0,0x0,0x3,0x3e,0xf,0xe0,0x3,0xf8,
0x0,0x30,0x0,0x0,0x0,0x0,0xe0,0x8,
0xf,0x7,0x80,0x60,0x3,0x18,0x0,0x0,
0x0,0x30,0x0,0x64,0x0,0x3,0x80,0x1f,
0x87,0xff,0xc1,0xf0,0x0,0x30,0x1e,0x0,
0x18,0x4,0x0,0x3,0xc0,0x3c,0x0,0x30,
0x0,0x0,0x0,0x0,0x30,0x0,0x7e,0x3,
0xe0,0xf,0xdf,0xff,0x0,0x1f,0x83,0xff,
0x80,0x7f,0xff,0x9f,0xe0,0x0,0x3f,0x0,
0x7f,0x87,0xfb,0xfc,0x1e,0x7,0xf8,0xff,
0xbf,0xff,0xcf,0xe1,0xf,0xf7,0xf0,0x4,
0x0,0xfe,0x3,0xfc,0x0,0xf,0xe0,0x3f,
0xc1,0xfe,0x87,0x80,0x3f,0xc0,0x1,0xfc,
0x0,0x0,0x80,0x0,0x4,0x1,0x0,0x7c,
0x3,0xf0,0x1f,0xe0,0x3f,0xff,0xcc,0x0,
0x4,0x18,0x0,0x0,0x0,0x70,0xe0,0x7c,
0x0,0xf0,0xf,0x20,0x3c,0x3f,0x0,0x81,
0xde,0xf,0x78,0xd,0xe1,0xfb,0xde,0xf,
0x7,0xbc,0x1e,0x7,0x1,0xfc,0x0,0xf3,
0x3c,0x9,0xe0,0x1c,0x7,0x20,0x4,0x0,
0x4,0x4,0x3,0xe3,0xf8,0x6,0x1,0xff,
0xc0,0x63,0x18,0x0,0x0,0x7,0xc0,0x1c,
0x80,0xe0,0x38,0x60,0x70,0xc0,0xe1,0x81,
0xc3,0x1,0xc0,0x38,0x7,0x0,0xe0,0xf0,
0xf3,0xe0,0xf,0xdf,0x0,0x7e,0x1c,0x7,
0xf,0x81,0xff,0xf8,0xc,0x0,0x60,0x7,
0x20,0x39,0x0,0x7c,0x0,0x1f,0x80,0x86,
0x3,0x3,0xc1,0x9f,0x0,0x38,0x63,0xc0,
0x60,0x7,0x23,0xc3,0xdf,0x0,0x20,0x3f,
0x3,0x0,0x0,0x0,0x0,0x40,0x18,0x0,
0xfc,0x0,0x1f,0x80,0x0,0x0,0x7,0x1c,
0x7f,0xf,0xff,0xff,0x0,0x0,0x0,0x0,
0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x0,
0x1,0xb0,0x0,0x0,0x0,0x30,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x3,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0xf,0xc0,0x1,0xf8,0x3,0xff,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x3,0x0,0xc0,0x0,0x0,0xc,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x80,0x0,0x3,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x6,0x0,0x0,0x0,0x38,0x70,0x0,
0x0,0x1,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x18,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x40,0x0,
0x0,0x0,0x0,0x0,0x0,0xa0,0x0,0x0,
0x0,0x60,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x1,0x80,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0xf,0xc0,
0x1,0xf8,0x3,0xff,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x7,0x0,
0x40,0x0,0x0,0xc,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x1,0x80,0x0,0x3,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0xc,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x18,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0xc0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0xc0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x7,0x80,0x80,0x0,0x3,0xc,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x80,0x0,0x3,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0xc,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x20,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x18,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x7f,0xff,0xf0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x30,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x3,0xff,
0x0,0x0,0x3,0x8c,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x1,0x80,0x0,0x3,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0xd8,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x40,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x20,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x18,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x7f,0x0,0x0,0x1f,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0xfc,0x0,0x0,0x1,0xd8,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x80,0x0,0x3,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0xf0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x3,0x80,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x1,0xc0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x18,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x60,0x0,0x7,0xf0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x70,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x3,0xc0,0x0,0x7,
0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0xe0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x18,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x3f,0xe0,0x3f,};

static WORD largefnt_ch_ofst[195] = {
0,9,14,24,43,58,74,94,
99,106,113,123,136,141,152,157,
169,183,197,211,225,239,253,267,
281,295,309,314,319,328,339,348,
363,382,403,421,438,457,475,492,
513,534,544,557,578,596,621,642,
662,678,698,720,733,752,774,796,
825,845,866,884,891,903,910,918,
929,934,950,964,976,990,1002,1013,
1027,1041,1046,1055,1070,1075,1098,1112,
1126,1140,1154,1164,1174,1185,1198,1214,
1238,1254,1271,1284,1294,1297,1307,1319,
1319,1335,1348,1359,1374,1389,1404,1419,
1430,1441,1452,1463,1470,1478,1478,1499,
1520,1520,1540,1567,1580,1593,1593,1606,
1619,1619,1637,1656,1669,1685,1703,1703,
1703,1718,1724,1737,1750,1763,1782,1782,
1782,1797,1797,1797,1797,1797,1802,1815,
1828,1828,1828,1828,1839,1839,1839,1839,
1839,1839,1839,1839,1850,1850,1869,1888,
1909,1909,1909,1909,1909,1909,1909,1909,
1909,1909,1909,1909,1909,1909,1909,1909,
1909,1909,1909,1909,1909,1909,1909,1909,
1909,1909,1909,1909,1909,1909,1909,1909,
1909,1909,1921,};

static struct font_hdr largefnt_font = {
0x0, 3584, "largefnt", 32, 225,
22, 20, 14, 6, 7,
28, 29, 1, 1,
2, 2, 0x5555, 0x5555,
0x0, NULL, largefnt_ch_ofst, largefnt_data,
244, 29,
NULL,};

MgFont *mgLargeFont()
{
return &largefnt_font;
}
