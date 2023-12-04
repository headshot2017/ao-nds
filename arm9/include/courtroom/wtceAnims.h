#ifndef WTCEANIMS_H_INCLUDED
#define WTCEANIMS_H_INCLUDED

#include <unordered_map>
#include <vector>
#include <string>

typedef int (*LerpCallback)(int t);

enum
{
	KEYFRAME_MOVE,
	KEYFRAME_SCALE,
	KEYFRAME_SHAKE,
	KEYFRAME_FLASH,
	KEYFRAME_IMGFRAME,
	KEYFRAME_VISIBLE,
	KEYFRAME_PALETTE,
	KEYFRAME_SFX,
	KEYFRAME_END,
};

struct vec2
{
	int x;
	int y;
};

struct keyFrame
{
	int type;
	union
	{
		int ticks; // shake or flash

		struct
		{
			int oamIndex;
			vec2 spriteSize;
			vec2 tileSize;
			int gfxIndex;
			int frame;
		} imgFrame;

		struct
		{
			int oamIndex;
			int max;
			bool on;
		} visible;

		struct
		{
			int oamIndex;
			int max;
			int paletteInd;
		} palette;

		struct
		{
			int oamIndex;
			vec2 spriteSize;
			vec2 tileSize;
			vec2 from;
			vec2 to;
			int tickLength;
			LerpCallback lerpFunc;
		} move;

		struct
		{
			int oamIndex;
			int from;
			int to;
			int tickLength;
			int affineIndex;
			LerpCallback lerpFunc;
		} scale;
	};
};

typedef std::unordered_map<int, std::vector<keyFrame> > keyFrameMap;
extern std::unordered_map<std::string, keyFrameMap> wtceAnims;

#endif // WTCEANIMS_H_INCLUDED
