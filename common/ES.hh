#pragma once

#include "common/Array.hh"
#include "common/ios/Resource.hh"

class ES : private IOS::Resource {
public:
    ES();
    ~ES();
    bool ok() const;

    bool launchTitle(u64 titleID, const Array<u8, 0xd8> &ticketView);
    bool sign(const void *data, u32 size, Array<u8, 0x3c> &signature,
            Array<u8, 0x180> &certificate);

private:
    class Ioctlv {
    public:
        enum {
            LaunchTitle = 0x8,
            Sign = 0x30,
        };

    private:
        Ioctlv();
    };
};
