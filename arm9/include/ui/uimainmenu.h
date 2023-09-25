#ifndef UIMAINMENU_H_INCLUDED
#define UIMAINMENU_H_INCLUDED

#include "uiscreen.h"

#include <nds/ndstypes.h>
#include "button.h"

class UIScreenMainMenu : public UIScreen
{
	int bgIndex;
	int subBgIndex;

	UIButton* btn_viewServerList;
	UIButton* btn_directConnect;
	UIButton* btn_settings;

	u32* sndGavel;
	u32 sndGavelSize;

public:
	UIScreenMainMenu() : UIScreen() {}
	~UIScreenMainMenu();

	void init();
	void updateInput();
	void update() {}

	static void onViewServerList(void* pUserData);
	static void onDirectConnect(void* pUserData);
	static void onSettings(void* pUserData);
};

#endif // UIMAINMENU_H_INCLUDED
