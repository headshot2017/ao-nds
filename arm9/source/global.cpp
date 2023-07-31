#include "global.h"

#include <stdio.h>

u8* readFile(const std::string& filename, u32* outLen)
{
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
    {
        if (outLen) *outLen = 0;
        return 0;
    }

    fseek(f, 0, SEEK_END);
    u32 len = ftell(f);
    if (outLen) *outLen = len;
    fseek(f, 0, SEEK_SET);

    u8* data = new u8[len];
    fread(data, len, 1, f);
    fclose(f);

    return data;
}
