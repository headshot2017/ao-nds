#include "wifikb.h"

#include <array>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_KEYPRESSES 128

static s32* keyPresses;
static int keyPressCount;

static int listenfd = -1;
static bool connected = false;
static bool listening = false;
static bool reverse = false;
static struct sockaddr_in udp_remote;
static int port = 9091;


void wifikb::setReverse(bool on)
{
	if (listenfd >= 0) return;
	reverse = on;
}

bool wifikb::init()
{
	if (listenfd >= 0) return true;

	keyPresses = new s32[MAX_KEYPRESSES];
	keyPressCount = 0;

	listenfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (reverse)
	{
		int on = 1;
		setsockopt(listenfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	}
	else
	{
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(port);

		if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
			return false;
	}

	// set non-blocking
	u32 one = 1;
	ioctl(listenfd, FIONBIO, &one);

	return true;
}

void wifikb::update()
{
	if (listenfd < 0) return;

	if (!connected && reverse)
	{
		static int ticks = 0;
		if (ticks++ >= 60)
		{
			ticks = 0;
			struct sockaddr_in s;

			memset(&s, 0, sizeof(s));
			s.sin_family = AF_INET;
			s.sin_addr.s_addr = INADDR_BROADCAST;
			s.sin_port = htons(port);

			sendto(listenfd, "\xff\xff\xff\xff", 4, 0, (struct sockaddr*)&s, sizeof(s));
		}
	}

	s32 recvdata;
	int dummy;

	int len = recvfrom(listenfd, &recvdata, sizeof(recvdata), 0, (struct sockaddr*)&udp_remote, &dummy);
	if (len < (int)sizeof(recvdata)) return;

	if (recvdata == -1) // NOKEY (this acts as connection argument)
	{
		connected = true;
		std::string hi = "wifikb v1.0\nYou can now start typing\n\n";
		send(hi);
	}
	else if (connected && listening && keyPressCount < MAX_KEYPRESSES)
		keyPresses[keyPressCount++] = recvdata;
}

void wifikb::start()
{
	if (listening) return;

	listening = true;
	keyPressCount = 0;
}

void wifikb::stop()
{
	if (!listening) return;

	listening = false;
}

bool wifikb::getKey(s32 *recv)
{
	if (!keyPressCount)
		return false;

	*recv = keyPresses[--keyPressCount];
	return true;
}

void wifikb::send(std::string message)
{
	if (!connected) return;

	sendto(listenfd, message.c_str(), message.size(), 0, (struct sockaddr*)&udp_remote, sizeof(udp_remote));
}
