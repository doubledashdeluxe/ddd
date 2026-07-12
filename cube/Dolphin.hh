#pragma once

#include "cube/ios/Resource.hh"

#include <portable/DolphinVersion.hh>

class Dolphin : private IOS::Resource {
public:
    Dolphin();
    ~Dolphin();
    bool ok() const;

    bool getVersion(Array<char, 64> &versionString, DolphinVersion &version);

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
