#ifndef SETTINGS_CHATBOX_H_INCLUDED
#define SETTINGS_CHATBOX_H_INCLUDED

#include <vector>
#include <string>

#include "ui/uisettings.h"
#include "courtroom/chatbox.h"

class UISettingsChatbox : public UISubSetting
{
	Chatbox* m_pChatbox;

	UIButton* btn_prev;
	UIButton* btn_next;
	UILabel* lbl_current;

	UILabel* lbl_allowChange;
	UIButton* btn_allowChange;

	UIButton* btn_preview;

	u32 m_currChatbox;
	std::vector<std::string> m_chatboxes;

public:
	UISettingsChatbox(UIScreenSettings* settingsUI) : UISubSetting(settingsUI) {}
	~UISettingsChatbox();
	const char* tabName() {return "Chatbox theme";}

	void init();
	void updateInput();
	void update();

	void reloadChatbox();

	static void onPrevClicked(void* pUserData);
	static void onNextClicked(void* pUserData);
	static void onAllowChangeToggled(void* pUserData);
	static void onPreviewClicked(void* pUserData);
};

#endif // SETTINGS_CHATBOX_H_INCLUDED
