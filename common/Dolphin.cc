#include "Dolphin.hh"

#include "common/Array.hh"

Dolphin::Dolphin() : IOS::Resource("/dev/dolphin", IOS::Mode::None, false) {}

Dolphin::~Dolphin() {}

bool Dolphin::ok() const {
    return Resource::ok();
}

bool Dolphin::getVersion(DolphinVersion &version) {
    Array<char, 64> versionString;

    alignas(0x20) IoctlvPair pairs[1];
    pairs[0].data = versionString.values();
    pairs[0].size = versionString.count();

    if (ioctlv(Ioctlv::GetVersion, 0, 1, pairs) != 0) {
        return false;
    }

    return DolphinVersion::Read(versionString, version);
}
