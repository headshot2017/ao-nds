#ifndef CHARSELECT_H_INCLUDED
#define CHARSELECT_H_INCLUDED

#include <string>
#include <vector>

#include <nds/arm9/keyboard.h>
#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"

class UICourtCharSelect : public UISubScreen
{
	int bgIndex;
	u32 currPage;
	int currCharSelected;

	std::string filter;
	std::string filterOld;
	std::vector<int> filteredChars;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_disconnect;
	UIButton* btn_confirm;
	UILabel* lbl_charname;
	UILabel* lbl_pages;

	UIButton* btn_chars[8];

	Keyboard m_kb;
	UILabel* lbl_plswrite;
	UILabel* lbl_written;

	int holdWait;
	int pageAdd;
	int cbPV;

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
};

#endif // CHARSELECT_H_INCLUDED
