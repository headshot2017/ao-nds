#include "ui/court/ingamemenu.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "ui/court/charselect.h"
#include "bg_ingameMain.h"
#include "spr_talkIC.h"
#include "spr_talkOOC.h"
#include "spr_musicAreas.h"
#include "spr_changeChar.h"
#include "spr_courtRecord.h"

UICourtIngameMenu::~UICourtIngameMenu()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_ingameMainTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_ingameMainMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete btn_talkIC;
	delete btn_talkOOC;
	delete btn_music;
	delete btn_changeChar;
	delete btn_courtRecord;
	delete lbl_currChar;
}

void UICourtIngameMenu::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_ingameMainTiles, bgGetGfxPtr(bgIndex), bg_ingameMainTilesLen);
	dmaCopy(bg_ingameMainMap, bgGetMapPtr(bgIndex), bg_ingameMainMapLen);
	dmaCopy(bg_ingameMainPal, BG_PALETTE_SUB, bg_ingameMainPalLen);

	btn_talkIC = new UIButton(&oamSub, (u8*)spr_talkICTiles, (u8*)spr_talkICPal, 0, 2, 1, SpriteSize_64x32, 8, 54, 112, 28, 64, 32, 0);
	btn_talkOOC = new UIButton(&oamSub, (u8*)spr_talkOOCTiles, (u8*)spr_talkOOCPal, btn_talkIC->nextOamInd(), 2, 1, SpriteSize_64x32, 8, 110, 112, 28, 64, 32, 1);
	btn_music = new UIButton(&oamSub, (u8*)spr_musicAreasTiles, (u8*)spr_musicAreasPal, btn_talkOOC->nextOamInd(), 2, 1, SpriteSize_64x32, 136, 54, 112, 28, 64, 32, 2);
	btn_changeChar = new UIButton(&oamSub, (u8*)spr_changeCharTiles, (u8*)spr_changeCharPal, btn_music->nextOamInd(), 2, 1, SpriteSize_64x32, 136, 110, 112, 28, 64, 32, 3);
	btn_courtRecord = new UIButton(&oamSub, (u8*)spr_courtRecordTiles, (u8*)spr_courtRecordPal, btn_changeChar->nextOamInd(), 3, 1, SpriteSize_32x64, 256-80, 0, 80, 33, 32, 64, 4);
	lbl_currChar = new UILabel(&oamSub, btn_courtRecord->nextOamInd(), 6, 1, RGB15(5,5,5), 5, 0);

	btn_talkIC->connect(onTalkICclicked, this);
	btn_talkOOC->connect(onTalkOOCclicked, this);
	btn_music->connect(onMusicClicked, this);
	btn_changeChar->connect(onChangeCharClicked, this);
	btn_courtRecord->connect(onCourtRecordClicked, this);

	lbl_currChar->setPos(4, 2, false);
	lbl_currChar->setText((pCourtUI->getCurrCharID() >= 0) ? pCourtUI->getCurrChar().name.c_str() : "Spectator");
}

void UICourtIngameMenu::updateInput()
{
	btn_talkIC->updateInput();
	btn_talkOOC->updateInput();
	btn_music->updateInput();
	btn_changeChar->updateInput();
	btn_courtRecord->updateInput();
}

void UICourtIngameMenu::update()
{

}

void UICourtIngameMenu::onTalkICclicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIngameMenu::onTalkOOCclicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIngameMenu::onMusicClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIngameMenu::onChangeCharClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtCharSelect(pSelf->pCourtUI));
}

void UICourtIngameMenu::onCourtRecordClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}
