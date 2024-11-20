#include "settings.h"

#include <nds.h>

#include "cfgFile.h"
#include "mini/ini.h"
#include "mp3_shared.h"
#include "content.h"
#include "global.h"

std::u16string Settings::defaultShowname;
std::u16string Settings::defaultOOCname;
bool Settings::chatlogIniswaps;
bool Settings::chatlogShownames;
std::vector<evidenceInfo> Settings::privateEvidence;

void Settings::load()
{
	cfgFile settings("/data/ao-nds/settings_nds.cfg");

	defaultShowname = utf8::utf8to16(settings.get("showname", ""));
	defaultOOCname = utf8::utf8to16(settings.get("oocname", ""));
	chatlogIniswaps = settings.get("chatlog_iniswaps", "") == "1";
	chatlogShownames = settings.get("chatlog_shownames", "") == "1";

	std::string contentArgs = settings.get("mounted_contents", "");
	if (!contentArgs.empty())
	{
		u32 totalContents = totalArguments(contentArgs, ',');
		for (u32 i=0; i<totalContents; i++)
			Content::add(argumentAt(contentArgs, i, ','));
	}
	Content::reload();

	loadPrivateEvidence();
}

void Settings::save()
{
	cfgFile f;

	f.set("showname", utf8::utf16to8(defaultShowname));
	f.set("oocname", utf8::utf16to8(defaultOOCname));
	f.set("chatlog_iniswaps", chatlogIniswaps ? "1" : "0");
	f.set("chatlog_shownames", chatlogShownames ? "1" : "0");

	std::string contentArgs;
	u32 totalContents = Content::getContents().size();
	for (u32 i=0; i<totalContents; i++)
		contentArgs += ((i != 0) ? "," : "") + Content::getContents()[i];

	f.set("mounted_contents", contentArgs);

	f.save("/data/ao-nds/settings_nds.cfg");
}

void Settings::loadPrivateEvidence()
{
	privateEvidence.clear();

	mINI::INIFile file("/data/ao-nds/private_evidence.ini");
	mINI::INIStructure ini;

	if (!file.read(ini))
		return;

	for (int i=0; ; i++)
	{
		std::string I = std::to_string(i);
		if (!ini.has(I))
			return;

		std::u16string name = utf8::utf8to16(ini[I]["name"]);
		std::u16string desc = utf8::utf8to16(ini[I]["description"]);
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

void Settings::savePrivateEvidence()
{
	mINI::INIFile file("/data/ao-nds/private_evidence.ini");
	mINI::INIStructure ini;
	mp3_fill_buffer();

	for (u32 i=0; i<privateEvidence.size(); i++)
	{
		std::string I = std::to_string(i);

		ini[I]["name"] = utf8::utf16to8(privateEvidence[i].name);
		ini[I]["description"] = utf8::utf16to8(privateEvidence[i].description);
		ini[I]["image"] = privateEvidence[i].image + ".png";
		mp3_fill_buffer();
	}

	file.generate(ini);
	mp3_fill_buffer();
}