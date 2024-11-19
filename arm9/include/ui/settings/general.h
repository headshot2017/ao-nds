#ifndef SETTINGS_GENERAL_H_INCLUDED
#define SETTINGS_GENERAL_H_INCLUDED

#include "ui/uisettings.h"
#include "ui/keyboard.h"

class UISettingsGeneral : public UISubSetting
{
	UILabel* lbl_showname;
	UILabel* lbl_shownameValue;
	UILabel* lbl_oocname;
	UILabel* lbl_oocnameValue;
	UIButton* btn_showname;
	UIButton* btn_oocname;

	AOkeyboard* kb_input;
	UIButton* currEditing;

public:
	UISettingsGeneral(UIScreenSettings* settingsUI) : UISubSetting(settingsUI) {}
	~UISettingsGeneral();
	const char* tabName() {return "General";}

	void init();
	void updateInput();
	void update();

	void setVisible(bool on);

	static void onShownameClicked(void* pUserData);
	static void onOOCnameClicked(void* pUserData);
};

#endif // SETTINGS_GENERAL_H_INCLUDED
