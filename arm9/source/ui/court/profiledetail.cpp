#include "ui/court/profiledetail.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/courtrecord.h"
#include "ui/court/icchatlog.h"
#include "ui/court/evidencedetail.h"

UICourtProfileDetail::~UICourtProfileDetail()
{
	delete btn_back;
	delete btn_profilesEvidence;
	delete btn_prevPage;
	delete btn_nextPage;
	delete btn_kickBan;
	delete lbl_name;
	delete lbl_desc;
	delete spr_profile;

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
	btn_kickBan = new UIButton(&oamSub, "/data/ao-nds/ui/spr_kickBan", btn_nextPage->nextOamInd(), 2, 1, SpriteSize_32x16, 96, 36, 64, 15, 32, 16, 4);

	lbl_name = new UILabel(&oamSub, btn_kickBan->nextOamInd(), 4, 1, RGB15(31, 16, 0), 5, 0);
	lbl_desc = new UILabel(&oamSub, lbl_name->nextOamInd(), 4, 4, RGB15(4, 4, 4), 6, 0);

	spr_profile = new UIButton(&oamSub, "", lbl_desc->nextOamInd(), 1, 1, SpriteSize_64x64, 23, 66, 60, 60, 64, 64, 7);

	btn_profilesEvidence->setFrame(1);
	lbl_desc->setPos(99, 80);

	btn_back->assignKey(KEY_B);
	btn_profilesEvidence->assignKey(KEY_R);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);
	btn_kickBan->assignKey(KEY_X);

	btn_back->connect(onBackClicked, this);
	btn_profilesEvidence->connect(onProfilesEvidenceClicked, this);
	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	btn_kickBan->connect(onKickBan, this);

	reloadPage();

	cbPR = gEngine->getSocket()->addMessageCallback("PR", onMessagePR, this);
	cbPU = gEngine->getSocket()->addMessageCallback("PU", onMessagePU, this);
}

void UICourtProfileDetail::updateInput()
{
	btn_back->updateInput();
	btn_profilesEvidence->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	btn_kickBan->updateInput();

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
}

void UICourtProfileDetail::update()
{

}

void UICourtProfileDetail::reloadPage()
{
	currProfileID = (currProfile >= pCourtUI->getPlayerListIDs().size()) ? 0 : pCourtUI->getPlayerListIDs()[currProfile];

	playerInfo* info = 0;
	if (pCourtUI->getPlayerList().count(currProfileID))
		info = &pCourtUI->getPlayerList()[currProfileID];
	u32 currAreaID = (info) ? (u32)info->area : 0;

	btn_kickBan->setVisible(!!info && pCourtUI->isMod());

	std::string character = (info) ? info->character : "";
	if (info && character.empty())
		character = "Spectator";
	lbl_name->setText(character);
	lbl_name->setPos(163, 65, true);

	std::string desc;
	if (pCourtUI->getPlayerListIDs().empty())
		desc = "Player list is empty";
	else if (currAreaID < pCourtUI->getAreaList().size())
		desc = "In area \"" + pCourtUI->getAreaList()[currAreaID].name + "\"";
	else
		desc = "In area " + std::to_string(currAreaID);

	if (info)
	{
		if (!info->showname.empty())
			desc += ", Showname \"" + info->showname + "\"";
		if (!info->oocName.empty())
			desc += ", OOC \"" + info->oocName + "\"";
	}

	std::u16string finalDesc = utf8::utf8to16(desc);
	lbl_desc->setText(finalDesc);

	spr_profile->setImage("/data/ao-nds/characters/" + (info ? info->character : "") + "/char_icon_big", 64, 64, 7);
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

void UICourtProfileDetail::onKickBan(void* pUserData)
{
	UICourtProfileDetail* pSelf = (UICourtProfileDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
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
