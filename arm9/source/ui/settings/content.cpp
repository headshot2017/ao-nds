#include "ui/settings/content.h"

#include <algorithm>
#include <string.h>
#include <dirent.h>
#include <math.h>

#include "content.h"
#include "settings.h"
#include "mp3_shared.h"

struct contentBtnData
{
	UISettingsContent* pObj;
	int btnInd;
};

UISettingsContent::~UISettingsContent()
{
	for (int i=0; i<6; i++)
	{
		delete btn_contentBtn[i];
		delete lbl_contentBtn[i];
	}
	delete btn_prevPage;
	delete btn_nextPage;
	delete btn_apply;
}

void UISettingsContent::init()
{
	currPage = 0;

	static contentBtnData btnData[6];

	for (int i=0; i<6; i++)
	{
		lbl_contentBtn[i] = new UILabel(&oamSub, (!i) ? pSettingsUI->getFirstOAM() : btn_contentBtn[i-1]->nextOamInd(), 7, 1, 0, 4, 0);
		btn_contentBtn[i] = new UIButton(&oamSub, "/data/ao-nds/ui/spr_musicBtn", lbl_contentBtn[i]->nextOamInd(), 8, 1, SpriteSize_32x16, 9, 39+(18*i), 238, 16, 32, 16, 5+i);

		btnData[i] = {this, i};
		btn_contentBtn[i]->connect(onContentClicked, &btnData[i]);
	}

	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", btn_contentBtn[5]->nextOamInd(), 1, 1, SpriteSize_32x16, 79, 148, 19, 14, 32, 16, 11);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prevPage->nextOamInd(), 1, 1, SpriteSize_32x16, 159, 148, 19, 14, 32, 16, 12);
	btn_apply = new UIButton(&oamSub, "/data/ao-nds/ui/spr_applySmall", btn_nextPage->nextOamInd(), 2, 1, SpriteSize_32x16, 106, 148, 46, 14, 32, 16, 13);

	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	btn_apply->connect(onApplyClicked, this);

	m_contents = Content::getContents();

	DIR *dir = opendir("/data/ao-nds/custom");
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (dent->d_type != DT_DIR || !strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;

		m_allContents.push_back(dent->d_name);
	}

	closedir(dir);

	reloadPage();
}

void UISettingsContent::updateInput()
{
	for (int i=0; i<6; i++)
		btn_contentBtn[i]->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	btn_apply->updateInput();
}

void UISettingsContent::update()
{

}

void UISettingsContent::setVisible(bool on)
{
	for (int i=0; i<6; i++)
	{
		lbl_contentBtn[i]->setVisible(on);
		btn_contentBtn[i]->setVisible(on);
	}
	btn_prevPage->setVisible(on);
	btn_nextPage->setVisible(on);
	btn_apply->setVisible(on);
}

void UISettingsContent::reloadPage()
{
	for (u32 i=0; i<6; i++)
	{
		mp3_fill_buffer();

		u32 ind = currPage*6 + i;
		if (ind >= m_allContents.size())
		{
			btn_contentBtn[i]->setVisible(false);
			lbl_contentBtn[i]->setVisible(false);
			continue;
		}

		bool isMounted = (std::find(m_contents.begin(), m_contents.end(), m_allContents[ind]) != m_contents.end());
		btn_contentBtn[i]->setFrame(!isMounted);
		btn_contentBtn[i]->setVisible(true);
		lbl_contentBtn[i]->setVisible(true);
		lbl_contentBtn[i]->setText(m_allContents[ind]);
		lbl_contentBtn[i]->setPos(btn_contentBtn[i]->getX() + (btn_contentBtn[i]->getW()/2), btn_contentBtn[i]->getY()+1, true);
		mp3_fill_buffer();
	}

	u32 maxPages = (u32)ceil(m_allContents.size()/6.f);
	btn_prevPage->setVisible(currPage > 0);
	btn_nextPage->setVisible(currPage+1 < maxPages);
}

void UISettingsContent::onContentClicked(void* pUserData)
{
	contentBtnData* pData = (contentBtnData*)pUserData;
	UISettingsContent* pSelf = pData->pObj;

	u32 ind = pSelf->currPage*6 + pData->btnInd;
	auto pos = std::find(pSelf->m_contents.begin(), pSelf->m_contents.end(), pSelf->m_allContents[ind]);
	bool isMounted = (pos != pSelf->m_contents.end());
	pSelf->btn_contentBtn[pData->btnInd]->setFrame(isMounted);

	if (isMounted)
		pSelf->m_contents.erase(pos);
	else
		pSelf->m_contents.push_back(pSelf->m_allContents[ind]);
}

void UISettingsContent::onPrevPage(void* pUserData)
{
	UISettingsContent* pSelf = (UISettingsContent*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UISettingsContent::onNextPage(void* pUserData)
{
	UISettingsContent* pSelf = (UISettingsContent*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UISettingsContent::onApplyClicked(void* pUserData)
{
	UISettingsContent* pSelf = (UISettingsContent*)pUserData;

	Content::clear();
	for (auto& c : pSelf->m_contents)
		Content::add(c);

	pSelf->pSettingsUI->hideEverything(true);
	Content::reload(pSelf->btn_apply->nextOamInd());
	Settings::save();
	pSelf->pSettingsUI->showEverything();
	pSelf->reloadPage();
}
