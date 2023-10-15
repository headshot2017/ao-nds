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

	UILabel* lbl_showname;
	UILabel* lbl_shownameValue;
	UILabel* lbl_oocname;
	UILabel* lbl_oocnameValue;
	UIButton* btn_showname;
	UIButton* btn_oocname;
	UIButton* btn_back;

	AOkeyboard* kb_input;
	UIButton* currEditing;

	u32* sndSelect;
	u32 sndSelectSize;
	u32* sndCancel;
	u32 sndCancelSize;

public:
	UIScreenSettings() : UIScreen() {}
	~UIScreenSettings();

	void init();
	void updateInput();
	void update() {}

	void hideEverything();
	void saveSettings();

	static void onShownameClicked(void* pUserData);
	static void onOOCnameClicked(void* pUserData);
	static void onBackClicked(void* pUserData);
};

#endif // UISETTINGS_H_INCLUDED
