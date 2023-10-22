#include "ui/court/pairselect.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <algorithm>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/pair.h"

struct charPairBtnData
{
	UICourtPairSelect* pObj;
	int btnInd;
};

UICourtPairSelect::~UICourtPairSelect()
{
	delete btn_pageLeft;
	delete btn_pageRight;
	delete btn_back;
	delete btn_pair;
	delete lbl_charname;
	delete lbl_pages;
	delete sel_btn;
	for (int i=0; i<8; i++)
		delete btn_chars[i];

	delete kb_search;
}

void UICourtPairSelect::init()
{
	currPage = 0;
	currCharSelected = -1;
	holdWait = -1;
	pageAdd = 0;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_charSelect");

	btn_pageLeft = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_tall", 0, 1, 3, SpriteSize_16x32, 4, 55, 16, 95, 16, 32, 0);
	btn_pageRight = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_tall", btn_pageLeft->nextOamInd(), 1, 3, SpriteSize_16x32, 236, 55, 16, 95, 16, 32, 1);
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", btn_pageRight->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 2);
	btn_pair = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pairConfirm", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 192-30, 79, 30, 32, 32, 3);

	lbl_charname = new UILabel(&oamSub, btn_pair->nextOamInd(), 6, 1, RGB15(31, 16, 0), 4, 0);
	lbl_pages = new UILabel(&oamSub, lbl_charname->nextOamInd(), 1, 1, RGB15(13, 2, 0), 5, 0);

	sel_btn = new UISelectCross(&oamSub, lbl_pages->nextOamInd(), 6);

	static charPairBtnData btnData[8];
	for (int y=0; y<2; y++)
	{
		for (int x=0; x<4; x++)
		{
			int i = y*4+x;
			btnData[i].btnInd = i;
			btnData[i].pObj = this;

			btn_chars[i] = new UIButton(&oamSub, "", (i>0) ? btn_chars[i-1]->nextOamInd() : sel_btn->nextOamInd(), 1, 1, SpriteSize_64x64, 37+(x*48), 63+(y*48), 38, 38, 64, 64, 7+i);
			btn_chars[i]->connect(onCharClicked, &btnData[i]);
		}
	}

	kb_search = new AOkeyboard(1, btn_chars[7]->nextOamInd(), 15);
	memcpy(BG_PALETTE_SUB, bgPal, 512);
	mp3_fill_buffer();

	btn_back->assignKey(KEY_B);
	btn_pair->assignKey(KEY_A);
	btn_pageLeft->assignKey(KEY_LEFT);
	btn_pageRight->assignKey(KEY_RIGHT);

	btn_pageLeft->setVisible(false);
	btn_pageLeft->connect(onPrevPage, this, UIButton::PRESSED);
	btn_pageRight->connect(onNextPage, this, UIButton::PRESSED);
	btn_pageLeft->connect(onPageBtnRelease, this, UIButton::RELEASED);
	btn_pageRight->connect(onPageBtnRelease, this, UIButton::RELEASED);
	btn_back->connect(onBackClicked, this);
	btn_pair->connect(onPairClicked, this);

	updateFilter();
	reloadPage();
}

void UICourtPairSelect::update()
{
	if (pageAdd)
	{
		holdWait--;
		if (holdWait <= 0)
		{
			soundPlaySample(pCourtUI->sndEvPage, SoundFormat_16Bit, pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);
			currPage += pageAdd;
			u32 maxPages = (u32)ceil(filteredChars.size()/8.f);

			updatePageText();

			if (currPage == 0 || currPage+1 >= maxPages)
			{
				(pageAdd == 1) ? btn_pageRight->forceRelease() : btn_pageLeft->forceRelease();
				pageAdd = 0;
			}

			holdWait = 5;
		}
	}
}

void UICourtPairSelect::updateInput()
{
	if (kb_search->isVisible())
	{
		int result = kb_search->updateInput();
		if (result != 0)
		{
			memcpy(BG_PALETTE_SUB, bgPal, 512);
			bgShow(bgIndex);

			btn_back->setVisible(true);

			filter = kb_search->getValue();
			if (result > 0) updateFilter();
			reloadPage();
		}
		return;
	}

	btn_pageLeft->updateInput();
	btn_pageRight->updateInput();
	btn_back->updateInput();
	btn_pair->updateInput();
	for (int i=0; i<8; i++) btn_chars[i]->updateInput();

	if (keysDown() & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);
		if (pos.px >= 213 && pos.py >= 36 && pos.px < 213+17 && pos.py < 36+16)
		{
			// search button
			soundPlaySample(pCourtUI->sndSelect, SoundFormat_16Bit, pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

			bgHide(bgIndex);

			btn_pageLeft->setVisible(false);
			btn_pageRight->setVisible(false);
			btn_back->setVisible(false);
			btn_pair->setVisible(false);
			lbl_charname->setVisible(false);
			lbl_pages->setVisible(false);
			sel_btn->setVisible(false);
			for (int i=0; i<8; i++)
				btn_chars[i]->setVisible(false);

			kb_search->show("Enter search terms", filter.c_str());
		}
	}
}

