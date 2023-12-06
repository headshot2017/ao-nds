#include "ui/uiwificonnect.h"

#include <string.h>
#include <string>

#include <nds/arm9/input.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>
#include <dswifi9.h>

#include "engine.h"
#include "ui/uimainmenu.h"

const char* assocStatusDetails[] = {
	"Not connecting",
	"Searching for an access point...",
	"Authenticating to access point...",
	"Connecting to access point...",
	"Getting DHCP...",
	"Connected successfully!",
	"Failed to connect to Wi-Fi. Press A to try again"
};

UIScreenWifi::~UIScreenWifi()
{
	delete lbl_loading;
	delete[] sprLoadingImg;

	oamFreeGfx(&oamSub, sprLoading);
	oamClearSprite(&oamSub, 0);

	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);
}

void UIScreenWifi::init()
{
	currAssocStatus = -1;
	ticks = 0;
	frame = 0;

	u8* bgTiles = readFile("/data/ao-nds/ui/bg_logo.img.bin", &bgTilesLen);
	u8* bgMap = readFile("/data/ao-nds/ui/bg_logo.map.bin");
	u8* bgPal = readFile("/data/ao-nds/ui/bg_logo.pal.bin");
	u8* bgSubTiles = readFile("/data/ao-nds/ui/bg_title.img.bin", &bgSubTilesLen);
	u8* bgSubMap = readFile("/data/ao-nds/ui/bg_title.map.bin");
	u8* bgSubPal = readFile("/data/ao-nds/ui/bg_title.pal.bin");

	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);
	dmaCopy(bgPal, BG_PALETTE, 512);

	dmaCopy(bgSubTiles, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaCopy(bgSubMap, bgGetMapPtr(subBgIndex), 1536);
	dmaCopy(bgSubPal, BG_PALETTE_SUB, 512);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;
	delete[] bgSubTiles;
	delete[] bgSubMap;
	delete[] bgSubPal;

	lbl_loading = new UILabel(&oamSub, 1, 8, 1, RGB15(31,31,31), 0, 0);

	sprLoading = oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_256Color);
	sprLoadingImg = readFile("/data/ao-nds/ui/spr_loading.img.bin");
	u8* sprLoadingPal = readFile("/data/ao-nds/ui/spr_loading.pal.bin");

	u8* offset = sprLoadingImg + (frame*16*16);
	dmaCopy(offset, sprLoading, 16*16);
	oamSet(&oamSub, 0, 256-24, 192-24, 0, 1, SpriteSize_16x16, SpriteColorFormat_256Color, sprLoading, -1, false, false, false, false, false);

	vramSetBankI(VRAM_I_LCD);
	dmaCopy(sprLoadingPal, &VRAM_I_EXT_SPR_PALETTE[1], 512);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	delete[] sprLoadingPal;

	Wifi_InitDefault(INIT_ONLY);

	unsigned char mac[6] = {0};
	Wifi_GetData(WIFIGETDATA_MACADDRESS, 6, mac);
	char macStr[32] = {0};
	for (int i=0; i<6; i++)
	{
		char buf[4];
		sprintf(buf, "%x", mac[i]);
		strcat(macStr, buf);
	}
	gEngine->setMacAddr(macStr);

	Wifi_AutoConnect();
}

void UIScreenWifi::setText(const char* text)
{
	lbl_loading->setText(text);
	lbl_loading->setPos(128, 96-4, true);
	oamUpdate(&oamSub);
}

void UIScreenWifi::updateInput()
{
	if (currAssocStatus == ASSOCSTATUS_CANNOTCONNECT && keysDown() & KEY_A)
	{
		Wifi_DisconnectAP();
		Wifi_AutoConnect();
	}
}

void UIScreenWifi::update()
{
	if (++ticks > 4)
	{
		ticks = 0;
		frame = (frame+1) % 8;
		u8* offset = sprLoadingImg + (frame*16*16);
		dmaCopy(offset, sprLoading, 16*16);
	}

	int assocStatus = Wifi_AssocStatus();
	if (currAssocStatus != assocStatus)
	{
		currAssocStatus = assocStatus;
		setText(assocStatusDetails[assocStatus]);
	}

	oamSetHidden(&oamSub, 0, (assocStatus == ASSOCSTATUS_CANNOTCONNECT));

	if (Wifi_AssocStatus() == ASSOCSTATUS_ASSOCIATED && !gEngine->isFading())
		gEngine->changeScreen(new UIScreenMainMenu);
}
