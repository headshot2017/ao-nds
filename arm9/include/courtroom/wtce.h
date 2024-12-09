#ifndef WTCE_H_INCLUDED
#define WTCE_H_INCLUDED

#include <nds/ndstypes.h>

#include "wtceAnims.h"
#include "wav_nds.h"

class Courtroom;

class WTCE
{
	Courtroom* m_pCourt;

	bool visible;
	int oamStart;
	u16** gfx;
	int gfxInUse;
	wav_handle* sfx;

	u16* spr_testimonyIndicator;
	int indicatorTicks;

	u8* imgGfx[2];
	u8* imgPal[2];

	int currFrame;
	std::string currAnim;

	struct TickLerp
	{
		int oamIndex;
		vec2 spriteSize;
		vec2 tileSize;
		vec2 from;
		vec2 to;
		int ticks;
		int maxTicks;
		int affineIndex;
		LerpCallback lerpFunc;
	};
	std::vector<TickLerp> allLerps;

public:
	WTCE(Courtroom* pCourt, int start);
	~WTCE();

	void setVisible(bool on);

	void clear(bool keepIndicator=false);
	void play(const std::string& msg);

	void update();
};

#endif // WTCE_H_INCLUDED
