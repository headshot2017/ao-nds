#include "ui/uimainmenu.h"

#include <nds/arm9/input.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>

#include "engine.h"
#include "ui/uiserverlist.h"
#include "ui/uidirectconn.h"
#include "ui/uisettings.h"
#include "mem.h"

UIScreenMainMenu::~UIScreenMainMenu()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	wav_free_handle(sndGavel);

	delete btn_viewServerList;
	delete btn_directConnect;
	delete btn_settings;
	if (lbl_dsi) delete lbl_dsi;
}

void UIScreenMainMenu::init()
{
	sndGavel = wav_load_handle("/data/ao-nds/sounds/general/sfx-gavel.wav");

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

	mem_free(bgTiles);
	mem_free(bgMap);
	mem_free(bgPal);
	mem_free(bgSubTiles);
	mem_free(bgSubMap);
	mem_free(bgSubPal);

	btn_viewServerList = new UIButton(&oamSub, "/data/ao-nds/ui/spr_viewServerList", 0, 3, 1, SpriteSize_64x64, 128-88, 32, 176, 58, 64, 64, 0);
	btn_directConnect = new UIButton(&oamSub, "/data/ao-nds/ui/spr_directConnect", btn_viewServerList->nextOamInd(), 7, 1, SpriteSize_32x32, 128-111, 104, 223, 26, 32, 32, 1);
	btn_settings = new UIButton(&oamSub, "/data/ao-nds/ui/spr_settings", btn_directConnect->nextOamInd(), 3, 1, SpriteSize_32x32, 128-(76/2), 144, 76, 26, 32, 32, 2);

	if (isDSiMode())
	{
		lbl_dsi = new UILabel(&oamMain, 0, 6, 1, RGB15(31,31,31), 0, 1);
		lbl_dsi->setText("Running on DSi Mode");
		lbl_dsi->setPos(128, 192-32, true);
	}
	else
		lbl_dsi = 0;

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
	wav_play(pSelf->sndGavel);

	gEngine->changeScreen(new UIScreenServerList);
}

void UIScreenMainMenu::onDirectConnect(void* pUserData)
{
	UIScreenMainMenu* pSelf = (UIScreenMainMenu*)pUserData;
	wav_play(pSelf->sndGavel);

	gEngine->changeScreen(new UIScreenDirectConn);
}

void UIScreenMainMenu::onSettings(void* pUserData)
{
	UIScreenMainMenu* pSelf = (UIScreenMainMenu*)pUserData;
	wav_play(pSelf->sndGavel);

	gEngine->changeScreen(new UIScreenSettings);
}
