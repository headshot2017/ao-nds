#include "ui/court/evidenceimage.h"

#include <stdio.h>
#include <math.h>
#include <algorithm>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/evidencedetail.h"

struct evidenceImgBtnData
{
	UICourtEvidenceImage* pObj;
	int btnInd;
};

UICourtEvidenceImage::~UICourtEvidenceImage()
{
	delete btn_pageLeft;
	delete btn_pageRight;
	delete btn_back;
	delete btn_confirm;
	delete lbl_evidence;
	delete sel_btn;
	for (int i=0; i<8; i++) delete btn_evidence[i];
}

void UICourtEvidenceImage::init()
{
	currPage = 0;
	adding = (editingEvidence == pCourtUI->getEvidenceList(isPrivate).size());

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_evidenceImage", true);

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_confirm = new UIButton(&oamSub, "/data/ao-nds/ui/spr_confirm", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-82, 192-32, 82, 32, 32, 32, 1);
	btn_pageLeft = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_tall", btn_confirm->nextOamInd(), 1, 3, SpriteSize_16x32, 4, 55, 16, 95, 16, 32, 2);
	btn_pageRight = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_tall", btn_pageLeft->nextOamInd(), 1, 3, SpriteSize_16x32, 236, 55, 16, 95, 16, 32, 3);

	lbl_evidence = new UILabel(&oamSub, btn_pageRight->nextOamInd(), 6, 1, RGB15(31, 16, 0), 5, 0);

	sel_btn = new UISelectCross(&oamSub, lbl_evidence->nextOamInd(), 6);

	static evidenceImgBtnData btnData[8];
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

	btn_back->assignKey(KEY_B);
	btn_confirm->assignKey(KEY_A);
	btn_pageLeft->assignKey(KEY_LEFT);
	btn_pageRight->assignKey(KEY_RIGHT);

	btn_pageLeft->connect(onPrevPage, this);
	btn_pageRight->connect(onNextPage, this);
	btn_back->connect(onBackClicked, this);
	btn_confirm->connect(onConfirmClicked, this);

	reloadPage();
}

void UICourtEvidenceImage::updateInput()
{
	btn_pageLeft->updateInput();
	btn_pageRight->updateInput();
	btn_back->updateInput();
	btn_confirm->updateInput();
	for (int i=0; i<8; i++) btn_evidence[i]->updateInput();
}

void UICourtEvidenceImage::update()
{

}

void UICourtEvidenceImage::reloadPage()
{
	if (currEviSelected != -1)
	{
		lbl_evidence->setVisible(false);
		currEviSelected = -1;
	}

	for (u32 i=0; i<8; i++)
	{
		mp3_fill_buffer();

		u32 ind = currPage*8 + i;
		if (ind >= gEngine->getEvidence().size())
		{
			btn_evidence[i]->setVisible(false);
			continue;
		}
		mp3_fill_buffer();

		const evidenceCacheInfo& info = gEngine->getEvidence()[ind];
		btn_evidence[i]->setImage("/data/ao-nds/evidence/small/" + info.subdir + "/" + info.name, 64, 64, 7+i);
		btn_evidence[i]->setVisible(true);
	}

	u32 maxPages = (u32)ceil(gEngine->getEvidence().size()/8.f);
	btn_pageLeft->setVisible(currPage > 0);
	btn_pageRight->setVisible(currPage+1 < maxPages);
	btn_confirm->setVisible(false);
	sel_btn->setVisible(false);
}

void UICourtEvidenceImage::onPrevPage(void* pUserData)
{
	UICourtEvidenceImage* pSelf = (UICourtEvidenceImage*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UICourtEvidenceImage::onNextPage(void* pUserData)
{
	UICourtEvidenceImage* pSelf = (UICourtEvidenceImage*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UICourtEvidenceImage::onBackClicked(void* pUserData)
{
	UICourtEvidenceImage* pSelf = (UICourtEvidenceImage*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtEvidenceDetail(pSelf->pCourtUI, pSelf->editingEvidence, pSelf->isPrivate, pSelf->currName, pSelf->currDesc, pSelf->lastImage));
}

void UICourtEvidenceImage::onConfirmClicked(void* pUserData)
{
	UICourtEvidenceImage* pSelf = (UICourtEvidenceImage*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);

	u32 ind = pSelf->currPage*8 + pSelf->currEviSelected;
	const evidenceCacheInfo& info = gEngine->getEvidence()[ind];

	if (!pSelf->adding)
	{
		if (!pSelf->isPrivate)
			gEngine->getSocket()->sendData("EE#" + std::to_string(pSelf->editingEvidence) + "#" + utf8::utf16to8(pSelf->currName) + "#" + utf8::utf16to8(pSelf->currDesc) + "#" + info.subdir + "/" + info.name + ".png#%");
		else
		{
			gEngine->getPrivateEvidence()[pSelf->editingEvidence].name = pSelf->currName;
			gEngine->getPrivateEvidence()[pSelf->editingEvidence].description = pSelf->currDesc;
			gEngine->getPrivateEvidence()[pSelf->editingEvidence].image = info.subdir + "/" + info.name;
			gEngine->savePrivateEvidence();
		}
	}
	pSelf->pCourtUI->changeScreen(new UICourtEvidenceDetail(pSelf->pCourtUI, pSelf->editingEvidence, pSelf->isPrivate, pSelf->currName, pSelf->currDesc, info.subdir + "/" + info.name));
}

void UICourtEvidenceImage::onEvidenceClicked(void* pUserData)
{
	evidenceImgBtnData* pData = (evidenceImgBtnData*)pUserData;
	UICourtEvidenceImage* pSelf = pData->pObj;

	u32 ind = pSelf->currPage*8 + pData->btnInd;
	if (pSelf->currEviSelected == pData->btnInd) return;

	pSelf->currEviSelected = pData->btnInd;
	wav_play(pSelf->pCourtUI->sndEvTap);

	pSelf->lbl_evidence->setVisible(true);
	pSelf->lbl_evidence->setText(gEngine->getEvidence()[ind].name);
	pSelf->lbl_evidence->setPos(128, 36+2, true);

	pSelf->sel_btn->selectButton(pSelf->btn_evidence[pData->btnInd], 2);

	pSelf->btn_confirm->setVisible(true);
}
