#include "ui/court/oocpresets.h"

#include <math.h>
#include <string.h>

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "global.h"
#include "ui/court/ooc.h"

struct presetBtnData
{
	UICourtOOCPresets* pObj;
	int btnInd;
};

UICourtOOCPresets::~UICourtOOCPresets()
{
	delete btn_back;
	delete btn_addOrConfirm;
	delete btn_delete;
	delete btn_edit;
	delete btn_prevPage;
	delete btn_nextPage;
	for (int i=0; i<4; i++)
	{
		delete btn_preset[i];
		delete lbl_preset[i];
	}
	delete lbl_pages;
	delete kb_input;
}

void UICourtOOCPresets::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(bgIndex, 1);
	loadBg("/data/ao-nds/ui/bg_oocPresets");

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_addOrConfirm = new UIButton(&oamSub, "/data/ao-nds/ui/spr_addConfirm", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 192-30, 79, 30, 32, 32, 1);
	btn_delete = new UIButton(&oamSub, "/data/ao-nds/ui/spr_delete", btn_addOrConfirm->nextOamInd(), 3, 1, SpriteSize_32x64, 0, 0, 80, 33, 32, 64, 2);
	btn_edit = new UIButton(&oamSub, "/data/ao-nds/ui/spr_edit", btn_delete->nextOamInd(), 3, 1, SpriteSize_32x64, 256-80, 0, 80, 33, 32, 64, 3);
	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", btn_edit->nextOamInd(), 1, 1, SpriteSize_32x16, 79+2, 192-15, 19, 14, 32, 16, 4);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prevPage->nextOamInd(), 1, 1, SpriteSize_32x16, 256-79-19-2, 192-15, 19, 14, 32, 16, 5);
	for (int i=0; i<4; i++)
	{
		int nextOam = (i == 0) ? btn_nextPage->nextOamInd() : lbl_preset[i-1]->nextOamInd();
		btn_preset[i] = new UIButton(&oamSub, "/data/ao-nds/ui/spr_serverBtn", nextOam, 7, 1, SpriteSize_32x32, 128-112, 36+(i*32), 224, 26, 32, 32, 7+i);
		lbl_preset[i] = new UILabel(&oamSub, btn_preset[i]->nextOamInd(), 8, 1, RGB15(13, 2, 0), 6, 0);
		btn_preset[i]->setPriority(1);
		btn_preset[i]->setVisible(false);
	}
	lbl_pages = new UILabel(&oamSub, lbl_preset[3]->nextOamInd(), 1, 1, RGB15(13, 2, 0), 6, 0);

	kb_input = new AOkeyboard(1, lbl_pages->nextOamInd(), 11);
	memcpy(BG_PALETTE_SUB, bgPal, 512);

	btn_back->assignKey(KEY_B);
	btn_addOrConfirm->assignKey(KEY_A);
	btn_delete->assignKey(KEY_L);
	btn_edit->assignKey(KEY_R);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	static presetBtnData btnData[4];
	btn_back->connect(onBackClicked, this);
	btn_addOrConfirm->connect(onAddOrConfirm, this);
	btn_delete->connect(onDeleteClicked, this);
	btn_edit->connect(onEditClicked, this);
	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	for (int i=0; i<4; i++)
	{
		btnData[i].btnInd = i;
		btnData[i].pObj = this;
		btn_preset[i]->connect(onPresetClicked, &btnData[i]);
	}

	currPreset = -1;
	currPage = 0;
	parsePresetsList();
}

void UICourtOOCPresets::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			memcpy(BG_PALETTE_SUB, bgPal, 512);
			bgShow(bgIndex);

			btn_back->setVisible(true);
			btn_addOrConfirm->setVisible(true);
			lbl_pages->setVisible(true);

			if (result > 0)
			{
				if (currPreset == -1)
					m_presets.push_back(kb_input->getValue());
				else
					m_presets[currPage*4 + currPreset] = kb_input->getValue();

				savePresets();
			}

			reloadPage();
		}
		return;
	}

	btn_back->updateInput();
	btn_addOrConfirm->updateInput();
	btn_delete->updateInput();
	btn_edit->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	for (int i=0; i<4; i++) btn_preset[i]->updateInput();
}

void UICourtOOCPresets::update()
{

}

void UICourtOOCPresets::deselect()
{
	btn_addOrConfirm->setFrame(0);
	btn_delete->setVisible(false);
	btn_edit->setVisible(false);

	if (currPreset == -1) return;

	btn_preset[currPreset]->setFrame(0);
	currPreset = -1;
}

