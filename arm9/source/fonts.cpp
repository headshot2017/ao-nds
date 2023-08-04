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

// renders font on 256-color sprite gfx
void renderFont(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int spriteGfxCount)
{
	int currGfx = 0;
	int x = 0;

	if (fontID < 0 || fontID >= loadedCount)
		return;

	LoadedFont& font = fonts[fontID];

	for (u32 i=0; i<strlen(text); i++)
	{
		// how wide is this character
		int ax;
		int lsb;
		stbtt_GetCodepointHMetrics(&font.info, text[i], &ax, &lsb);
		// (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].)

		// get bounding box for character (may be offset to account for chars that dip above or below the line)
		int c_x1, c_y1, c_x2, c_y2;
		stbtt_GetCodepointBitmapBox(&font.info, text[i], font.scale, font.scale, &c_x1, &c_y1, &c_x2, &c_y2);

		int out_x = c_x2 - c_x1;
		bool oob = (x + out_x >= w);
		if (oob)
			out_x = w-x;

		// compute y (different characters have different heights)
		int y = font.ascent + c_y1;

		// render character (stride and offset is important here)
		int byteOffset = x + roundf(lsb * font.scale) + (y * w);
		stbtt_MakeCodepointBitmap(&font.info, bmpTarget + byteOffset, out_x, c_y2 - c_y1, w, font.scale, font.scale, text[i]);

		// focus around the bounding box of the rendered character...
		for (int yy = y; yy<y+font.line_height; yy++)
		{
			for (int xx = x; xx<x + out_x; xx++)
			{
				int ind = yy*w+xx;
				if (bmpTarget[ind] == 0xff)
				{
					// render this to the sprite gfx!
					int leftOrRight = ind&1;
					bool oobFlag;
					u32 targetInd = bmpIndexTo256SpriteIndex(xx, yy, w, h, spritesize, &oobFlag);
					if (oobFlag) continue;

					spriteGfxTargets[currGfx][targetInd] = (leftOrRight) ?
						(spriteGfxTargets[currGfx][targetInd] & 0xf) | (palIndex<<8) :
						palIndex | ((spriteGfxTargets[currGfx][targetInd] >> 8) << 8);
				}
			}
		}

		if (oob)
		{
			currGfx++;
			if (currGfx >= spriteGfxCount) // reached the limit, stop here
				return;

			// blank the bitmap and render the character again
			memset(bmpTarget, 0, w*h);
			x -= w;
			i--;
			continue;
		}

		// advance x
		x += roundf(ax * font.scale);

		// add kerning
		int kern;
		kern = stbtt_GetCodepointKernAdvance(&font.info, text[i], text[i + 1]);
		x += roundf(kern * font.scale);
	}
}
