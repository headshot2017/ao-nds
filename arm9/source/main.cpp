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
#include "bg_disclaimer.h"
#include "engine.h"
#include "sockets/aowebsocket.h"
#include "sockets/aotcpsocket.h"
#include "ui/uiwificonnect.h"

#include "acename_ttf.h"
#include "Igiari_ttf.h"

void showDisclaimer()
{
	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_disclaimerTiles, bgGetGfxPtr(0), bg_disclaimerTilesLen);
	dmaCopy(bg_disclaimerMap, bgGetMapPtr(0), bg_disclaimerMapLen);
	dmaCopy(bg_disclaimerPal, BG_PALETTE, bg_disclaimerPalLen);

	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0;
	REG_BLDY = 16;
}

void fadeDisclaimer() {
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

	dmaFillHalfWords(0, bgGetGfxPtr(0), bg_disclaimerTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(0), bg_disclaimerMapLen);
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

	if (!fatInitDefault())
	{
		consoleDemoInit();
		iprintf("Failed to initialize libfat\nPlease check your microSD card\n");
		while (1) swiWaitForVBlank();
	}

	videoSetMode(MODE_3_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, true);
	oamInit(&oamSub, SpriteMapping_1D_128, true);

	initFont(acename_ttf, 13);	// index 0
	initFont(Igiari_ttf, 16);	// index 1

	showDisclaimer();

	fadeDisclaimer();

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

	return 0;
}
