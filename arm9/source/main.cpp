#include <stdio.h>
#include <time.h>
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

#include "libadx.h"
#include "fonts.h"
#include "engine.h"
#include "settings.h"
#include "content.h"
#include "wifikb/wifikb.h"
#include "ui/uiwificonnect.h"

u32 showDisclaimer()
{
	REG_BLDCNT = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP;
	REG_BLDCNT_SUB = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP;
	REG_BLDY = 16;
	REG_BLDY_SUB = 16;

	int alpha = 32;
	while (1)
	{
		swiWaitForVBlank();

		alpha--;
		REG_BLDY = alpha>>1;
		REG_BLDY_SUB = alpha>>1;
		if (alpha == 0)
			break;
	}

	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0;
	REG_BLDCNT_SUB = BLEND_NONE;
	REG_BLDY = 16;

	u32 bgTilesLen, bgMapLen, bgPalLen;
	u8* bgTiles = readFile("/data/ao-nds/ui/bg_disclaimer.img.bin", &bgTilesLen);
	u8* bgMap = readFile("/data/ao-nds/ui/bg_disclaimer.map.bin", &bgMapLen);
	u8* bgPal = readFile("/data/ao-nds/ui/bg_disclaimer.pal.bin", &bgPalLen);

	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bgTiles, bgGetGfxPtr(0), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(0), bgMapLen);
	dmaCopy(bgPal, BG_PALETTE, bgPalLen);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;

	return bgTilesLen;
}

void fadeDisclaimer(u32 tilesLen) {
	int ticks = 0;
	int alpha = 32;
	int alphaAdd = -1;
	while (1)
	{
		scanKeys();
		if (keysDown()) break;
		swiWaitForVBlank();

		alpha += alphaAdd;
		REG_BLDY = alpha>>1;
		if (alpha == 0 && alphaAdd)
			alphaAdd = 0;
		else if (alpha == 32)
			break;
		else if (!alphaAdd && ticks++ >= 60*3)
			alphaAdd = 1;
	}

	dmaFillHalfWords(0, bgGetGfxPtr(0), tilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(0), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);
	REG_BLDCNT = BLEND_NONE;
}

int main()
{
	defaultExceptionHandler();
	adx_init();
	srand(time(0));

	if (!fatInitDefault())
	{
		consoleDemoInit();
		printf("Failed to initialize libfat\nPlease check your SD card\n");
		while (1) swiWaitForVBlank();
	}

	DIR *dir1 = opendir("/data/ao-nds");
	DIR *dir2 = opendir("/data/ao-nds/ui");
	if (!dir1)
	{
		consoleDemoInit();
		printf("'/data/ao-nds' folder does not exist in the SD card\n\nPlease check your AO NDS installation");
		while (1) swiWaitForVBlank();
	}
	else if (!dir2)
	{
		consoleDemoInit();
		printf("'/data/ao-nds/ui' folder does not exist in the SD card\n\nPlease check your AO NDS installation");
		while (1) swiWaitForVBlank();
	}

	closedir(dir1);
	closedir(dir2);

	REG_BLDCNT = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP;
	REG_BLDCNT_SUB = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP;
	REG_BLDY = 16;
	REG_BLDY_SUB = 16;

	videoSetMode(MODE_0_2D);
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

	Settings::load();

	gEngine = new Engine;
	gEngine->changeScreen(new UIScreenWifi);

	while (gEngine->isRunning())
	{
		swiWaitForVBlank();
		scanKeys();

		wifikb::update();
		gEngine->updateInput();
		gEngine->update();

		bgUpdate();
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);

		adx_update();
	}

	delete gEngine;
	delete[] acename;
	delete[] igiari;

	return 0;
}
