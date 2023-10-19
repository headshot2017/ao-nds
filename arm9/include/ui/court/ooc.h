#ifndef OOC_H_INCLUDED
#define OOC_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtOOC : public UISubScreen
{
	u32 scrollPos;
	bool atBottom;

	UIButton* btn_back;
	UIButton* btn_presets;
	UIButton* btn_scrollUp;
	UIButton* btn_scrollDown;
	UIButton* btn_sliderHandle;
	UILabel* lbl_log;
	UILabel* lbl_oocName;

	AOkeyboard* kb_input;
	bool isWritingChat;

	int holdWait;
	int pageAdd;
	bool draggingHandle;

	int cbCT;


public:
	UICourtOOC(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtOOC();

	void init();
	void updateInput();
	void update();

	void hideEverything();
	void reloadScroll();
	void setSliderHandle();

	static void onBackClicked(void* pUserData);
	static void onPresetsClicked(void* pUserData);
	static void onScrollUpPressed(void* pUserData);
	static void onScrollDownPressed(void* pUserData);
	static void onScrollBtnReleased(void* pUserData);
	static void onSliderPressed(void* pUserData);
	static void onSliderReleased(void* pUserData);

	static void onMessageCT(void* pUserData, std::string msg);
};

#endif // OOC_H_INCLUDED
