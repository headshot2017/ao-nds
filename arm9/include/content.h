#ifndef CONTENT_H_INCLUDED
#define CONTENT_H_INCLUDED

#include <string>
#include <vector>

struct evidenceCacheInfo
{
	std::string name;
	std::string subdir;
	std::string content;
};

namespace Content
{
	void reload(int oamStart=0);
	void add(std::string name);
	void remove(std::string name);
	void clear();

	std::vector<std::string>& getContents();
	const std::vector<evidenceCacheInfo>& getEvidence();

	bool exists(const std::string& filename);
	bool exists(const std::string& filename, std::string& out);
	const std::string getFile(const std::string& filename);
	const std::string getCharBlip(const std::string& charname);
	const std::string getCharChatbox(const std::string& charname);
	bool musicExists(const std::string& file, std::string& out);
}

#endif // CONTENT_H_INCLUDED
