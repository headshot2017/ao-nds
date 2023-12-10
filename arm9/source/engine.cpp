#include "engine.h"

#include <dirent.h>
#include <string.h>
#include <algorithm>

#include <nds/ndstypes.h>
#include <nds/interrupts.h>
#include <dswifi9.h>

#include "ui/uidisconnected.h"
#include "ui/label.h"
#include "cfgFile.h"
#include "mini/ini.h"
#include "mp3_shared.h"

Engine* gEngine = nullptr;

Engine::Engine() : screen(nullptr), nextScreen(nullptr), aosocket(nullptr)
{
	alpha = 16;
	fading = false;
	running = true;
	wifiSwitch = false;

	UILabel* lbl_loading = new UILabel(&oamSub, 0, 8, 1, RGB15(31,31,31), 0, 1);

	lbl_loading->setText("Creating music cache...");
	lbl_loading->setPos(128, 96-12, true);
	oamUpdate(&oamSub);
	cacheMusic("/data/ao-nds/sounds/music");

	lbl_loading->setText("Creating evidence cache...");
	lbl_loading->setPos(128, 96-12, true);
	oamUpdate(&oamSub);
	cacheEvidence("/data/ao-nds/evidence/small");

	lbl_loading->setText("Creating character blip cache...");
	lbl_loading->setPos(128, 96-12, true);
	oamUpdate(&oamSub);
	cacheCharBlips("/data/ao-nds/characters");

	cfgFile settings("/data/ao-nds/settings_nds.cfg");
	defaultShowname = settings.get("showname", "");
	defaultOOCname = settings.get("oocname", "");
	chatlogIniswaps = settings.get("chatlog_iniswaps", "") == "1";
	chatlogShownames = settings.get("chatlog_shownames", "") == "1";

	loadPrivateEvidence();

	delete lbl_loading;
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

void Engine::cacheEvidence(const std::string& folder)
{
	DIR *dir = opendir(folder.c_str());
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..") || !strcmp(dent->d_name + strlen(dent->d_name) - 8, ".pal.bin")) continue;

		std::string value = dent->d_name;
		value = value.substr(0, value.size()-8);

		cachedEvidence.push_back(value);
	}

	closedir(dir);
}

void Engine::cacheCharBlips(const std::string& folder)
{
	DIR *dir = opendir(folder.c_str());
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;

		std::string charname = dent->d_name;
		mINI::INIFile file(folder + "/" + charname + "/char.ini");
		mINI::INIStructure ini;

		if (!file.read(ini))
			return;

		std::string& blip = ini["options"]["blips"];
		if (blip.empty()) blip = ini["options"]["gender"];

		std::transform(charname.begin(), charname.end(), charname.begin(), [](char c){return std::tolower(c);});
		cachedCharBlips[charname] = blip;
	}

	closedir(dir);
}

void Engine::loadPrivateEvidence()
{
	mINI::INIFile file("/data/ao-nds/private_evidence.ini");
	mINI::INIStructure ini;

	if (!file.read(ini))
		return;

	for (int i=0; ; i++)
	{
		std::string I = std::to_string(i);
		if (!ini.has(I))
			return;

		std::string name = ini[I]["name"];
		std::string desc = ini[I]["description"];
		std::string image = ini[I]["image"];

		// remove file extension from image
		size_t newExtPos = 0;
		size_t extPos = 0;
		while (newExtPos != std::string::npos)
		{
			extPos = newExtPos;
			newExtPos = image.find(".", extPos+1);
		}
		if (extPos)
			image = image.substr(0, extPos);

		privateEvidence.push_back({name, desc, image});
	}
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

void Engine::savePrivateEvidence()
{
	mINI::INIFile file("/data/ao-nds/private_evidence.ini");
	mINI::INIStructure ini;
	mp3_fill_buffer();

	for (u32 i=0; i<privateEvidence.size(); i++)
	{
		std::string I = std::to_string(i);

		ini[I]["name"] = privateEvidence[i].name;
		ini[I]["description"] = privateEvidence[i].description;
		ini[I]["image"] = privateEvidence[i].image + ".png";
		mp3_fill_buffer();
	}

	file.generate(ini);
	mp3_fill_buffer();
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
