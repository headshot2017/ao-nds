#ifndef INGAMEMENU_H_INCLUDED
#define INGAMEMENU_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtIngameMenu : public UISubScreen
{
	UIButton* btn_talkIC;
	UIButton* btn_talkOOC;
	UIButton* btn_music;
	UIButton* btn_changeChar;
	UIButton* btn_courtRecord;
	UIButton* btn_guard;
	UIButton* btn_callMod;
	UILabel* lbl_currChar;
	UILabel* lbl_guard;

	AOkeyboard* kb_input;

	int cbPV;
	int cbAUTH;

public:
	UICourtIngameMenu(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtIngameMenu();

	void init();
	void updateInput();
	void update();

	void hideEverything();
	void showEverything();

	static void onTalkICclicked(void* pUserData);
	static void onTalkOOCclicked(void* pUserData);
	static void onMusicClicked(void* pUserData);
	static void onChangeCharClicked(void* pUserData);
	static void onCourtRecordClicked(void* pUserData);
	static void onGuardToggled(void* pUserData);
	static void onCallModClicked(void* pUserData);

	static void onMessagePV(void* pUserData, std::string msg);
	static void onMessageAUTH(void* pUserData, std::string msg);
};

#endif // INGAMEMENU_H_INCLUDED
