#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

bool fileExists(const std::string& filename);
u8* readFile(const std::string& filename, u32* outLen=0);
u32 bmpIndexTo256SpriteIndex(int x, int y, int w, int h, SpriteSize size, bool* oobFlag=0);

#endif // GLOBAL_H_INCLUDED
