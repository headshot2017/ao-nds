#include "ui/court/mute.h"

#include <stdio.h>
#include <math.h>
#include <algorithm>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "content.h"
#include "ui/court/ic.h"

struct charMuteBtnData
{
	UICourtMute* pObj;
	int btnInd;
};

UICourtMute::~UICourtMute()
{
	delete btn_back;
	delete btn_muteToggle;
	delete btn_pageLeft;
	delete btn_pageRight;
	delete lbl_charname;
	delete lbl_pages;
	delete sel_btn;
	for (int i=0; i<8; i++)
		delete btn_chars[i];

	delete kb_search;
}

void UICourtMute::init()
{
	currPage = 0;
	currCharSelected = -1;
	holdWait = -1;
	pageAdd = 0;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_charSelect");

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_muteToggle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_muteToggle", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-82, 192-32, 82, 32, 32, 32, 1);
	btn_pageLeft = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_tall", btn_muteToggle->nextOamInd(), 1, 3, SpriteSize_16x32, 4, 55, 16, 95, 16, 32, 2);
	btn_pageRight = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_tall", btn_pageLeft->nextOamInd(), 1, 3, SpriteSize_16x32, 236, 55, 16, 95, 16, 32, 3);

	lbl_charname = new UILabel(&oamSub, btn_pageRight->nextOamInd(), 6, 1, RGB15(31, 16, 0), 4, 0);
	lbl_pages = new UILabel(&oamSub, lbl_charname->nextOamInd(), 1, 1, RGB15(13, 2, 0), 5, 0);

	sel_btn = new UISelectCross(&oamSub, lbl_pages->nextOamInd(), 6);

	static charMuteBtnData btnData[8];
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
	mp3_fill_buffer();

	btn_back->assignKey(KEY_B);
	btn_muteToggle->assignKey(KEY_A);
	btn_pageLeft->assignKey(KEY_LEFT);
	btn_pageRight->assignKey(KEY_RIGHT);

	btn_back->connect(onBackClicked, this);
	btn_muteToggle->connect(onMuteToggled, this);
	btn_pageLeft->setVisible(false);
	btn_pageLeft->connect(onPrevPage, this, UIButton::PRESSED);
	btn_pageRight->connect(onNextPage, this, UIButton::PRESSED);
	btn_pageLeft->connect(onPageBtnRelease, this, UIButton::RELEASED);
	btn_pageRight->connect(onPageBtnRelease, this, UIButton::RELEASED);

	updateFilter();
	reloadPage();
}

void UICourtMute::update()
{
	if (pageAdd)
	{
		holdWait--;
		if (holdWait <= 0)
		{
			wav_play(pCourtUI->sndEvPage);
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

void UICourtMute::updateInput()
{
	if (kb_search->isVisible())
	{
		int result = kb_search->updateInput();
		if (result != 0)
		{
			dmaCopy(bgPal, BG_PALETTE_SUB, 512);
			bgShow(bgIndex);

			btn_back->setVisible(true);

			filter = kb_search->getValueUTF8();
			if (result > 0) updateFilter();
			reloadPage();
		}
		return;
	}

	btn_back->updateInput();
	btn_muteToggle->updateInput();
	btn_pageLeft->updateInput();
	btn_pageRight->updateInput();
	for (int i=0; i<8; i++) btn_chars[i]->updateInput();

	if (keysDown() & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);
		if (pos.px >= 213 && pos.py >= 36 && pos.px < 213+17 && pos.py < 36+16)
		{
			// search button
			wav_play(pCourtUI->sndSelect);

			bgHide(bgIndex);

			btn_back->setVisible(false);
			btn_muteToggle->setVisible(false);
			btn_pageLeft->setVisible(false);
			btn_pageRight->setVisible(false);
			lbl_charname->setVisible(false);
			lbl_pages->setVisible(false);
			sel_btn->setVisible(false);
			for (int i=0; i<8; i++)
				btn_chars[i]->setVisible(false);

			kb_search->show("Enter search terms", filter);
		}
	}
}

void UICourtMute::reloadPage()
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

		if (pCourtUI->getCharList()[ind].muted)
			btn_chars[i]->darken();
	}

	u32 maxPages = (u32)ceil(filteredChars.size()/8.f);
	btn_pageLeft->setVisible(currPage > 0);
	btn_pageRight->setVisible(currPage+1 < maxPages);
	btn_muteToggle->setVisible(false);
	sel_btn->setVisible(false);

	updatePageText();
}

void UICourtMute::updatePageText()
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

void UICourtMute::updateFilter()
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

void UICourtMute::onPrevPage(void* pUserData)
{
	UICourtMute* pSelf = (UICourtMute*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->holdWait = 35;
	pSelf->pageAdd = -1;

	pSelf->currPage--;
	pSelf->updatePageText();
}

void UICourtMute::onNextPage(void* pUserData)
{
	UICourtMute* pSelf = (UICourtMute*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->holdWait = 35;
	pSelf->pageAdd = 1;

	pSelf->currPage++;
	pSelf->updatePageText();
}

void UICourtMute::onPageBtnRelease(void* pUserData)
{
	UICourtMute* pSelf = (UICourtMute*)pUserData;

	pSelf->pageAdd = 0;
	pSelf->reloadPage();
}

void UICourtMute::onBackClicked(void* pUserData)
{
	UICourtMute* pSelf = (UICourtMute*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtIC(pSelf->pCourtUI));
}

void UICourtMute::onMuteToggled(void* pUserData)
{
	UICourtMute* pSelf = (UICourtMute*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);

	u32 ind = pSelf->currPage*8 + pSelf->currCharSelected;
	charInfo& character = pSelf->pCourtUI->getCharList()[pSelf->filteredChars[ind]];
	character.muted = !character.muted;

	if (character.muted)
	{
		pSelf->btn_muteToggle->setFrame(1);
		pSelf->btn_chars[pSelf->currCharSelected]->darken();
	}
	else
	{
		pSelf->btn_muteToggle->setFrame(0);
		pSelf->btn_chars[pSelf->currCharSelected]->restorePalette();
	}
}

void UICourtMute::onCharClicked(void* pUserData)
{
	charMuteBtnData* pData = (charMuteBtnData*)pUserData;
	UICourtMute* pSelf = pData->pObj;

	if (pSelf->currCharSelected == pData->btnInd)
	{
		pSelf->onMuteToggled(pSelf);
		return;
	}

	pSelf->currCharSelected = pData->btnInd;
	wav_play(pSelf->pCourtUI->sndEvTap);

	u32 ind = pSelf->currPage*8 + pSelf->currCharSelected;
	const charInfo& info = pSelf->pCourtUI->getCharList()[pSelf->filteredChars[ind]];

	pSelf->lbl_charname->setVisible(true);
	pSelf->lbl_charname->setText(info.name);
	pSelf->lbl_charname->setPos(128, 36+2, true);

	pSelf->sel_btn->selectButton(pSelf->btn_chars[pData->btnInd], 2);

	pSelf->btn_muteToggle->setVisible((int)ind != pSelf->pCourtUI->getCurrCharID());
	pSelf->btn_muteToggle->setFrame((info.muted) ? 1 : 0);
}
