#ifndef SETTINGS_CHATLOG_H_INCLUDED
#define SETTINGS_CHATLOG_H_INCLUDED

#include "ui/uisettings.h"

class UISettingsChatlog : public UISubSetting
{
	UILabel* lbl_iniswaps;
	UILabel* lbl_shownames;
	UIButton* btn_iniswaps;
	UIButton* btn_shownames;
	UILabel* lbl_logPreview;
	UILabel* lbl_logInfo;

public:
	UISettingsChatlog(UIScreenSettings* settingsUI) : UISubSetting(settingsUI) {}
	~UISettingsChatlog();
	const char* tabName() {return "Chatlog";}

	void init();
	void updateInput();
	void update();

	void refreshChatlogPreview();

	static void onIniswapsToggled(void* pUserData);
	static void onShownamesToggled(void* pUserData);
};

#endif // SETTINGS_CHATLOG_H_INCLUDED
