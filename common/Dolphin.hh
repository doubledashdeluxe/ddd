#pragma once

#include "common/DolphinVersion.hh"
#include "common/ios/Resource.hh"

class Dolphin : private IOS::Resource {
public:
    Dolphin();
    ~Dolphin();
    bool ok() const;

    bool getVersion(DolphinVersion &version);

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
