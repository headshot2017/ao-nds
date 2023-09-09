#include "ui/court/console.h"

#include <stdio.h>

#include <nds/arm9/console.h>
#include <nds/arm9/input.h>

#include "engine.h"
#include "ui/uiserverlist.h"

UICourtConsole::~UICourtConsole()
{
	consoleSelect(0);
	bgHide(7);
}

void UICourtConsole::init()
{
	consoleInit(0, 3, BgType_Text4bpp, BgSize_T_256x256, 2, 2, false, true);
}

void UICourtConsole::updateInput()
{
	if (keysDown() & KEY_SELECT)
	{
		iprintf("disconnecting\n");
		gEngine->changeScreen(new UIScreenServerList);
	}
}
