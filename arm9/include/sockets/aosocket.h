#ifndef AOSOCKET_H_INCLUDED
#define AOSOCKET_H_INCLUDED

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include "global.h"

struct NetCBInfo
{
	stringCallback cb;
	void* pUserData;
};

class AOsocket
{
public:
	AOsocket() : connected(false), ticks(0) {}
	virtual ~AOsocket() {}

	virtual void update() {}
	virtual void connectIP(std::string ip) {}
	virtual void disconnect() {}
	virtual void sendData(std::string data) {}

	void clearCallbacks() {callbacks.clear();}
	size_t addMessageCallback(std::string headerName, stringCallback cb, void* pUserData)
	{
		if (!callbacks.count(headerName))
			callbacks.insert(std::pair<std::string, std::vector<NetCBInfo> >(headerName, std::vector<NetCBInfo>()));
		callbacks[headerName].push_back({cb, pUserData});
		return callbacks[headerName].size()-1;
	}
	void removeMessageCallback(std::string headerName, size_t id)
	{
		if (!callbacks.count(headerName) || id >= callbacks[headerName].size()) return;
		callbacks[headerName].erase(callbacks[headerName].begin() + id);
	}

protected:
	std::unordered_map<std::string, std::vector<NetCBInfo> > callbacks;
	bool connected;
	int ticks;
};

#endif // AOSOCKET_H_INCLUDED
