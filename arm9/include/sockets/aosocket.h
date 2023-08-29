#ifndef AOSOCKET_H_INCLUDED
#define AOSOCKET_H_INCLUDED

#include <string>
#include <unordered_map>

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

	void setMessageCallback(std::string headerName, stringCallback cb, void* pUserData) {callbacks[headerName] = {cb, pUserData};}

protected:
	std::unordered_map<std::string, NetCBInfo> callbacks;
	bool connected;
	int ticks;
};

#endif // AOSOCKET_H_INCLUDED
