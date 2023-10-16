#pragma once

#include "common/Array.hh"
#include "common/ios/Resource.hh"

class Dolphin : private IOS::Resource {
public:
    Dolphin();
    ~Dolphin();
    bool ok();

    bool getVersion(Array<char, 64> &version);

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
