#include "ui/court/courtrecord.h"

#include <stdio.h>
#include <math.h>
#include <algorithm>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/icchatlog.h"
#include "ui/court/evidencedetail.h"

struct evidenceBtnData
{
	UICourtEvidence* pObj;
	int btnInd;
};

UICourtEvidence::~UICourtEvidence()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), (isProfiles) ? bg_profilesTilesLen : bg_evidenceTilesLen);

	delete[] bg_evidenceTiles;
	delete[] bg_evidenceMap;
	delete[] bg_evidencePal;
	delete[] bg_profilesTiles;
	delete[] bg_profilesMap;
	delete[] bg_profilesPal;

	delete btn_pageLeft;
	delete btn_pageRight;
	delete btn_back;
	delete btn_present;
	delete btn_privatePublic;
	delete btn_profilesEvidence;
	delete lbl_name;
	delete sel_btn;
	for (int i=0; i<8; i++) delete btn_evidence[i];

	gEngine->getSocket()->removeMessageCallback("LE", cbLE);
	gEngine->getSocket()->removeMessageCallback("PR", cbPR);
}

void UICourtEvidence::init()
{
	currPage = 0;
	isPrivate = false;
	isProfiles = false;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	bg_evidenceTiles = readFile("/data/ao-nds/ui/bg_evidence.img.bin", &bg_evidenceTilesLen);
	bg_evidenceMap = readFile("/data/ao-nds/ui/bg_evidence.map.bin");
	bg_evidencePal = readFile("/data/ao-nds/ui/bg_evidence.pal.bin");
	bg_profilesTiles = readFile("/data/ao-nds/ui/bg_profiles.img.bin", &bg_profilesTilesLen);
	bg_profilesMap = readFile("/data/ao-nds/ui/bg_profiles.map.bin");
	bg_profilesPal = readFile("/data/ao-nds/ui/bg_profiles.pal.bin");

	dmaCopy(bg_evidenceTiles, bgGetGfxPtr(bgIndex), bg_evidenceTilesLen);
	dmaCopy(bg_evidenceMap, bgGetMapPtr(bgIndex), 1536);
	dmaCopy(bg_evidencePal, BG_PALETTE_SUB, 512);

	btn_pageLeft = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_tall", 0, 1, 3, SpriteSize_16x32, 4, 55, 16, 95, 16, 32, 0);
	btn_pageRight = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_tall", btn_pageLeft->nextOamInd(), 1, 3, SpriteSize_16x32, 236, 55, 16, 95, 16, 32, 1);
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", btn_pageRight->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 2);
	btn_present = new UIButton(&oamSub, "/data/ao-nds/ui/spr_present", btn_back->nextOamInd(), 3, 1, SpriteSize_32x64, 86, 0, 85, 33, 32, 64, 3);
	btn_privatePublic = new UIButton(&oamSub, "/data/ao-nds/ui/spr_privatePublic2", btn_present->nextOamInd(), 2, 1, SpriteSize_32x16, 128-(64/2), btn_back->getY()-2, 64, 15, 32, 16, 4);
	btn_profilesEvidence = new UIButton(&oamSub, "/data/ao-nds/ui/spr_profilesEvidence", btn_privatePublic->nextOamInd(), 1, 1, SpriteSize_64x32, 256-55, 0, 55, 31, 64, 32, 5);

	lbl_name = new UILabel(&oamSub, btn_profilesEvidence->nextOamInd(), 6, 1, RGB15(31, 16, 0), 6, 0);

	sel_btn = new UISelectCross(&oamSub, lbl_name->nextOamInd(), 7);

	static evidenceBtnData btnData[8];
	for (int y=0; y<2; y++)
	{
		for (int x=0; x<4; x++)
		{
			int i = y*4+x;
			btnData[i].btnInd = i;
			btnData[i].pObj = this;

			btn_evidence[i] = new UIButton(&oamSub, "", (i>0) ? btn_evidence[i-1]->nextOamInd() : sel_btn->nextOamInd(), 1, 1, SpriteSize_64x64, 37+(x*48), 63+(y*48), 38, 38, 64, 64, 8+i);
			btn_evidence[i]->connect(onEvidenceClicked, &btnData[i]);
		}
	}

	if (pCourtUI->icControls.evidence > -1)
		btn_present->setFrame(1);

	btn_pageLeft->assignKey(KEY_LEFT);
	btn_pageRight->assignKey(KEY_RIGHT);
	btn_back->assignKey(KEY_B);
	btn_present->assignKey(KEY_X);
	btn_profilesEvidence->assignKey(KEY_R);

	btn_pageLeft->connect(onPrevPage, this);
	btn_pageRight->connect(onNextPage, this);
	btn_back->connect(onBackClicked, this);
	btn_present->connect(onPresentClicked, this);
	btn_privatePublic->connect(onPrivatePublicClicked, this);
	btn_profilesEvidence->connect(onProfilesEvidenceClicked, this);

	reloadPage();

	cbLE = gEngine->getSocket()->addMessageCallback("LE", onMessageLE, this);
	cbPR = gEngine->getSocket()->addMessageCallback("PR", onMessagePR, this);
}

