#ifndef AOTCPSOCKET_H_INCLUDED
#define AOTCPSOCKET_H_INCLUDED

#include "aosocket.h"

#include <nds/ndstypes.h>

class AOtcpSocket : public AOsocket
{
	int sockfd;
	std::string tempData;

public:
	AOtcpSocket();
	~AOtcpSocket();

	void update();
	void connectIP(std::string ip, u16 port);
	void disconnect();
	void sendData(std::string data);
};

#endif // AOTCPSOCKET_H_INCLUDED
