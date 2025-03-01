#ifndef CHARSELECT_H_INCLUDED
#define CHARSELECT_H_INCLUDED

#include <string>
#include <vector>

#include <nds/arm9/keyboard.h>
#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"
#include "ui/keyboard.h"

class UICourtCharSelect : public UISubScreen
{
	u32 currPage;
	int currCharSelected;

	std::string filter;
	std::vector<int> filteredChars;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_disconnect;
	UIButton* btn_confirm;
	UILabel* lbl_charname;
	UILabel* lbl_pages;
	UISelectCross* sel_btn;

	UIButton* btn_chars[8];

	AOkeyboard* kb_search;

	int holdWait;
	int pageAdd;
	int cbPV;
	int cbCharsCheck;

public:
	UICourtCharSelect(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtCharSelect();

	void init();
	void updateInput();
	void update();

	void reloadPage();
	void updatePageText();
	void updateFilter();

	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onPageBtnRelease(void* pUserData);
	static void onDisconnectClicked(void* pUserData);
	static void onConfirmClicked(void* pUserData);
	static void onCharClicked(void* pUserData);

	static void onMessagePV(void* pUserData, std::string msg);
	static void onMessageCharsCheck(void* pUserData, std::string msg);
};

#endif // CHARSELECT_H_INCLUDED