void UICourtEvidence::updateInput()
{
	btn_pageLeft->updateInput();
	btn_pageRight->updateInput();
	btn_back->updateInput();
	btn_present->updateInput();
	btn_privatePublic->updateInput();
	btn_profilesEvidence->updateInput();
	for (int i=0; i<8; i++) btn_evidence[i]->updateInput();

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
}

void UICourtEvidence::update()
{

}

void UICourtEvidence::reloadPage()
{
	if (currEviSelected != -1)
	{
		// hide selection reticle
		lbl_name->setVisible(false);
		currEviSelected = -1;
	}

	u32 maxPages = 0;
	if (!isProfiles)
	{
		for (u32 i=0; i<8; i++)
		{
			mp3_fill_buffer();

			u32 ind = currPage*8 + i;
			if (ind >= pCourtUI->getEvidenceList(isPrivate).size()+1)
			{
				btn_evidence[i]->setVisible(false);
				continue;
			}

			bool addButton = (ind == pCourtUI->getEvidenceList(isPrivate).size());
			std::string file;
			bool exists;
			if (addButton)
			{
				file = "/data/ao-nds/ui/spr_addEvidence";
				exists = true;
			}
			else
			{
				const evidenceInfo& info = pCourtUI->getEvidenceList(isPrivate)[ind];
				file = "/data/ao-nds/evidence/small/" + info.image;
				exists = fileExists(file+".img.bin");
			}
			mp3_fill_buffer();

			btn_evidence[i]->setImage((exists ? file : "/data/ao-nds/ui/spr_unknownMugshot"), 64, 64, 8+i);
			btn_evidence[i]->setVisible(true);

			if (!isPrivate && pCourtUI->icControls.evidence == (int)i)
				btn_evidence[i]->darken();
		}

		maxPages = (u32)ceil((pCourtUI->getEvidenceList(isPrivate).size()+1)/8.f);
	}
	else
	{
		for (u32 i=0; i<8; i++)
		{
			mp3_fill_buffer();

			u32 ind = currPage*8 + i;
			if (ind >= pCourtUI->getPlayerListIDs().size())
			{
				btn_evidence[i]->setVisible(false);
				continue;
			}

			playerInfo& info = pCourtUI->getPlayerList()[pCourtUI->getPlayerListIDs()[ind]];
			std::string file = "/data/ao-nds/characters/" + info.character + "/char_icon";
			bool exists = fileExists(file+".img.bin");
			mp3_fill_buffer();

			btn_evidence[i]->setImage((exists ? file : "/data/ao-nds/ui/spr_unknownMugshot"), 64, 64, 8+i);
			btn_evidence[i]->setVisible(true);
		}

		maxPages = (u32)ceil(pCourtUI->getPlayerListIDs().size()/8.f);
	}

	btn_pageLeft->setVisible(currPage > 0);
	btn_pageRight->setVisible(currPage+1 < maxPages);
	btn_present->setVisible(false);
	sel_btn->setVisible(false);
}

