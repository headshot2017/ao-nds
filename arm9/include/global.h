#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#define AO_DATA_DIR "/data/ao-nds"

#include <nds/ndstypes.h>
#include <string>

u8* readFile(const std::string& filename, u32* outLen=0);

#endif // GLOBAL_H_INCLUDED
