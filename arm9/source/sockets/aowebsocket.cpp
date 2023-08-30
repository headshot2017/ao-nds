#include "sockets/aowebsocket.h"
#include "global.h"

AOwebSocket::AOwebSocket() : AOsocket()
{
	mg_log_set(MG_LL_ERROR);  // Set log level
	c = 0;
}

AOwebSocket::~AOwebSocket()
{
	mg_mgr_free(&mgr);
}

void AOwebSocket::wsHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
	AOwebSocket* pSelf = (AOwebSocket*)fn_data;
	std::string header;

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
			header = argumentAt(wm->data.ptr, 0);
			iprintf("S: [%.*s]\n", (int) wm->data.len, wm->data.ptr);

			if (pSelf->callbacks.count(header))
				pSelf->callbacks[header].cb(pSelf->callbacks[header].pUserData, wm->data.ptr);
			break;
	}
}

void AOwebSocket::connectIP(std::string ip)
{
	if (connected) return;

	mg_mgr_init(&mgr);        // Initialise event manager
	c = mg_ws_connect(&mgr, ip.c_str(), wsHandler, this, NULL);     // Create client
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
