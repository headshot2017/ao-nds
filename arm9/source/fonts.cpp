#include "fonts.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdio.h>

#include "global.h"

struct LoadedFont
{
	stbtt_fontinfo info;
	int line_height;
	float scale;
	int ascent;
};

LoadedFont* fonts = nullptr;
int loadedCount = 0;

int initFont(const u8* data, int line_height)
{
	fonts = (LoadedFont*)realloc(fonts, sizeof(LoadedFont) * (++loadedCount));

	LoadedFont& font = fonts[loadedCount-1];
	if (!stbtt_InitFont(&font.info, data, 0))
		return -1;

	font.line_height = line_height;
	font.scale = stbtt_ScaleForPixelHeight(&font.info, line_height);

	int descent, lineGap;
	stbtt_GetFontVMetrics(&font.info, &font.ascent, &descent, &lineGap);

	font.ascent = roundf(font.ascent * font.scale);

	return loadedCount-1;
}

// renders font on 256-color sprite gfx. returns text width with chosen font
int renderText(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int spriteGfxCount)
{
	int currGfx = 0;
	int x = 0;
	int textWidth = 0;

	if (fontID < 0 || fontID >= loadedCount)
		return 0;

	for (u32 i=0; i<strlen(text); i++)
	{
		int oobFlag;
		int outWidth;
		int new_x = renderChar(fontID, text+i, palIndex, x, w, w, h, bmpTarget, spritesize, spriteGfxTargets[currGfx], false, &oobFlag, &outWidth);

		if (!oobFlag)
		{
			textWidth += outWidth;
			x = new_x;
		}
		else
		{
			currGfx++;
			if (currGfx >= spriteGfxCount) // reached the limit, stop here
				return textWidth;

			// blank the bitmap and render the character again
			memset(bmpTarget, 0, w*h);
			x -= w;
			i--;
			continue;
		}
	}

	return textWidth;
}

int renderChar(int fontID, const char* text, int palIndex, int x, int spriteW, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16* spriteGfxTarget, bool skipOnOob, int* oobFlag, int* outWidth)
{
	if (fontID < 0 || fontID >= loadedCount)
		return 0;

	LoadedFont& font = fonts[fontID];

	// how wide is this character
	int ax;
	int lsb;
	stbtt_GetCodepointHMetrics(&font.info, text[0], &ax, &lsb);
	// (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].)

	// get bounding box for character (may be offset to account for chars that dip above or below the line)
	int c_x1, c_y1, c_x2, c_y2;
	stbtt_GetCodepointBitmapBox(&font.info, text[0], font.scale, font.scale, &c_x1, &c_y1, &c_x2, &c_y2);

	if (outWidth) *outWidth = 0;
	int out_x = c_x2 - c_x1;
	bool oob = (x + out_x >= w);
	if (oob)
	{
		if (skipOnOob)
		{
			if (oobFlag) *oobFlag = 2;
			return x;
		}
		out_x = w-x;
	}
	else if (outWidth)
		*outWidth = out_x;

	// compute y (different characters have different heights)
	int y = font.ascent + c_y1;

	// render character (stride and offset is important here)
	int byteOffset = x + roundf(lsb * font.scale) + (y * spriteW);
	stbtt_MakeCodepointBitmap(&font.info, bmpTarget + byteOffset, out_x, c_y2 - c_y1, spriteW, font.scale, font.scale, text[0]);

	// focus around the bounding box of the rendered character...
	for (int yy = y; yy<y+font.line_height; yy++)
	{
		for (int xx = x; xx<x + out_x; xx++)
		{
			int ind = yy*spriteW+xx;
			if (bmpTarget[ind] == 0xff)
			{
				// render this to the sprite gfx!
				int leftOrRight = ind&1;
				bool oobFlag2;
				u32 targetInd = bmpIndexTo256SpriteIndex(xx, yy, spriteW, h, spritesize, &oobFlag2);
				if (!oobFlag2)
				{
					spriteGfxTarget[targetInd] = (leftOrRight) ?
						(spriteGfxTarget[targetInd] & 0xf) | (palIndex<<8) :
						palIndex | ((spriteGfxTarget[targetInd] >> 8) << 8);
				}
			}
		}
	}

	if (oob)
	{
		if (oobFlag) *oobFlag = 1;
		return x;
	}

	// advance x
	x += roundf(ax * font.scale);

	// add kerning
	int kern;
	kern = stbtt_GetCodepointKernAdvance(&font.info, text[0], text[1]);
	x += roundf(kern * font.scale);

	if (oobFlag) *oobFlag = 0;
	return x;
}
