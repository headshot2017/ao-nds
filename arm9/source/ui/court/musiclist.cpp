#include "ui/court/musiclist.h"

#include <algorithm>

#include <nds/arm9/background.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "mp3_shared.h"
#include "global.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/arealist.h"

struct musicBtnData
{
	UICourtMusicList* pObj;
	int btnInd;
};

UICourtMusicList::~UICourtMusicList()
{
	delete btn_back;
	delete btn_listToggle;
	delete btn_scrollUp;
	delete btn_scrollDown;
	for (int i=0; i<7; i++)
	{
		delete btn_musicBtn[i];
		delete lbl_musicBtn[i];
	}
	delete btn_sliderHandle;
	delete kb_search;
}

void UICourtMusicList::init()
{
	scrollPos = 0;
	scrollPosOld = 0;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_musicList");

	static musicBtnData btnData[7];
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_listToggle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_areas", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 0, 79, 30, 32, 32, 1);
	btn_scrollUp = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollUp", btn_listToggle->nextOamInd(), 1, 1, SpriteSize_16x32, 242, 31, 14, 19, 16, 32, 2);
	btn_scrollDown = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollDown", btn_scrollUp->nextOamInd(), 1, 1, SpriteSize_16x32, 242, 156, 14, 19, 16, 32, 3);
	for (int i=0; i<7; i++)
	{
		lbl_musicBtn[i] = new UILabel(&oamSub, (!i) ? btn_scrollDown->nextOamInd() : btn_musicBtn[i-1]->nextOamInd(), 7, 1, 0, 4, 0);
		lbl_musicBtn[i]->setPos(4, 32+(17*i)+1);
		btn_musicBtn[i] = new UIButton(&oamSub, "/data/ao-nds/ui/spr_musicBtn", lbl_musicBtn[i]->nextOamInd(), 8, 1, SpriteSize_32x16, 2, 32+(17*i), 238, 16, 32, 16, 5+i);

		btnData[i] = {this, i};
		btn_musicBtn[i]->connect(onMusicClicked, &btnData[i]);
	}
	btn_sliderHandle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_sliderHandle", btn_musicBtn[6]->nextOamInd(), 1, 1, SpriteSize_16x32, btn_scrollUp->getX(), btn_scrollUp->getY()+btn_scrollUp->getH(), 14, 19, 16, 32, 12);

	kb_search = new AOkeyboard(1, btn_sliderHandle->nextOamInd(), 13);
	dmaCopy(bgPal, BG_PALETTE_SUB, 512);

	btn_back->assignKey(KEY_B);
	btn_listToggle->assignKey(KEY_R);
	btn_scrollUp->assignKey(KEY_UP);
	btn_scrollDown->assignKey(KEY_DOWN);

	btn_back->connect(onBackClicked, this);
	btn_listToggle->connect(onToggleList, this);
	btn_scrollUp->connect(onScrollUpPressed, this, UIButton::PRESSED);
	btn_scrollDown->connect(onScrollDownPressed, this, UIButton::PRESSED);
	btn_scrollUp->connect(onScrollBtnReleased, this, UIButton::RELEASED);
	btn_scrollDown->connect(onScrollBtnReleased, this, UIButton::RELEASED);
	btn_sliderHandle->connect(onSliderPressed, this, UIButton::PRESSED);
	btn_sliderHandle->connect(onSliderReleased, this, UIButton::RELEASED);

	holdWait = -1;
	pageAdd = 0;
	draggingHandle = false;

	updateFilter();
	reloadScroll(true);
}

void UICourtMusicList::updateInput()
{
	if (kb_search->isVisible())
	{
		int result = kb_search->updateInput();
		if (result != 0)
		{
			dmaCopy(bgPal, BG_PALETTE_SUB, 512);
			bgShow(bgIndex);

			btn_back->setVisible(true);
			btn_listToggle->setVisible(true);
			btn_scrollUp->setVisible(true);
			btn_scrollDown->setVisible(true);
			btn_sliderHandle->setVisible(true);

			filter = kb_search->getValueUTF8();
			if (result > 0) updateFilter();
			reloadScroll(true);
		}
		return;
	}

	btn_back->updateInput();
	btn_listToggle->updateInput();
	btn_scrollUp->updateInput();
	btn_scrollDown->updateInput();
	for (int i=0; i<7; i++) btn_musicBtn[i]->updateInput();
	btn_sliderHandle->updateInput();

	if (keysDown() & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);
		if (pos.px >= 142 && pos.py >= 177 && pos.px < 142+114 && pos.py < 177+15)
		{
			// search button
			wav_play(pCourtUI->sndSelect);

			bgHide(bgIndex);

			btn_back->setVisible(false);
			btn_listToggle->setVisible(false);
			btn_scrollUp->setVisible(false);
			btn_scrollDown->setVisible(false);
			for (int i=0; i<7; i++)
			{
				btn_musicBtn[i]->setVisible(false);
				lbl_musicBtn[i]->setVisible(false);
			}
			btn_sliderHandle->setVisible(false);

			kb_search->show("Enter search terms", filter);
		}
	}
}

void UICourtMusicList::update()
{
	if (pageAdd)
	{
		holdWait--;
		if (holdWait <= 0)
		{
			setScroll(scrollPos+pageAdd);

			if (scrollPos == 0 || scrollPos+7 >= filteredMusic.size())
			{
				(pageAdd == 1) ? btn_scrollUp->forceRelease() : btn_scrollDown->forceRelease();
				pageAdd = 0;
			}

			holdWait = 1;
		}
	}

	if (draggingHandle)
	{
		touchPosition touchPos;
		touchRead(&touchPos);

		int handleEdges[2] = {btn_scrollUp->getY()+btn_scrollUp->getH(), btn_scrollDown->getY()-btn_scrollDown->getH()};
		touchPos.py -= 9;
		int yPos = (touchPos.py < handleEdges[0]) ? handleEdges[0] : (touchPos.py > handleEdges[1]) ? handleEdges[1] : touchPos.py;
		btn_sliderHandle->setPos(btn_sliderHandle->getX(), yPos);
	}
}

