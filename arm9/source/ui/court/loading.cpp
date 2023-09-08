#include "ui/court/loading.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>

#include "engine.h"
#include "ui/court/console.h"
#include "bg_talkEmpty.h"
#include "spr_loading.h"

UICourtLoading::~UICourtLoading()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_talkEmptyTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_talkEmptyMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	oamFreeGfx(&oamSub, sprLoading);
	oamClearSprite(&oamSub, 0);

	AOsocket* sock = gEngine->getSocket();
	sock->removeMessageCallback("SI", cbSI);
	sock->removeMessageCallback("SC", cbSC);
	sock->removeMessageCallback("SM", cbSM);
	sock->removeMessageCallback("DONE", cbDONE);

	delete lbl_loading;
}

void UICourtLoading::init()
{
	ticks = 0;
	frame = 0;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_talkEmptyTiles, bgGetGfxPtr(bgIndex), bg_talkEmptyTilesLen);
	dmaCopy(bg_talkEmptyMap, bgGetMapPtr(bgIndex), bg_talkEmptyMapLen);
	dmaCopy(bg_talkEmptyPal, BG_PALETTE_SUB, bg_talkEmptyPalLen);

	sprLoading = oamAllocateGfx(&oamSub, SpriteSize_16x16, SpriteColorFormat_256Color);
	u8* offset = (u8*)spr_loadingTiles + (frame*16*16);
	dmaCopy(offset, sprLoading, 16*16);
	oamSet(&oamSub, 0, 256-24, 192-36, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, sprLoading, -1, false, false, false, false, false);

	vramSetBankI(VRAM_I_LCD);
	dmaCopy(spr_loadingPal, &VRAM_I_EXT_SPR_PALETTE[0], spr_loadingPalLen);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	lbl_loading = new UILabel(&oamSub, 1, 8, 1, RGB15(31,31,31), 1, 0);
	setText("Connecting...");

	AOsocket* sock = gEngine->getSocket();
	cbSI = sock->addMessageCallback("SI", onMessageSI, this);
	cbSC = sock->addMessageCallback("SC", onMessageSC, this);
	cbSM = sock->addMessageCallback("SM", onMessageSM, this);
	cbDONE = sock->addMessageCallback("DONE", onMessageDone, this);
}

void UICourtLoading::updateInput()
{

}

void UICourtLoading::update()
{
	if (++ticks > 4)
	{
		ticks = 0;
		frame = (frame+1) % 8;
		u8* offset = (u8*)spr_loadingTiles + (frame*16*16);
		dmaCopy(offset, sprLoading, 16*16);
	}
}

void UICourtLoading::setText(const char* text)
{
	lbl_loading->setText(text);
	lbl_loading->setPos(128, 96-6, true);
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
	pSelf->pCourtUI->changeScreen(new UICourtConsole(pSelf->pCourtUI));
}
