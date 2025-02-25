#include "ui/settings/chatbox.h"

#include <dirent.h>
#include <algorithm>

#include "settings.h"
#include "colors.h"
#include "utf8.h"
#include "mini/ini.h"

UISettingsChatbox::~UISettingsChatbox()
{
	delete m_pChatbox;
	delete btn_prev;
	delete btn_next;
	delete lbl_current;
	delete lbl_allowChange;
	delete btn_allowChange;
	delete lbl_blendA;
	delete btn_blendA_down;
	delete btn_blendA_up;
	delete lbl_blendB;
	delete btn_blendB_down;
	delete btn_blendB_up;
	delete btn_resetBlend;
	delete btn_preview;
}

void UISettingsChatbox::init()
{
	m_pChatbox = new Chatbox(nullptr);
	m_pChatbox->setVisible(true);

	btn_prev = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", pSettingsUI->getFirstOAM(), 1, 1, SpriteSize_32x16, 128-(19/2)-64, 40, 19, 14, 32, 16, 4);
	btn_next = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prev->nextOamInd(), 1, 1, SpriteSize_32x16, 128-(19/2)+64, 40, 19, 14, 32, 16, 5);
	lbl_current = new UILabel(&oamSub, btn_next->nextOamInd(), 5, 1, RGB15(31,31,31), 3, 0);

	lbl_allowChange = new UILabel(&oamSub, lbl_current->nextOamInd(), 6, 2, RGB15(31,31,31), 3, 0);
	btn_allowChange = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", lbl_allowChange->nextOamInd(), 1, 1, SpriteSize_16x16, 52, 64, 16, 16, 16, 16, 6);

	lbl_blendA = new UILabel(&oamSub, btn_allowChange->nextOamInd(), 2, 1, RGB15(31,31,31), 3, 0);
	btn_blendA_down = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", lbl_blendA->nextOamInd(), 1, 1, SpriteSize_32x16, 128-19-4, 96-2, 19, 14, 32, 16, 4);
	btn_blendA_up = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_blendA_down->nextOamInd(), 1, 1, SpriteSize_32x16, 128+4, 96-2, 19, 14, 32, 16, 5);
	lbl_blendB = new UILabel(&oamSub, btn_blendA_up->nextOamInd(), 2, 1, RGB15(31,31,31), 3, 0);
	btn_blendB_down = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", lbl_blendB->nextOamInd(), 1, 1, SpriteSize_32x16, 128-19-4, 96+16+2, 19, 14, 32, 16, 4);
	btn_blendB_up = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_blendB_down->nextOamInd(), 1, 1, SpriteSize_32x16, 128+4, 96+16+2, 19, 14, 32, 16, 5);
	btn_resetBlend = new UIButton(&oamSub, "/data/ao-nds/ui/spr_reset", btn_blendB_up->nextOamInd(), 2, 1, SpriteSize_32x16, 128+48, 96+6, 48, 16, 32, 16, 7);

	btn_preview = new UIButton(&oamSub, "/data/ao-nds/ui/spr_previewBtn", btn_resetBlend->nextOamInd(), 2, 1, SpriteSize_32x16, 128-32, 144, 64, 15, 32, 16, 8);

	lbl_allowChange->setText("Allow characters to change\nthe chatbox (When possible)");
	lbl_allowChange->setPos(btn_allowChange->getX()+btn_allowChange->getW()+2, btn_allowChange->getY()-4, false);
	btn_allowChange->setFrame(Settings::allowChatboxChange);

	btn_prev->connect(onPrevClicked, this);
	btn_next->connect(onNextClicked, this);
	btn_allowChange->connect(onAllowChangeToggled, this);
	btn_blendA_down->connect(onBlendA_Down, this);
	btn_blendA_up->connect(onBlendA_Up, this);
	btn_blendB_down->connect(onBlendB_Down, this);
	btn_blendB_up->connect(onBlendB_Up, this);
	btn_resetBlend->connect(onResetBlendClicked, this);
	btn_preview->connect(onPreviewClicked, this);

	btn_prev->assignKey(KEY_LEFT);
	btn_next->assignKey(KEY_RIGHT);

	m_chatboxes.push_back("default");

	DIR *dir = opendir("/data/ao-nds/misc/chatboxes");
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		std::string name = dent->d_name;

		if (dent->d_type != DT_DIR || name == "." || name == ".." || name == "default")
			continue;

		mINI::INIFile file("/data/ao-nds/misc/chatboxes/" + name + "/chatbox.ini");
		mINI::INIStructure ini;
		if (!file.read(ini) || (ini["general"].has("hiddenFromSettings") && ini["general"]["hiddenFromSettings"] == "1"))
			continue;

		m_chatboxes.push_back(name);
	}

	closedir(dir);

	auto it = std::find(m_chatboxes.begin(), m_chatboxes.end(), Settings::defaultChatbox);
	m_currChatbox = (it != m_chatboxes.end()) ? (u32)(it-m_chatboxes.begin()) : 0;

	reloadChatbox();
	reloadBlend();
}

