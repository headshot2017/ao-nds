#include "ui/button.h"

#include <nds/dma.h>

#include "mp3_shared.h"

UIButton::UIButton(OamState* chosenOam, u8* data, u8* palData, int oamStartInd, int horTiles, int vertTiles, SpriteSize sprSize, int xPos, int yPos, int width, int height, int sprWidth, int sprHeight, int palSlot)
{
	oam = chosenOam;
	oamStart = oamStartInd;

	spriteHorTiles = horTiles;
	spriteVertTiles = vertTiles;
	spriteGfx = new u16*[horTiles*vertTiles];
	spriteSize = sprSize;

	x = xPos;
	y = yPos;
	w = width;
	h = height;
	sprW = sprWidth;
	sprH = sprHeight;
	visible = true;
	pressing = false;

	for (int i=0; i<2; i++)
	{
		callbacks[i].cb = 0;
		callbacks[i].pUserData = 0;
	}

	for (int vert=0; vert<spriteVertTiles; vert++)
	{
		for (int hor=0; hor<spriteHorTiles; hor++)
		{
			int i = vert * spriteHorTiles + hor;
			spriteGfx[i] = oamAllocateGfx(oam, spriteSize, SpriteColorFormat_256Color);

			u8* offset = data + (i*sprWidth*sprHeight);
			dmaCopy(offset, spriteGfx[i], sprWidth*sprHeight);
			oamSet(oam, oamStart+i, x+(hor*sprWidth), y+(vert*sprHeight), 0, palSlot, spriteSize, SpriteColorFormat_256Color, spriteGfx[i], -1, false, false, false, false, false);
		}
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
	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
	{
		oamClearSprite(oam, oamStart+i);
		oamFreeGfx(oam, spriteGfx[i]);
	}
	delete[] spriteGfx;
}

void UIButton::setImage(u8* data, u8* palData, int sprWidth, int sprHeight, int palSlot)
{
	for (int vert=0; vert<spriteVertTiles; vert++)
	{
		for (int hor=0; hor<spriteHorTiles; hor++)
		{
			int i = vert * spriteHorTiles + hor;
			u8* offset = data + (i*sprWidth*sprHeight);
			dmaCopy(offset, spriteGfx[i], sprWidth*sprHeight);
		}
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

	mp3_fill_buffer();
}

void UIButton::setVisible(bool on)
{
	visible = on;
	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
		oamSetHidden(oam, oamStart+i, !on);

	if (!visible) forceRelease();
}

void UIButton::setPos(int xPos, int yPos)
{
	x = xPos;
	y = yPos;
	for (int vert=0; vert<spriteVertTiles; vert++)
	{
		for (int hor=0; hor<spriteHorTiles; hor++)
		{
			int i = vert * spriteHorTiles + hor;
			oamSetXY(oam, oamStart+i, x+(hor*sprW), y+(vert*sprH));
		}
	}
}

void UIButton::setPriority(int pr)
{
	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
		oamSetPriority(oam, oamStart+i, pr);
}

void UIButton::forceRelease()
{
	if (!pressing) return;

	pressing = false;
	if (callbacks[1].cb) callbacks[1].cb(callbacks[1].pUserData);
}

void UIButton::updateInput()
{
	if (!visible) return;

	if (keysDown() & KEY_TOUCH)
	{
		touchRead(&touchPos);
		if (callbacks[0].cb && touchPos.px >= (u16)x && touchPos.py >= (u16)y && touchPos.px < (u16)(x+w) && touchPos.py < (u16)(y+h))
		{
			pressing = true;
			callbacks[0].cb(callbacks[0].pUserData);
		}
	}

	if (pressing && keysUp() & KEY_TOUCH)
	{
		forceRelease();
	}
}
