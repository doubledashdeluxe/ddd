#include "Platform.hh"

#include "common/ios/Resource.hh"

bool Platform::IsDolphin() {
    // Modern versions
    IOS::Resource dolphin("/dev/dolphin");
    if (dolphin.ok()) {
        return true;
    }

    // Old versions
    IOS::Resource sha("/dev/sha");
    if (!sha.ok()) {
        return true;
    }

    return false;
}
