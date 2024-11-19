#include "ui/settings/chatlog.h"

#include "settings.h"

UISettingsChatlog::~UISettingsChatlog()
{
	delete lbl_iniswaps;
	delete lbl_shownames;
	delete btn_iniswaps;
	delete btn_shownames;
	delete lbl_logPreview;
	delete lbl_logInfo;
}

void UISettingsChatlog::init()
{
	lbl_iniswaps = new UILabel(&oamSub, pSettingsUI->getFirstOAM(), 3, 1, RGB15(31,31,31), 3, 0);
	lbl_shownames = new UILabel(&oamSub, lbl_iniswaps->nextOamInd(), 3, 1, RGB15(31,31,31), 3, 0);
	btn_iniswaps = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", lbl_shownames->nextOamInd(), 1, 1, SpriteSize_16x16, 128-96, 96-16, 16, 16, 16, 16, 6);
	btn_shownames = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", btn_iniswaps->nextOamInd(), 1, 1, SpriteSize_16x16, 128+32, 96-16, 16, 16, 16, 16, 6);
	lbl_logPreview = new UILabel(&oamSub, btn_shownames->nextOamInd(), 8, 1, RGB15(31,31,31), 3, 0);
	lbl_logInfo = new UILabel(&oamSub, lbl_logPreview->nextOamInd(), 6, 2, RGB15(31,31,31), 3, 0);

	lbl_iniswaps->setText("Ini-swaps");
	lbl_iniswaps->setPos(btn_iniswaps->getX()+btn_iniswaps->getW()+2, btn_iniswaps->getY()+2, false);
	lbl_shownames->setText("Shownames");
	lbl_shownames->setPos(btn_shownames->getX()+btn_shownames->getW()+2, btn_shownames->getY()+2, false);
	lbl_logInfo->setText("What details would you like to\ndisplay in IC chatlog names?");
	lbl_logInfo->setPos(128, 40, true);

	btn_iniswaps->connect(onIniswapsToggled, this);
	btn_shownames->connect(onShownamesToggled, this);

	btn_iniswaps->setFrame(Settings::chatlogIniswaps);
	btn_shownames->setFrame(Settings::chatlogShownames);

	refreshChatlogPreview();
}

void UISettingsChatlog::updateInput()
{
	btn_iniswaps->updateInput();
	btn_shownames->updateInput();
}

void UISettingsChatlog::update()
{

}

void UISettingsChatlog::refreshChatlogPreview()
{
	std::string name = "Apollo";
	if (Settings::chatlogIniswaps)
		name += " (ApolloSOJ)";
	if (Settings::chatlogShownames)
		name += " [Headshotnoby]";
	name += ": Test message";

	lbl_logPreview->setText(name);
	lbl_logPreview->setPos(128, 96+32, true);
}

void UISettingsChatlog::onIniswapsToggled(void* pUserData)
{
	UISettingsChatlog* pSelf = (UISettingsChatlog*)pUserData;

	bool newValue = !Settings::chatlogIniswaps;
	Settings::chatlogIniswaps = newValue;
	Settings::save();

	pSelf->btn_iniswaps->setFrame(newValue);
	pSelf->refreshChatlogPreview();
}

void UISettingsChatlog::onShownamesToggled(void* pUserData)
{
	UISettingsChatlog* pSelf = (UISettingsChatlog*)pUserData;

	bool newValue = !Settings::chatlogShownames;
	Settings::chatlogShownames = newValue;
	Settings::save();

	pSelf->btn_shownames->setFrame(newValue);
	pSelf->refreshChatlogPreview();
}
