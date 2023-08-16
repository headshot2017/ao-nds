#include "packets.h"

void handleNetworkPacket(Courtroom& court, std::string cargs) {
    if(cargs.at(0)=='M') {
        if(cargs.at(1)=='S')
            handleMS(court,cargs.substr(3));
        if(cargs.at(1)=='C')
            handleMC(court,cargs.substr(3));
    }
}

void handleMC(Courtroom& court, std::string sargs) {
    std::string trackname = sargs.substr(0, sargs.find("#"));
    court.playMusic(trackname);
}

void handleMS(Courtroom& court, std::string sargs) {
    //std::string
}