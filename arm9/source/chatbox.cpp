#include "chatbox.h"

#include <stdio.h>
#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/decompress.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>

#include "global.h"
#include "fonts.h"

Chatbox::Chatbox()
{
	textCanvas = new u8[32*16];
	for (int i=0; i<2; i++)
		nameGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);

	memset(textCanvas, 0, 32*16); // black bg

	// bug: if the sprite gfx is filled once with palette index 0, it'll never work afterwards.
	// as a workaround i fill it with palette 1, then again with palette 0
	for (int i=0; i<2; i++)
	{
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
		oamSet(&oamMain, 24+i, 4+(i*32), 115, 0, 1, SpriteSize_32x16, SpriteColorFormat_256Color, nameGfx[i], -1, false, false, false, false, false);
	}


	bgIndex = bgInit(1, BgType_Text8bpp, BgSize_T_256x256, 10, 0);
	bgSetPriority(bgIndex, 1);
	bgSetScroll(bgIndex, 0, -192+80);
	bgHide(bgIndex);
	bgUpdate();

	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);

	u32 dataLen, mapLen, palLen;
	bgData = readFile("/data/ao-nds/misc/chatbox.img.bin", &dataLen);
	bgMap = readFile("/data/ao-nds/misc/chatbox.map.bin", &mapLen);
	bgPal = readFile("/data/ao-nds/misc/chatbox.pal.bin", &palLen);

	dmaCopy(bgData, bgGetGfxPtr(bgIndex), dataLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
	dmaCopy(bgPal, (void *)&VRAM_F_EXT_PALETTE[bgIndex][1], palLen);

	VRAM_G_EXT_SPR_PALETTE[1][COLOR_WHITE] = 	RGB15(31,31,31);
	VRAM_G_EXT_SPR_PALETTE[1][COLOR_GREEN] = 	RGB15(0,31,0);
	VRAM_G_EXT_SPR_PALETTE[1][COLOR_RED] = 		RGB15(31,0,0);
	VRAM_G_EXT_SPR_PALETTE[1][COLOR_ORANGE] = 	RGB15(31,20,0);
	VRAM_G_EXT_SPR_PALETTE[1][COLOR_BLUE] = 	RGB15(5,18,31);
	VRAM_G_EXT_SPR_PALETTE[1][COLOR_YELLOW] = 	RGB15(31,31,0);
	VRAM_G_EXT_SPR_PALETTE[1][COLOR_BLACK] = 	RGB15(0,0,0);

	vramSetBankF(VRAM_F_BG_EXT_PALETTE);
	vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);

	// makes the chatbox transparent.
	// chatbox is in bg1 and we want to see bg0 (court) and sprites behind it
	REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG1 | BLEND_DST_BG0 | BLEND_DST_SPRITE;
	REG_BLDALPHA = 0x70f;
}

Chatbox::~Chatbox()
{
	bgHide(bgIndex);

	REG_BLDCNT = BLEND_NONE;

	dmaFillHalfWords(0, (void *)&VRAM_F_EXT_PALETTE[bgIndex][0], 512);

	delete[] bgData;
	delete[] bgMap;
	delete[] bgPal;

	delete[] textCanvas;
	for (int i=0; i<2; i++)
	{
		oamSet(&oamMain, 24+i, 4+(i*32), 115, 0, 1, SpriteSize_32x16, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		oamFreeGfx(&oamMain, nameGfx[i]);
	}
}

void Chatbox::setVisible(bool on)
{
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Chatbox::setName(const char* name)
{
	// clear old text
	memset(textCanvas, 0, 32*16);
	for (int i=0; i<2; i++)
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
	renderFont(1, name, COLOR_WHITE, 32, 16, textCanvas, SpriteSize_32x16, nameGfx, 2);
}
