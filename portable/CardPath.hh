#pragma once

#include <portable/Array.hh>

class CardPath {
public:
    static Array<char, 256> Get(const char *dirPath, const char fileName[32],
            const char gameName[4], const char company[2]);

private:
    CardPath();
};
