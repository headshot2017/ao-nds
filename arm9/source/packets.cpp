#include "packets.h"

int AOcolorToPalette[] = {
	COLOR_WHITE,
	COLOR_GREEN,
	COLOR_RED,
	COLOR_ORANGE,
	COLOR_BLUE,
	COLOR_YELLOW
};

void handleMC(Courtroom& court, std::string sargs) {
	std::string trackname = sargs.substr(0, sargs.find("#"));
	court.playMusic("/data/ao-nds/sounds/music/"+trackname);
}

void handleBN(Courtroom& court, std::string sargs) {
	std::string bgname = sargs.substr(0, sargs.find("#"));
	court.getBackground()->setBg(bgname);
}

std::string argumentAt(std::string s, int id) {
	const std::string del = "#";
	// Use find function to find 1st position of delimiter.
	int end = s.find(del);
	int argi = 0;
	while (end != -1) { // Loop until no delimiter is left in the string.
		std::string currentArg = s.substr(0, end);
		s.erase(s.begin(), s.begin() + end + 1);
		end = s.find(del);
		if(argi==id)
			return currentArg;
		else
			argi++;
	}

	return "";
}

void handleMS(Courtroom& court, std::string s) {
	//iprintf(argumentAt(s,4).c_str()); // the text

	int colorID = std::stoi(argumentAt(s,14));
	if (colorID < 0 || colorID >= 6)
		colorID = 0;

	std::string name = argumentAt(s,15);
	if (name.empty())
		std::string name = argumentAt(s, 2);
	if (name.size() > 10)
		name.resize(10);

	court.getBackground()->setBgSide(argumentAt(s,5));
	court.getChatbox()->setName(name);
	court.getChatbox()->setText(argumentAt(s,4), AOcolorToPalette[colorID]);
	court.getCharacter()->setCharImage(argumentAt(s,2), "(a)"+argumentAt(s,3));
}

void handleNetworkPacket(Courtroom& court, std::string cargs) {
	if(cargs.at(0)=='M') {
		if(cargs.at(1)=='S')
			handleMS(court,cargs.substr(3));
		if(cargs.at(1)=='C')
			handleMC(court,cargs.substr(3));
	} else if(cargs.at(0)=='B') {
		if(cargs.at(1)=='N')
			handleBN(court,cargs.substr(3));
	}
}
