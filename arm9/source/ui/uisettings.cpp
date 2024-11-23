#include "ui/uisettings.h"

#include <nds/dma.h>
#include <nds/arm9/background.h>

#include "utf8.h"
#include "engine.h"
#include "ui/uimainmenu.h"
#include "ui/settings/general.h"
#include "ui/settings/chatlog.h"
#include "ui/settings/content.h"
#include "ui/settings/contentOrder.h"
#include "ui/settings/wifikb.h"


template<typename T>
UISubSetting* newSetting(UIScreenSettings* pSelf)
{
    return new T(pSelf);
}

static std::vector<UISubSetting *(*)(UIScreenSettings*)> settingChooser =
{
	&newSetting<UISettingsGeneral>,
	&newSetting<UISettingsChatlog>,
	&newSetting<UISettingsContent>,
	&newSetting<UISettingsContentOrder>,
	&newSetting<UISettingsKeyboard>,
};


UIScreenSettings::~UIScreenSettings()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete[] bgSubPal;

	delete btn_back;
	delete btn_prevTab;
	delete btn_nextTab;
	delete lbl_currentTab;

	if (m_subSetting)
		delete m_subSetting;

	wav_free_handle(sndCancel);
	wav_free_handle(sndCrtRcrd);
	wav_free_handle(sndEvPage);
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
	dmaCopy(bgSubPal, BG_PALETTE_SUB, 512);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;
	delete[] bgSubTiles;
	delete[] bgSubMap;

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 30, 32, 32, 0);
	btn_prevTab = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", btn_back->nextOamInd(), 1, 1, SpriteSize_32x16, 128-9-96, 20, 19, 14, 32, 16, 1);
	btn_nextTab = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prevTab->nextOamInd(), 1, 1, SpriteSize_32x16, 128-9+96, 20, 19, 14, 32, 16, 2);
	lbl_currentTab = new UILabel(&oamSub, btn_nextTab->nextOamInd(), 4, 1, RGB15(31,31,31), 3, 0);

	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav");
	sndCrtRcrd = wav_load_handle("/data/ao-nds/sounds/general/sfx-scroll.wav");
	sndEvPage = wav_load_handle("/data/ao-nds/sounds/general/sfx-blink.wav");

	btn_back->connect(onBackClicked, this);
	btn_prevTab->connect(onPrevTab, this);
	btn_nextTab->connect(onNextTab, this);

	btn_back->assignKey(KEY_B);
	btn_prevTab->assignKey(KEY_L);
	btn_nextTab->assignKey(KEY_R);

	m_tabIndex = 0;
	m_nextSetting = new UISettingsGeneral(this);
	m_subSetting = 0;
}

void UIScreenSettings::loadNewTab()
{
	changeTab(settingChooser[m_tabIndex](this));
}

void UIScreenSettings::updateInput()
{
	btn_back->updateInput();
	btn_prevTab->updateInput();
	btn_nextTab->updateInput();

	if (m_subSetting)
		m_subSetting->updateInput();
}

void UIScreenSettings::update()
{
	if (m_nextSetting)
	{
		if (m_subSetting) delete m_subSetting;
		m_subSetting = m_nextSetting;
		m_subSetting->init();
		m_nextSetting = nullptr;
		lbl_currentTab->setText(m_subSetting->tabName());
		lbl_currentTab->setPos(128, 20, true);
	}

	if (m_subSetting)
		m_subSetting->update();
}

int UIScreenSettings::getFirstOAM()
{
	return lbl_currentTab->nextOamInd();
}

void UIScreenSettings::hideEverything(bool keepBG)
{
	if (!keepBG) bgHide(subBgIndex);
	btn_back->setVisible(false);
	btn_prevTab->setVisible(false);
	btn_nextTab->setVisible(false);
	lbl_currentTab->setVisible(false);

	if (m_subSetting)
		m_subSetting->setVisible(false);
}

void UIScreenSettings::showEverything()
{
	bgShow(subBgIndex);
	btn_back->setVisible(true);
	btn_prevTab->setVisible(true);
	btn_nextTab->setVisible(true);
	lbl_currentTab->setVisible(true);

	if (m_subSetting)
		m_subSetting->setVisible(true);
}

void UIScreenSettings::changeTab(UISubSetting* newTab)
{
	m_nextSetting = newTab;
}

void UIScreenSettings::onBackClicked(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCancel);

	gEngine->changeScreen(new UIScreenMainMenu);
}

void UIScreenSettings::onPrevTab(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCrtRcrd);

	if (pSelf->m_tabIndex == 0) pSelf->m_tabIndex = settingChooser.size();
	pSelf->m_tabIndex--;
	pSelf->loadNewTab();
}

void UIScreenSettings::onNextTab(void* pUserData)
{
	UIScreenSettings* pSelf = (UIScreenSettings*)pUserData;
	wav_play(pSelf->sndCrtRcrd);

	pSelf->m_tabIndex++;
	if (pSelf->m_tabIndex >= settingChooser.size()) pSelf->m_tabIndex = 0;
	pSelf->loadNewTab();
}
