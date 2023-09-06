#ifndef FONTS_H_INCLUDED
#define FONTS_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

int initFont(const u8* data, int line_height);
int renderText(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int spriteGfxCount);
int renderChar(int fontID, const char* text, int palIndex, int x, int spriteW, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16* spriteGfxTarget, bool skipOnOob=false, int* oobFlag=0, int* outWidth=0);
void renderMultiLine(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int gfxPerLine, int maxLines);
int getTextWidth(int fontID, const char* text);

#endif // FONTS_H_INCLUDED
