#ifndef UIDIRECTCONN_H_INCLUDED
#define UIDIRECTCONN_H_INCLUDED

#include <nds/ndstypes.h>

#include "uiscreen.h"
#include "button.h"
#include "label.h"
#include "keyboard.h"

class UIScreenDirectConn : public UIScreen
{
	int bgIndex;
	u32 bgTilesLen;

	UIButton* btn_ws;
	UIButton* btn_tcp;
	UILabel* lbl_ws;
	UILabel* lbl_tcp;
	bool useWS;

	AOkeyboard* kb_ipInput;

public:
	UIScreenDirectConn() : UIScreen() {}
	~UIScreenDirectConn();
	int ID() {return 4;}

	void init();
	void updateInput();
	void update();

	static void onWSButton(void* pUserData);
	static void onTcpButton(void* pUserData);
};

#endif // UIDIRECTCONN_H_INCLUDED
