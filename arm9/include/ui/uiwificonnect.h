#ifndef UIWIFICONNECT_H_INCLUDED
#define UIWIFICONNECT_H_INCLUDED

#include "uiscreen.h"

#include <nds/ndstypes.h>

class UIScreenWifi : public UIScreen
{
	int bgIndex;
	int subBgIndex;
	u32 bgTilesLen;
	u32 bgSubTilesLen;

	u8* sprLoadingImg;
	u16* sprLoading;

	u16* textGfx[8];
	u8* textCanvas;

	int ticks;
	int frame;
	int currAssocStatus;

public:
	UIScreenWifi() : UIScreen() {}
	~UIScreenWifi();

	void init();
	void updateInput();
	void update();

	void setText(const char* text);
};

#endif // UIWIFICONNECT_H_INCLUDED
