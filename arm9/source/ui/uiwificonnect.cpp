#include "ui/uiwificonnect.h"

#include <string.h>
#include <string>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <dswifi9.h>

#include "engine.h"
#include "fonts.h"
#include "ui/uicourt.h"
#include "sockets/aowebsocket.h"
#include "bg_logo.h"
#include "bg_title.h"
#include "spr_loading.h"

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

	oamFreeGfx(&oamSub, sprLoading);
	oamClearSprite(&oamSub, 0);
	for (int i=0; i<8; i++)
	{
		oamFreeGfx(&oamSub, textGfx[i]);
		oamClearSprite(&oamSub, 1+i);
	}

	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bg_titleTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), bg_titleMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);
}

void UIScreenWifi::init()
{
	currAssocStatus = -1;
	ticks = 0;
	frame = 0;
	textCanvas = new u8[32*16];

	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgShow(bgIndex);
	bgShow(subBgIndex);
	sprLoading = oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_256Color);
	for (int i=0; i<8; i++)
	{
		textGfx[i] = oamAllocateGfx(&oamSub, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
	}

	u8* offset = (u8*)spr_loadingTiles + (frame*16*16);
	dmaCopy(offset, sprLoading, 16*16);
	oamSet(&oamSub, 0, 256-24, 192-24, 0, 1, SpriteSize_16x16, SpriteColorFormat_256Color, sprLoading, -1, false, false, false, false, false);

	vramSetBankI(VRAM_I_LCD);
	VRAM_I_EXT_SPR_PALETTE[0][2] = RGB15(31,31,31);
	dmaCopy(spr_loadingPal, &VRAM_I_EXT_SPR_PALETTE[1], spr_loadingPalLen);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	dmaCopy(bg_logoTiles, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaCopy(bg_logoMap, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaCopy(bg_logoPal, BG_PALETTE, bg_logoPalLen);

	dmaCopy(bg_titleTiles, bgGetGfxPtr(subBgIndex), bg_titleTilesLen);
	dmaCopy(bg_titleMap, bgGetMapPtr(subBgIndex), bg_titleMapLen);
	dmaCopy(bg_titlePal, BG_PALETTE_SUB, bg_titlePalLen);

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
		u8* offset = (u8*)spr_loadingTiles + (frame*16*16);
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
	{
		std::string ip = "ws://vanilla.aceattorneyonline.com:2095/";
		AOwebSocket* sock = new AOwebSocket;
		sock->connectIP(ip);

		gEngine->setSocket(sock);
		gEngine->changeScreen(new UIScreenCourt);
	}
}
