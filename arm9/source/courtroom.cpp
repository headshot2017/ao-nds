#include "courtroom.h"

#include <dirent.h>

#include <nds/arm9/background.h>
#include <nds/arm9/decompress.h>
#include <nds/arm9/sprite.h>

#include "global.h"

std::unordered_map<std::string, std::string> sideToDesk = {
    {"def", "defensedesk"},
    {"pro", "prosecutiondesk"},
    {"wit", "stand"},
    {"jud", "judgedesk"}
};

void readDeskTiles(const std::string& value, int* horizontal, int* vertical)
{
    std::size_t delimiterPos = value.find(",");
    if (delimiterPos == std::string::npos)
    {
        *horizontal = 0;
        *vertical = 0;
        return;
    }

    *horizontal = std::stoi(value.substr(0, delimiterPos));
    *vertical = std::stoi(value.substr(delimiterPos + 1));
}


Courtroom::Courtroom()
{
    bgIndex = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgHide(bgIndex);

    for (int i=0; i<4*6; i++)
    {
        int x = (i%4) * 64;
        int y = (i/4) * 32;

        deskGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
        oamSet(&oamMain, i, x, y, 0, 0, SpriteSize_64x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
    }
}

bool Courtroom::setBg(const std::string& name)
{
    std::string bgPath = AO_DATA_DIR "/background/";
    bgPath += name;

    DIR* dir = opendir(bgPath.c_str());
    if (!dir) return false;
    closedir(dir);

    if (!deskTiles.load(bgPath + "/desk_tiles.cfg"))
        return false;

    if (!currentBg.empty())
    {
        delete[] currentBg["def"].mainBg.data;
        delete[] currentBg["pro"].mainBg.data;
        delete[] currentBg["wit"].mainBg.data;
        delete[] currentBg["hld"].mainBg.data;
        delete[] currentBg["hlp"].mainBg.data;
        delete[] currentBg["jud"].mainBg.data;

        if (currentBg["def"].deskBg.data)
        {
            delete[] currentBg["def"].deskBg.data;
            delete[] currentBg["def"].deskPal.data;
        }
        if (currentBg["pro"].deskBg.data)
        {
            delete[] currentBg["pro"].deskBg.data;
            delete[] currentBg["pro"].deskPal.data;
        }
        if (currentBg["wit"].deskBg.data)
        {
            delete[] currentBg["wit"].deskBg.data;
            delete[] currentBg["wit"].deskPal.data;
        }
        if (currentBg["jud"].deskBg.data)
        {
            delete[] currentBg["jud"].deskBg.data;
            delete[] currentBg["jud"].deskPal.data;
        }
    }

    currentBg["def"].mainBg.data = readFile(bgPath + "/defenseempty.img.bin",      &currentBg["def"].mainBg.len);
    currentBg["pro"].mainBg.data = readFile(bgPath + "/prosecutorempty.img.bin",   &currentBg["pro"].mainBg.len);
    currentBg["wit"].mainBg.data = readFile(bgPath + "/witnessempty.img.bin",      &currentBg["wit"].mainBg.len);
    currentBg["hld"].mainBg.data = readFile(bgPath + "/helperstand.img.bin",       &currentBg["hld"].mainBg.len);
    currentBg["hlp"].mainBg.data = readFile(bgPath + "/prohelperstand.img.bin",    &currentBg["hlp"].mainBg.len);
    currentBg["jud"].mainBg.data = readFile(bgPath + "/judgestand.img.bin",        &currentBg["jud"].mainBg.len);

    currentBg["def"].deskBg.data = readFile(bgPath + "/defensedesk.img.bin",       &currentBg["def"].deskBg.len);
    currentBg["pro"].deskBg.data = readFile(bgPath + "/prosecutiondesk.img.bin",   &currentBg["pro"].deskBg.len);
    currentBg["wit"].deskBg.data = readFile(bgPath + "/stand.img.bin",             &currentBg["wit"].deskBg.len);
    currentBg["jud"].deskBg.data = readFile(bgPath + "/judgedesk.img.bin",         &currentBg["jud"].deskBg.len);
    currentBg["hld"].deskBg.data = 0;
    currentBg["hlp"].deskBg.data = 0;

    currentBg["def"].deskPal.data = readFile(bgPath + "/defensedesk.pal.bin",      &currentBg["def"].deskPal.len);
    currentBg["pro"].deskPal.data = readFile(bgPath + "/prosecutiondesk.pal.bin",  &currentBg["pro"].deskPal.len);
    currentBg["wit"].deskPal.data = readFile(bgPath + "/stand.pal.bin",            &currentBg["wit"].deskPal.len);
    currentBg["jud"].deskPal.data = readFile(bgPath + "/judgedesk.pal.bin",        &currentBg["jud"].deskPal.len);
    currentBg["hld"].deskPal.data = 0;
    currentBg["hlp"].deskPal.data = 0;

    currentSide.clear();
    setBgSide("def", true);

    return true;
}

void Courtroom::setBgSide(const std::string& side, bool force)
{
    if (!currentBg.count(side) || (!force && side == currentSide))
        return;

    // copy main background
    decompress(currentBg[side].mainBg.data, bgGetGfxPtr(bgIndex), LZ77Vram);
    //dmaCopy(currentBg[side].mainBg.data, bgGetGfxPtr(bgIndex), currentBg[side].mainBg.len);

    // handle desk sprite
    int horTiles, verTiles;
    if (!currentSide.empty())
    {
        readDeskTiles(deskTiles.get(sideToDesk[currentSide]), &horTiles, &verTiles);

        // make the old ones blank first
        for (int y=0; y<verTiles; y++)
        {
            int yScreen = 6-verTiles+y;

            for (int x=0; x<horTiles; x++)
            {
                int iScreen = yScreen*4+x;

                oamSetHidden(&oamMain, iScreen, true);
            }
        }
    }

    if (currentBg[side].deskBg.data)
    {
        readDeskTiles(deskTiles.get(sideToDesk[side]), &horTiles, &verTiles);

        for (int y=0; y<verTiles; y++)
        {
            int yScreen = 6-verTiles+y;

            for (int x=0; x<horTiles; x++)
            {
                int iScreen = yScreen*4+x;
                int i = y*4+x;

                // copy specific 64x32 tile from image data
                u8* offset = currentBg[side].deskBg.data + i * 64*32;
                dmaCopy(offset, deskGfx[iScreen], 64*32);
                dmaCopy(currentBg[side].deskPal.data, SPRITE_PALETTE, currentBg[side].deskPal.len); // copy palette
                oamSetHidden(&oamMain, iScreen, false);
            }
        }
    }

    currentSide = side;
}

void Courtroom::setVisible(bool on)
{
    (on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Courtroom::update()
{
    bgUpdate();

    for (int i=0; i<4*6; i++)
    {
        int x = (i%4) * 64;
        int y = (i/4) * 32;

        oamSet(&oamMain, i, x, y, 0, 0, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, oamMain.oamMemory[i].isHidden, false, false, false);
    }
    oamUpdate(&oamMain);
}
