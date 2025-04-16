#pragma once

#include <portable/Array.hh>

struct DolphinVersion {
    static bool Read(const Array<char, 64> &versionString, DolphinVersion &version);

    u32 major;
    u32 minor;
    u32 patch;
};

bool operator==(const DolphinVersion &a, const DolphinVersion &b);
bool operator!=(const DolphinVersion &a, const DolphinVersion &b);
bool operator<(const DolphinVersion &a, const DolphinVersion &b);
bool operator>(const DolphinVersion &a, const DolphinVersion &b);
bool operator<=(const DolphinVersion &a, const DolphinVersion &b);
bool operator>=(const DolphinVersion &a, const DolphinVersion &b);
