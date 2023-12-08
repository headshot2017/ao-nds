#include "fonts.h"

#include <stdio.h>

#include <nds/arm9/math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype_fixed.h"

#include "mp3_shared.h"
#include "global.h"

fix32 roundf32(fix32 num)
{
	fix32 ceiled = ceilf32(num);
	return (num >= ceiled-2048) ? ceiled : inttof32(f32toint(num));
}

struct LoadedFont
{
	stbtt_fontinfo info;
	int line_height;
	fix32 scale;
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
	//font.scale = floattof32(stbtt_ScaleForPixelHeight(&font.info, line_height));
	font.scale = stbtt_ScaleForPixelHeight(&font.info, inttof32(line_height));


	int descent, lineGap;
	stbtt_GetFontVMetrics(&font.info, &font.ascent, &descent, &lineGap);

	//font.ascent = roundf(font.ascent * f32tofloat(font.scale));
	font.ascent = f32toint(roundf32(mulf32(inttof32(font.ascent), font.scale)));
	printf("%d - %d (%f) %d\n", line_height, font.scale, f32tofloat(font.scale), font.ascent);

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
		mp3_fill_buffer();

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
	iprintf("%d: %d %d %d %d\n", fontID, c_x1, c_y1, c_x2, c_y2);
	//stbtt_GetCodepointBitmapBox(&font.info, text[0], f32tofloat(font.scale), f32tofloat(font.scale), &c_x1, &c_y1, &c_x2, &c_y2);

	if (outWidth) *outWidth = 0;
	int out_x = c_x2 - c_x1+1;
	bool oob = (x + out_x >= w);
	if (oob)
	{
		if (skipOnOob)
		{
			if (oobFlag) *oobFlag = 2;
			return x;
		}
		//out_x = w-x;
	}
	else if (outWidth)
		*outWidth = out_x;

	// compute y (different characters have different heights)
	int y = font.ascent + c_y1;

	// render character (stride and offset is important here)
	//int byteOffset = 0 + roundf(lsb * f32tofloat(font.scale)) + (y * spriteW);
	int byteOffset = 0 + f32toint(roundf32(mulf32(inttof32(lsb), font.scale))) + (y * spriteW);
	//stbtt_MakeCodepointBitmap(&font.info, bmpTarget + byteOffset, out_x, c_y2 - c_y1, spriteW, f32tofloat(font.scale), f32tofloat(font.scale), text[0]);
	stbtt_MakeCodepointBitmap(&font.info, bmpTarget + byteOffset, out_x, c_y2 - c_y1, spriteW, font.scale, font.scale, text[0]);
	mp3_fill_buffer();

	// focus around the bounding box of the rendered character...
	for (int yy = y; yy<y+font.line_height; yy++)
	{
		for (int xx = 0; xx<out_x; xx++)
		{
			int ind = yy*spriteW+xx;
			int realInd = yy*spriteW+x+xx;
			if (bmpTarget[ind] == 0xff)
			{
				// render this to the sprite gfx!
				int leftOrRight = realInd&1;
				bool oobFlag2;
				u32 targetInd = bmpIndexTo256SpriteIndex(x+xx, yy, spriteW, h, spritesize, &oobFlag2);
				if (!oobFlag2)
				{
					spriteGfxTarget[targetInd] = (leftOrRight) ?
						(spriteGfxTarget[targetInd] & 0xf) | (palIndex<<8) :
						palIndex | ((spriteGfxTarget[targetInd] >> 8) << 8);
				}
				bmpTarget[ind] = 0;
			}
		}
	}

	if (oob)
	{
		if (oobFlag) *oobFlag = 1;
		return x;
	}

	// advance x
	x += f32toint(roundf32(mulf32(inttof32(ax), font.scale)));
	//x += roundf(ax * f32tofloat(font.scale));

	// add kerning
	int kern;
	kern = stbtt_GetCodepointKernAdvance(&font.info, text[0], text[1]);
	x += f32toint(roundf32(mulf32(inttof32(kern), font.scale)));
	//x += roundf(kern * f32tofloat(font.scale));

	mp3_fill_buffer();

	if (oobFlag) *oobFlag = 0;
	return x;
}

void renderMultiLine(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int gfxPerLine, int maxLines)
{
	int textX = 0;
	int currTextGfxInd = 0;

	for (u32 i=0; i<strlen(text); i++)
	{
		while (text[i] == '\n')
		{
			int line = currTextGfxInd/gfxPerLine;
			currTextGfxInd = (line+1) * gfxPerLine;
			mp3_fill_buffer();

			i++;
			if (i >= strlen(text))
				return;

			textX = 0;
		}

		if (currTextGfxInd >= gfxPerLine*maxLines)
			return;

		bool lastBox = (currTextGfxInd % gfxPerLine == gfxPerLine-1);
		int oobFlag = 0;
		int outWidth;
		int new_x = renderChar(fontID, text+i, palIndex, textX, 32, 32, 16, bmpTarget, SpriteSize_32x16, spriteGfxTargets[currTextGfxInd], lastBox, &oobFlag, &outWidth);
		mp3_fill_buffer();

		if (oobFlag)
		{
			currTextGfxInd++;

			if (currTextGfxInd % gfxPerLine == 0)
			{
				// entered a new line
				textX = 0;
				if (oobFlag == 2)
					i--;
			}
			else
			{
				textX -= 32;
				textX = renderChar(fontID, text+i, palIndex, textX, 32, 32, 16, bmpTarget, SpriteSize_32x16, spriteGfxTargets[currTextGfxInd], lastBox, &oobFlag, &outWidth);
				mp3_fill_buffer();
			}
		}
		else
		{
			textX = new_x;
			if (textX > 32)
			{
				currTextGfxInd++;
				if (currTextGfxInd % gfxPerLine == 0)
					textX = 0;
				else
					textX -= 32;
			}
		}
	}
}

