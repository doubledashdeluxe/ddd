#include "DolphinVersion.hh"

extern "C" {
#include <ctype.h>
}

bool DolphinVersion::Read(const Array<char, 64> &versionString, DolphinVersion &version) {
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

bool operator==(const DolphinVersion &a, const DolphinVersion &b) {
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}

bool operator!=(const DolphinVersion &a, const DolphinVersion &b) {
    return !(a == b);
}

bool operator<(const DolphinVersion &a, const DolphinVersion &b) {
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

bool operator>(const DolphinVersion &a, const DolphinVersion &b) {
    return b < a;
}

bool operator<=(const DolphinVersion &a, const DolphinVersion &b) {
    return !(b < a);
}

bool operator>=(const DolphinVersion &a, const DolphinVersion &b) {
    return !(a < b);
}