void UICourtEvidence::onPrevPage(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UICourtEvidence::onNextPage(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UICourtEvidence::onBackClicked(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtEvidence::onPresentClicked(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	if (pSelf->pCourtUI->icControls.evidence > -1)
	{
		pSelf->pCourtUI->icControls.evidence = -1;
		pSelf->btn_present->setFrame(0);
	}
	else
	{
		u32 ind = pSelf->currPage*8 + pSelf->currEviSelected;
		pSelf->pCourtUI->icControls.evidence = ind;
		pSelf->btn_present->setFrame(1);
	}
}

void UICourtEvidence::onPrivatePublicClicked(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->isPrivate = !pSelf->isPrivate;

	pSelf->btn_privatePublic->setFrame(pSelf->isPrivate ? 1 : 0);
	pSelf->currPage = 0;
	pSelf->reloadPage();
}

void UICourtEvidence::onProfilesEvidenceClicked(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->isProfiles = !pSelf->isProfiles;

	if (!pSelf->isProfiles)
	{
		dmaCopy(pSelf->bg_evidenceTiles, bgGetGfxPtr(pSelf->bgIndex), pSelf->bg_evidenceTilesLen);
		dmaCopy(pSelf->bg_evidenceMap, bgGetMapPtr(pSelf->bgIndex), 1536);
		dmaCopy(pSelf->bg_evidencePal, BG_PALETTE_SUB, 512);
	}
	else
	{
		dmaCopy(pSelf->bg_profilesTiles, bgGetGfxPtr(pSelf->bgIndex), pSelf->bg_profilesTilesLen);
		dmaCopy(pSelf->bg_profilesMap, bgGetMapPtr(pSelf->bgIndex), 1536);
		dmaCopy(pSelf->bg_profilesPal, BG_PALETTE_SUB, 512);
	}

	pSelf->btn_privatePublic->setVisible(!pSelf->isProfiles);
	pSelf->btn_profilesEvidence->setFrame(pSelf->isProfiles ? 1 : 0);
	pSelf->currPage = 0;
	pSelf->reloadPage();
}

void UICourtEvidence::onEvidenceClicked(void* pUserData)
{
	evidenceBtnData* pData = (evidenceBtnData*)pUserData;
	UICourtEvidence* pSelf = pData->pObj;

	u32 ind = pSelf->currPage*8 + pData->btnInd;
	if (pSelf->currEviSelected == pData->btnInd)
	{
		wav_play(pSelf->pCourtUI->sndEvShow);
		if (!pSelf->isProfiles)
			pSelf->pCourtUI->changeScreen(new UICourtEvidenceDetail(pSelf->pCourtUI, ind, pSelf->isPrivate));
		//else
			//pSelf->pCourtUI->changeScreen(new UICourtPlayersDetail(pSelf->pCourtUI, 0));
		return;
	}

	pSelf->currEviSelected = pData->btnInd;
	wav_play(pSelf->pCourtUI->sndEvTap);

	pSelf->lbl_name->setVisible(true);
	pSelf->sel_btn->selectButton(pSelf->btn_evidence[pData->btnInd], 2);

	if (!pSelf->isProfiles)
	{
		bool addButton = (ind == pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate).size());
		pSelf->lbl_name->setText((addButton) ? u"Add evidence..." : pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate)[ind].name);
		pSelf->btn_present->setVisible(!addButton && !pSelf->isPrivate);
	}
	else
		pSelf->lbl_name->setText(pSelf->pCourtUI->getPlayerList()[pSelf->pCourtUI->getPlayerListIDs()[ind]].character);
	pSelf->lbl_name->setPos(128, 36+2, true);
}

void UICourtEvidence::onMessageLE(void* pUserData, std::string msg)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;
	if (pSelf->isProfiles) return;

	u32 maxPages = (u32)ceil((pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate).size()+1)/8.f);
	if (pSelf->currPage+1 > maxPages)
		pSelf->currPage--;

	pSelf->reloadPage();
}

void UICourtEvidence::onMessagePR(void* pUserData, std::string msg)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;
	if (!pSelf->isProfiles) return;

	u32 maxPages = (u32)ceil((pSelf->pCourtUI->getPlayerList().size()+1)/8.f);
	if (pSelf->currPage+1 > maxPages)
		pSelf->currPage--;

	pSelf->reloadPage();
}