void UICourtMusicList::updateFilter()
{
	filteredMusic.clear();
	for (u32 i=0; i<pCourtUI->getMusicList().size(); i++)
	{
		mp3_fill_buffer();
		if (filter.empty())
		{
			filteredMusic.push_back(i);
			continue;
		}

		std::string nameLower(pCourtUI->getMusicList()[i].name);
		std::string filterLower(filter);
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](char c){return std::tolower(c);});
		std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), [](char c){return std::tolower(c);});
		mp3_fill_buffer();

		if (nameLower.find(filterLower) != std::string::npos)
		{
			filteredMusic.push_back(i);
			mp3_fill_buffer();
		}
	}
}

void UICourtMusicList::setScroll(u32 i)
{
	scrollPos = i;
	reloadScroll();
}

void UICourtMusicList::reloadScroll(bool all)
{
	int diff = scrollPos - scrollPosOld;
	if (!all && !diff) return;

	u32 diffAbs = abs(scrollPos - scrollPosOld);
	int diffSign = (diff>0) ? 1 : (diff<0) ? -1 : 0;

	u32 start, end;
	int add;

	if (all || diffAbs >= 7)
	{
		start = 0;
		end = 7;
		add = 1;
	}
	else
	{
		// method for faster scrolling

		// first, move old ones
		start = (diffSign==1) ? 0 : 7-1;
		end = (diffSign==1) ? 7-diffAbs : diffAbs-1;
		add = diffSign;

		for (u32 i=start; i!=end; i+=add)
		{
			mp3_fill_buffer();

			btn_musicBtn[i]->setFrame(btn_musicBtn[i+diffSign]->getFrame());
			dmaCopy(*lbl_musicBtn[i+diffSign]->getGfx(), *lbl_musicBtn[i]->getGfx(), 32*16*7);

			mp3_fill_buffer();
		}

		// then generate new text
		start = end;
		end = (diffSign==1) ? 7 : -1;
	}

	for (u32 i=start; i!=end; i+=add)
	{
		mp3_fill_buffer();

		u32 ind = i+scrollPos;
		if (ind >= filteredMusic.size())
		{
			btn_musicBtn[i]->setVisible(false);
			lbl_musicBtn[i]->setVisible(false);
			continue;
		}

		const musicInfo& mp3Music = pCourtUI->getMusicList()[filteredMusic[ind]];

		btn_musicBtn[i]->setVisible(true);
		btn_musicBtn[i]->setFrame( (gEngine->musicExists(mp3Music.nameLower)) ? 0 : 1 );
		lbl_musicBtn[i]->setVisible(true);
		lbl_musicBtn[i]->setText(mp3Music.nameDecoded);
		mp3_fill_buffer();
	}

	scrollPosOld = scrollPos;

	int handleEdges[2] = {btn_scrollUp->getY()+btn_scrollUp->getH(), btn_scrollDown->getY()-btn_scrollDown->getH()};
	int yPos = handleEdges[0];
	if (filteredMusic.size() > 7)
		yPos = handleEdges[0] + ((handleEdges[1] - handleEdges[0]) * (scrollPos / (float)(filteredMusic.size()-7)));
	btn_sliderHandle->setPos(btn_sliderHandle->getX(), yPos);
}

void UICourtMusicList::onBackClicked(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtMusicList::onToggleList(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtAreaList(pSelf->pCourtUI));
}

void UICourtMusicList::onScrollUpPressed(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	if (!pSelf->scrollPos) return;
	pSelf->setScroll(pSelf->scrollPos-1);

	pSelf->holdWait = 20;
	pSelf->pageAdd = -1;
}

void UICourtMusicList::onScrollDownPressed(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	if (pSelf->scrollPos+7 >= pSelf->filteredMusic.size()) return;
	pSelf->setScroll(pSelf->scrollPos+1);

	pSelf->holdWait = 20;
	pSelf->pageAdd = 1;
}

void UICourtMusicList::onScrollBtnReleased(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	pSelf->pageAdd = 0;
}

void UICourtMusicList::onSliderPressed(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	pSelf->draggingHandle = true;
}

void UICourtMusicList::onSliderReleased(void* pUserData)
{
	UICourtMusicList* pSelf = (UICourtMusicList*)pUserData;

	pSelf->draggingHandle = false;

	int handleEdges[2] = {pSelf->btn_scrollUp->getY()+pSelf->btn_scrollUp->getH(), pSelf->btn_scrollDown->getY()-pSelf->btn_scrollDown->getH()};
	int yPos = 0;
	if (pSelf->filteredMusic.size() > 7)
	{
		yPos = (pSelf->btn_sliderHandle->getY() - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * (pSelf->filteredMusic.size()-7);
	}
	pSelf->setScroll(yPos);
}

void UICourtMusicList::onMusicClicked(void* pUserData)
{
	musicBtnData* pData = (musicBtnData*)pUserData;
	UICourtMusicList* pSelf = pData->pObj;
	int ind = pSelf->scrollPos + pData->btnInd;

	gEngine->getSocket()->sendData("MC#" + pSelf->pCourtUI->getMusicList()[pSelf->filteredMusic[ind]].name + "#" + std::to_string(pSelf->pCourtUI->getCurrCharID()) + "#" + utf8::utf16to8(pSelf->pCourtUI->showname) + "#%");
}
