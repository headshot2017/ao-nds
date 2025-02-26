#include "ui/court/profiledetail.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "libadx.h"
#include "engine.h"
#include "content.h"
#include "ui/court/courtrecord.h"
#include "ui/court/icchatlog.h"
#include "ui/court/evidencedetail.h"
#include "ui/court/moderatordialog.h"
#include "ui/court/message.h"

UICourtProfileDetail::~UICourtProfileDetail()
{
	delete btn_back;
	delete btn_profilesEvidence;
	delete btn_prevPage;
	delete btn_nextPage;
	delete btn_report;
	delete btn_kick;
	delete btn_ban;
	delete btn_follow;
	delete lbl_name;
	delete lbl_desc;
	delete spr_profile;
	delete kb_input;

	gEngine->getSocket()->removeMessageCallback("PR", cbPR);
	gEngine->getSocket()->removeMessageCallback("PU", cbPU);
}

void UICourtProfileDetail::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_profileDetails");

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_profilesEvidence = new UIButton(&oamSub, "/data/ao-nds/ui/spr_profilesEvidence", btn_back->nextOamInd(), 1, 1, SpriteSize_64x32, 256-55, 0, 55, 31, 64, 32, 1);
	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_medium", btn_profilesEvidence->nextOamInd(), 1, 2, SpriteSize_16x32, 0, 64, 16, 63, 16, 32, 2);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_medium", btn_prevPage->nextOamInd(), 1, 2, SpriteSize_16x32, 256-16, 64, 16, 63, 16, 32, 3);
	btn_report = new UIButton(&oamSub, "/data/ao-nds/ui/spr_report", btn_nextPage->nextOamInd(), 2, 1, SpriteSize_32x16, 96, 36, 64, 15, 32, 16, 4);
	btn_kick = new UIButton(&oamSub, "/data/ao-nds/ui/spr_kickBan", btn_report->nextOamInd(), 2, 1, SpriteSize_32x16, 96-68, 36, 64, 15, 32, 16, 5);
	btn_ban = new UIButton(&oamSub, "/data/ao-nds/ui/spr_kickBan", btn_kick->nextOamInd(), 2, 1, SpriteSize_32x16, 96+68, 36, 64, 15, 32, 16, 5);
	btn_follow = new UIButton(&oamSub, "/data/ao-nds/ui/spr_goToArea", btn_ban->nextOamInd(), 2, 1, SpriteSize_32x16, 96, 192-15-36, 64, 15, 32, 16, 6);

	lbl_name = new UILabel(&oamSub, btn_follow->nextOamInd(), 4, 1, RGB15(31, 16, 0), 7, 0);
	lbl_desc = new UILabel(&oamSub, lbl_name->nextOamInd(), 4, 4, RGB15(4, 4, 4), 8, 0);

	spr_profile = new UIButton(&oamSub, "", lbl_desc->nextOamInd(), 1, 1, SpriteSize_64x64, 23, 66, 60, 60, 64, 64, 9);

	kb_input = new AOkeyboard(2, spr_profile->nextOamInd(), 10);

	btn_profilesEvidence->setFrame(1);
	btn_ban->setFrame(1);
	lbl_desc->setPos(99, 80);

	btn_back->assignKey(KEY_B);
	btn_profilesEvidence->assignKey(KEY_R);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	btn_back->connect(onBackClicked, this);
	btn_profilesEvidence->connect(onProfilesEvidenceClicked, this);
	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	btn_report->connect(onReport, this);
	btn_kick->connect(onKick, this);
	btn_ban->connect(onBan, this);
	btn_follow->connect(onFollow, this);

	reloadPage();

	cbPR = gEngine->getSocket()->addMessageCallback("PR", onMessagePR, this);
	cbPU = gEngine->getSocket()->addMessageCallback("PU", onMessagePU, this);
}

void UICourtProfileDetail::updateInput()
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
				gEngine->getSocket()->sendData("ZZ#" + kb_input->getValueUTF8() + "#" + std::to_string(currProfileID) + "#%");
			}
		}
		return;
	}

	btn_back->updateInput();
	btn_profilesEvidence->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	btn_report->updateInput();
	btn_kick->updateInput();
	btn_ban->updateInput();
	btn_follow->updateInput();

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
}

void UICourtProfileDetail::hideEverything()
{
	bgHide(bgIndex);

	btn_back->setVisible(false);
	btn_profilesEvidence->setVisible(false);
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	btn_report->setVisible(false);
	btn_kick->setVisible(false);
	btn_ban->setVisible(false);
	btn_follow->setVisible(false);
	lbl_name->setVisible(false);
	lbl_desc->setVisible(false);
	spr_profile->setVisible(false);
}

