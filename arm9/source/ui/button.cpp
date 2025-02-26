#include "ui/button.h"

#include <nds/dma.h>

#include "libadx.h"
#include "mem.h"

UIButton::UIButton(OamState* chosenOam, std::string file, int oamStartInd, int horTiles, int vertTiles, SpriteSize sprSize, int xPos, int yPos, int width, int height, int sprWidth, int sprHeight, int palSlot)
{
	oam = chosenOam;
	oamStart = oamStartInd;
	paletteSlot = palSlot;

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
	currFrame = 0;
	visible = true;
	pressing = false;
	hflip = false;
	vflip = false;
	assignedKey = 0;

	if (!file.empty())
	{
		currData = readFile(file+".img.bin");
		currPal = readFile(file+".pal.bin");
	}
	else
	{
		currData = 0;
		currPal = 0;
	}

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

			if (currData)
			{
				u8* offset = currData + (i*sprWidth*sprHeight);
				dmaCopy(offset, spriteGfx[i], sprWidth*sprHeight);
			}
			else
				dmaFillHalfWords(0, spriteGfx[i], sprWidth*sprHeight);
			oamSet(oam, oamStart+i, x+(hor*sprWidth), y+(vert*sprHeight), 0, palSlot, spriteSize, SpriteColorFormat_256Color, spriteGfx[i], -1, false, false, false, false, false);
		}
	}

	// copy palette to ext palette vram slot
	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		if (currPal) dmaCopy(currPal, &VRAM_F_EXT_SPR_PALETTE[palSlot], 512);
		else dmaFillHalfWords(0, &VRAM_F_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		if (currPal) dmaCopy(currPal, &VRAM_I_EXT_SPR_PALETTE[palSlot], 512);
		else dmaFillHalfWords(0, &VRAM_I_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
}

UIButton::~UIButton()
{
	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
	{
		adx_update();
		oamClearSprite(oam, oamStart+i);
		oamFreeGfx(oam, spriteGfx[i]);
	}
	delete[] spriteGfx;
	if (currData) mem_free(currData);
	if (currPal) mem_free(currPal);
}

void UIButton::setImage(std::string file, int sprWidth, int sprHeight, int palSlot)
{
	paletteSlot = palSlot;

	if (currData) mem_free(currData);
	if (currPal) mem_free(currPal);
	if (!file.empty())
	{
		currData = readFile(file+".img.bin");
		currPal = readFile(file+".pal.bin");
		adx_update();
	}
	else
	{
		currData = 0;
		currPal = 0;
	}

	for (int vert=0; vert<spriteVertTiles; vert++)
	{
		for (int hor=0; hor<spriteHorTiles; hor++)
		{
			int i = vert * spriteHorTiles + hor;
			if (currData)
			{
				u8* offset = currData + (i*sprWidth*sprHeight);
				dmaCopy(offset, spriteGfx[i], sprWidth*sprHeight);
			}
			else
				dmaFillHalfWords(0, spriteGfx[i], sprWidth*sprHeight);
		}
	}

	// copy palette to ext palette vram slot
	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		if (currPal) dmaCopy(currPal, &VRAM_F_EXT_SPR_PALETTE[palSlot], 512);
		else dmaFillHalfWords(0, &VRAM_F_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		if (currPal) dmaCopy(currPal, &VRAM_I_EXT_SPR_PALETTE[palSlot], 512);
		else dmaFillHalfWords(0, &VRAM_I_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}

	currFrame = 0;
}

void UIButton::setFrame(int frame)
{
	currFrame = frame;

	for (int vert=0; vert<spriteVertTiles; vert++)
	{
		for (int hor=0; hor<spriteHorTiles; hor++)
		{
			int frameOffset = frame*spriteHorTiles*spriteVertTiles;
			int i = vert * spriteHorTiles + hor;
			u8* offset = currData + (frameOffset*sprW*sprH) + (i*sprW*sprH);
			dmaCopy(offset, spriteGfx[i], sprW*sprH);
		}
	}
}

void UIButton::setVisible(bool on)
{
	visible = on;
	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
	{
		oamSetHidden(oam, oamStart+i, !on);
	}

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
			int xFlip = (hflip) ? (sprW - w) : 0;
			int yFlip = (vflip) ? (sprH - h) : 0;
			oamSetXY(oam, oamStart+i, x+(hor*sprW) - xFlip, y+(vert*sprH) - yFlip);
		}
	}
}

void UIButton::setPriority(int pr)
{
	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
	{
		oamSetPriority(oam, oamStart+i, pr);
	}
}

void UIButton::setFlip(bool h, bool v)
{
	hflip = h;
	vflip = v;

	for (int i=0; i<spriteHorTiles*spriteVertTiles; i++)
	{
		oamSetFlip(oam, oamStart+i, h, v);
	}

	setPos(x, y);
}

void UIButton::forceRelease()
{
	if (!pressing) return;

	pressing = false;
	if (callbacks[1].cb) callbacks[1].cb(callbacks[1].pUserData);
}

void UIButton::darken()
{
	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		for (u32 j=0; j<256; j++)
		{
			u8 r=0, g=0, b=0;
			fromRGB15(VRAM_F_EXT_SPR_PALETTE[paletteSlot][j], r, g, b);
			VRAM_F_EXT_SPR_PALETTE[paletteSlot][j] = RGB15(r>>1, g>>1, b>>1);
			adx_update();
		}
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		for (u32 j=0; j<256; j++)
		{
			u8 r=0, g=0, b=0;
			fromRGB15(VRAM_I_EXT_SPR_PALETTE[paletteSlot][j], r, g, b);
			VRAM_I_EXT_SPR_PALETTE[paletteSlot][j] = RGB15(r>>1, g>>1, b>>1);
			adx_update();
		}
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
}

void UIButton::restorePalette()
{
	if (!currPal) return;

	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		dmaCopy(currPal, &VRAM_F_EXT_SPR_PALETTE[paletteSlot], 512);
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		dmaCopy(currPal, &VRAM_I_EXT_SPR_PALETTE[paletteSlot], 512);
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
}

void UIButton::unloadRAM(bool deletePal)
{
	if (currData)
	{
		mem_free(currData);
		currData = 0;
	}
	if (deletePal && currPal)
	{
		mem_free(currPal);
		currPal = 0;
	}
}

void UIButton::updateInput()
{
	if (!visible) return;

	u32 keyDown = keysDown();
	u32 keyUp = keysUp();

	if (!pressing && keyDown & KEY_TOUCH)
	{
		touchRead(&touchPos);
		if (callbacks[0].cb && touchPos.px >= (u16)x && touchPos.py >= (u16)y && touchPos.px < (u16)(x+w) && touchPos.py < (u16)(y+h))
		{
			pressing = true;
			callbacks[0].cb(callbacks[0].pUserData);
		}
	}

	if (!pressing && assignedKey && (keyDown & assignedKey))
	{
		pressing = true;
		callbacks[0].cb(callbacks[0].pUserData);
	}

	if (pressing && (keyUp & KEY_TOUCH || (assignedKey && keyUp & assignedKey)))
	{
		forceRelease();
	}
}
