#include "engine.h"

#include <dirent.h>
#include <string.h>
#include <algorithm>

#include <nds/ndstypes.h>
#include <nds/interrupts.h>

#include "cfgFile.h"

Engine* gEngine = nullptr;

Engine::Engine() : screen(nullptr), nextScreen(nullptr), aosocket(nullptr)
{
	alpha = 16;
	fading = false;
	running = true;

	cacheMusic("/data/ao-nds/sounds/music");

	cfgFile settings("/data/ao-nds/settings_nds.cfg");
	defaultShowname = settings.get("showname", "");
	defaultOOCname = settings.get("oocname", "");
}

Engine::~Engine()
{
	if (screen) delete screen;
	if (aosocket) delete aosocket;
}

void Engine::cacheMusic(const std::string& folder, std::string extra)
{
	std::string dirStr = folder + "/" + extra;
	DIR *dir = opendir(dirStr.c_str());
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;

		std::string value = (extra.empty()) ? dent->d_name : extra+"/"+dent->d_name;
		std::transform(value.begin(), value.end(), value.begin(), [](char c){return std::tolower(c);});
		if (dent->d_type == DT_DIR)
			cacheMusic(folder, value);
		else
			cachedMusic[value] = true;
	}

	closedir(dir);
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
				REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
				REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;

				alpha--;
				REG_BLDY = alpha;
				REG_BLDY_SUB = alpha;
			}
		}
	}

	if (nextScreen)
	{
		REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;

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

		REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
	}

	if (aosocket)
	{
		aosocket->update();
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
