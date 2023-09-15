#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/keyboard.h>

#include "global.h"
#include "label.h"

class AOkeyboard
{
	Keyboard m_kb;
	std::string& value;
	std::string valueOld;
	UILabel* lbl_plswrite;
	UILabel* lbl_written;

	cbInfo enterPressed;
	cbInfo escPressed;

public:
	AOkeyboard(std::string& valueRef, int lines, int oamStart, int palSlot);
	~AOkeyboard();

	void show(const char* plsWrite, const char* startValue=0);
	int updateInput();

	int nextOamInd() {return lbl_written->nextOamInd();}
	bool isVisible() {return m_kb.visible;}
	std::string& getValue();
};

#endif // KEYBOARD_H_INCLUDED
