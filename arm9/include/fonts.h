#ifndef FONTS_H_INCLUDED
#define FONTS_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

int initFont(const u8* data, int line_height);
int renderText(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int spriteGfxCount);
int renderChar(int fontID, const char* text, int palIndex, int x, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16* spriteGfxTarget, bool* oobFlag=0, int* outWidth=0);

#endif // FONTS_H_INCLUDED
