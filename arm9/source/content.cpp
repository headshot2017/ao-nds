#include "content.h"

#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <dirent.h>
#include <string.h>

#include "ui/label.h"
#include "mini/ini.h"
#include "global.h"

static std::vector<std::string> contents;

struct ContentCache
{
	std::unordered_set<std::string> music;
	std::unordered_map<std::string, std::string> charBlips;
};
static std::unordered_map<std::string, ContentCache> cache;

static std::vector<evidenceCacheInfo> cachedEvidence;

static void cacheMusic(const std::string& content, std::string extra="")
{
	std::string dirStr = "/data/ao-nds" + (content.empty() ? "/" : "/custom/"+content+"/") + "sounds/music/" + extra;
	DIR *dir = opendir(dirStr.c_str());
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;

		std::string value = (extra.empty()) ? dent->d_name : extra+"/"+dent->d_name;
		std::transform(value.begin(), value.end(), value.begin(), [](char c){return std::tolower(c);});
		if (dent->d_type == DT_DIR)
			cacheMusic(content, value);
		else
			cache[content].music.insert(value);
	}

	closedir(dir);
}

static void cacheEvidence(const std::string& content, std::string extra="")
{
	std::string dirStr = "/data/ao-nds" + (content.empty() ? "/" : "/custom/"+content+"/") + "evidence/small/" + extra;
	DIR *dir = opendir(dirStr.c_str());
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..") || !strcmp(dent->d_name + strlen(dent->d_name) - 8, ".pal.bin")) continue;

		std::string value = dent->d_name;
		std::string valueNoExt = value.substr(0, value.size()-8);

		if (dent->d_type == DT_DIR)
			cacheEvidence(content, value);
		else
			cachedEvidence.push_back({ valueNoExt, extra, (content.empty() ? "" : "custom/"+content) });
	}

	closedir(dir);
}

static void cacheCharBlips(const std::string& content)
{
	std::string fullPath = "/data/ao-nds" + (content.empty() ? "/" : "/custom/"+content+"/") + "characters";
	DIR *dir = opendir(fullPath.c_str());
	if (!dir) return;

	struct dirent* dent;
	while( (dent = readdir(dir)) )
	{
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;

		std::string charname = dent->d_name;
		mINI::INIFile file(fullPath + "/" + charname + "/char.ini");
		mINI::INIStructure ini;

		if (!file.read(ini))
			return;

		std::string& blip = ini["options"]["blips"];
		if (blip.empty()) blip = ini["options"]["gender"];

		std::transform(charname.begin(), charname.end(), charname.begin(), [](char c){return std::tolower(c);});
		cache[content].charBlips[charname] = blip;
	}

	closedir(dir);
}

static void createCache(int oam, const std::string& content, const std::string& displayName)
{
	UILabel* lbl_loading = new UILabel(&oamSub, oam, 8, 1, RGB15(31,31,31), 0, 1);
	UILabel* lbl_content = new UILabel(&oamSub, lbl_loading->nextOamInd(), 7, 1, RGB15(31,31,31), 0, 0);

	lbl_content->setText(displayName);
	lbl_content->setPos(128, 96+24, true);

	lbl_loading->setText("Creating music cache...");
	lbl_loading->setPos(128, 96-12, true);
	oamUpdate(&oamSub);
	cacheMusic(content);

	lbl_loading->setText("Creating evidence cache...");
	lbl_loading->setPos(128, 96-12, true);
	oamUpdate(&oamSub);
	cacheEvidence(content);

	lbl_loading->setText("Creating character blip cache...");
	lbl_loading->setPos(128, 96-12, true);
	oamUpdate(&oamSub);
	cacheCharBlips(content);

	delete lbl_loading;
	delete lbl_content;
}


void Content::reload(int oamStart)
{
	cache.clear();

	for (auto& content : contents)
	{
		if (content == "") continue;
		cache[content] = {};

		createCache(oamStart, content, content);
	}

	createCache(oamStart, "", "Default");
}

void Content::add(std::string name)
{
	contents.push_back(name);
}

void Content::remove(std::string name)
{
	auto pos = std::find(contents.begin(), contents.end(), name);
	if (pos != contents.end())
		contents.erase(pos);
}

void Content::clear()
{
	contents.clear();
}

std::vector<std::string>& Content::getContents()
{
	return contents;
}

const std::vector<evidenceCacheInfo>& Content::getEvidence()
{
	return cachedEvidence;
}


bool Content::exists(const std::string& filename)
{
	std::string check;

	for (auto& k : contents)
	{
		check = "/data/ao-nds/custom/" + k + "/" + filename;
		if (fileExists(check))
			return true;
	}

	check = "/data/ao-nds/" + filename;
	if (fileExists(check))
		return true;

	return false;
}

bool Content::exists(const std::string& filename, std::string& out)
{
	std::string check;

	for (auto& k : contents)
	{
		check = "/data/ao-nds/custom/" + k + "/" + filename;
		if (fileExists(check))
		{
			out = check;
			return true;
		}
	}

	check = "/data/ao-nds/" + filename;
	if (fileExists(check))
	{
		out = check;
		return true;
	}

	return false;
}

const std::string Content::getFile(const std::string& filename)
{
	std::string check;

	for (auto& k : contents)
	{
		check = "/data/ao-nds/custom/" + k + "/" + filename;
		if (fileExists(check))
			return check;
	}

	check = "/data/ao-nds/" + filename;
	if (fileExists(check))
		return check;

	return "";
}

const std::string Content::getCharBlip(const std::string& charname)
{
	for (auto& k : contents)
	{
		if (cache[k].charBlips.count(charname))
			return cache[k].charBlips[charname];
	}

	if (cache[""].charBlips.count(charname))
		return cache[""].charBlips[charname];

	return "";
}

bool Content::musicExists(const std::string& file, std::string& out)
{
	for (auto& k : contents)
	{
		if (cache[k].music.count(file))
		{
			out = k;
			return true;
		}
	}

	if (cache[""].music.count(file))
	{
		out = "";
		return true;
	}

	return false;
}
