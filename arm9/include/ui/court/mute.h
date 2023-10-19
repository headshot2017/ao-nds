#ifndef MUTE_H_INCLUDED
#define MUTE_H_INCLUDED

#include <string>
#include <vector>

#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"
#include "ui/keyboard.h"

class UICourtMute : public UISubScreen
{
	u32 currPage;
	int currCharSelected;

	std::string filter;
	std::vector<int> filteredChars;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_back;
	UIButton* btn_muteToggle;
	UILabel* lbl_charname;
	UILabel* lbl_pages;
	UISelectCross* sel_btn;

	UIButton* btn_chars[8];

	AOkeyboard* kb_search;

	int holdWait;
	int pageAdd;

public:
	UICourtMute(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtMute();

	void init();
	void updateInput();
	void update();

	void reloadPage();
	void updatePageText();
	void updateFilter();

	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onPageBtnRelease(void* pUserData);
	static void onBackClicked(void* pUserData);
	static void onMuteToggled(void* pUserData);
	static void onCharClicked(void* pUserData);
};

#endif // MUTE_H_INCLUDED
