#include "ui/court/ingamemenu.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "ui/court/charselect.h"
#include "ui/court/musiclist.h"
#include "ui/court/icchatlog.h"
#include "ui/court/ooc.h"
#include "ui/court/ic.h"
#include "ui/court/courtrecord.h"

UICourtIngameMenu::~UICourtIngameMenu()
{
	delete btn_talkIC;
	delete btn_talkOOC;
	delete btn_music;
	delete btn_changeChar;
	delete btn_courtRecord;
	delete btn_guard;
	delete lbl_currChar;
	delete lbl_guard;

	gEngine->getSocket()->removeMessageCallback("PV", cbPV);
	gEngine->getSocket()->removeMessageCallback("AUTH", cbAUTH);
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
	btn_guard = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", btn_courtRecord->nextOamInd(), 1, 1, SpriteSize_16x16, 4, 192-16, 16, 16, 16, 16, 5);
	lbl_currChar = new UILabel(&oamSub, btn_guard->nextOamInd(), 6, 1, RGB15(5,5,5), 6, 0);
	lbl_guard = new UILabel(&oamSub, lbl_currChar->nextOamInd(), 5, 1, RGB15(5,5,5), 6, 0);

	btn_courtRecord->assignKey(KEY_R);
	btn_guard->setFrame(pCourtUI->guard);

	btn_talkIC->connect(onTalkICclicked, this);
	btn_talkOOC->connect(onTalkOOCclicked, this);
	btn_music->connect(onMusicClicked, this);
	btn_changeChar->connect(onChangeCharClicked, this);
	btn_courtRecord->connect(onCourtRecordClicked, this);
	btn_guard->connect(onGuardToggled, this);

	lbl_currChar->setPos(4, 2);
	lbl_currChar->setText((pCourtUI->getCurrCharID() >= 0) ? pCourtUI->getCurrChar().name : "Spectator");
	lbl_guard->setPos(btn_guard->getX()+btn_guard->getW()+2, btn_guard->getY()+2);
	lbl_guard->setText("Guard Mode");

	if (!pCourtUI->isMod())
	{
		btn_guard->setVisible(false);
		lbl_guard->setVisible(false);
	}

	cbPV = gEngine->getSocket()->addMessageCallback("PV", onMessagePV, this);
	cbAUTH = gEngine->getSocket()->addMessageCallback("AUTH", onMessageAUTH, this);
}

void UICourtIngameMenu::updateInput()
{
	btn_talkIC->updateInput();
	btn_talkOOC->updateInput();
	btn_music->updateInput();
	btn_changeChar->updateInput();
	btn_courtRecord->updateInput();
	btn_guard->updateInput();

	if (keysDown() & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
}

void UICourtIngameMenu::update()
{

}

void UICourtIngameMenu::onTalkICclicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	pSelf->pCourtUI->changeScreen(new UICourtIC(pSelf->pCourtUI));
}

void UICourtIngameMenu::onTalkOOCclicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	pSelf->pCourtUI->changeScreen(new UICourtOOC(pSelf->pCourtUI));
}

void UICourtIngameMenu::onMusicClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	pSelf->pCourtUI->changeScreen(new UICourtMusicList(pSelf->pCourtUI));
}

void UICourtIngameMenu::onChangeCharClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	pSelf->pCourtUI->changeScreen(new UICourtCharSelect(pSelf->pCourtUI));
}

void UICourtIngameMenu::onCourtRecordClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtIngameMenu::onGuardToggled(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	pSelf->pCourtUI->guard = !pSelf->pCourtUI->guard;
	pSelf->btn_guard->setFrame(pSelf->pCourtUI->guard);
}

void UICourtIngameMenu::onMessagePV(void* pUserData, std::string msg)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	pSelf->lbl_currChar->setText((pSelf->pCourtUI->getCurrCharID() >= 0) ? pSelf->pCourtUI->getCurrChar().name : "Spectator");
}

void UICourtIngameMenu::onMessageAUTH(void* pUserData, std::string msg)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	pSelf->btn_guard->setVisible(pSelf->pCourtUI->isMod());
	pSelf->lbl_guard->setVisible(pSelf->pCourtUI->isMod());
}
