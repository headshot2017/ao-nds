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

	UILabel* lbl_blendA;
	UIButton* btn_blendA_down;
	UIButton* btn_blendA_up;
	UILabel* lbl_blendB;
	UIButton* btn_blendB_down;
	UIButton* btn_blendB_up;
	UIButton* btn_resetBlend;

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
	void reloadBlend();

	static void onPrevClicked(void* pUserData);
	static void onNextClicked(void* pUserData);
	static void onAllowChangeToggled(void* pUserData);
	static void onBlendA_Down(void* pUserData);
	static void onBlendA_Up(void* pUserData);
	static void onBlendB_Down(void* pUserData);
	static void onBlendB_Up(void* pUserData);
	static void onResetBlendClicked(void* pUserData);
	static void onPreviewClicked(void* pUserData);
};

#endif // SETTINGS_CHATBOX_H_INCLUDED
