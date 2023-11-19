#include "ui/uidisconnected.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>

#include "engine.h"
#include "global.h"
#include "ui/uiwificonnect.h"
#include "ui/uimainmenu.h"

UIScreenDisconnected::~UIScreenDisconnected()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete lbl_disconnectMsg;
	delete lbl_reason;
	delete btn_ok;
	wav_free_handle(sndSelect);
}

void UIScreenDisconnected::init()
{
	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav");

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

	lbl_disconnectMsg = new UILabel(&oamSub, 0, 8, 1, RGB15(31,31,31), 0, 1);
	lbl_reason = new UILabel(&oamSub, lbl_disconnectMsg->nextOamInd(), 8, 6, RGB15(31,31,31), 0, 0);
	btn_ok = new UIButton(&oamSub, "/data/ao-nds/ui/spr_ok", lbl_reason->nextOamInd(), 3, 1, SpriteSize_32x32, 128-(76/2), 160, 76, 26, 32, 32, 1);

	lbl_disconnectMsg->setText(disconnectMsg.c_str());
	lbl_disconnectMsg->setPos(128, 16, true);
	lbl_reason->setText(reason.c_str());
	lbl_reason->setPos(128, 36, true);

	btn_ok->assignKey(KEY_A);
	btn_ok->connect(onOK, this);
}

void UIScreenDisconnected::updateInput()
{
	btn_ok->updateInput();
}

void UIScreenDisconnected::onOK(void* pUserData)
{
	UIScreenDisconnected* pSelf = (UIScreenDisconnected*)pUserData;
	wav_play(pSelf->sndSelect);

	if (pSelf->goToWiFi)
		gEngine->changeScreen(new UIScreenWifi);
	else
		gEngine->changeScreen(new UIScreenMainMenu);
}
