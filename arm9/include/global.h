#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

u8* readFile(const std::string& filename, u32* outLen=0);
u32 bmpIndexTo256SpriteIndex(u32 x, u32 y, u32 w, u32 h, SpriteSize size, bool* oobFlag=0);

#endif // GLOBAL_H_INCLUDED
