#ifndef AREALIST_H_INCLUDED
#define AREALIST_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"
#include "ui/keyboard.h"

class UICourtAreaList : public UISubScreen
{
	int bgIndex;
	u32 currPage;
	int currAreaSelected;

	UIButton* btn_back;
	UIButton* btn_listToggle;
	UIButton* btn_confirm;
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_area[4];
	UILabel* lbl_area[4];
	UILabel* lbl_pages;
	UILabel* lbl_areaInfo;
	UISelectCross* sel_btn;

	int holdWait;
	int pageAdd;
	int cbARUP;

public:
	UICourtAreaList(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtAreaList();

	void init();
	void updateInput();
	void update();

	void reloadPage();
	void updatePageText();
	void updateAreaInfo();

	static void onBackClicked(void* pUserData);
	static void onToggleList(void* pUserData);
	static void onConfirmClicked(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onPageBtnRelease(void* pUserData);
	static void onAreaClicked(void* pUserData);

	static void onMessageARUP(void* pUserData, std::string msg);
};

#endif // AREALIST_H_INCLUDED
