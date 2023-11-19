#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"

class UICourtMessage : public UISubScreen
{
	UIButton* btn_ok;
	UILabel* lbl_msg;

	std::string msg;

public:
	UICourtMessage(UIScreenCourt* courtUI, std::string MSG) : UISubScreen(courtUI), msg(MSG) {}
	~UICourtMessage();

	void init();
	void updateInput();
	void update();

	static void onOKClicked(void* pUserData);
};

#endif // MESSAGE_H_INCLUDED
