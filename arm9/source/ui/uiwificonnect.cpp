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
#include "fonts.h"
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
	delete[] textCanvas;
	delete[] sprLoadingImg;

	oamFreeGfx(&oamSub, sprLoading);
	oamClearSprite(&oamSub, 0);
	for (int i=0; i<8; i++)
	{
		oamFreeGfx(&oamSub, textGfx[i]);
		oamClearSprite(&oamSub, 1+i);
	}

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
	textCanvas = new u8[32*16];

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
	memcpy(BG_PALETTE, bgPal, 512);

	dmaCopy(bgSubTiles, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaCopy(bgSubMap, bgGetMapPtr(subBgIndex), 1536);
	memcpy(BG_PALETTE_SUB, bgSubPal, 512);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;
	delete[] bgSubTiles;
	delete[] bgSubMap;
	delete[] bgSubPal;

	sprLoading = oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_256Color);
	for (int i=0; i<8; i++)
	{
		textGfx[i] = oamAllocateGfx(&oamSub, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
	}

	sprLoadingImg = readFile("/data/ao-nds/ui/spr_loading.img.bin");
	u8* sprLoadingPal = readFile("/data/ao-nds/ui/spr_loading.pal.bin");

	u8* offset = sprLoadingImg + (frame*16*16);
	dmaCopy(offset, sprLoading, 16*16);
	oamSet(&oamSub, 0, 256-24, 192-24, 0, 1, SpriteSize_16x16, SpriteColorFormat_256Color, sprLoading, -1, false, false, false, false, false);

	vramSetBankI(VRAM_I_LCD);
	VRAM_I_EXT_SPR_PALETTE[0][2] = RGB15(31,31,31);
	memcpy(&VRAM_I_EXT_SPR_PALETTE[1], sprLoadingPal, 512);
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
	for (int i=0; i<8; i++)
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
	memset(textCanvas, 0, 32*16);

	int textWidth = getTextWidth(0, text);
	for (int i=0; i<8; i++)
		oamSet(&oamSub, i+1, 128-(textWidth/2)+(i*32), 96-6, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, textGfx[i], -1, false, false, false, false, false);

	oamUpdate(&oamSub);
	renderText(0, text, 2, 32, 16, textCanvas, SpriteSize_32x16, textGfx, 8);
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
