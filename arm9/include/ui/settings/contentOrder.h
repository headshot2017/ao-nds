#ifndef SETTINGS_CONTENTORDER_H_INCLUDED
#define SETTINGS_CONTENTORDER_H_INCLUDED


#include "ui/uisettings.h"

class UISettingsContentOrder : public UISubSetting
{
	UILabel* lbl_info;
	UIButton* btn_contentBtn[5];
	UILabel* lbl_contentBtn[5];
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_moveUp;
	UIButton* btn_moveDown;

	u32 currPage;
	int currBtn;

public:
	UISettingsContentOrder(UIScreenSettings* settingsUI) : UISubSetting(settingsUI) {}
	~UISettingsContentOrder();
	const char* tabName() {return "Custom contents (order)";}

	void init();
	void updateInput();
	void update();

	void reloadPage();

	static void onContentClicked(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onMoveUp(void* pUserData);
	static void onMoveDown(void* pUserData);
};
#endif // SETTINGS_CONTENTORDER_H_INCLUDED
