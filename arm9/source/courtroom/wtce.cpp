#include "courtroom/wtce.h"

#include <nds/dma.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>

#include "global.h"

WTCE::WTCE(int start)
{
	visible = false;
	oamStart = start;

	animMode = -1;
}

WTCE::~WTCE()
{

}

void WTCE::setVisible(bool on)
{
	visible = on;
}

void WTCE::play(const std::string& msg)
{
	//testimony1 - "Witness Testimony"
	//testimony2 - "Cross Examination"
	//judgeruling#0 - "Not Guilty" (since 2.6)
	//judgeruling#1 - "Guilty" (since 2.6)
	//testimony1#1 - Hides the "Testimony" indicator (since 2.9)

	std::string mode = argumentAt(msg, 1);

	// i really don't wanna do this, but then again it's not executed every frame...
	if (mode == "testimony1")
	{
		if (argumentAt(msg, 2) != "1")
			animMode = 0;
		else
		{
			// hide "Testimony" indicator
		}
	}
	else if (mode == "testimony2") animMode = 1;
	else if (mode == "judgeruling") animMode = (argumentAt(msg, 2) == "0") ? 2 : 3;
}

void WTCE::update()
{

}
