
#ifndef PACKETS_H_INCLUDED
#define PACKETS_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include "courtroom/chatbox.h"
#include "courtroom/courtroom.h"

void handleNetworkPacket(Courtroom& court, std::string cargs);
void handleMS(Courtroom& court, std::string cargs);
void handleMC(Courtroom& court, std::string cargs);

#endif // PACKETS_H_INCLUDED
