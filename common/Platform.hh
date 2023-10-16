#pragma once

#include "common/Array.hh"

class Platform {
public:
    static bool IsDolphin();
    static bool GetDolphinVersion(Array<char, 64> &dolphinVersion);

private:
    Platform();
};
