#include "wifikb.h"

#include <array>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_KEYPRESSES 128

static wifikb::KeyStruct* keyPresses;
static int keyPressCount;

static int listenfd = -1;
static bool connected = false;
static bool listening = false;
static struct sockaddr_in udp_remote;
static int port = 9091;


bool wifikb::init()
{
	if (listenfd >= 0) return true;

	keyPresses = new KeyStruct[MAX_KEYPRESSES];
	keyPressCount = 0;

	listenfd = socket(PF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		return false;

	// set non-blocking
	u32 one = 1;
	ioctl(listenfd, FIONBIO, &one);

	return true;
}

void wifikb::update()
{
	if (listenfd < 0) return;

	struct KeyStruct recvdata;
	int dummy;

	int len = recvfrom(listenfd, &recvdata, sizeof(recvdata), 0, (struct sockaddr*)&udp_remote, &dummy);
	if (len < (int)sizeof(recvdata)) return;

	if (recvdata.ndsKeyCode == -1 && recvdata.asciiCode == 65535)
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

bool wifikb::getKey(KeyStruct *recv)
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
