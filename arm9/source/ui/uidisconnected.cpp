#include "ui/uidisconnected.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/uiwificonnect.h"
#include "ui/uimainmenu.h"
//#include "spr_ok.h"
#include "bg_title.h"
#include "bg_logo.h"

UIScreenDisconnected::~UIScreenDisconnected()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bg_titleTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), bg_titleMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete lbl_disconnectMsg;
	delete lbl_reason;
	//delete btn_ok;
	delete[] sndSelect;
}

void UIScreenDisconnected::init()
{
	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav", &sndSelectSize);

	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	dmaCopy(bg_logoTiles, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaCopy(bg_logoMap, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaCopy(bg_logoPal, BG_PALETTE, bg_logoPalLen);

	dmaCopy(bg_titleTiles, bgGetGfxPtr(subBgIndex), bg_titleTilesLen);
	dmaCopy(bg_titleMap, bgGetMapPtr(subBgIndex), bg_titleMapLen);
	dmaCopy(bg_titlePal, BG_PALETTE_SUB, bg_titlePalLen);

	lbl_disconnectMsg = new UILabel(&oamSub, 0, 8, 1, RGB15(31,31,31), 0, 1);
	lbl_reason = new UILabel(&oamSub, lbl_disconnectMsg->nextOamInd(), 6, 6, RGB15(31,31,31), 1, 0);
	//btn_ok = new UIButton(&oamSub, (u8*)spr_okTiles, (u8*)spr_okPal, lbl_reason->nextOamInd(), 2, 1, SpriteSize_32x32, 128-16, 160, 64, 32, 32, 32, 2);

	lbl_disconnectMsg->setText(disconnectMsg.c_str());
	lbl_disconnectMsg->setPos(128, 16, true);
	lbl_reason->setText(reason.c_str());
	lbl_reason->setPos(128, 36, true);
	//btn_ok->connect(onOK, this);
}

void UIScreenDisconnected::updateInput()
{
	//btn_ok->updateInput();
}

void UIScreenDisconnected::onOK(void* pUserData)
{
	UIScreenDisconnected* pSelf = (UIScreenDisconnected*)pUserData;

	if (pSelf->goToWiFi)
		gEngine->changeScreen(new UIScreenWifi);
	else
		gEngine->changeScreen(new UIScreenMainMenu);
}
