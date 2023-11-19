#include "ui/court/arealist.h"

#include <math.h>

#include <nds/arm9/background.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/musiclist.h"

struct areaBtnData
{
	UICourtAreaList* pObj;
	int btnInd;
};

UICourtAreaList::~UICourtAreaList()
{
	delete btn_back;
	delete btn_listToggle;
	delete btn_confirm;
	delete btn_prevPage;
	delete btn_nextPage;
	for (int i=0; i<4; i++)
	{
		delete btn_area[i];
		delete lbl_area[i];
	}
	delete lbl_pages;
	delete lbl_areaInfo;
	delete sel_btn;

	gEngine->getSocket()->removeMessageCallback("ARUP", cbARUP);
}

void UICourtAreaList::init()
{
	currPage = 0;
	currAreaSelected = -1;
	holdWait = -1;
	pageAdd = 0;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(bgIndex, 1);
	loadBg("/data/ao-nds/ui/bg_areas", true);

	static areaBtnData btnData[4];
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_listToggle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_music", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 0, 79, 30, 32, 32, 1);
	btn_confirm = new UIButton(&oamSub, "/data/ao-nds/ui/spr_confirm", btn_listToggle->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 192-30, 79, 30, 32, 32, 2);
	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", btn_confirm->nextOamInd(), 1, 1, SpriteSize_32x16, 79+2, 192-15, 19, 14, 32, 16, 4);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prevPage->nextOamInd(), 1, 1, SpriteSize_32x16, 256-79-19-2, 192-15, 19, 14, 32, 16, 5);
	for (int i=0; i<4; i++)
	{
		int nextOam = (i == 0) ? btn_nextPage->nextOamInd() : lbl_area[i-1]->nextOamInd();
		btn_area[i] = new UIButton(&oamSub, "/data/ao-nds/ui/spr_area", nextOam, 3, 1, SpriteSize_64x32, 108, 34+(i*31), 142, 26, 64, 32, 6+i);
		lbl_area[i] = new UILabel(&oamSub, btn_area[i]->nextOamInd(), 8, 1, RGB15(13, 2, 0), 10, 0);
		btn_area[i]->setVisible(false);
		btn_area[i]->setPriority(1);

		btnData[i] = {this, i};
		btn_area[i]->connect(onAreaClicked, &btnData[i]);
	}
	lbl_pages = new UILabel(&oamSub, lbl_area[3]->nextOamInd(), 1, 1, RGB15(13, 2, 0), 10, 0);
	lbl_areaInfo = new UILabel(&oamSub, lbl_pages->nextOamInd(), 3, 9, RGB15(31,31,31), 11, 0);
	sel_btn = new UISelectCross(&oamSub, lbl_areaInfo->nextOamInd(), 12);

	lbl_areaInfo->setPos(5, 38);

	btn_back->assignKey(KEY_B);
	btn_listToggle->assignKey(KEY_R);
	btn_confirm->assignKey(KEY_A);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	btn_back->connect(onBackClicked, this);
	btn_listToggle->connect(onToggleList, this);
	btn_confirm->connect(onConfirmClicked, this);
	btn_prevPage->connect(onPrevPage, this, UIButton::PRESSED);
	btn_nextPage->connect(onNextPage, this, UIButton::PRESSED);
	btn_prevPage->connect(onPageBtnRelease, this, UIButton::RELEASED);
	btn_nextPage->connect(onPageBtnRelease, this, UIButton::RELEASED);

	cbARUP = gEngine->getSocket()->addMessageCallback("ARUP", onMessageARUP, this);

	reloadPage();
}

void UICourtAreaList::updateInput()
{
	btn_back->updateInput();
	btn_listToggle->updateInput();
	btn_confirm->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	for (int i=0; i<4; i++) btn_area[i]->updateInput();
}

void UICourtAreaList::update()
{
	if (pageAdd)
	{
		holdWait--;
		if (holdWait <= 0)
		{
			wav_play(pCourtUI->sndEvPage);
			currPage += pageAdd;
			u32 maxPages = (u32)ceil(pCourtUI->getAreaList().size()/4.f);

			updatePageText();

			if (currPage == 0 || currPage+1 >= maxPages)
			{
				(pageAdd == 1) ? btn_nextPage->forceRelease() : btn_prevPage->forceRelease();
				pageAdd = 0;
			}

			holdWait = 5;
		}
	}
}

