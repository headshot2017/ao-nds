#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <vector>

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/exceptions.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/interrupts.h>
#include <nds/ndstypes.h>
#include <fat.h>

#include "mp3_shared.h"
#include "global.h"
#include "fonts.h"
#include "engine.h"
#include "ui/uiwificonnect.h"

u32 showDisclaimer()
{
	u32 bgTilesLen, bgMapLen, bgPalLen;
	u8* bgTiles = readFile("/data/ao-nds/ui/bg_disclaimer.img.bin", &bgTilesLen);
	u8* bgMap = readFile("/data/ao-nds/ui/bg_disclaimer.map.bin", &bgMapLen);
	u8* bgPal = readFile("/data/ao-nds/ui/bg_disclaimer.pal.bin", &bgPalLen);

	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bgTiles, bgGetGfxPtr(0), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(0), bgMapLen);
	memcpy(BG_PALETTE, bgPal, bgPalLen);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;

	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0;
	REG_BLDY = 16;

	return bgTilesLen;
}

void fadeDisclaimer(u32 tilesLen) {
	int ticks = 0;
	int alpha = 16;
	int alphaAdd = -1;
	while (1)
	{
		swiWaitForVBlank();

		alpha += alphaAdd;
		REG_BLDY = alpha;
		if (alpha == 0 && alphaAdd)
			alphaAdd = 0;
		else if (alpha == 16)
			break;
		else if (!alphaAdd && ticks++ >= 60*3)
			alphaAdd = 1;
	}

	dmaFillHalfWords(0, bgGetGfxPtr(0), tilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(0), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);
	REG_BLDCNT = BLEND_NONE;
}

void Vblank()
{
	mp3_fill_buffer();
}

int main()
{
	defaultExceptionHandler();
	mp3_init();
	srand(time(0));

	irqSet(IRQ_VBLANK, Vblank);

	if (!isDSiMode() && !fatInitDefault())
	{
		consoleDemoInit();
		iprintf("Failed to initialize libfat\nPlease check your SD card\n");
		while (1) swiWaitForVBlank();
	}

	DIR *dir1 = opendir("/data/ao-nds");
	DIR *dir2 = opendir("/data/ao-nds/ui");
	if (!dir1)
	{
		consoleDemoInit();
		iprintf("'/data/ao-nds' folder does not exist in the SD card\n\nPlease check your AO NDS installation");
		while (1) swiWaitForVBlank();
	}
	else if (!dir2)
	{
		consoleDemoInit();
		iprintf("'/data/ao-nds/ui' folder does not exist in the SD card\n\nPlease check your AO NDS installation");
		while (1) swiWaitForVBlank();
	}

	closedir(dir1);
	closedir(dir2);

	videoSetMode(MODE_3_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, true);
	oamInit(&oamSub, SpriteMapping_1D_128, true);

	u8* acename = readFile("/data/ao-nds/ui/AceName/acename.ttf");
	u8* igiari = readFile("/data/ao-nds/ui/Igiari/Igiari.otf");
	initFont(acename, 13);	// index 0
	initFont(igiari, 16);	// index 1

	u32 tilesLen = showDisclaimer();

	fadeDisclaimer(tilesLen);

	gEngine = new Engine;
	gEngine->changeScreen(new UIScreenWifi);

	while (gEngine->isRunning())
	{
		scanKeys();

		gEngine->updateInput();
		gEngine->update();

		bgUpdate();
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);

		swiWaitForVBlank();
	}

	delete gEngine;
	delete[] acename;
	delete[] igiari;

	return 0;
}
