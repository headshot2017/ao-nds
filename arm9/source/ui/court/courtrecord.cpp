#include "ui/court/courtrecord.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
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
	delete btn_pageLeft;
	delete btn_pageRight;
	delete btn_back;
	delete btn_present;
	delete btn_privatePublic;
	delete lbl_evidence;
	delete sel_btn;
	for (int i=0; i<8; i++) delete btn_evidence[i];

	gEngine->getSocket()->removeMessageCallback("LE", cbLE);
}

void UICourtEvidence::init()
{
	currPage = 0;
	isPrivate = false;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_evidence", true);

	btn_pageLeft = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_tall", 0, 1, 3, SpriteSize_16x32, 4, 55, 16, 95, 16, 32, 0);
	btn_pageRight = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_tall", btn_pageLeft->nextOamInd(), 1, 3, SpriteSize_16x32, 236, 55, 16, 95, 16, 32, 1);
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", btn_pageRight->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 2);
	btn_present = new UIButton(&oamSub, "/data/ao-nds/ui/spr_present", btn_back->nextOamInd(), 3, 1, SpriteSize_32x64, 86, 0, 85, 33, 32, 64, 3);
	btn_privatePublic = new UIButton(&oamSub, "/data/ao-nds/ui/spr_privatePublic", btn_present->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 0, 79, 30, 32, 32, 4);

	lbl_evidence = new UILabel(&oamSub, btn_privatePublic->nextOamInd(), 6, 1, RGB15(31, 16, 0), 5, 0);

	sel_btn = new UISelectCross(&oamSub, lbl_evidence->nextOamInd(), 6);

	static evidenceBtnData btnData[8];
	for (int y=0; y<2; y++)
	{
		for (int x=0; x<4; x++)
		{
			int i = y*4+x;
			btnData[i].btnInd = i;
			btnData[i].pObj = this;

			btn_evidence[i] = new UIButton(&oamSub, "", (i>0) ? btn_evidence[i-1]->nextOamInd() : sel_btn->nextOamInd(), 1, 1, SpriteSize_64x64, 37+(x*48), 63+(y*48), 38, 38, 64, 64, 7+i);
			btn_evidence[i]->connect(onEvidenceClicked, &btnData[i]);
		}
	}

	if (pCourtUI->icControls.evidence > -1)
		btn_present->setFrame(1);

	btn_pageLeft->assignKey(KEY_LEFT);
	btn_pageRight->assignKey(KEY_RIGHT);
	btn_back->assignKey(KEY_B);
	btn_present->assignKey(KEY_X);
	btn_privatePublic->assignKey(KEY_R);

	btn_pageLeft->connect(onPrevPage, this);
	btn_pageRight->connect(onNextPage, this);
	btn_back->connect(onBackClicked, this);
	btn_present->connect(onPresentClicked, this);
	btn_privatePublic->connect(onPrivatePublicClicked, this);

	reloadPage();

	cbLE = gEngine->getSocket()->addMessageCallback("LE", onMessageLE, this);
}

void UICourtEvidence::updateInput()
{
	btn_pageLeft->updateInput();
	btn_pageRight->updateInput();
	btn_back->updateInput();
	btn_present->updateInput();
	btn_privatePublic->updateInput();
	for (int i=0; i<8; i++) btn_evidence[i]->updateInput();

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		soundPlaySample(pCourtUI->sndSelect, SoundFormat_16Bit, pCourtUI->sndSelectSize, 16000, 127, 64, false, 0);
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
		lbl_evidence->setVisible(false);
		currEviSelected = -1;
	}

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

		btn_evidence[i]->setImage((exists ? file : "/data/ao-nds/ui/spr_unknownMugshot"), 64, 64, 7+i);
		btn_evidence[i]->setVisible(true);

		if (pCourtUI->icControls.evidence == (int)i)
			btn_evidence[i]->darken();
	}

	u32 maxPages = (u32)ceil((pCourtUI->getEvidenceList(isPrivate).size()+1)/8.f);
	btn_pageLeft->setVisible(currPage > 0);
	btn_pageRight->setVisible(currPage+1 < maxPages);
	btn_present->setVisible(false);
	sel_btn->setVisible(false);
}

void UICourtEvidence::onPrevPage(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 16000, 127, 64, false, 0);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UICourtEvidence::onNextPage(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 16000, 127, 64, false, 0);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UICourtEvidence::onBackClicked(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndCancel, SoundFormat_16Bit, pSelf->pCourtUI->sndCancelSize, 16000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtEvidence::onPresentClicked(void* pUserData)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 16000, 127, 64, false, 0);
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

	soundPlaySample(pSelf->pCourtUI->sndCrtRcrd, SoundFormat_16Bit, pSelf->pCourtUI->sndCrtRcrdSize, 16000, 127, 64, false, 0);
	pSelf->isPrivate = !pSelf->isPrivate;

	pSelf->btn_present->setVisible(!pSelf->isPrivate);
	pSelf->btn_privatePublic->setFrame(pSelf->isPrivate ? 1 : 0);
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
		soundPlaySample(pSelf->pCourtUI->sndEvShow, SoundFormat_16Bit, pSelf->pCourtUI->sndEvShowSize, 16000, 127, 64, false, 0);
		pSelf->pCourtUI->changeScreen(new UICourtEvidenceDetail(pSelf->pCourtUI, ind, pSelf->isPrivate));
		return;
	}

	pSelf->currEviSelected = pData->btnInd;
	soundPlaySample(pSelf->pCourtUI->sndEvTap, SoundFormat_16Bit, pSelf->pCourtUI->sndEvTapSize, 16000, 127, 64, false, 0);

	bool addButton = (ind == pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate).size());

	pSelf->lbl_evidence->setVisible(true);
	pSelf->lbl_evidence->setText((addButton) ? "Add evidence..." : pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate)[ind].name.c_str());
	pSelf->lbl_evidence->setPos(128, 36+2, true);

	pSelf->sel_btn->selectButton(pSelf->btn_evidence[pData->btnInd], 2);

	pSelf->btn_present->setVisible(!addButton && !pSelf->isPrivate);
}

void UICourtEvidence::onMessageLE(void* pUserData, std::string msg)
{
	UICourtEvidence* pSelf = (UICourtEvidence*)pUserData;

	u32 maxPages = (u32)ceil((pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate).size()+1)/8.f);
	if (pSelf->currPage+1 > maxPages)
		pSelf->currPage--;

	pSelf->reloadPage();
}
