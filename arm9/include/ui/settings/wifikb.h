#ifndef SETTINGS_WIFIKB_H_INCLUDED
#define SETTINGS_WIFIKB_H_INCLUDED

#include "ui/uisettings.h"

class UISettingsKeyboard : public UISubSetting
{
	UILabel* lbl_restartRequired;

	UIButton* btn_enable;
	UILabel* lbl_enable;

	UIButton* btn_reverse;
	UILabel* lbl_reverse;

	UILabel* lbl_ipInfo;
	UILabel* lbl_info;

public:
	UISettingsKeyboard(UIScreenSettings* settingsUI) : UISubSetting(settingsUI) {}
	~UISettingsKeyboard();
	const char* tabName() {return "Wi-Fi Keyboard";}

	void init();
	void updateInput();
	void update();

	static void onEnableToggled(void* pUserData);
	static void onReverseToggled(void* pUserData);
};

#endif // SETTINGS_WIFIKB_H_INCLUDED
