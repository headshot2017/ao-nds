#ifndef UIMAINMENU_H_INCLUDED
#define UIMAINMENU_H_INCLUDED

#include "uiscreen.h"

#include <nds/ndstypes.h>

#include "button.h"
#include "label.h"
#include "wav_nds.h"

class UIScreenMainMenu : public UIScreen
{
	int bgIndex;
	int subBgIndex;
	u32 bgTilesLen;
	u32 bgSubTilesLen;

	UIButton* btn_viewServerList;
	UIButton* btn_directConnect;
	UIButton* btn_settings;
	UILabel* lbl_dsi;

	wav_handle* sndGavel;

public:
	UIScreenMainMenu() : UIScreen() {}
	~UIScreenMainMenu();
	int ID() {return 2;}

	void init();
	void updateInput();
	void update() {}

	static void onViewServerList(void* pUserData);
	static void onDirectConnect(void* pUserData);
	static void onSettings(void* pUserData);
};

#endif // UIMAINMENU_H_INCLUDED
