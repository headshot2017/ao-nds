#ifndef UIDISCONNECTED_H_INCLUDED
#define UIDISCONNECTED_H_INCLUDED

#include "uiscreen.h"

#include <string>

#include "button.h"
#include "label.h"
#include "wav_nds.h"

class UIScreenDisconnected : public UIScreen
{
	int bgIndex;
	int subBgIndex;
	u32 bgTilesLen;
	u32 bgSubTilesLen;

	std::string disconnectMsg;
	std::string reason;
	bool goToWiFi;

	UILabel* lbl_disconnectMsg;
	UILabel* lbl_reason;
	UIButton* btn_ok;

	wav_handle* sndSelect;

public:
	UIScreenDisconnected(std::string msg, std::string rsn, bool wifi) : UIScreen(), disconnectMsg(msg), reason(rsn), goToWiFi(wifi) {}
	~UIScreenDisconnected();

	void init();
	void updateInput();
	void update() {}

	static void onOK(void* pUserData);
};

#endif // UIDISCONNECTED_H_INCLUDED
