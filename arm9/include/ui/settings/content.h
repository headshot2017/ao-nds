#ifndef SETTINGS_CONTENT_H_INCLUDED
#define SETTINGS_CONTENT_H_INCLUDED

#include "ui/uisettings.h"

class UISettingsContent : public UISubSetting
{
	UIButton* btn_contentBtn[6];
	UILabel* lbl_contentBtn[6];
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_apply;

	u32 currPage;

	std::vector<std::string> m_contents;
	std::vector<std::string> m_allContents;

public:
	UISettingsContent(UIScreenSettings* settingsUI) : UISubSetting(settingsUI) {}
	~UISettingsContent();
	const char* tabName() {return "Custom contents";}

	void init();
	void updateInput();
	void update();

	void setVisible(bool on);
	void reloadPage();

	static void onContentClicked(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onApplyClicked(void* pUserData);
};

#endif // SETTINGS_CONTENT_H_INCLUDED
