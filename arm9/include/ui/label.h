#ifndef LABEL_H_INCLUDED
#define LABEL_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

class UILabel
{
	u16** textGfx;
	int gfxPerLine;
	int maxLines;
	int lineOffset;
	u8* textCanvas;

	OamState* oam;
	int oamStart;
	int paletteSlot;

	std::u16string currText;
	int fontID;

public:
	UILabel(OamState* chosenOam, int oamStartInd, int perLine, int lines, u32 textColor, int palSlot, int font);
	~UILabel();

	void setVisible(bool on);
	void setPos(int x, int y, bool center=false);
	void setText(const std::string& text);
	void setText(const std::u16string& text);
	void setTextOnLine(const std::string& text, int line);
	void setTextOnLine(const std::u16string& text, int line);
	void setColor(u32 textColor);
	void setLineOffset(int offset) {lineOffset = offset;}

	u16** getGfx() {return textGfx;}
	int getOamInd() {return oamStart;}
	int nextOamInd() {return oamStart+(gfxPerLine*maxLines);}
};

#endif // LABEL_H_INCLUDED
