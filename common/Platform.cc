#include "Platform.hh"

#include "common/ios/Resource.hh"

bool Platform::IsDolphin() {
    // Modern versions
    IOS::Resource dolphin("/dev/dolphin", IOS::Mode::None);
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
