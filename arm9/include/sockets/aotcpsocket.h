#ifndef AOTCPSOCKET_H_INCLUDED
#define AOTCPSOCKET_H_INCLUDED

#include "aosocket.h"

#include <nds/ndstypes.h>

class AOtcpSocket : public AOsocket
{
	int sockfd;
	std::string tempData;
	std::string m_IP;
	u16 m_Port;

public:
	AOtcpSocket();
	~AOtcpSocket();

	void update();
	void setIP(std::string ip, u16 port);
	void connect();
	void disconnect();
	void sendData(std::string data);
};

#endif // AOTCPSOCKET_H_INCLUDED
