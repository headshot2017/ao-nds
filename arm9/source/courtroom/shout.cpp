#include "courtroom/shout.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "global.h"

Shout::Shout()
{
	bgIndex = bgInit(2, BgType_Text8bpp, BgSize_T_512x256, 14, 5);
	visible = false;

	xOffset = 0;
	yOffset = 0;

	sndShout = 0;
	sndShoutSize = 0;
}

Shout::~Shout()
{
	bgHide(bgIndex);
	freeSound();
}

void Shout::setShout(const std::string& name)
{

}

void Shout::setVisible(bool on)
{
	visible = on;
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Shout::freeSound()
{
	if (sndShout)
	{
		delete[] sndShout;
		sndShout = 0;
	}
}

void Shout::update()
{

}
