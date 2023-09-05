#include "ui/button.h"

#include <nds/dma.h>

UIButton::UIButton(OamState* chosenOam, u8* data, u8* palData, int oamStartInd, int gfxCount, SpriteSize sprSize, int xPos, int yPos, int width, int height, int palSlot)
{
	oam = chosenOam;
	oamStart = oamStartInd;

	spriteGfxCount = gfxCount;
	spriteGfx = new u16*[gfxCount];
	spriteSize = sprSize;

	x = xPos;
	y = yPos;
	w = width;
	h = height;

	callback = 0;
	pUserData = 0;

	for (int i=0; i<gfxCount; i++)
	{
		spriteGfx[i] = oamAllocateGfx(oam, spriteSize, SpriteColorFormat_256Color);

		u8* offset = data + (i*w*h);
		dmaCopy(offset, spriteGfx[i], w*h);
		oamSet(oam, oamStart+i, x+(i*w), y, 0, palSlot, spriteSize, SpriteColorFormat_256Color, spriteGfx[i], -1, false, false, false, false, false);
	}

	// copy palette to ext palette vram slot
	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		dmaCopy(palData, &VRAM_F_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		dmaCopy(palData, &VRAM_I_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
}

UIButton::~UIButton()
{
	for (int i=0; i<spriteGfxCount; i++)
	{
		oamSet(oam, oamStart+i, 0, 0, 0, 0, spriteSize, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		oamFreeGfx(oam, spriteGfx[i]);
	}
	delete[] spriteGfx;
}

void UIButton::setPos(int xPos, int yPos)
{
	x = xPos;
	y = yPos;
	oamSetXY(oam, oamStart, x, y);
}

void UIButton::updateInput()
{
	if (keysDown() & KEY_TOUCH)
	{
		touchRead(&touchPos);
		if (callback && touchPos.px >= (u16)x && touchPos.py >= (u16)y && touchPos.px < (u16)(x+(spriteGfxCount*w)) && touchPos.py < (u16)(y+h))
			callback(pUserData);
	}
}
