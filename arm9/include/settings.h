#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include <vector>
#include <string>

#include "utf8.h"

struct evidenceInfo
{
	std::u16string name;
	std::u16string description;
	std::string image;
};

namespace Settings
{
	extern std::u16string defaultShowname;
	extern std::u16string defaultOOCname;
	extern bool chatlogIniswaps;
	extern bool chatlogShownames;
	extern bool wifikbEnabled;
	extern bool wifikbReverseMode;
	extern std::vector<evidenceInfo> privateEvidence;

	void load();
	void save();
	void loadPrivateEvidence();
	void savePrivateEvidence();
};

#endif // SETTINGS_H_INCLUDED
