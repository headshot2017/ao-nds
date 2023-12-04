#include "courtroom/wtceAnims.h"

#include <nds/arm9/math.h>

#define GUILTY_X 6
#define GUILTY_Y 29

#define NOTGUILTY_X1 -44
#define NOTGUILTY_Y1 29

#define NOTGUILTY_X2 53
#define NOTGUILTY_Y2 29

int linearLerp(int t)
{
	return t;
}

// https://github.com/nicolausYes/easing-functions
// Easing functions modified to use fixed-point math
int easeInCubicFixed(int t)
{
	return mulf32(mulf32(t, t), t);
}

int easeOutCubicFixed(int t)
{
	t -= inttof32(1);
	return inttof32(1) + mulf32(mulf32(t, t), t);
}

keyFrameMap anim_testimony1 = {
	{0, {
		{KEYFRAME_MOVE, {.move={0, {32,16}, {5,3}, {-160, 48-20}, {128-(160/2), 48-20}, 15, linearLerp }}}, // move "Witness" from left to center
		{KEYFRAME_MOVE, {.move={5*3, {32,16}, {6,3}, {256, 48+20}, {128-(192/2), 48+20}, 15, linearLerp }}}, // move "Testimony" from right to center
		{KEYFRAME_SFX},
	}},

	{15, {
		{KEYFRAME_FLASH, {.ticks=3}},
	}},

	{18, {
		{KEYFRAME_IMGFRAME, {.imgFrame={5*3, {32,16}, {6,3}, 1, 1}}},
	}},
	{23, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {5,3}, 0, 1}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={5*3, {32,16}, {6,3}, 1, 2}}},
	}},
	{28, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {5,3}, 0, 2}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={5*3, {32,16}, {6,3}, 1, 3}}},
	}},
	{33, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {5,3}, 0, 3}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={5*3, {32,16}, {6,3}, 1, 4}}},
	}},
	{38, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {5,3}, 0, 0}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={5*3, {32,16}, {6,3}, 1, 0}}},
	}},

	{43, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 6}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 11}}},
	}},
	{49, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 7}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 12}}},
	}},
	{55, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 8}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 13}}},
	}},
	{61, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 9}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 14}}},
	}},
	{67, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 8}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 13}}},
	}},
	{73, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 7}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 12}}},
	}},
	{79, {
		{KEYFRAME_PALETTE, {.palette={0, 5*3, 6}}},
		{KEYFRAME_PALETTE, {.palette={5*3, 6*3, 11}}},
	}},

	{105, {
		{KEYFRAME_MOVE, {.move={0, {32,16}, {5,3}, {128-(160/2), 48-20}, {256, 48-20}, 25, easeInCubicFixed }}}, // move "Witness" from center to right
		{KEYFRAME_MOVE, {.move={5*3, {32,16}, {6,3}, {128-(192/2), 48+20}, {-192, 48+20}, 25, easeInCubicFixed }}}, // move "Testimony" from center to left
	}},

	{140, {
		{KEYFRAME_END}
	}}
};

keyFrameMap anim_testimony2 = {
	{0, {
		{KEYFRAME_MOVE, {.move={0, {32,16}, {4,3}, {-128, 48-20}, {128-(128/2), 48-20}, 15, linearLerp }}}, // move "Cross" from left to center
		{KEYFRAME_MOVE, {.move={4*3, {32,16}, {6,3}, {256, 48+20}, {128-(192/2), 48+20}, 15, linearLerp }}}, // move "Examination" from right to center
		{KEYFRAME_SFX},
	}},

	{15, {
		{KEYFRAME_FLASH, {.ticks=3}},
	}},

	{18, {
		{KEYFRAME_IMGFRAME, {.imgFrame={4*3, {32,16}, {6,3}, 1, 1}}},
	}},
	{23, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {4,3}, 0, 1}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={4*3, {32,16}, {6,3}, 1, 2}}},
	}},
	{28, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {4,3}, 0, 2}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={4*3, {32,16}, {6,3}, 1, 3}}},
	}},
	{33, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {4,3}, 0, 3}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={4*3, {32,16}, {6,3}, 1, 4}}},
	}},
	{38, {
		{KEYFRAME_IMGFRAME, {.imgFrame={0, {32,16}, {4,3}, 0, 0}}},
		{KEYFRAME_IMGFRAME, {.imgFrame={4*3, {32,16}, {6,3}, 1, 0}}},
	}},

	{43, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 6}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 11}}},
	}},
	{49, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 7}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 12}}},
	}},
	{55, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 8}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 13}}},
	}},
	{61, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 9}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 14}}},
	}},
	{67, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 8}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 13}}},
	}},
	{73, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 7}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 12}}},
	}},
	{79, {
		{KEYFRAME_PALETTE, {.palette={0, 4*3, 6}}},
		{KEYFRAME_PALETTE, {.palette={4*3, 6*3, 11}}},
	}},

	{105, {
		{KEYFRAME_MOVE, {.move={0, {32,16}, {4,3}, {128-(128/2), 48-20}, {128-(128/2), -40}, 25, easeInCubicFixed }}}, // move "Cross" from center to top
		{KEYFRAME_MOVE, {.move={4*3, {32,16}, {6,3}, {128-(192/2), 48+20}, {128-(192/2), 192}, 25, easeInCubicFixed }}}, // move "Examination" from center to bottom
	}},

	{140, {
		{KEYFRAME_END}
	}}
};

