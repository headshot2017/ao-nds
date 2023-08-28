#include "engine.h"

#include <nds/ndstypes.h>

Engine* gEngine = nullptr;

Engine::Engine() : screen(nullptr), nextScreen(nullptr), aosocket(nullptr)
{

}

Engine::~Engine()
{
	if (screen) delete screen;
	if (aosocket) delete aosocket;
}

void Engine::updateInput()
{
	if (screen)
		screen->updateInput();
}

void Engine::update()
{
	if (nextScreen)
	{
		if (screen) delete screen;
		screen = nextScreen;
		screen->init();
		nextScreen = nullptr;
	}

	if (screen)
	{
		screen->update();
	}

	if (aosocket)
	{
		aosocket->update();
	}
}
