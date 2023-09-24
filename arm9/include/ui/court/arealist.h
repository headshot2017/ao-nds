#ifndef AREALIST_H_INCLUDED
#define AREALIST_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtAreaList : public UISubScreen
{
	int bgIndex;

	UIButton* btn_back;
	UIButton* btn_listToggle;

public:
	UICourtAreaList(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtAreaList();

	void init();
	void updateInput();
	void update();

	static void onBackClicked(void* pUserData);
	static void onToggleList(void* pUserData);
};

#endif // AREALIST_H_INCLUDED
