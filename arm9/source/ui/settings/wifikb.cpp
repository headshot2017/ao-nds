#include "ui/settings/wifikb.h"

#include <stdio.h>
#include <dswifi9.h>
#include <netinet/in.h>

#include "settings.h"

UISettingsKeyboard::~UISettingsKeyboard()
{
	delete lbl_restartRequired;
	delete btn_enable;
	delete lbl_enable;
	delete btn_reverse;
	delete lbl_reverse;
	delete lbl_ipInfo;
	delete lbl_info;
}

void UISettingsKeyboard::init()
{
	lbl_restartRequired = new UILabel(&oamSub, pSettingsUI->getFirstOAM(), 8, 1, RGB15(31,31,31), 3, 0);
	btn_enable = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", lbl_restartRequired->nextOamInd(), 1, 1, SpriteSize_16x16, 16, 56, 16, 16, 16, 16, 4);
	lbl_enable = new UILabel(&oamSub, btn_enable->nextOamInd(), 6, 1, RGB15(31,31,31), 3, 0);
	btn_reverse = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", lbl_enable->nextOamInd(), 1, 1, SpriteSize_16x16, 16, 56+24, 16, 16, 16, 16, 4);
	lbl_reverse = new UILabel(&oamSub, btn_reverse->nextOamInd(), 6, 2, RGB15(31,31,31), 3, 0);
	lbl_ipInfo = new UILabel(&oamSub, lbl_reverse->nextOamInd(), 5, 2, RGB15(31,31,31), 3, 0);
	lbl_info = new UILabel(&oamSub, lbl_ipInfo->nextOamInd(), 8, 5, RGB15(31,31,31), 3, 0);

	lbl_restartRequired->setText("To apply changes in this tab, restart the game.");
	lbl_restartRequired->setPos(128, 36, true);
	lbl_enable->setPos(btn_enable->getX()+18, btn_enable->getY()+1);
	lbl_enable->setText("Enable Wi-Fi Keyboard");
	lbl_reverse->setPos(btn_reverse->getX()+18, btn_reverse->getY()-4);
	lbl_reverse->setText("Reverse connection mode\n(for emulators)");

	struct in_addr ip = Wifi_GetIPInfo(0,0,0,0);
	std::string ipStr = inet_ntoa(ip);

	lbl_ipInfo->setText("Your IP:\n" + ipStr);
	lbl_ipInfo->setPos(192-8, btn_enable->getY(), false);

	lbl_info->setText("This feature allows you to type using\nyour PC or phone keyboard.\nDownload wifikb client for your OS here:\nhttp://github.com/headshot2017/wifikb/releases");
	lbl_info->setPos(20, 107);

	btn_enable->setFrame(Settings::wifikbEnabled);
	btn_reverse->setFrame(Settings::wifikbReverseMode);
	btn_enable->connect(onEnableToggled, this);
	btn_reverse->connect(onReverseToggled, this);
}

void UISettingsKeyboard::updateInput()
{
	btn_enable->updateInput();
	btn_reverse->updateInput();
}

void UISettingsKeyboard::update()
{

}

void UISettingsKeyboard::onEnableToggled(void* pUserData)
{
	UISettingsKeyboard* pSelf = (UISettingsKeyboard*)pUserData;

	Settings::wifikbEnabled ^= 1;
	Settings::save();
	pSelf->btn_enable->setFrame(Settings::wifikbEnabled);
}

void UISettingsKeyboard::onReverseToggled(void* pUserData)
{
	UISettingsKeyboard* pSelf = (UISettingsKeyboard*)pUserData;

	Settings::wifikbReverseMode ^= 1;
	Settings::save();
	pSelf->btn_reverse->setFrame(Settings::wifikbReverseMode);
}
