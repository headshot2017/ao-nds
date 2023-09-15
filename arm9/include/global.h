#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <vector>
#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

#define fromRGB15(rgb, r, g, b)\
	r = rgb & 31;\
	g = (rgb >> 5) & 31;\
	b = (rgb >> 10) & 31;

typedef void (*voidCallback)(void *pUserData);
typedef void (*stringCallback)(void *pUserData, std::string str);
struct cbInfo
{
	voidCallback cb;
	void* pUserData;
};

std::string argumentAt(const std::string& s, int id);
void fillArguments(std::vector<std::string>& out, std::string& s, int id);
u32 totalArguments(const std::string& s);

bool fileExists(const std::string& filename);
u8* readFile(const std::string& filename, u32* outLen=0);
u32 bmpIndexTo256SpriteIndex(int x, int y, int w, int h, SpriteSize size, bool* oobFlag=0);

#endif // GLOBAL_H_INCLUDED
