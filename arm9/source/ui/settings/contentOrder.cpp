#include "ui/settings/contentOrder.h"

#include <algorithm>
#include <math.h>

#include "content.h"
#include "settings.h"
#include "mp3_shared.h"

struct contentOrderBtnData
{
	UISettingsContentOrder* pObj;
	int btnInd;
};

UISettingsContentOrder::~UISettingsContentOrder()
{
	delete lbl_info;
	for (int i=0; i<5; i++)
	{
		delete btn_contentBtn[i];
		delete lbl_contentBtn[i];
	}
	delete btn_prevPage;
	delete btn_nextPage;
	delete btn_moveUp;
	delete btn_moveDown;
}

void UISettingsContentOrder::init()
{
	currPage = 0;
	currBtn = -1;

	lbl_info = new UILabel(&oamSub, pSettingsUI->getFirstOAM(), 7, 2, RGB15(31,31,31), 3, 0);
	lbl_info->setLineOffset(10);

	static contentOrderBtnData btnData[5];

	for (int i=0; i<5; i++)
	{
		lbl_contentBtn[i] = new UILabel(&oamSub, (!i) ? lbl_info->nextOamInd() : btn_contentBtn[i-1]->nextOamInd(), 6, 1, 0, 4, 0);
		btn_contentBtn[i] = new UIButton(&oamSub, "/data/ao-nds/ui/spr_musicBtnMedium", lbl_contentBtn[i]->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 60+(18*i), 192, 16, 32, 16, 5+i);

		btnData[i] = {this, i};
		btn_contentBtn[i]->connect(onContentClicked, &btnData[i]);
	}

	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", btn_contentBtn[4]->nextOamInd(), 1, 1, SpriteSize_32x16, 128-(19/2)-16, 150, 19, 14, 32, 16, 10);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prevPage->nextOamInd(), 1, 1, SpriteSize_32x16, 128-(19/2)+16, 150, 19, 14, 32, 16, 11);
	btn_moveUp = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollUp", btn_nextPage->nextOamInd(), 1, 1, SpriteSize_16x32, 256-19-8, btn_contentBtn[0]->getY(), 14, 19, 16, 32, 12);
	btn_moveDown = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollDown", btn_moveUp->nextOamInd(), 1, 1, SpriteSize_16x32, 256-19-8, btn_contentBtn[4]->getY()-3, 14, 19, 16, 32, 12);

	lbl_info->setText("Change the order in which the game looks for\ncustom content, from top to bottom");
	lbl_info->setPos(128, 36, true);

	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);
	btn_moveUp->assignKey(KEY_UP);
	btn_moveDown->assignKey(KEY_DOWN);

	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	btn_moveUp->connect(onMoveUp, this);
	btn_moveDown->connect(onMoveDown, this);

	reloadPage();
}

void UISettingsContentOrder::updateInput()
{
	for (int i=0; i<5; i++)
		btn_contentBtn[i]->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	btn_moveUp->updateInput();
	btn_moveDown->updateInput();
}

void UISettingsContentOrder::update()
{

}

void UISettingsContentOrder::reloadPage()
{
	currBtn = -1;
	auto& contents = Content::getContents();

	for (u32 i=0; i<5; i++)
	{
		mp3_fill_buffer();

		u32 ind = currPage*5 + i;
		if (ind >= contents.size())
		{
			btn_contentBtn[i]->setVisible(false);
			lbl_contentBtn[i]->setVisible(false);
			continue;
		}

		btn_contentBtn[i]->setFrame(1);
		btn_contentBtn[i]->setVisible(true);
		lbl_contentBtn[i]->setVisible(true);
		lbl_contentBtn[i]->setText(contents[ind]);
		lbl_contentBtn[i]->setPos(btn_contentBtn[i]->getX() + (btn_contentBtn[i]->getW()/2), btn_contentBtn[i]->getY()+1, true);
		mp3_fill_buffer();
	}

	u32 maxPages = (u32)ceil(contents.size()/5.f);
	btn_prevPage->setVisible(currPage > 0);
	btn_nextPage->setVisible(currPage+1 < maxPages);
	btn_moveUp->setVisible(false);
	btn_moveDown->setVisible(false);
}

void UISettingsContentOrder::onContentClicked(void* pUserData)
{
	contentOrderBtnData* pData = (contentOrderBtnData*)pUserData;
	UISettingsContentOrder* pSelf = pData->pObj;

	if (pSelf->currBtn >= 0) pSelf->btn_contentBtn[pSelf->currBtn]->setFrame(1);
	pSelf->currBtn = pData->btnInd;
	pSelf->btn_contentBtn[pSelf->currBtn]->setFrame(0);

	auto& contents = Content::getContents();
	u32 ind = pSelf->currPage*5 + pSelf->currBtn;

	pSelf->btn_moveUp->setVisible(ind > 0);
	pSelf->btn_moveDown->setVisible(ind < contents.size()-1);
}

void UISettingsContentOrder::onPrevPage(void* pUserData)
{
	UISettingsContentOrder* pSelf = (UISettingsContentOrder*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UISettingsContentOrder::onNextPage(void* pUserData)
{
	UISettingsContentOrder* pSelf = (UISettingsContentOrder*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UISettingsContentOrder::onMoveUp(void* pUserData)
{
	UISettingsContentOrder* pSelf = (UISettingsContentOrder*)pUserData;

	auto& contents = Content::getContents();
	u32 ind = pSelf->currPage*5 + pSelf->currBtn;
	if (ind <= 0) return;

	iter_swap(contents.begin() + ind, contents.begin() + ind-1);
	Settings::save();

	pSelf->reloadPage();
}

void UISettingsContentOrder::onMoveDown(void* pUserData)
{
	UISettingsContentOrder* pSelf = (UISettingsContentOrder*)pUserData;

	auto& contents = Content::getContents();
	u32 ind = pSelf->currPage*5 + pSelf->currBtn;
	if (ind >= contents.size()-1) return;

	iter_swap(contents.begin() + ind, contents.begin() + ind+1);
	Settings::save();

	pSelf->reloadPage();
}
