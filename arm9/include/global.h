#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <vector>
#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>

// SoundT values in char.ini must be multiplied by this constant to get the actual time
#define TIME_MOD 40

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

void debugPressA(const char* msg);
void debugLabelPressA(const char* msg);

void AOdecode(std::string& s);
void AOdecode(std::u16string& s);
void AOencode(std::string& s);
std::string argumentAt(const std::string& s, int id, char delimiter='#');
void fillArguments(std::vector<std::string>& out, std::string& s, int id, char delimiter='#');
u32 totalArguments(const std::string& s, char delimiter='#');

bool fileExists(const std::string& filename);
u8* readFile(const std::string& filename, u32* outLen=0, const char* mode="rb");
u32 bmpIndexTo256SpriteIndex(int x, int y, int w, int h, SpriteSize size, bool* oobFlag=0);

void readAndDecompressLZ77Stream(const char* filename, u8* dest);

#endif // GLOBAL_H_INCLUDED
