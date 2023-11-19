#include "ui/uisettings.h"

#include <nds/dma.h>
#include <nds/arm9/background.h>

#include "engine.h"
#include "cfgFile.h"
#include "ui/uimainmenu.h"

UIScreenSettings::~UIScreenSettings()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete[] bgSubPal;

	delete btn_generalTab;
	delete btn_chatlogTab;
	delete btn_back;

	delete lbl_showname;
	delete lbl_shownameValue;
	delete lbl_oocname;
	delete lbl_oocnameValue;
	delete btn_showname;
	delete btn_oocname;

	delete lbl_iniswaps;
	delete lbl_shownames;
	delete btn_iniswaps;
	delete btn_shownames;
	delete lbl_logPreview;
	delete lbl_logInfo;

	delete kb_input;
	wav_free_handle(sndCancel);
	wav_free_handle(sndCrtRcrd);
}

void UIScreenSettings::init()
{
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(subBgIndex, 1);

	u8* bgTiles = readFile("/data/ao-nds/ui/bg_top.img.bin", &bgTilesLen);
	u8* bgMap = readFile("/data/ao-nds/ui/bg_top.map.bin");
	u8* bgPal = readFile("/data/ao-nds/ui/bg_top.pal.bin");
	u8* bgSubTiles = readFile("/data/ao-nds/ui/bg_bottomAlt.img.bin", &bgSubTilesLen);
	u8* bgSubMap = readFile("/data/ao-nds/ui/bg_bottomAlt.map.bin");
	bgSubPal = readFile("/data/ao-nds/ui/bg_bottomAlt.pal.bin");

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);
	dmaCopy(bgPal, BG_PALETTE, 512);

	dmaCopy(bgSubTiles, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaCopy(bgSubMap, bgGetMapPtr(subBgIndex), 1536);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;
	delete[] bgSubTiles;
	delete[] bgSubMap;

	btn_generalTab = new UIButton(&oamSub, "/data/ao-nds/ui/spr_generalTab", 0, 2, 1, SpriteSize_32x16, 128-46-2, 16+1, 46, 14, 32, 16, 0);
	btn_chatlogTab = new UIButton(&oamSub, "/data/ao-nds/ui/spr_chatlogTab", btn_generalTab->nextOamInd(), 2, 1, SpriteSize_32x16, 128+2, 16+1, 46, 14, 32, 16, 1);
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", btn_chatlogTab->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 2);

	// general tab
	lbl_showname = new UILabel(&oamSub, btn_back->nextOamInd(), 4, 1, RGB15(31,31,31), 3, 0);
	lbl_shownameValue = new UILabel(&oamSub, lbl_showname->nextOamInd(), 4, 1, 0, 4, 0);
	lbl_oocname = new UILabel(&oamSub, lbl_shownameValue->nextOamInd(), 4, 1, RGB15(31,31,31), 3, 0);
	lbl_oocnameValue = new UILabel(&oamSub, lbl_oocname->nextOamInd(), 4, 1, 0, 4, 0);
	btn_showname = new UIButton(&oamSub, "/data/ao-nds/ui/spr_settingsInput", lbl_oocnameValue->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 48, 192, 16, 32, 16, 5);
	btn_oocname = new UIButton(&oamSub, "/data/ao-nds/ui/spr_settingsInput", btn_showname->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 104, 192, 16, 32, 16, 5);

	// chatlog tab
	lbl_iniswaps = new UILabel(&oamSub, btn_oocname->nextOamInd(), 3, 1, RGB15(31,31,31), 3, 0);
	lbl_shownames = new UILabel(&oamSub, lbl_iniswaps->nextOamInd(), 3, 1, RGB15(31,31,31), 3, 0);
	btn_iniswaps = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", lbl_shownames->nextOamInd(), 1, 1, SpriteSize_16x16, 128-96, 96-16, 16, 16, 16, 16, 6);
	btn_shownames = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", btn_iniswaps->nextOamInd(), 1, 1, SpriteSize_16x16, 128+32, 96-16, 16, 16, 16, 16, 6);
	lbl_logPreview = new UILabel(&oamSub, btn_shownames->nextOamInd(), 8, 1, RGB15(31,31,31), 3, 0);
	lbl_logInfo = new UILabel(&oamSub, lbl_logPreview->nextOamInd(), 6, 2, RGB15(31,31,31), 3, 0);

	kb_input = new AOkeyboard(2, lbl_logInfo->nextOamInd(), 3);
	dmaCopy(bgSubPal, BG_PALETTE_SUB, 512);

	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav");
	sndCrtRcrd = wav_load_handle("/data/ao-nds/sounds/general/sfx-scroll.wav");

	lbl_showname->setPos(btn_showname->getX(), btn_showname->getY()-12);
	lbl_shownameValue->setPos(btn_showname->getX()+3, btn_showname->getY()+1);
	lbl_oocname->setPos(btn_oocname->getX(), btn_oocname->getY()-12);
	lbl_oocnameValue->setPos(btn_oocname->getX()+3, btn_oocname->getY()+1);

	lbl_showname->setText("Default showname");
	lbl_shownameValue->setText(gEngine->getShowname().c_str());
	lbl_oocname->setText("Default OOC name");
	lbl_oocnameValue->setText(gEngine->getOOCname().c_str());

	lbl_iniswaps->setText("Ini-swaps");
	lbl_iniswaps->setPos(btn_iniswaps->getX()+btn_iniswaps->getW()+2, btn_iniswaps->getY()+2, false);
	lbl_shownames->setText("Shownames");
	lbl_shownames->setPos(btn_shownames->getX()+btn_shownames->getW()+2, btn_shownames->getY()+2, false);
	lbl_logInfo->setText("What details would you like to\ndisplay in IC chatlog names?");
	lbl_logInfo->setPos(128, 40, true);

	btn_generalTab->connect(onGeneralTab, this);
	btn_chatlogTab->connect(onChatlogTab, this);
	btn_showname->connect(onShownameClicked, this);
	btn_oocname->connect(onOOCnameClicked, this);
	btn_iniswaps->connect(onIniswapsToggled, this);
	btn_shownames->connect(onShownamesToggled, this);
	btn_back->connect(onBackClicked, this);

	btn_iniswaps->setFrame(gEngine->showChatlogIniswaps());
	btn_shownames->setFrame(gEngine->showChatlogShownames());
	btn_back->assignKey(KEY_B);

	refreshChatlogPreview();
	setTab(0);
}

