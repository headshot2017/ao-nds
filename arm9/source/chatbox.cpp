#include "chatbox.h"

#include <stdio.h>

#include <nds/arm9/background.h>
#include <nds/arm9/decompress.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>

#include "global.h"

Chatbox::Chatbox()
{
	bgIndex = bgInit(1, BgType_Text8bpp, BgSize_T_256x256, 10, 0);
	bgSetPriority(bgIndex, 1);
	bgSetScroll(bgIndex, 0, -192+80);
	bgHide(bgIndex);

	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);

	u32 dataLen, mapLen, palLen;
	bgData = readFile("/data/ao-nds/misc/chatbox.img.bin", &dataLen);
	bgMap = readFile("/data/ao-nds/misc/chatbox.map.bin", &mapLen);
	bgPal = readFile("/data/ao-nds/misc/chatbox.pal.bin", &palLen);

	dmaCopy(bgData, bgGetGfxPtr(bgIndex), dataLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
	dmaCopy(bgPal, (void *)&VRAM_F_EXT_PALETTE[bgIndex][1], palLen);

	// set text colors
	VRAM_G_EXT_SPR_PALETTE[0][0] = RGB15(0,0,0); // transparency
	VRAM_G_EXT_SPR_PALETTE[0][1] = RGB15(31,31,31); // white
	VRAM_G_EXT_SPR_PALETTE[0][2] = RGB15(0,31,0); // green
	VRAM_G_EXT_SPR_PALETTE[0][3] = RGB15(31,0,0); // red
	VRAM_G_EXT_SPR_PALETTE[0][4] = RGB15(31,20,0); // orange
	VRAM_G_EXT_SPR_PALETTE[0][5] = RGB15(5,18,31); // blue
	VRAM_G_EXT_SPR_PALETTE[0][6] = RGB15(31,31,0); // yellow

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
	vramSetBankF(VRAM_F_LCD);

	REG_BLDCNT = BLEND_NONE;

	dmaFillHalfWords(0, (void *)&VRAM_F_EXT_PALETTE[bgIndex][0], 512);

	delete[] bgData;
	delete[] bgMap;
	delete[] bgPal;
}

void Chatbox::setVisible(bool on)
{
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}
