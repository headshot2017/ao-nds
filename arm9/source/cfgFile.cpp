#include "cfgFile.h"

#include <iostream>
#include <fstream>
#include <algorithm>

cfgFile::cfgFile(const std::string& filename)
{
    load(filename);
}

std::string cfgFile::get(const std::string& key, std::string defaultValue)
{
    return (!keys.count(key)) ? defaultValue : keys[key];
}

void cfgFile::set(const std::string& key, const std::string& value)
{
    keys[key] = value;
}

void cfgFile::save(const std::string& filename)
{
    std::ofstream configfile(filename);
    for (const auto& mapkey : keys)
        configfile << mapkey.first << ": " << mapkey.second << "\n";
}

bool cfgFile::load(const std::string& filename)
{
    std::ifstream configfile(filename);
    if (!configfile.is_open())
        return false;

    keys.clear();

    std::string line;
    while (getline(configfile, line))
    {
        if (line[0] == '#' || line.empty())
            continue; // ignore comments and empty lines

        std::size_t delimiterPos = line.find(": ");
        if (delimiterPos == std::string::npos)
            continue; // invalid config syntax

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 2);
        while (value.at(value.size()-1) == '\r' || value.at(value.size()-1) == '\n') value.erase(value.size()-1);
        keys[key] = value;
    }

    return true;
}
