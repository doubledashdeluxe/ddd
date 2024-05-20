#pragma once

#include "common/Dolphin.hh"

class Platform {
public:
    static bool IsDolphin();
    static bool GetDolphinVersion(Dolphin::Version &dolphinVersion);

private:
    Platform();
};
