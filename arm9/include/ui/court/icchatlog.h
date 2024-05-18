#ifndef ICCHATLOG_H_INCLUDED
#define ICCHATLOG_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"

class UICourtICChatLog : public UISubScreen
{
	u32 scrollPos;
	u32 scrollPosOld;
	bool atBottom;

	UIButton* btn_back;
	UIButton* btn_courtRecord;
	UIButton* btn_scrollUp;
	UIButton* btn_scrollDown;
	UIButton* btn_sliderHandle;
	UILabel* lbl_log;

	int holdWait;
	int pageAdd;
	bool draggingHandle;

	int cbMS;
	int cbMC;

public:
	UICourtICChatLog(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtICChatLog();

	void init();
	void updateInput();
	void update();

	void setScroll(u32 i);
	void reloadScroll(bool all=false);
	void setSliderHandle();

	static void onBackClicked(void* pUserData);
	static void onCourtRecord(void* pUserData);
	static void onScrollUpPressed(void* pUserData);
	static void onScrollDownPressed(void* pUserData);
	static void onScrollBtnReleased(void* pUserData);
	static void onSliderPressed(void* pUserData);
	static void onSliderReleased(void* pUserData);

	static void onMessageMS(void* pUserData, std::string msg);
	static void onMessageMC(void* pUserData, std::string msg);
};

#endif // ICCHATLOG_H_INCLUDED
