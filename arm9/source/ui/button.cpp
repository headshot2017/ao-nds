#include "ui/button.h"

#include <nds/dma.h>

UIButton::UIButton(OamState* chosenOam, u8* data, u8* palData, int oamStartInd, int gfxCount, SpriteSize sprSize, int xPos, int yPos, int width, int height, int sprWidth, int sprHeight, int palSlot)
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
	sprW = sprWidth;
	sprH = sprHeight;
	visible = true;

	callback = 0;
	pUserData = 0;

	for (int i=0; i<gfxCount; i++)
	{
		spriteGfx[i] = oamAllocateGfx(oam, spriteSize, SpriteColorFormat_256Color);

		u8* offset = data + (i*sprWidth*sprHeight);
		dmaCopy(offset, spriteGfx[i], sprWidth*sprHeight);
		oamSet(oam, oamStart+i, x+(i*sprWidth), y, 0, palSlot, spriteSize, SpriteColorFormat_256Color, spriteGfx[i], -1, false, false, false, false, false);
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
		oamClearSprite(oam, oamStart+i);
		oamFreeGfx(oam, spriteGfx[i]);
	}
	delete[] spriteGfx;
}

void UIButton::setImage(u8* data, u8* palData, int sprWidth, int sprHeight, int palSlot)
{
	for (int i=0; i<spriteGfxCount; i++)
	{
		u8* offset = data + (i*sprWidth*sprHeight);
		dmaCopy(offset, spriteGfx[i], sprWidth*sprHeight);
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

void UIButton::setVisible(bool on)
{
	visible = on;
	for (int i=0; i<spriteGfxCount; i++)
		oamSetHidden(oam, oamStart+i, !on);
}

void UIButton::setPos(int xPos, int yPos)
{
	x = xPos;
	y = yPos;
	for (int i=0; i<spriteGfxCount; i++)
		oamSetXY(oam, oamStart+i, x+(i*sprW), y);
}

void UIButton::setPriority(int pr)
{
	for (int i=0; i<spriteGfxCount; i++)
		oamSetPriority(oam, oamStart+i, pr);
}

void UIButton::updateInput()
{
	if (visible && keysDown() & KEY_TOUCH)
	{
		touchRead(&touchPos);
		if (callback && touchPos.px >= (u16)x && touchPos.py >= (u16)y && touchPos.px < (u16)(x+w) && touchPos.py < (u16)(y+h))
			callback(pUserData);
	}
}
