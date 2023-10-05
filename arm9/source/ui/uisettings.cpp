#include "ui/uisettings.h"

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "cfgFile.h"
#include "ui/uimainmenu.h"
#include "bg_top.h"
#include "bg_bottomAlt.h"
#include "spr_settingsInput.h"
#include "spr_back.h"

UIScreenSettings::~UIScreenSettings()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_topTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_topMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bg_bottomAltTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), bg_bottomAltMapLen);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete lbl_showname;
	delete lbl_shownameValue;
	delete lbl_oocname;
	delete lbl_oocnameValue;
	delete btn_showname;
	delete btn_oocname;
	delete btn_back;
	delete kb_input;
	delete[] sndSelect;
	delete[] sndCancel;
}

void UIScreenSettings::init()
{
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(subBgIndex, 1);

	dmaCopy(bg_topTiles, bgGetGfxPtr(bgIndex), bg_topTilesLen);
	dmaCopy(bg_topMap, bgGetMapPtr(bgIndex), bg_topMapLen);
	dmaCopy(bg_topPal, BG_PALETTE, bg_topPalLen);

	dmaCopy(bg_bottomAltTiles, bgGetGfxPtr(subBgIndex), bg_bottomAltTilesLen);
	dmaCopy(bg_bottomAltMap, bgGetMapPtr(subBgIndex), bg_bottomAltMapLen);

	lbl_showname = new UILabel(&oamSub, 0, 4, 1, RGB15(31,31,31), 0, 0);
	lbl_shownameValue = new UILabel(&oamSub, lbl_showname->nextOamInd(), 4, 1, 0, 1, 0);
	lbl_oocname = new UILabel(&oamSub, lbl_shownameValue->nextOamInd(), 4, 1, RGB15(31,31,31), 0, 0);
	lbl_oocnameValue = new UILabel(&oamSub, lbl_oocname->nextOamInd(), 4, 1, 0, 1, 0);
	btn_showname = new UIButton(&oamSub, (u8*)spr_settingsInputTiles, (u8*)spr_settingsInputPal, lbl_oocnameValue->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 48, 192, 16, 32, 16, 2);
	btn_oocname = new UIButton(&oamSub, (u8*)spr_settingsInputTiles, (u8*)spr_settingsInputPal, btn_showname->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 104, 192, 16, 32, 16, 2);
	btn_back = new UIButton(&oamSub, (u8*)spr_backTiles, (u8*)spr_backPal, btn_oocname->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 3);

	kb_input = new AOkeyboard(2, btn_back->nextOamInd(), 4);
	dmaCopy(bg_bottomAltPal, BG_PALETTE_SUB, bg_bottomAltPalLen);

	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav", &sndSelectSize);
	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav", &sndCancelSize);

	lbl_showname->setPos(btn_showname->getX(), btn_showname->getY()-12);
	lbl_shownameValue->setPos(btn_showname->getX()+3, btn_showname->getY()+1);
	lbl_oocname->setPos(btn_oocname->getX(), btn_oocname->getY()-12);
	lbl_oocnameValue->setPos(btn_oocname->getX()+3, btn_oocname->getY()+1);

	lbl_showname->setText("Default showname");
	lbl_shownameValue->setText(gEngine->getShowname().c_str());
	lbl_oocname->setText("Default OOC name");
	lbl_oocnameValue->setText(gEngine->getOOCname().c_str());

	btn_showname->connect(onShownameClicked, this);
	btn_oocname->connect(onOOCnameClicked, this);
	btn_back->connect(onBackClicked, this);
	btn_back->assignKey(KEY_B);
}

void UIScreenSettings::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			dmaCopy(bg_bottomAltPal, BG_PALETTE_SUB, bg_bottomAltPalLen);
			bgShow(subBgIndex);

			lbl_showname->setVisible(true);
			lbl_shownameValue->setVisible(true);
			lbl_oocname->setVisible(true);
			lbl_oocnameValue->setVisible(true);
			btn_showname->setVisible(true);
			btn_oocname->setVisible(true);
			btn_back->setVisible(true);

			if (result > 0)
			{
				if (currEditing == btn_showname)
				{
					gEngine->setShowname(kb_input->getValue());
					lbl_shownameValue->setText(kb_input->getValue().c_str());
				}
				else if (currEditing == btn_oocname)
				{
					gEngine->setOOCname(kb_input->getValue());
					lbl_oocnameValue->setText(kb_input->getValue().c_str());
				}

				saveSettings();
			}
		}
		return;
	}

	btn_showname->updateInput();
	btn_oocname->updateInput();
	btn_back->updateInput();
}

void UIScreenSettings::hideEverything()
{
	bgHide(subBgIndex);

	lbl_showname->setVisible(false);
	lbl_shownameValue->setVisible(false);
	lbl_oocname->setVisible(false);
	lbl_oocnameValue->setVisible(false);
	btn_showname->setVisible(false);
	btn_oocname->setVisible(false);
	btn_back->setVisible(false);
}

void UIScreenSettings::saveSettings()
{
	cfgFile f;
	f.set("showname", gEngine->getShowname());
	f.set("oocname", gEngine->getOOCname());
	f.save("/data/ao-nds/settings_nds.cfg");
}

void UIScreenSettings::onShownameClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->hideEverything();
	pSelf->currEditing = pSelf->btn_showname;
	pSelf->kb_input->show("Enter default showname", gEngine->getShowname().c_str());
}

void UIScreenSettings::onOOCnameClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->hideEverything();
	pSelf->currEditing = pSelf->btn_oocname;
	pSelf->kb_input->show("Enter default OOC name", gEngine->getOOCname().c_str());
}

void UIScreenSettings::onBackClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	soundPlaySample(pSelf->sndCancel, SoundFormat_16Bit, pSelf->sndCancelSize, 32000, 127, 64, false, 0);

	gEngine->changeScreen(new UIScreenMainMenu);
}