keyFrameMap anim_notGuilty = {
	{0, { // hide "Guilty" and scale "Not"
		{KEYFRAME_VISIBLE, {.visible={3, 6, false}}},
		{KEYFRAME_MOVE, {.move={0, {64,64}, {1,1}, {NOTGUILTY_X1,NOTGUILTY_Y1}, {NOTGUILTY_X1,NOTGUILTY_Y1}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={1, {64,64}, {1,1}, {NOTGUILTY_X1+33,NOTGUILTY_Y1+1}, {NOTGUILTY_X1+33,NOTGUILTY_Y1+1}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={2, {64,64}, {1,1}, {NOTGUILTY_X1+58,NOTGUILTY_Y1}, {NOTGUILTY_X1+58,NOTGUILTY_Y1}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={0, 140, 256, 10, 1, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={1, 140, 256, 10, 1, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={2, 140, 256, 10, 1, linearLerp}}},
	}},

	{10, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{60, { // show "Guilty" and scale
		{KEYFRAME_VISIBLE, {.visible={3, 6, true}}},
		{KEYFRAME_MOVE, {.move={3, {64,64}, {1,1}, {NOTGUILTY_X2,NOTGUILTY_Y2}, {NOTGUILTY_X2,NOTGUILTY_Y2}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={4, {64,64}, {1,1}, {NOTGUILTY_X2+33,NOTGUILTY_Y2}, {NOTGUILTY_X2+33,NOTGUILTY_Y2}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={5, {64,64}, {1,1}, {NOTGUILTY_X2+58,NOTGUILTY_Y2}, {NOTGUILTY_X2+58,NOTGUILTY_Y2}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={6, {64,64}, {1,1}, {NOTGUILTY_X2+76,NOTGUILTY_Y2}, {NOTGUILTY_X2+76,NOTGUILTY_Y2}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={7, {64,64}, {1,1}, {NOTGUILTY_X2+95,NOTGUILTY_Y2}, {NOTGUILTY_X2+95,NOTGUILTY_Y2}, 1, linearLerp }}},
		{KEYFRAME_MOVE, {.move={8, {64,64}, {1,1}, {NOTGUILTY_X2+122,NOTGUILTY_Y2+11}, {NOTGUILTY_X2+122,NOTGUILTY_Y2+11}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={3, 140, 256, 10, 2, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={4, 140, 256, 10, 2, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={5, 140, 256, 10, 2, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={6, 140, 256, 10, 2, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={7, 140, 256, 10, 2, linearLerp}}},
		{KEYFRAME_SCALE, {.scale={8, 140, 256, 10, 2, linearLerp}}},
	}},

	{70, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{172, {
		{KEYFRAME_END}
	}}
};

keyFrameMap anim_guilty = {
	{0, {
		{KEYFRAME_VISIBLE, {.visible={1, 5, false}}}, // hide "uilty" and move "G"
		{KEYFRAME_MOVE, {.move={0, {64,64}, {1,1}, {GUILTY_X,GUILTY_Y}, {GUILTY_X,GUILTY_Y}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={0, 160, 256, 8, 1, linearLerp}}},
	}},
	{8, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{10, {
		{KEYFRAME_VISIBLE, {.visible={1, 1, true}}}, // show "u"
		{KEYFRAME_MOVE, {.move={1, {64,64}, {1,1}, {GUILTY_X+33,GUILTY_Y}, {GUILTY_X+33,GUILTY_Y}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={1, 160, 256, 8, 2, linearLerp}}},
	}},
	{18, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{20, {
		{KEYFRAME_VISIBLE, {.visible={2, 1, true}}}, // show "i"
		{KEYFRAME_MOVE, {.move={2, {64,64}, {1,1}, {GUILTY_X+58,GUILTY_Y}, {GUILTY_X+58,GUILTY_Y}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={2, 160, 256, 8, 3, linearLerp}}},
	}},
	{28, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{30, {
		{KEYFRAME_VISIBLE, {.visible={3, 1, true}}}, // show "l"
		{KEYFRAME_MOVE, {.move={3, {64,64}, {1,1}, {GUILTY_X+76,GUILTY_Y}, {GUILTY_X+76,GUILTY_Y}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={3, 160, 256, 8, 4, linearLerp}}},
	}},
	{38, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{40, {
		{KEYFRAME_VISIBLE, {.visible={4, 1, true}}}, // show "t"
		{KEYFRAME_MOVE, {.move={4, {64,64}, {1,1}, {GUILTY_X+95,GUILTY_Y}, {GUILTY_X+95,GUILTY_Y}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={4, 160, 256, 8, 5, linearLerp}}},
	}},
	{48, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{50, {
		{KEYFRAME_VISIBLE, {.visible={5, 1, true}}}, // show "y"
		{KEYFRAME_MOVE, {.move={5, {64,64}, {1,1}, {GUILTY_X+122,GUILTY_Y+11}, {GUILTY_X+122,GUILTY_Y+11}, 1, linearLerp }}},
		{KEYFRAME_SCALE, {.scale={5, 160, 256, 8, 6, linearLerp}}},
	}},
	{58, {
		{KEYFRAME_SFX},
		{KEYFRAME_SHAKE, {.ticks=2}},
	}},

	{160, {
		{KEYFRAME_END}
	}}
};

std::unordered_map<std::string, keyFrameMap> wtceAnims = {
	{"testimony1", anim_testimony1},
	{"testimony2", anim_testimony2},
	{"judgeruling0", anim_notGuilty},
	{"judgeruling1", anim_guilty},
};
