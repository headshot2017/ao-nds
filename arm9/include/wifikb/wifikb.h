#ifndef __WIFIKB_DS__
#define __WIFIKB_DS__

#include <string>
#include <nds.h>

namespace wifikb
{
	struct KeyStruct
	{
		s16 ndsKeyCode;
		u16 asciiCode;
	};

	// Init wifikb
	bool init();

	// Call this every frame
	void update();

	// Start/stop listening for keyboard input
	void start();
	void stop();

	// Get a keypress and store it in recv. Returns false if there are no key presses left to read
	bool getKey(KeyStruct *recv);

	// Send a text message to the client
	void send(std::string message);
}

#endif
