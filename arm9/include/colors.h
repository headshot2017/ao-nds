#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/video.h>

enum
{
	PAL_WHITE =		RGB15(31,31,31),
	PAL_GREEN = 	RGB15(0,31,0),
	PAL_RED = 		RGB15(31,0,0),
	PAL_ORANGE = 	RGB15(31,20,0),
	PAL_BLUE =  	RGB15(5,18,31),
	PAL_YELLOW = 	RGB15(31,31,0),
	PAL_BLACK = 	RGB15(0,0,0),
	PAL_GRAY = 		RGB15(22,22,22),
};

enum
{
	COLOR_WHITE=1,
	COLOR_GREEN=2,
	COLOR_RED=3,
	COLOR_ORANGE=4,
	COLOR_BLUE=5,
	COLOR_YELLOW=6,
	COLOR_BLACK=7,
	COLOR_GRAY=8,
	COLOR_NAME=9,
	COLOR_MAX
};

extern int AOcolorToPalette[];
extern u32 paletteColorInd[];

#endif // COLORS_H_INCLUDED
