#ifndef FONTS_H_INCLUDED
#define FONTS_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

int initFont(const u8* data, int line_height);
int renderFont(int fontID, const char* text, int palIndex, int w, int h, u8* bmpTarget, SpriteSize spritesize, u16** spriteGfxTargets, int spriteGfxCount);

#endif // FONTS_H_INCLUDED
