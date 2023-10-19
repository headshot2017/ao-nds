#ifndef PAIRSELECT_H_INCLUDED
#define PAIRSELECT_H_INCLUDED

#include <string>
#include <vector>

#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"
#include "ui/keyboard.h"

class UICourtPairSelect : public UISubScreen
{
	u32 currPage;
	int currCharSelected;

	std::string filter;
	std::vector<int> filteredChars;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_back;
	UIButton* btn_pair;
	UILabel* lbl_charname;
	UILabel* lbl_pages;
	UISelectCross* sel_btn;

	UIButton* btn_chars[8];

	AOkeyboard* kb_search;

	int holdWait;
	int pageAdd;

public:
	UICourtPairSelect(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtPairSelect();

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
	static void onPairClicked(void* pUserData);
	static void onCharClicked(void* pUserData);
};

#endif // PAIRSELECT_H_INCLUDED
