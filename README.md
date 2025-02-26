# ao-nds

![img1](screenshot1.png)
![img2](screenshot2.png)

[Attorney Online](https://aceattorneyonline.com) client for the Nintendo DS.

Made with BlocksDS by Headshotnoby<br/>
Additional code by stonedDiscord<br/>
UI design by Samevi

## Assets and libraries used
* "Igiari Cyrillic" by trtrtrtr, based on "Igiari" font by [Caveras](https://caveras.net)
* "AceName" font by unknown (found on [webAO](https://github.com/AttorneyOnline/webAO))
* [BlocksDS](http://github.com/blocksds/sdk)
* [dswifi](http://github.com/blocksds/dswifi)
* [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h)
  * Using a modified version of stb_truetype that replaces (some, not all) floating point math with fixed-point math, thus providing a speed boost, since the DS doesn't have a FPU
* [libadx-nds](https://github.com/headshot2017/libadx-nds) for music
* [dr_wav](https://github.com/mackron/dr_libs/blob/master/dr_wav.h) for reading WAV audio files
* [rapidjson](https://github.com/Tencent/rapidjson), used when parsing the public server list
* [mINI](https://github.com/metayeti/mINI) for parsing .ini files
* [mongoose](https://github.com/cesanta/mongoose) for WebSocket support
* [utfcpp](https://github.com/nemtrif/utfcpp) for UTF8/16/32 strings
