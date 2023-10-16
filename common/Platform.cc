#include "Platform.hh"

#include "common/Dolphin.hh"

bool Platform::IsDolphin() {
    // Modern versions
    Dolphin dolphin;
    if (dolphin.ok()) {
        return true;
    }

    // Old versions
    IOS::Resource sha("/dev/sha", IOS::Mode::None);
    if (!sha.ok()) {
        return true;
    }

    return false;
}

bool Platform::GetDolphinVersion(Array<char, 64> &dolphinVersion) {
    Dolphin dolphin;
    if (!dolphin.ok()) {
        return false;
    }

    return dolphin.getVersion(dolphinVersion);
}
