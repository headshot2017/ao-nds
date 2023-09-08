#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

#include "ui/uicourt.h"

class UICourtConsole : public UISubScreen
{
public:
	UICourtConsole(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}

	void init();
	void updateInput();
};

#endif // CONSOLE_H_INCLUDED
