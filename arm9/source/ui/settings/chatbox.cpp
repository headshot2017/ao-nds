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

	btn_preview = new UIButton(&oamSub, "/data/ao-nds/ui/spr_previewBtn", btn_allowChange->nextOamInd(), 2, 1, SpriteSize_32x16, 128-32, 144, 64, 15, 32, 16, 7);

	lbl_allowChange->setText("Allow characters to change\nthe chatbox (When possible)");
	lbl_allowChange->setPos(btn_allowChange->getX()+btn_allowChange->getW()+2, btn_allowChange->getY()-4, false);
	btn_allowChange->setFrame(Settings::allowChatboxChange);

	btn_prev->connect(onPrevClicked, this);
	btn_next->connect(onNextClicked, this);
	btn_allowChange->connect(onAllowChangeToggled, this);
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
}

void UISettingsChatbox::updateInput()
{
	btn_prev->updateInput();
	btn_next->updateInput();
	btn_allowChange->updateInput();
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
