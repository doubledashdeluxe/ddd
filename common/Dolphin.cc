#include "Dolphin.hh"

#include "common/Array.hh"

extern "C" {
#include <ctype.h>
}

Dolphin::Dolphin() : IOS::Resource("/dev/dolphin", IOS::Mode::None, false) {}

Dolphin::~Dolphin() {}

bool Dolphin::ok() const {
    return Resource::ok();
}

bool Dolphin::getVersion(Version &version) {
    Array<char, 64> versionString;

    alignas(0x20) IoctlvPair pairs[1];
    pairs[0].data = versionString.values();
    pairs[0].size = versionString.count();

    if (ioctlv(Ioctlv::GetVersion, 0, 1, pairs) != 0) {
        return false;
    }

    if (versionString[versionString.count() - 1] != '\0') {
        return false;
    }

    const char *c = versionString.values();
    Array<u32 *, 3> parts;
    parts[0] = &version.major;
    parts[1] = &version.minor;
    parts[2] = &version.patch;
    for (u32 i = 0; i < parts.count(); i++) {
        while (*c && !isdigit(*c)) {
            c++;
        }
        *parts[i] = 0;
        while (isdigit(*c)) {
            *parts[i] = *parts[i] * 10 + (*c - '0');
            c++;
        }
    }

    return true;
}

bool operator==(const Dolphin::Version &a, const Dolphin::Version &b) {
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}

bool operator!=(const Dolphin::Version &a, const Dolphin::Version &b) {
    return !(a == b);
}

bool operator<(const Dolphin::Version &a, const Dolphin::Version &b) {
    if (a.major != b.major) {
        return a.major < b.major;
    }

    if (a.minor != b.minor) {
        return a.minor < b.minor;
    }

    if (a.patch != b.patch) {
        return a.patch < b.patch;
    }

    return false;
}

bool operator>(const Dolphin::Version &a, const Dolphin::Version &b) {
    return b < a;
}

bool operator<=(const Dolphin::Version &a, const Dolphin::Version &b) {
    return !(b < a);
}

bool operator>=(const Dolphin::Version &a, const Dolphin::Version &b) {
    return !(a < b);
}
