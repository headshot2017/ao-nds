#ifndef UISETTINGS_H_INCLUDED
#define UISETTINGS_H_INCLUDED

#include "uiscreen.h"

#include <string>

#include "button.h"
#include "label.h"
#include "wav_nds.h"

class UIScreenSettings;
class UISubSetting
{
public:
	UISubSetting(UIScreenSettings* settingsUI) : pSettingsUI(settingsUI) {}
	virtual ~UISubSetting() {}
	virtual const char* tabName() {return "unknown";}

	virtual void init() {}
	virtual void updateInput() {}
	virtual void update() {}

	virtual void setVisible(bool on) {}

protected:
	UIScreenSettings* pSettingsUI;
};

class UIScreenSettings : public UIScreen
{
	int bgIndex;
	int subBgIndex;
	u32 bgTilesLen;
	u32 bgSubTilesLen;

	UIButton* btn_back;
	UIButton* btn_prevTab;
	UIButton* btn_nextTab;
	UILabel* lbl_currentTab;

	UISubSetting* m_subSetting;
	UISubSetting* m_nextSetting;
	u32 m_tabIndex;

	void loadNewTab();

public:
	wav_handle* sndCancel;
	wav_handle* sndSelect;
	wav_handle* sndCrtRcrd;
	wav_handle* sndEvPage;
	u8* bgSubPal;

public:
	UIScreenSettings() : UIScreen() {}
	~UIScreenSettings();

	void init();
	void updateInput();
	void update();

	int getFirstOAM();
	void hideEverything(bool keepBG=false);
	void showEverything();
	void changeTab(UISubSetting* newTab);

	static void onBackClicked(void* pUserData);
	static void onPrevTab(void* pUserData);
	static void onNextTab(void* pUserData);
};

#endif // UISETTINGS_H_INCLUDED
