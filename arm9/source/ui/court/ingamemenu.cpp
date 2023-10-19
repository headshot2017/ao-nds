#include "ui/court/ingamemenu.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "ui/court/charselect.h"
#include "ui/court/musiclist.h"
#include "ui/court/icchatlog.h"
#include "ui/court/ooc.h"
#include "ui/court/ic.h"

UICourtIngameMenu::~UICourtIngameMenu()
{
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
	loadBg("/data/ao-nds/ui/bg_ingameMain", true);

	btn_talkIC = new UIButton(&oamSub, "/data/ao-nds/ui/spr_talkIC", 0, 2, 1, SpriteSize_64x32, 8, 54, 112, 28, 64, 32, 0);
	btn_talkOOC = new UIButton(&oamSub, "/data/ao-nds/ui/spr_talkOOC", btn_talkIC->nextOamInd(), 2, 1, SpriteSize_64x32, 8, 110, 112, 28, 64, 32, 1);
	btn_music = new UIButton(&oamSub, "/data/ao-nds/ui/spr_musicAreas", btn_talkOOC->nextOamInd(), 2, 1, SpriteSize_64x32, 136, 54, 112, 28, 64, 32, 2);
	btn_changeChar = new UIButton(&oamSub, "/data/ao-nds/ui/spr_changeChar", btn_music->nextOamInd(), 2, 1, SpriteSize_64x32, 136, 110, 112, 28, 64, 32, 3);
	btn_courtRecord = new UIButton(&oamSub, "/data/ao-nds/ui/spr_courtRecord", btn_changeChar->nextOamInd(), 3, 1, SpriteSize_32x32, 256-80, 0, 80, 32, 32, 32, 4);
	lbl_currChar = new UILabel(&oamSub, btn_courtRecord->nextOamInd(), 6, 1, RGB15(5,5,5), 5, 0);

	btn_courtRecord->assignKey(KEY_R);

	btn_talkIC->connect(onTalkICclicked, this);
	btn_talkOOC->connect(onTalkOOCclicked, this);
	btn_music->connect(onMusicClicked, this);
	btn_changeChar->connect(onChangeCharClicked, this);
	btn_courtRecord->connect(onCourtRecordClicked, this);

	lbl_currChar->setPos(4, 2);
	lbl_currChar->setText((pCourtUI->getCurrCharID() >= 0) ? pCourtUI->getCurrChar().name.c_str() : "Spectator");
}

void UICourtIngameMenu::updateInput()
{
	btn_talkIC->updateInput();
	btn_talkOOC->updateInput();
	btn_music->updateInput();
	btn_changeChar->updateInput();
	btn_courtRecord->updateInput();

	if (keysDown() & KEY_Y)
	{
		soundPlaySample(pCourtUI->sndSelect, SoundFormat_16Bit, pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
}

void UICourtIngameMenu::update()
{

}

void UICourtIngameMenu::onTalkICclicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtIC(pSelf->pCourtUI));
}

void UICourtIngameMenu::onTalkOOCclicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtOOC(pSelf->pCourtUI));
}

void UICourtIngameMenu::onMusicClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtMusicList(pSelf->pCourtUI));
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
