#ifndef LOADING_H_INCLUDED
#define LOADING_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"

class UICourtLoading : public UISubScreen
{
	UIButton* btn_disconnect;
	UILabel* lbl_loading;

	u8* sprLoadingImg;
	u16* sprLoading;
	int ticks;
	int frame;

	u32 cbDecryptor;
	u32 cbSI;
	u32 cbSC;
	u32 cbSM;

public:
	UICourtLoading(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtLoading();

	void init();
	void updateInput();
	void update();
	void setText(const char* text);

	static void onDisconnectClicked(void* pUserData);
	static void onMessageDecryptor(void* pUserData, std::string msg);
	static void onMessageSI(void* pUserData, std::string msg);
	static void onMessageSC(void* pUserData, std::string msg);
	static void onMessageSM(void* pUserData, std::string msg);
};

#endif // LOADING_H_INCLUDED
