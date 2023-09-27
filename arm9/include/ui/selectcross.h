#ifndef SELECTCROSS_H_INCLUDED
#define SELECTCROSS_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

#include "button.h"

class UISelectCross
{
	u16* spriteGfx;
	OamState* oam;
	int oamStart;

	UIButton* selectedBtn;
	bool visible;

public:
	UISelectCross(OamState* chosenOam, int oamStartInd, int palSlot);
	~UISelectCross();

	void setVisible(bool on);
	void setPriority(int pr);
	void selectButton(UIButton* btn, int offset=1);

	int nextOamInd() {return oamStart+4;}
};

#endif // SELECTCROSS_H_INCLUDED
