#ifndef EVIDENCE_H_INCLUDED
#define EVIDENCE_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>

#include "mp3_shared.h"

class Evidence
{
	bool visible;

	int offsetX;
	int offsetY;

	// oamRotateScale(&oamMain, 0, 0, scale, scale)
	// original scale: 256, smallest scale: 8192
	// add amount: 496

	bool right;
	int scale;
	int scaleAdd;

	std::string nextEvidence;
	bool nextEvidenceRight;

	u16* spriteGfx;

public:
	Evidence();
	~Evidence();

	void setOffsets(int x, int y) {offsetX = x; offsetY = y;}
	void setVisible(bool on);

	void hideEvidence();
	void showEvidence(const std::string& image, bool rightSide);
	void update();


	wav_handle* sndEvShow;
};

#endif // EVIDENCE_H_INCLUDED
