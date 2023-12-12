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
	std::u16string value;
	std::u16string valueOld;
	UILabel* lbl_plswrite;
	UILabel* lbl_written;

	cbInfo enterPressed;
	cbInfo escPressed;

public:
	AOkeyboard(int lines, int oamStart, int palSlot);
	~AOkeyboard();

	void show16(const char* plsWrite, std::u16string startValue=u"\0");
	void show(const char* plsWrite, std::string startValue="\0");
	void setInputYOffset(int offset) {lbl_written->setPos(8, 32+offset);}
	int updateInput();
	void setValue(std::u16string newValue);

	int nextOamInd() {return lbl_written->nextOamInd();}
	bool isVisible() {return m_kb.visible;}
	std::u16string& getValue();
	std::string getValueUTF8();
};

#endif // KEYBOARD_H_INCLUDED