void UICourtProfileDetail::showEverything()
{
	bgShow(bgIndex);

	btn_back->setVisible(true);
	btn_profilesEvidence->setVisible(true);
	btn_prevPage->setVisible(true);
	btn_nextPage->setVisible(true);
	btn_report->setVisible(true);
	btn_follow->setVisible(true);
	lbl_name->setVisible(true);
	lbl_desc->setVisible(true);
	spr_profile->setVisible(true);

	btn_kick->setVisible(pCourtUI->isMod());
	btn_ban->setVisible(pCourtUI->isMod());
}

void UICourtProfileDetail::update()
{

}

void UICourtProfileDetail::reloadPage()
{
	currProfileID = (currProfile >= pCourtUI->getPlayerListIDs().size()) ? 0 : pCourtUI->getPlayerListIDs()[currProfile];
	bool isSelf = (currProfileID == pCourtUI->getClientID());

	playerInfo* info = 0;
	if (pCourtUI->getPlayerList().count(currProfileID))
		info = &pCourtUI->getPlayerList()[currProfileID];
	u32 currAreaID = (info) ? (u32)info->area : 0;

	btn_report->setVisible(!!info && !isSelf);
	btn_follow->setVisible(!!info && !isSelf && pCourtUI->isMod() && info->area >= 0);
	btn_kick->setVisible(!!info && !isSelf && pCourtUI->isMod());
	btn_ban->setVisible(!!info && !isSelf && pCourtUI->isMod());

	std::string character = (info) ? info->character : "";
	if (info && character.empty())
		character = "Spectator";
	lbl_name->setText(character);
	lbl_name->setPos(163, 65, true);

	std::string desc;
	if (isSelf)
		desc = "You, in ";
	else
		desc = "In ";

	if (pCourtUI->getPlayerListIDs().empty())
		desc = "Player list is empty";
	else if (currAreaID < pCourtUI->getAreaList().size())
		desc += "area \"" + pCourtUI->getAreaList()[currAreaID].name + "\"";
	else
		desc += "area " + std::to_string(currAreaID);

	if (info)
	{
		if (!info->showname.empty())
			desc += ", Showname \"" + info->showname + "\"";
		if (!info->oocName.empty())
			desc += ", OOC \"" + info->oocName + "\"";
	}

	std::u16string finalDesc = utf8::utf8to16(desc);
	lbl_desc->setText(finalDesc);

	std::string file = "characters/" + (info ? info->character : "") + "/char_icon_big";
	bool exists = Content::exists(file+".img.bin", file);
	if (exists) file = file.substr(0, file.length()-8); // remove extension

	spr_profile->setImage(file, 64, 64, 9);
}

void UICourtProfileDetail::onBackClicked(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtProfileDetail::onProfilesEvidenceClicked(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtEvidenceDetail(pSelf->pCourtUI, 0, pSelf->wasInPrivateEvidence));
}

void UICourtProfileDetail::onPrevPage(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndEvPage);
	if (pSelf->currProfile == 0) pSelf->currProfile = pSelf->pCourtUI->getPlayerListIDs().size();
	pSelf->currProfile--;

	pSelf->reloadPage();
}

void UICourtProfileDetail::onNextPage(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndEvPage);
	pSelf->currProfile++;
	if (pSelf->currProfile >= pSelf->pCourtUI->getPlayerListIDs().size()) pSelf->currProfile = 0;

	pSelf->reloadPage();
}

void UICourtProfileDetail::onReport(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->hideEverything();
	pSelf->kb_input->show16("Enter report reason");
}

void UICourtProfileDetail::onKick(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtModeratorDialog(pSelf->pCourtUI, pSelf->currProfile, pSelf->wasInPrivateEvidence, false));
}

void UICourtProfileDetail::onBan(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtModeratorDialog(pSelf->pCourtUI, pSelf->currProfile, pSelf->wasInPrivateEvidence, true));
}

void UICourtProfileDetail::onFollow(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	if (!pSelf->pCourtUI->getPlayerList().count(pSelf->currProfileID))
		return;
	playerInfo* info = &pSelf->pCourtUI->getPlayerList()[pSelf->currProfileID];

	wav_play(pSelf->pCourtUI->sndSelect);
	gEngine->getSocket()->sendData("MC#" + pSelf->pCourtUI->getAreaList()[info->area].name + "#" + std::to_string(pSelf->pCourtUI->getCurrCharID()) + "#" + utf8::utf16to8(pSelf->pCourtUI->showname) + "#%");
}

void UICourtProfileDetail::onMessagePR(void* pUserData, std::string msg)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	u32 currID = pSelf->currProfile;
	if (pSelf->pCourtUI->getPlayerListIDs().empty())
	{
		pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
		return;
	}

	while (currID >= pSelf->pCourtUI->getPlayerListIDs().size())
		currID--;

	if (currID != pSelf->currProfile)
	{
		pSelf->currProfile = currID;
		pSelf->reloadPage();
	}
}

void UICourtProfileDetail::onMessagePU(void* pUserData, std::string msg)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	u32 id = std::stoi(argumentAt(msg, 1));

	if (pSelf->currProfileID == id)
		pSelf->reloadPage();
}