void UICourtAreaList::reloadPage()
{
	if (currAreaSelected != -1)
	{
		lbl_areaInfo->setVisible(false);
		currAreaSelected = -1;
	}

	for (u32 i=0; i<4; i++)
	{
		mp3_fill_buffer();

		u32 ind = currPage*4 + i;
		if (ind >= pCourtUI->getAreaList().size())
		{
			btn_area[i]->setVisible(false);
			lbl_area[i]->setVisible(false);
			continue;
		}

		btn_area[i]->setVisible(true);
		lbl_area[i]->setVisible(true);
		lbl_area[i]->setText(pCourtUI->getAreaList()[ind].name.c_str());
		lbl_area[i]->setPos(108+(142/2), 34+(i*31)+6, true);
		mp3_fill_buffer();
	}

	u32 maxPages = (u32)ceil(pCourtUI->getAreaList().size()/4.f);
	btn_prevPage->setVisible(currPage > 0);
	btn_nextPage->setVisible(currPage+1 < maxPages);
	btn_confirm->setVisible(false);
	sel_btn->setVisible(false);

	updatePageText();
}

void UICourtAreaList::updatePageText()
{
	char buf[32];
	u32 maxPages = (u32)ceil(pCourtUI->getAreaList().size()/4.f);

	if (maxPages)
		sprintf(buf, "%lu/%lu", currPage+1, maxPages);
	else
		sprintf(buf, "Empty");

	lbl_pages->setVisible(true);
	lbl_pages->setText(buf);
	lbl_pages->setPos(128, 192-15, true);
	mp3_fill_buffer();
}

void UICourtAreaList::updateAreaInfo()
{
	if (currAreaSelected == -1)
	{
		lbl_areaInfo->setVisible(false);
		return;
	}

	u32 ind = currPage*4 + currAreaSelected;
	const areaInfo& area = pCourtUI->getAreaList()[ind];

	std::string info = area.name + "\n" + area.status + " | " + area.lock + "\n" + std::to_string(area.players) + " player" + (area.players!=1 ? "s" : "") + "\n\nCM: " + area.cm;
	lbl_areaInfo->setVisible(true);
	lbl_areaInfo->setText(info.c_str());
}

void UICourtAreaList::onBackClicked(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtAreaList::onToggleList(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtMusicList(pSelf->pCourtUI));
}

void UICourtAreaList::onConfirmClicked(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	u32 ind = pSelf->currPage*4 + pSelf->currAreaSelected;
	gEngine->getSocket()->sendData("MC#" + pSelf->pCourtUI->getAreaList()[ind].name + "#" + std::to_string(pSelf->pCourtUI->getCurrCharID()) + "#" + pSelf->pCourtUI->showname + "#%");

	pSelf->btn_confirm->setVisible(false);
}

void UICourtAreaList::onPrevPage(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->holdWait = 35;
	pSelf->pageAdd = -1;

	pSelf->currPage--;
	pSelf->updatePageText();
}

void UICourtAreaList::onNextPage(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->holdWait = 35;
	pSelf->pageAdd = 1;

	pSelf->currPage++;
	pSelf->updatePageText();
}

void UICourtAreaList::onPageBtnRelease(void* pUserData)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;

	pSelf->pageAdd = 0;
	pSelf->reloadPage();
}

void UICourtAreaList::onAreaClicked(void* pUserData)
{
	areaBtnData* pData = (areaBtnData*)pUserData;
	UICourtAreaList* pSelf = pData->pObj;

	if (pSelf->currAreaSelected == pData->btnInd) return;
	pSelf->currAreaSelected = pData->btnInd;
	wav_play(pSelf->pCourtUI->sndEvTap);

	pSelf->btn_confirm->setVisible(true);
	pSelf->sel_btn->selectButton(pSelf->btn_area[pData->btnInd]);
	pSelf->updateAreaInfo();
}

void UICourtAreaList::onMessageARUP(void* pUserData, std::string msg)
{
	UICourtAreaList* pSelf = (UICourtAreaList*)pUserData;
	if (pSelf->currAreaSelected == -1) return;

	pSelf->updateAreaInfo();
}
