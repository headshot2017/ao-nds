#ifndef MUSICLIST_H_INCLUDED
#define MUSICLIST_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"

class UICourtMusicList : public UISubScreen
{
	int bgIndex;
	u32 scrollPos;

	UIButton* btn_back;
	UIButton* btn_listToggle;
	UIButton* btn_scrollUp;
	UIButton* btn_scrollDown;
	UIButton* btn_musicBtn[7];
	UILabel* lbl_musicBtn[7];
	UIButton* btn_sliderHandle;

	int holdWait;
	int pageAdd;
	bool draggingHandle;

public:
	UICourtMusicList(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtMusicList();

	void init();
	void updateInput();
	void update();
	void reloadScroll();

	static void onBackClicked(void* pUserData);
	static void onToggleList(void* pUserData);
	static void onScrollUpPressed(void* pUserData);
	static void onScrollDownPressed(void* pUserData);
	static void onScrollBtnReleased(void* pUserData);
	static void onSliderPressed(void* pUserData);
	static void onSliderReleased(void* pUserData);
	static void onMusicClicked(void* pUserData);
};

#endif // MUSICLIST_H_INCLUDED
