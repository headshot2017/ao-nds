#ifndef CFGFILE_H_INCLUDED
#define CFGFILE_H_INCLUDED

/*
    simple CFG file parser:

    # cfg file comment
    key1: value1
    key2: value2

    etc...
*/

#include <string>
#include <unordered_map>

class cfgFile
{
    std::unordered_map<std::string, std::string> keys;

public:
    cfgFile() {}
    cfgFile(const std::string& filename);

    std::string get(const std::string& key, std::string defaultValue="");
    void set(const std::string& key, std::string& value);

    void save(const std::string& filename);
    bool load(const std::string& filename);

    void clear() {keys.clear();}
};

#endif // CFGFILE_H_INCLUDED
