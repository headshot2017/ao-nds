#ifndef WTCE_H_INCLUDED
#define WTCE_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>

#include "mp3_shared.h"

class WTCE
{
	bool visible;
	int oamStart;

	int offsetX;
	int offsetY;

	int animMode;

public:
	WTCE(int start);
	~WTCE();

	void setOffsets(int x, int y) {offsetX = x; offsetY = y;}
	void setVisible(bool on);

	void play(const std::string& msg);

	void update();
};

#endif // WTCE_H_INCLUDED
