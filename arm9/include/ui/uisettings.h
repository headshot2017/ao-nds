#ifndef UISETTINGS_H_INCLUDED
#define UISETTINGS_H_INCLUDED

#include "uiscreen.h"

#include <string>

#include "button.h"
#include "label.h"
#include "keyboard.h"

class UIScreenSettings : public UIScreen
{
	int bgIndex;
	int subBgIndex;
	u32 bgTilesLen;
	u32 bgSubTilesLen;
	u8* bgSubPal;

	UIButton* btn_generalTab;
	UIButton* btn_chatlogTab;
	UIButton* btn_back;

	// general tab
	UILabel* lbl_showname;
	UILabel* lbl_shownameValue;
	UILabel* lbl_oocname;
	UILabel* lbl_oocnameValue;
	UIButton* btn_showname;
	UIButton* btn_oocname;

	// chatlog tab
	UILabel* lbl_iniswaps;
	UILabel* lbl_shownames;
	UIButton* btn_iniswaps;
	UIButton* btn_shownames;
	UILabel* lbl_logPreview;
	UILabel* lbl_logInfo;

	AOkeyboard* kb_input;
	UIButton* currEditing;

	u32* sndSelect;
	u32 sndSelectSize;
	u32* sndCancel;
	u32 sndCancelSize;
	u32* sndCrtRcrd;
	u32 sndCrtRcrdSize;

	int tab;

public:
	UIScreenSettings() : UIScreen() {}
	~UIScreenSettings();

	void init();
	void updateInput();
	void update() {}

	void hideEverything(bool keepTabs=false);
	void setTab(int i);
	void refreshChatlogPreview();
	void saveSettings();

	static void onGeneralTab(void* pUserData);
	static void onChatlogTab(void* pUserData);
	static void onShownameClicked(void* pUserData);
	static void onOOCnameClicked(void* pUserData);
	static void onIniswapsToggled(void* pUserData);
	static void onShownamesToggled(void* pUserData);
	static void onBackClicked(void* pUserData);
};

#endif // UISETTINGS_H_INCLUDED
