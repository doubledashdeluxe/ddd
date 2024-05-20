#pragma once

#include "common/ios/Resource.hh"

class Dolphin : private IOS::Resource {
public:
    struct Version {
        u32 major;
        u32 minor;
        u32 patch;
    };

    Dolphin();
    ~Dolphin();
    bool ok() const;

    bool getVersion(Version &version);

private:
    class Ioctlv {
    public:
        enum {
            GetVersion = 0x2,
        };

    private:
        Ioctlv();
    };
};

bool operator==(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator!=(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator<(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator>(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator<=(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator>=(const Dolphin::Version &a, const Dolphin::Version &b);