void UICourtPairSelect::reloadPage()
{
	if (currCharSelected != -1)
	{
		// hide char selection reticle
		lbl_charname->setVisible(false);
		currCharSelected = -1;
	}

	for (u32 i=0; i<8; i++)
	{
		mp3_fill_buffer();

		u32 ind = currPage*8 + i;
		if (ind >= filteredChars.size())
		{
			btn_chars[i]->setVisible(false);
			continue;
		}
		ind = filteredChars[ind];

		bool exists = fileExists("/data/ao-nds/characters/" + pCourtUI->getCharList()[ind].name + "/char_icon.img.bin");
		mp3_fill_buffer();

		btn_chars[i]->setImage((exists ? ("/data/ao-nds/characters/" + pCourtUI->getCharList()[ind].name + "/char_icon") : "/data/ao-nds/ui/spr_unknownMugshot"), 64, 64, 7+i);
		btn_chars[i]->setVisible(true);

		if (pCourtUI->icControls.pairID == (int)i)
			btn_chars[i]->darken();
	}

	u32 maxPages = (u32)ceil(filteredChars.size()/8.f);
	btn_pageLeft->setVisible(currPage > 0);
	btn_pageRight->setVisible(currPage+1 < maxPages);
	btn_pair->setVisible(false);
	sel_btn->setVisible(false);

	updatePageText();
}

void UICourtPairSelect::updatePageText()
{
	char buf[32];
	u32 maxPages = (u32)ceil(filteredChars.size()/8.f);

	if (maxPages)
		sprintf(buf, "%lu/%lu", currPage+1, maxPages);
	else
		sprintf(buf, "Empty");
	lbl_pages->setVisible(true);
	lbl_pages->setText(buf);
	lbl_pages->setPos(128, 192-15, true);
	mp3_fill_buffer();
}

void UICourtPairSelect::updateFilter()
{
	filteredChars.clear();
	currPage = 0;
	for (u32 i=0; i<pCourtUI->getCharList().size(); i++)
	{
		mp3_fill_buffer();

		if (filter.empty())
		{
			filteredChars.push_back(i);
			mp3_fill_buffer();
			continue;
		}

		std::string nameLower(pCourtUI->getCharList()[i].name);
		std::string filterLower(filter);
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](char c){return std::tolower(c);});
		std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), [](char c){return std::tolower(c);});
		mp3_fill_buffer();

		if (nameLower.find(filterLower) != std::string::npos)
		{
			filteredChars.push_back(i);
			mp3_fill_buffer();
		}
	}
}

void UICourtPairSelect::onPrevPage(void* pUserData)
{
	UICourtPairSelect* pSelf = (UICourtPairSelect*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	pSelf->holdWait = 35;
	pSelf->pageAdd = -1;

	pSelf->currPage--;
	pSelf->updatePageText();
}

void UICourtPairSelect::onNextPage(void* pUserData)
{
	UICourtPairSelect* pSelf = (UICourtPairSelect*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	pSelf->holdWait = 35;
	pSelf->pageAdd = 1;

	pSelf->currPage++;
	pSelf->updatePageText();
}

void UICourtPairSelect::onPageBtnRelease(void* pUserData)
{
	UICourtPairSelect* pSelf = (UICourtPairSelect*)pUserData;

	pSelf->pageAdd = 0;
	pSelf->reloadPage();
}

void UICourtPairSelect::onBackClicked(void* pUserData)
{
	UICourtPairSelect* pSelf = (UICourtPairSelect*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndCancel, SoundFormat_16Bit, pSelf->pCourtUI->sndCancelSize, 32000, 127, 64, false, 0);
	pSelf->pCourtUI->changeScreen(new UICourtPair(pSelf->pCourtUI));
}

void UICourtPairSelect::onPairClicked(void* pUserData)
{
	UICourtPairSelect* pSelf = (UICourtPairSelect*)pUserData;

	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	u32 ind = pSelf->currPage*8 + pSelf->currCharSelected;
	pSelf->pCourtUI->icControls.pairID = ind;

	pSelf->pCourtUI->changeScreen(new UICourtPair(pSelf->pCourtUI));
}

void UICourtPairSelect::onCharClicked(void* pUserData)
{
	charPairBtnData* pData = (charPairBtnData*)pUserData;
	UICourtPairSelect* pSelf = pData->pObj;

	if (pSelf->currCharSelected == pData->btnInd) return;
	pSelf->currCharSelected = pData->btnInd;
	soundPlaySample(pSelf->pCourtUI->sndEvTap, SoundFormat_16Bit, pSelf->pCourtUI->sndEvTapSize, 32000, 127, 64, false, 0);

	u32 ind = pSelf->currPage*8 + pSelf->currCharSelected;
	const charInfo& info = pSelf->pCourtUI->getCharList()[pSelf->filteredChars[ind]];

	pSelf->lbl_charname->setVisible(true);
	pSelf->lbl_charname->setText(info.name.c_str());
	pSelf->lbl_charname->setPos(128, 36+2, true);

	pSelf->sel_btn->selectButton(pSelf->btn_chars[pData->btnInd], 2);

	pSelf->btn_pair->setVisible(true);
}