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
#include "ui/court/message.h"

UICourtIngameMenu::~UICourtIngameMenu()
{
	delete btn_talkIC;
	delete btn_talkOOC;
	delete btn_music;
	delete btn_changeChar;
	delete btn_courtRecord;
	delete btn_guard;
	delete btn_callMod;
	delete lbl_currChar;
	delete lbl_guard;
	delete kb_input;

	gEngine->getSocket()->removeMessageCallback("PV", cbPV);
	gEngine->getSocket()->removeMessageCallback("AUTH", cbAUTH);
}

void UICourtIngameMenu::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_ingameMain");

	btn_talkIC = new UIButton(&oamSub, "/data/ao-nds/ui/spr_talkIC", 0, 2, 1, SpriteSize_64x32, 8, 54, 112, 28, 64, 32, 0);
	btn_talkOOC = new UIButton(&oamSub, "/data/ao-nds/ui/spr_talkOOC", btn_talkIC->nextOamInd(), 2, 1, SpriteSize_64x32, 8, 110, 112, 28, 64, 32, 1);
	btn_music = new UIButton(&oamSub, "/data/ao-nds/ui/spr_musicAreas", btn_talkOOC->nextOamInd(), 2, 1, SpriteSize_64x32, 136, 54, 112, 28, 64, 32, 2);
	btn_changeChar = new UIButton(&oamSub, "/data/ao-nds/ui/spr_changeChar", btn_music->nextOamInd(), 2, 1, SpriteSize_64x32, 136, 110, 112, 28, 64, 32, 3);
	btn_courtRecord = new UIButton(&oamSub, "/data/ao-nds/ui/spr_courtRecord", btn_changeChar->nextOamInd(), 3, 1, SpriteSize_32x32, 256-80, 0, 80, 32, 32, 32, 4);
	btn_guard = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", btn_courtRecord->nextOamInd(), 1, 1, SpriteSize_16x16, 4, 192-16, 16, 16, 16, 16, 5);
	btn_callMod = new UIButton(&oamSub, "/data/ao-nds/ui/spr_callMod", btn_guard->nextOamInd(), 2, 1, SpriteSize_32x16, 96, 192-36, 64, 15, 32, 16, 6);
	lbl_currChar = new UILabel(&oamSub, btn_callMod->nextOamInd(), 6, 1, RGB15(5,5,5), 7, 0);
	lbl_guard = new UILabel(&oamSub, lbl_currChar->nextOamInd(), 5, 1, RGB15(5,5,5), 7, 0);

	kb_input = new AOkeyboard(2, lbl_guard->nextOamInd(), 8);
	dmaCopy(bgPal, BG_PALETTE_SUB, 512);

	btn_courtRecord->assignKey(KEY_R);
	btn_guard->setFrame(pCourtUI->guard);

	btn_talkIC->connect(onTalkICclicked, this);
	btn_talkOOC->connect(onTalkOOCclicked, this);
	btn_music->connect(onMusicClicked, this);
	btn_changeChar->connect(onChangeCharClicked, this);
	btn_courtRecord->connect(onCourtRecordClicked, this);
	btn_guard->connect(onGuardToggled, this);
	btn_callMod->connect(onCallModClicked, this);

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
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			dmaCopy(bgPal, BG_PALETTE_SUB, 512);
			showEverything();

			if (result > 0)
			{
				if (kb_input->getValue().empty())
				{
					pCourtUI->changeScreen(new UICourtMessage(pCourtUI, "Please enter a reason"));
					return;
				}
				gEngine->getSocket()->sendData("ZZ#" + kb_input->getValueUTF8() + "#-1#%");
			}
		}
		return;
	}

	btn_talkIC->updateInput();
	btn_talkOOC->updateInput();
	btn_music->updateInput();
	btn_changeChar->updateInput();
	btn_courtRecord->updateInput();
	btn_guard->updateInput();
	btn_callMod->updateInput();

	if (keysDown() & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
}

void UICourtIngameMenu::update()
{

}

void UICourtIngameMenu::hideEverything()
{
	bgHide(bgIndex);

	btn_talkIC->setVisible(false);
	btn_talkOOC->setVisible(false);
	btn_music->setVisible(false);
	btn_changeChar->setVisible(false);
	btn_courtRecord->setVisible(false);
	btn_guard->setVisible(false);
	btn_callMod->setVisible(false);
	lbl_currChar->setVisible(false);
	lbl_guard->setVisible(false);
}

void UICourtIngameMenu::showEverything()
{
	bgShow(bgIndex);

	btn_talkIC->setVisible(true);
	btn_talkOOC->setVisible(true);
	btn_music->setVisible(true);
	btn_changeChar->setVisible(true);
	btn_courtRecord->setVisible(true);
	btn_guard->setVisible(pCourtUI->isMod());
	btn_callMod->setVisible(true);
	lbl_currChar->setVisible(true);
	lbl_guard->setVisible(pCourtUI->isMod());
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

void UICourtIngameMenu::onCallModClicked(void* pUserData)
{
	UICourtIngameMenu* pSelf = (UICourtIngameMenu*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->hideEverything();
	pSelf->kb_input->show16("Enter a reason");
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
