#include "ui/uimainmenu.h"

#include <nds/arm9/input.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/uiserverlist.h"
#include "ui/uidirectconn.h"
#include "bg_title.h"
#include "bg_logo.h"
#include "spr_viewServerList.h"
#include "spr_directConnect.h"
#include "spr_settings.h"

UIScreenMainMenu::~UIScreenMainMenu()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bg_titleTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), bg_titleMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete[] sndGavel;
	delete btn_viewServerList;
	delete btn_directConnect;
	delete btn_settings;
}

void UIScreenMainMenu::init()
{
	sndGavel = wav_load_handle("/data/ao-nds/sounds/general/sfx-gavel.wav", &sndGavelSize);

	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgShow(bgIndex);
	bgShow(subBgIndex);

	dmaCopy(bg_logoTiles, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaCopy(bg_logoMap, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaCopy(bg_logoPal, BG_PALETTE, bg_logoPalLen);

	dmaCopy(bg_titleTiles, bgGetGfxPtr(subBgIndex), bg_titleTilesLen);
	dmaCopy(bg_titleMap, bgGetMapPtr(subBgIndex), bg_titleMapLen);
	dmaCopy(bg_titlePal, BG_PALETTE_SUB, bg_titlePalLen);

	btn_viewServerList = new UIButton(&oamSub, (u8*)spr_viewServerListTiles, (u8*)spr_viewServerListPal, 0, 3, 1, SpriteSize_64x64, 128-88, 32, 176, 58, 64, 64, 0);
	btn_directConnect = new UIButton(&oamSub, (u8*)spr_directConnectTiles, (u8*)spr_directConnectPal, btn_viewServerList->nextOamInd(), 7, 1, SpriteSize_32x32, 128-111, 104, 223, 26, 32, 32, 1);
	btn_settings = new UIButton(&oamSub, (u8*)spr_settingsTiles, (u8*)spr_settingsPal, btn_directConnect->nextOamInd(), 3, 1, SpriteSize_32x32, 128-(76/2), 144, 76, 26, 32, 32, 2);

	btn_viewServerList->connect(onViewServerList, this);
	btn_directConnect->connect(onDirectConnect, this);
	btn_settings->connect(onSettings, this);
}

void UIScreenMainMenu::updateInput()
{
	btn_viewServerList->updateInput();
	btn_directConnect->updateInput();
	btn_settings->updateInput();
}

void UIScreenMainMenu::onViewServerList(void* pUserData)
{
	UIScreenMainMenu* pSelf = (UIScreenMainMenu*)pUserData;
	soundPlaySample(pSelf->sndGavel, SoundFormat_16Bit, pSelf->sndGavelSize, 32000, 127, 64, false, 0);

	gEngine->changeScreen(new UIScreenServerList);
}

void UIScreenMainMenu::onDirectConnect(void* pUserData)
{
	UIScreenMainMenu* pSelf = (UIScreenMainMenu*)pUserData;
	soundPlaySample(pSelf->sndGavel, SoundFormat_16Bit, pSelf->sndGavelSize, 32000, 127, 64, false, 0);

	gEngine->changeScreen(new UIScreenDirectConn);
}

void UIScreenMainMenu::onSettings(void* pUserData)
{
	UIScreenMainMenu* pSelf = (UIScreenMainMenu*)pUserData;
	soundPlaySample(pSelf->sndGavel, SoundFormat_16Bit, pSelf->sndGavelSize, 32000, 127, 64, false, 0);
}
