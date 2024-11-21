#ifndef __WIFIKB_DS__
#define __WIFIKB_DS__

#include <string>
#include <nds.h>

namespace wifikb
{
	// s32 key format:
	// < 0: NDS keyboard enum. See https://github.com/blocksds/libnds/blob/master/include/nds/arm9/keyboard.h#L94
	// >= 0: ASCII code.


	// Sets reverse connection mode. False by default. Useful for connecting with emulators like melonDS.
	// If you wish to change this setting, call this before init()
	// false: DS acts as server, PC acts as client (sends broadcast)
	// true: DS acts as client (sends broadcast), PC acts as server
	void setReverse(bool on);

	// Init wifikb
	bool init();

	// Call this every frame
	void update();

	// Start/stop listening for keyboard input
	void start();
	void stop();

	// Get a keypress and store it in recv. Returns false if there are no key presses left to read
	bool getKey(s32 *recv);

	// Send a text message to the client
	void send(std::string message);
}

#endif
