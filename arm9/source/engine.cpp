#include "engine.h"

#include <dirent.h>
#include <string.h>

#include <nds/ndstypes.h>
#include <nds/interrupts.h>
#include <nds/cothread.h>
#include <dswifi9.h>

#include "utf8.h"
#include "ui/uidisconnected.h"

// for debug purposes
//#define NO_FADING

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
#ifndef NO_FADING
				REG_BLDCNT = BLEND_NONE;
				REG_BLDCNT_SUB = BLEND_NONE;
#endif
			}
			else
			{
#ifndef NO_FADING
				REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
				REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
#endif

				alpha--;

#ifndef NO_FADING
				REG_BLDY = alpha;
				REG_BLDY_SUB = alpha;
#endif
			}
		}
	}

	if (nextScreen)
	{
#ifndef NO_FADING
		REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
#endif

		alpha++;

#ifndef NO_FADING
		REG_BLDY = alpha;
		REG_BLDY_SUB = alpha;
#endif

		if (alpha == 16)
		{
			if (screen) delete screen;
			screen = nextScreen;
			screen->init();
			nextScreen = nullptr;
		}

#ifndef NO_FADING
		REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
#endif
	}
}

void Engine::quit()
{
	alpha = 0;

#ifndef NO_FADING
	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
	REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
#endif

	while (alpha++ != 16)
	{
#ifndef NO_FADING
		REG_BLDY = alpha;
		REG_BLDY_SUB = alpha;
#endif
		cothread_yield_irq(IRQ_VBLANK);
	}

	running = false;
}
