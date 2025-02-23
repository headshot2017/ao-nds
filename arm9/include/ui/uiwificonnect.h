#ifndef UIWIFICONNECT_H_INCLUDED
#define UIWIFICONNECT_H_INCLUDED

#include "uiscreen.h"

#include <nds/ndstypes.h>

#include "label.h"

class UIScreenWifi : public UIScreen
{
	int bgIndex;
	int subBgIndex;
	u32 bgTilesLen;
	u32 bgSubTilesLen;

	u8* sprLoadingImg;
	u16* sprLoading;

	UILabel* lbl_loading;

	int ticks;
	int frame;
	int currAssocStatus;

public:
	UIScreenWifi() : UIScreen() {}
	~UIScreenWifi();
	int ID() {return 1;}

	void init();
	void updateInput();
	void update();

	void setText(const char* text);
};

#endif // UIWIFICONNECT_H_INCLUDED