void UIScreenSettings::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			dmaCopy(bgSubPal, BG_PALETTE_SUB, 512);
			bgShow(subBgIndex);

			btn_generalTab->setVisible(true);
			btn_chatlogTab->setVisible(true);
			btn_back->setVisible(true);
			setTab(tab);

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

	btn_generalTab->updateInput();
	btn_chatlogTab->updateInput();
	btn_back->updateInput();
	btn_showname->updateInput();
	btn_oocname->updateInput();
	btn_iniswaps->updateInput();
	btn_shownames->updateInput();
}

void UIScreenSettings::hideEverything(bool keepTabs)
{
	if (!keepTabs)
	{
		bgHide(subBgIndex);
		btn_generalTab->setVisible(false);
		btn_chatlogTab->setVisible(false);
		btn_back->setVisible(false);
	}

	lbl_showname->setVisible(false);
	lbl_shownameValue->setVisible(false);
	lbl_oocname->setVisible(false);
	lbl_oocnameValue->setVisible(false);
	btn_showname->setVisible(false);
	btn_oocname->setVisible(false);

	lbl_iniswaps->setVisible(false);
	lbl_shownames->setVisible(false);
	btn_iniswaps->setVisible(false);
	btn_shownames->setVisible(false);
	lbl_logPreview->setVisible(false);
	lbl_logInfo->setVisible(false);
}

void UIScreenSettings::setTab(int i)
{
	hideEverything(true);
	tab = i;

	switch(i)
	{
		case 0:
			lbl_showname->setVisible(true);
			lbl_shownameValue->setVisible(true);
			lbl_oocname->setVisible(true);
			lbl_oocnameValue->setVisible(true);
			btn_showname->setVisible(true);
			btn_oocname->setVisible(true);
			break;

		case 1:
			lbl_iniswaps->setVisible(true);
			lbl_shownames->setVisible(true);
			btn_iniswaps->setVisible(true);
			btn_shownames->setVisible(true);
			lbl_logPreview->setVisible(true);
			lbl_logInfo->setVisible(true);
			break;
	}
}

void UIScreenSettings::refreshChatlogPreview()
{
	std::string name = "Apollo";
	if (gEngine->showChatlogIniswaps())
		name += " (ApolloSOJ)";
	if (gEngine->showChatlogShownames())
		name += " [Headshotnoby]";
	name += ": Test message";

	lbl_logPreview->setText(name.c_str());
	lbl_logPreview->setPos(128, 96+32, true);
}

void UIScreenSettings::saveSettings()
{
	cfgFile f;
	f.set("showname", gEngine->getShowname());
	f.set("oocname", gEngine->getOOCname());
	f.set("chatlog_iniswaps", gEngine->showChatlogIniswaps() ? "1" : "0");
	f.set("chatlog_shownames", gEngine->showChatlogShownames() ? "1" : "0");
	f.save("/data/ao-nds/settings_nds.cfg");
}

void UIScreenSettings::onGeneralTab(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCrtRcrd);

	pSelf->setTab(0);
}

void UIScreenSettings::onChatlogTab(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCrtRcrd);

	pSelf->setTab(1);
}

void UIScreenSettings::onShownameClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCrtRcrd);

	pSelf->hideEverything();
	pSelf->currEditing = pSelf->btn_showname;
	pSelf->kb_input->show("Enter default showname", gEngine->getShowname().c_str());
}

void UIScreenSettings::onOOCnameClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCrtRcrd);

	pSelf->hideEverything();
	pSelf->currEditing = pSelf->btn_oocname;
	pSelf->kb_input->show("Enter default OOC name", gEngine->getOOCname().c_str());
}

void UIScreenSettings::onIniswapsToggled(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;

	bool newValue = !gEngine->showChatlogIniswaps();
	gEngine->setChatlogIniswaps(newValue);
	pSelf->saveSettings();

	pSelf->btn_iniswaps->setFrame(newValue);
	pSelf->refreshChatlogPreview();
}

void UIScreenSettings::onShownamesToggled(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;

	bool newValue = !gEngine->showChatlogShownames();
	gEngine->setChatlogShownames(newValue);
	pSelf->saveSettings();

	pSelf->btn_shownames->setFrame(newValue);
	pSelf->refreshChatlogPreview();
}

void UIScreenSettings::onBackClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCancel);

	gEngine->changeScreen(new UIScreenMainMenu);
}
