#include "ui/court/arealist.h"

#include <nds/arm9/background.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/musiclist.h"
#include "bg_talkEmpty.h"
#include "spr_back.h"
#include "spr_music.h"

UICourtAreaList::~UICourtAreaList()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_talkEmptyTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_talkEmptyMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete btn_back;
	delete btn_listToggle;
}

void UICourtAreaList::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_talkEmptyTiles, bgGetGfxPtr(bgIndex), bg_talkEmptyTilesLen);
	dmaCopy(bg_talkEmptyMap, bgGetMapPtr(bgIndex), bg_talkEmptyMapLen);
	dmaCopy(bg_talkEmptyPal, BG_PALETTE_SUB, bg_talkEmptyPalLen);

	btn_back = new UIButton(&oamSub, (u8*)spr_backTiles, (u8*)spr_backPal, 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_listToggle = new UIButton(&oamSub, (u8*)spr_musicTiles, (u8*)spr_musicPal, btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 0, 79, 30, 32, 32, 1);

	btn_back->connect(onBackClicked, this);
	btn_listToggle->connect(onToggleList, this);
}

void UICourtAreaList::updateInput()
{
	btn_back->updateInput();
	btn_listToggle->updateInput();
}

void UICourtAreaList::update()
{

}

void UICourtAreaList::onBackClicked(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndCancel, SoundFormat_16Bit, pSelf->pCourtUI->sndCancelSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtAreaList::onToggleList(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtMusicList(pSelf->pCourtUI));
}
