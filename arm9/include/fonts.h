#ifndef FONTS_H_INCLUDED
#define FONTS_H_INCLUDED

#include <vector>
#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

int initFont(const u8* data, int line_height);
int renderText(int fontID, std::u16string& text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int spriteGfxCount);
int renderChar(int fontID, int codepoint, int nextChar, int palIndex, int x, int spriteW, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16* spriteGfxTarget, bool skipOnOob=false, int* oobFlag=0, int* outWidth=0);
void renderMultiLine(int fontID, std::u16string& text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int gfxPerLine, int maxLines);
void separateLines(int fontID, std::u16string& text, int gfxPerLine, bool chatbox, std::vector<std::u16string>& out);
int getTextWidth(int fontID, std::u16string& text, int maxWidth=0);

#endif // FONTS_H_INCLUDED
