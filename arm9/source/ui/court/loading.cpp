#include "ui/court/loading.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "engine.h"
#include "ui/uiserverlist.h"
#include "ui/court/charselect.h"

UICourtLoading::~UICourtLoading()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete[] sprLoadingImg;
	oamFreeGfx(&oamSub, sprLoading);
	oamClearSprite(&oamSub, 0);

	AOsocket* sock = gEngine->getSocket();
	sock->removeMessageCallback("SI", cbSI);
	sock->removeMessageCallback("SC", cbSC);
	sock->removeMessageCallback("SM", cbSM);
	sock->removeMessageCallback("DONE", cbDONE);

	delete btn_disconnect;
	delete lbl_loading;
}

void UICourtLoading::init()
{
	ticks = 0;
	frame = 0;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	u8* bgTiles = readFile("nitro:/bg_talkEmpty.img.bin", &bgTilesLen);
	u8* bgMap = readFile("nitro:/bg_talkEmpty.map.bin");
	u8* bgPal = readFile("nitro:/bg_talkEmpty.pal.bin");

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);
	dmaCopy(bgPal, BG_PALETTE_SUB, 512);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;

	sprLoading = oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_256Color);
	sprLoadingImg = readFile("nitro:/spr_loading.img.bin");
	u8* sprLoadingPal = readFile("nitro:/spr_loading.pal.bin");
	u8* offset = sprLoadingImg + (frame*16*16);
	dmaCopy(offset, sprLoading, 16*16);
	oamSet(&oamSub, 0, 256-24, 192-36, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, sprLoading, -1, false, false, false, false, false);

	vramSetBankI(VRAM_I_LCD);
	dmaCopy(sprLoadingPal, &VRAM_I_EXT_SPR_PALETTE[0], 512);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	delete[] sprLoadingPal;

	btn_disconnect = new UIButton(&oamSub, "nitro:/spr_disconnect", 1, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 1);
	lbl_loading = new UILabel(&oamSub, btn_disconnect->nextOamInd(), 8, 1, RGB15(31,31,31), 2, 0);
	setText("Connecting...");

	btn_disconnect->assignKey(KEY_B);
	btn_disconnect->connect(onDisconnectClicked, this);

	AOsocket* sock = gEngine->getSocket();
	cbSI = sock->addMessageCallback("SI", onMessageSI, this);
	cbSC = sock->addMessageCallback("SC", onMessageSC, this);
	cbSM = sock->addMessageCallback("SM", onMessageSM, this);
	cbDONE = sock->addMessageCallback("DONE", onMessageDone, this);
}

void UICourtLoading::updateInput()
{
	btn_disconnect->updateInput();
}

void UICourtLoading::update()
{
	if (++ticks > 4)
	{
		ticks = 0;
		frame = (frame+1) % 8;
		u8* offset = sprLoadingImg + (frame*16*16);
		dmaCopy(offset, sprLoading, 16*16);
	}
}

void UICourtLoading::setText(const char* text)
{
	lbl_loading->setText(text);
	lbl_loading->setPos(128, 96-6, true);
}

void UICourtLoading::onDisconnectClicked(void* pUserData)
{
	UICourtLoading* pSelf = (UICourtLoading*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndCancel, SoundFormat_16Bit, pSelf->pCourtUI->sndCancelSize, 32000, 127, 64, false, 0);
	gEngine->changeScreen(new UIScreenServerList);
}

void UICourtLoading::onMessageSI(void* pUserData, std::string msg)
{
	UICourtLoading* pSelf = (UICourtLoading*)pUserData;
	pSelf->setText("Getting character list...");
}

void UICourtLoading::onMessageSC(void* pUserData, std::string msg)
{
	UICourtLoading* pSelf = (UICourtLoading*)pUserData;
	pSelf->setText("Getting music list...");
}

void UICourtLoading::onMessageSM(void* pUserData, std::string msg)
{
	UICourtLoading* pSelf = (UICourtLoading*)pUserData;
	pSelf->setText("Getting taken character slots...");
}

void UICourtLoading::onMessageDone(void* pUserData, std::string msg)
{
	UICourtLoading* pSelf = (UICourtLoading*)pUserData;
	pSelf->pCourtUI->changeScreen(new UICourtCharSelect(pSelf->pCourtUI));
}
