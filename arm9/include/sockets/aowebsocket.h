#ifndef AOWEBSOCKET_H_INCLUDED
#define AOWEBSOCKET_H_INCLUDED

#include "aosocket.h"
#include "mongoose.h"

class AOwebSocket : public AOsocket
{
	struct mg_mgr mgr;        // Event manager
	struct mg_connection *c;

	static void wsHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

public:
	AOwebSocket();
	~AOwebSocket();

	void update();
	void connectIP(std::string ip);
	void disconnect();
	void sendData(std::string data);
};

#endif // AOWEBSOCKET_H_INCLUDED
