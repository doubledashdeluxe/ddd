#pragma once

#include "common/Array.hh"
#include "common/ios/Resource.hh"

class ES : private IOS::Resource {
public:
    ES();
    ~ES();
    bool ok() const;

    bool launchTitle(u64 titleID, const Array<u8, 0xd8> &ticketView);

private:
    class Ioctlv {
    public:
        enum {
            LaunchTitle = 0x8,
        };

    private:
        Ioctlv();
    };
};
