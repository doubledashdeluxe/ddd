#include "Dolphin.hh"

Dolphin::Dolphin() : IOS::Resource("/dev/dolphin", IOS::Mode::None, false) {}

Dolphin::~Dolphin() {}

bool Dolphin::ok() const {
    return Resource::ok();
}

bool Dolphin::getVersion(Array<char, 64> &version) {
    alignas(0x20) IoctlvPair pairs[1];
    pairs[0].data = version.values();
    pairs[0].size = version.count();

    if (ioctlv(Ioctlv::GetVersion, 0, 1, pairs) != 0) {
        return false;
    }

    if (version[version.count() - 1] != '\0') {
        return false;
    }

    return true;
}
