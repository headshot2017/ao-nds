#include "engine.h"

#include <dirent.h>
#include <string.h>

#include <nds/ndstypes.h>
#include <nds/interrupts.h>
#include <dswifi9.h>

#include "utf8.h"
#include "ui/uidisconnected.h"

Engine* gEngine = nullptr;

Engine::Engine() : screen(nullptr), nextScreen(nullptr), aosocket(nullptr)
{
	alpha = 16;
	fading = false;
	running = true;
	wifiSwitch = false;
}

Engine::~Engine()
{
	if (screen) delete screen;
	if (aosocket) delete aosocket;
}

void Engine::changeScreen(UIScreen* next)
{
	nextScreen = next;
	alpha = (screen) ? 0 : 15;
	fading = true;
}

void Engine::setSocket(AOsocket* sock)
{
	if (aosocket) delete aosocket;
	aosocket = sock;
}

void Engine::updateInput()
{
	if (screen && !nextScreen)
		screen->updateInput();
}

void Engine::update()
{
	if (aosocket)
	{
		aosocket->update();
	}

	if (!nextScreen && wifiSwitch && Wifi_AssocStatus() != ASSOCSTATUS_ASSOCIATED)
	{
		wifiSwitch = false;
		gEngine->changeScreen(new UIScreenDisconnected("Disconnected from Wi-Fi", "Connection to Wi-Fi lost.\nPress OK to reconnect.", true));
	}

	if (screen)
	{
		screen->update();

		if (fading && !nextScreen)
		{
			if (!alpha)
			{
				fading = false;
				REG_BLDCNT = BLEND_NONE;
				REG_BLDCNT_SUB = BLEND_NONE;
			}
			else
			{
				REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
				REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;

				alpha--;
				REG_BLDY = alpha;
				REG_BLDY_SUB = alpha;
			}
		}
	}

	if (nextScreen)
	{
		REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;

		alpha++;
		REG_BLDY = alpha;
		REG_BLDY_SUB = alpha;

		if (alpha == 16)
		{
			if (screen) delete screen;
			screen = nextScreen;
			screen->init();
			nextScreen = nullptr;
		}

		REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
	}
}

void Engine::quit()
{
	alpha = 0;
	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
	REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;

	while (alpha++ != 16)
	{
		REG_BLDY = alpha;
		REG_BLDY_SUB = alpha;
		swiWaitForVBlank();
	}

	running = false;
}
