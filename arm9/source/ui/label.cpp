#include "ui/label.h"

#include <string.h>

#include <nds/dma.h>

#include "fonts.h"

UILabel::UILabel(OamState* chosenOam, int oamStartInd, int perLine, int lines, u32 textColor, int palSlot, int font)
{
	oam = chosenOam;
	oamStart = oamStartInd;
	gfxPerLine = perLine;
	maxLines = lines;
	fontID = font;

	textCanvas = new u8[32*16];
	memset(textCanvas, 0, 32*16);

	textGfx = new u16*[gfxPerLine*maxLines];
	for (int i=0; i<gfxPerLine*maxLines; i++)
	{
		textGfx[i] = oamAllocateGfx(oam, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
		oamSet(oam, oamStart+i, 0, 0, 0, palSlot, SpriteSize_32x16, SpriteColorFormat_256Color, textGfx[i], -1, false, false, false, false, false);
	}

	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		VRAM_F_EXT_SPR_PALETTE[palSlot][2] = textColor;
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		VRAM_I_EXT_SPR_PALETTE[palSlot][2] = textColor;
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
}

UILabel::~UILabel()
{
	delete[] textCanvas;

	for (int i=0; i<gfxPerLine*maxLines; i++)
	{
		oamFreeGfx(oam, textGfx[i]);
		oamClearSprite(oam, oamStart+i);
	}
	delete[] textGfx;
}

void UILabel::setVisible(bool on)
{
	for (int i=0; i<gfxPerLine*maxLines; i++)
		oamSetHidden(oam, oamStart+i, !on);
}

void UILabel::setPos(int x, int y, bool center)
{
	int w = getTextWidth(fontID, currText.c_str());

	for (int i=0; i<gfxPerLine*maxLines; i++)
	{
		int xPos = x + ((i % gfxPerLine) * 32) - ((center) ? w/2 : 0);
		int yPos = y + ((i / gfxPerLine) * 12);

		oamSetXY(oam, oamStart+i, xPos, yPos);
	}
}

void UILabel::setText(const char* text)
{
	for (int i=0; i<gfxPerLine*maxLines; i++)
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);

	currText = text;
	renderMultiLine(fontID, text, 2, 32, 16, textCanvas, SpriteSize_32x16, textGfx, gfxPerLine, maxLines);
}