int advanceXPos(int fontID, const char* text, int x, int w, bool skipOnOob, int* oobFlag, int* outWidth)
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
	//stbtt_GetCodepointBitmapBox(&font.info, text[0], f32tofloat(font.scale), f32tofloat(font.scale), &c_x1, &c_y1, &c_x2, &c_y2);

	if (outWidth) *outWidth = 0;
	int out_x = c_x2 - c_x1+1;
	bool oob = (x + out_x >= w);
	if (oob)
	{
		if (skipOnOob)
		{
			if (oobFlag) *oobFlag = 2;
			return x;
		}
	}
	else if (outWidth)
		*outWidth = out_x;

	if (oob)
	{
		if (oobFlag) *oobFlag = 1;
		return x;
	}

	// advance x
	x += f32toint(roundf32(mulf32(inttof32(ax), font.scale)));
	//x += roundf(ax * f32tofloat(font.scale));

	// add kerning
	int kern;
	kern = stbtt_GetCodepointKernAdvance(&font.info, text[0], text[1]);
	x += f32toint(roundf32(mulf32(inttof32(kern), font.scale)));
	//x += roundf(kern * f32tofloat(font.scale));

	mp3_fill_buffer();

	if (oobFlag) *oobFlag = 0;
	return x;
}

void separateLines(int fontID, const char* text, int gfxPerLine, bool chatbox, std::vector<std::string>& out)
{
	std::string thisLine;
	int textX = 0;
	int currTextGfxInd = 0;

	for (u32 i=0; i<strlen(text); i++)
	{
		while (text[i] == '\n')
		{
			int line = currTextGfxInd/gfxPerLine;
			currTextGfxInd = (line+1) * gfxPerLine;
			out.push_back(thisLine);
			thisLine.clear();
			mp3_fill_buffer();

			i++;
			if (i >= strlen(text))
				return;

			textX = 0;
		}

		bool lastBox = (currTextGfxInd % gfxPerLine == gfxPerLine-1);
		int oobFlag = 0;
		int outWidth;

		int new_x = advanceXPos(fontID, text+i, textX, (chatbox && lastBox) ? 20 : 32, lastBox, &oobFlag, &outWidth);
		mp3_fill_buffer();

		if (oobFlag)
		{
			currTextGfxInd++;

			if (currTextGfxInd % gfxPerLine == 0)
			{
				// entered a new line
				textX = 0;
				out.push_back(thisLine);
				thisLine.clear();
				if (oobFlag == 2)
					i--;
			}
			else
			{
				thisLine += text[i];
				textX -= 32;
				textX = advanceXPos(fontID, text+i, textX, (chatbox && lastBox) ? 20 : 32, lastBox, &oobFlag, &outWidth);
				mp3_fill_buffer();
			}
		}
		else
		{
			textX = new_x;
			if (textX > 32)
			{
				currTextGfxInd++;
				if (currTextGfxInd % gfxPerLine == 0)
				{
					textX = 0;
					out.push_back(thisLine);
					thisLine.clear();
				}
				else
				{
					thisLine += text[i];
					textX -= 32;
				}
			}
			else
				thisLine += text[i];
		}
	}

	out.push_back(thisLine);
}

int getTextWidth(int fontID, const char* text, int maxWidth)
{
	if (fontID < 0 || fontID >= loadedCount)
		return 0;

	LoadedFont& font = fonts[fontID];

	int lines = 1;
	for (u32 i=0; i<strlen(text); i++)
	{
		if (text[i] == '\n') lines++;
	}

	int* textWidth = new int[lines];
	for (int i=0; i<lines; i++) textWidth[i] = 0;

	int line = 0;
	for (u32 i=0; i<strlen(text); i++)
	{
		if (text[i] == '\n')
		{
			line++;
			continue;
		}

		// how wide is this character
		int ax;
		int lsb;
		stbtt_GetCodepointHMetrics(&font.info, text[i], &ax, &lsb);
		// (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].)

		// get bounding box for character (may be offset to account for chars that dip above or below the line)
		//int c_x1, c_y1, c_x2, c_y2;
		//stbtt_GetCodepointBitmapBox(&font.info, text[i], font.scale, font.scale, &c_x1, &c_y1, &c_x2, &c_y2);

		// advance x
		textWidth[line] += f32toint(roundf32(mulf32(inttof32(ax), font.scale)));
		//textWidth[line] += roundf(ax * f32tofloat(font.scale));

		if (maxWidth && textWidth[line] >= maxWidth)
			return maxWidth;
	}

	int maxLine = 0;
	for (int i=0; i<lines; i++)
		maxLine = (textWidth[i] > maxLine) ? textWidth[i] : maxLine;
	delete[] textWidth;
	return maxLine;
}
