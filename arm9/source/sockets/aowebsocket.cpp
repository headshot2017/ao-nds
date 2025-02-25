#include "sockets/aowebsocket.h"
#include "global.h"

AOwebSocket::AOwebSocket() : AOsocket()
{
	mg_log_set(MG_LL_ERROR);  // Set log level
	c = 0;
}

AOwebSocket::~AOwebSocket()
{
	closesocket((int)c->fd);
	mg_mgr_free(&mgr);
}

void AOwebSocket::wsHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
	AOwebSocket* pSelf = (AOwebSocket*)fn_data;

	switch(ev)
	{
		case MG_EV_OPEN:
			c->is_hexdumping = 0;
			break;

		case MG_EV_ERROR:
			MG_ERROR(("%p %s", c->fd, (char *) ev_data));
		case MG_EV_CLOSE:
			pSelf->disconnect();
			break;

		case MG_EV_WS_MSG:
			struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
			printf("S: [%.*s]\n", (int) wm->data.len, wm->data.ptr);

			// loop through every percent %
			std::string tempData(wm->data.ptr);
			std::size_t lastPos = 0;
			std::size_t delimiterPos = tempData.find("%");

			while (delimiterPos != std::string::npos)
			{
				const std::string& data = tempData.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1);
				std::string header = argumentAt(data, 0);
				if (pSelf->callbacks.count(header))
				{
					for (const NetCBInfo& cbInfo : pSelf->callbacks[header])
						cbInfo.cb(cbInfo.pUserData, data);
				}

				lastPos = delimiterPos;
				delimiterPos = tempData.find("%", delimiterPos+1);
			}

			break;
	}
}

void AOwebSocket::setIP(std::string ip)
{
	if (connected) return;
	m_IP = ip;
}

void AOwebSocket::connect()
{
	if (connected) return;

	mg_mgr_init(&mgr);        // Initialise event manager
	c = mg_ws_connect(&mgr, m_IP.c_str(), wsHandler, this, NULL);     // Create client
	connected = true;
	ticks = 0;
}

void AOwebSocket::disconnect()
{
	if (!connected) return;

	c->is_draining = 1;

	connected = false;
}

void AOwebSocket::sendData(std::string data)
{
	if (!connected) return;

	mg_ws_send(c, data.c_str(), data.size(), WEBSOCKET_OP_TEXT);
}

void AOwebSocket::update()
{
	if (!connected) return;

	ticks++;
	if (ticks % 600 == 0)
		sendData("CH#%");

	mg_mgr_poll(&mgr, 0);
}