void UISettingsChatbox::updateInput()
{
	btn_prev->updateInput();
	btn_next->updateInput();
	btn_allowChange->updateInput();
	btn_blendA_down->updateInput();
	btn_blendA_up->updateInput();
	btn_blendB_down->updateInput();
	btn_blendB_up->updateInput();
	btn_resetBlend->updateInput();
	btn_preview->updateInput();
}

void UISettingsChatbox::update()
{
	m_pChatbox->update();
}

void UISettingsChatbox::reloadChatbox()
{
	m_pChatbox->setTheme(m_chatboxes[m_currChatbox]);
	lbl_current->setText(m_chatboxes[m_currChatbox]);
	lbl_current->setPos(128, 40, true);
}

void UISettingsChatbox::reloadBlend()
{
	char buf[64];

	sprintf(buf, "Alpha: %d", Settings::chatboxBlendA);
	lbl_blendA->setText(buf);
	lbl_blendA->setPos(btn_blendA_down->getX()-64, btn_blendA_down->getY(), false);

	sprintf(buf, "Black: %d", Settings::chatboxBlendB);
	lbl_blendB->setText(buf);
	lbl_blendB->setPos(btn_blendB_down->getX()-64, btn_blendB_down->getY(), false);

	btn_blendA_down->setVisible(Settings::chatboxBlendA > 0);
	btn_blendA_up->setVisible(Settings::chatboxBlendA < 15);
	btn_blendB_down->setVisible(Settings::chatboxBlendB > 0);
	btn_blendB_up->setVisible(Settings::chatboxBlendB < 15);
}

void UISettingsChatbox::onPrevClicked(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	if (pSelf->m_currChatbox <= 0)
		pSelf->m_currChatbox = pSelf->m_chatboxes.size();
	pSelf->m_currChatbox--;
	pSelf->reloadChatbox();

	Settings::defaultChatbox = pSelf->m_chatboxes[pSelf->m_currChatbox];
	Settings::save();
}

void UISettingsChatbox::onNextClicked(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	if (++pSelf->m_currChatbox >= pSelf->m_chatboxes.size())
		pSelf->m_currChatbox = 0;
	pSelf->reloadChatbox();

	Settings::defaultChatbox = pSelf->m_chatboxes[pSelf->m_currChatbox];
	Settings::save();
}

void UISettingsChatbox::onAllowChangeToggled(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;

	Settings::allowChatboxChange ^= 1;
	Settings::save();
	pSelf->btn_allowChange->setFrame(Settings::allowChatboxChange);
}

void UISettingsChatbox::onBlendA_Down(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	if (Settings::chatboxBlendA > 0)
		Settings::chatboxBlendA--;
	Settings::save();
	pSelf->reloadBlend();
}

void UISettingsChatbox::onBlendA_Up(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	if (Settings::chatboxBlendA < 15)
		Settings::chatboxBlendA++;
	Settings::save();
	pSelf->reloadBlend();
}

void UISettingsChatbox::onBlendB_Down(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	if (Settings::chatboxBlendB > 0)
		Settings::chatboxBlendB--;
	Settings::save();
	pSelf->reloadBlend();
}

void UISettingsChatbox::onBlendB_Up(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndEvPage);

	if (Settings::chatboxBlendB < 15)
		Settings::chatboxBlendB++;
	Settings::save();
	pSelf->reloadBlend();
}

void UISettingsChatbox::onResetBlendClicked(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndSelect);

	Settings::chatboxBlendA = 7;
	Settings::chatboxBlendB = 15;
	Settings::save();
	pSelf->reloadBlend();
}

void UISettingsChatbox::onPreviewClicked(void* pUserData)
{
	UISettingsChatbox* pSelf = (UISettingsChatbox*)pUserData;
	wav_play(pSelf->pSettingsUI->sndSelect);

	pSelf->m_pChatbox->setName(utf8::utf8to16(std::string("Sample Text")));
	pSelf->m_pChatbox->setText(
		utf8::utf8to16(std::string("~~Lorem ipsum dolor sit amet,\\nconsectetur adipiscing elit.\\nProin iaculis auctor neque.")),
		COLOR_WHITE,
		"male"
	);
}
