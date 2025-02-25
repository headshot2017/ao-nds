#include "sockets/aotcpsocket.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "global.h"

AOtcpSocket::AOtcpSocket() : AOsocket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
}

AOtcpSocket::~AOtcpSocket()
{
	closesocket(sockfd);
}

void AOtcpSocket::setIP(std::string ip, u16 port)
{
	if (connected) return;
	m_IP = ip;
	m_Port = port;
}

void AOtcpSocket::connect()
{
	if (connected) return;

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(m_Port);
	if (inet_aton(m_IP.c_str(), &serv_addr.sin_addr) <= 0)
		return;

	if (::connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("TCP socket connection fail %d\n", errno);
		return;
	}

	// set non-blocking
	u32 one = 1;
	ioctl(sockfd, FIONBIO, &one);

	connected = true;
}

void AOtcpSocket::disconnect()
{
	if (!connected) return;

	shutdown(sockfd, 0);
	connected = false;
}

void AOtcpSocket::sendData(std::string data)
{
	if (!connected) return;
	send(sockfd, data.c_str(), data.size(), 0);
}

void AOtcpSocket::update()
{
	if (!connected) return;

	ticks++;
	if (ticks % 600 == 0)
		sendData("CH#%");

	char buf[4096];
	int total = recv(sockfd, buf, 4096, 0);
	if (total <= 0)
	{
		/*if (errno >= 102 && errno <= 104) // disconnected
		{
			printf("got disconnected\n");
			disconnect();
		}*/
		return;
	}

	buf[total] = 0;
	tempData += buf;

	printf("%d %s\n", tempData.size(), tempData.c_str());
	if (buf[total-1] == '%')
	{
		// loop through every percent %
		std::size_t lastPos = 0;
		std::size_t delimiterPos = tempData.find("%");

		while (delimiterPos != std::string::npos)
		{
			const std::string& data = tempData.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1);
			std::string header = argumentAt(data, 0);
			if (callbacks.count(header))
			{
				for (const NetCBInfo& cbInfo : callbacks[header])
					cbInfo.cb(cbInfo.pUserData, data);
			}

			lastPos = delimiterPos;
			delimiterPos = tempData.find("%", delimiterPos+1);
		}
		tempData.clear();
	}
}
