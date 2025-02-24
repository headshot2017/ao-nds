#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <vector>

#include <nds/arm9/background.h>
#include <nds/arm9/cache.h>
#include <nds/arm9/console.h>
#include <nds/arm9/exceptions.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/interrupts.h>
#include <nds/ndstypes.h>
#include <fat.h>

#include "mem.h"
#include "libadx.h"
#include "fonts.h"
#include "engine.h"
#include "settings.h"
#include "content.h"
#include "wifikb/wifikb.h"
#include "ui/uiwificonnect.h"
#include "ui/uicourt.h"

static u32 showDisclaimer()
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

	mem_free(bgTiles);
	mem_free(bgMap);
	mem_free(bgPal);

	return bgTilesLen;
}

static void fadeDisclaimer(u32 tilesLen)
{
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

static int adx_cothread(void* arg)
{
	while (gEngine->isRunning())
	{
		adx_update();
		cothread_yield_irq(IRQ_VBLANK);
	}
	return 0;
}

static void handler()
{
	debugLabelPressA("memory alloc fail");
	UIScreen* scr = gEngine->getScreen();
	if (scr->ID() == 0)
	{
		UIScreenCourt* uicourt = (UIScreenCourt*)scr;
		Courtroom* court = uicourt->getCourtroom();
		court->getShout()->freeSound();
		court->stopMusic();
		court->getCharacter(0)->unloadSound();
	}
}

int main()
{
	std::set_new_handler(handler);

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

	REG_BLDCNT = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1;
	REG_BLDCNT_SUB = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1;
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
	u8* igiari = readFile("/data/ao-nds/ui/igiari-cyrillic/igiari-cyrillic.ttf");
	initFont(acename, 13);	// index 0
	initFont(igiari, 16);	// index 1

	u32 tilesLen = showDisclaimer();

	fadeDisclaimer(tilesLen);

	Settings::load();

	gEngine = new Engine;
	gEngine->changeScreen(new UIScreenWifi);

	cothread_t adxThread = cothread_create(adx_cothread, 0, 1024*4, 0);

	while (gEngine->isRunning())
	{
		cothread_yield_irq(IRQ_VBLANK);
		scanKeys();

		if (keysHeld() & KEY_SELECT && keysHeld() & KEY_START)
		{
			u32 total = mem_get_allocated();
			std::string msg = std::to_string(total) + " - " + std::to_string(total/1024) + " KB";
			debugLabelPressA(msg.c_str());
		}

		wifikb::update();
		gEngine->updateInput();
		gEngine->update();

		bgUpdate();
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
	}

	while (!cothread_has_joined(adxThread))
		cothread_yield_irq(IRQ_VBLANK);
	cothread_delete(adxThread);

	delete gEngine;
	mem_free(acename);
	mem_free(igiari);

	return 0;
}