void UICourtOOCPresets::reloadPage()
{
	deselect();

	for (u32 i=0; i<4; i++)
	{
		u32 ind = currPage*4 + i;
		if (ind >= m_presets.size())
		{
			btn_preset[i]->setVisible(false);
			lbl_preset[i]->setVisible(false);
			continue;
		}

		btn_preset[i]->setVisible(true);
		lbl_preset[i]->setVisible(true);
		lbl_preset[i]->setText(m_presets[ind].c_str());
		lbl_preset[i]->setPos(128, 42+(i*32), true);
	}

	u32 maxPages = (u32)ceil(m_presets.size()/4.f);
	btn_prevPage->setVisible(currPage > 0);
	btn_nextPage->setVisible(!m_presets.empty() && currPage < maxPages-1);

	if (!m_presets.empty())
	{
		char buf[8];
		sprintf(buf, "%lu/%lu", currPage+1, maxPages);
		lbl_pages->setText(buf);
	}
	else
		lbl_pages->setText("Empty");
	lbl_pages->setPos(128, 192-15, true);
}

void UICourtOOCPresets::hideEverything()
{
	bgHide(bgIndex);

	btn_back->setVisible(false);
	btn_addOrConfirm->setVisible(false);
	btn_delete->setVisible(false);
	btn_edit->setVisible(false);
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	for (int i=0; i<4; i++)
	{
		btn_preset[i]->setVisible(false);
		lbl_preset[i]->setVisible(false);
	}
	lbl_pages->setVisible(false);
}

void UICourtOOCPresets::onBackClicked(void* pUserData)
{
	UICourtOOCPresets* pSelf = (UICourtOOCPresets*)pUserData;
	wav_play(pSelf->pCourtUI->sndCancel);

	if (pSelf->currPreset == -1)
		pSelf->pCourtUI->changeScreen(new UICourtOOC(pSelf->pCourtUI));
	else
		pSelf->deselect();
}

void UICourtOOCPresets::onAddOrConfirm(void* pUserData)
{
	UICourtOOCPresets* pSelf = (UICourtOOCPresets*)pUserData;

	if (pSelf->currPreset == -1)
	{
		wav_play(pSelf->pCourtUI->sndCrtRcrd);
		pSelf->hideEverything();
		pSelf->kb_input->show("Enter an OOC preset", "");
	}
	else
	{
		wav_play(pSelf->pCourtUI->sndSelect);
		std::string preset = pSelf->m_presets[pSelf->currPage*4 + pSelf->currPreset];
		gEngine->getSocket()->sendData("CT#" + pSelf->pCourtUI->oocName + "#" + preset + "#%");
		pSelf->pCourtUI->changeScreen(new UICourtOOC(pSelf->pCourtUI));
	}
}

void UICourtOOCPresets::onDeleteClicked(void* pUserData)
{
	UICourtOOCPresets* pSelf = (UICourtOOCPresets*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	if (pSelf->currPreset == -1) return;
	pSelf->m_presets.erase(pSelf->m_presets.begin() + (pSelf->currPage*4 + pSelf->currPreset));
	pSelf->savePresets();
	pSelf->reloadPage();

}

void UICourtOOCPresets::onEditClicked(void* pUserData)
{
	UICourtOOCPresets* pSelf = (UICourtOOCPresets*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	if (pSelf->currPreset == -1) return;
	pSelf->hideEverything();
	pSelf->kb_input->show("Enter an OOC preset", pSelf->m_presets[pSelf->currPage*4 + pSelf->currPreset].c_str());
}

void UICourtOOCPresets::onPrevPage(void* pUserData)
{
	UICourtOOCPresets* pSelf = (UICourtOOCPresets*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UICourtOOCPresets::onNextPage(void* pUserData)
{
	UICourtOOCPresets* pSelf = (UICourtOOCPresets*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UICourtOOCPresets::onPresetClicked(void* pUserData)
{
	presetBtnData* pData = (presetBtnData*)pUserData;
	UICourtOOCPresets* pSelf = pData->pObj;

	if (pSelf->currPreset == pData->btnInd) // already selected
		return;

	wav_play(pSelf->pCourtUI->sndSelect);

	// unselect preset
	if (pSelf->currPreset != -1)
		pSelf->btn_preset[pSelf->currPreset]->setFrame(0);

	// select preset
	pSelf->currPreset = pData->btnInd;
	pSelf->btn_preset[pSelf->currPreset]->setFrame(1);

	pSelf->btn_addOrConfirm->setFrame(1);
	pSelf->btn_delete->setVisible(true);
	pSelf->btn_edit->setVisible(true);
}

void UICourtOOCPresets::parsePresetsList()
{
	char* data = (char*)readFile("/data/ao-nds/ooc_presets.txt", 0, "r");
	if (data)
	{
		m_presets.clear();
		std::string dataStr = data;
		fillArguments(m_presets, dataStr, 0, '\n');
		m_presets.pop_back();
		delete[] data;
	}

	currPage = 0;
	reloadPage();
}

void UICourtOOCPresets::savePresets()
{
	FILE* f = fopen("/data/ao-nds/ooc_presets.txt", "w");

	for (u32 i=0; i<m_presets.size(); i++)
	{
		fprintf(f, "%s\n", m_presets[i].c_str());
		mp3_fill_buffer();
	}

	fclose(f);
}
