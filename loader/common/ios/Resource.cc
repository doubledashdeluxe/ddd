#include <common/ios/Resource.hh>

#include <common/Clock.hh>
#include <common/DCache.hh>
#include <common/Memory.hh>

extern "C" {
#include <string.h>
}

enum {
    X1 = 1 << 0,
    Y2 = 1 << 1,
    Y1 = 1 << 2,
    X2 = 1 << 3,
    IY1 = 1 << 4,
    IY2 = 1 << 5,
};

extern "C" volatile u32 ppcmsg;
extern "C" volatile u32 ppcctrl;
extern "C" volatile u32 armmsg;

namespace IOS {

void Resource::Sync(Request &request) {
    DCache::Flush(&request, offsetof(Request, user));

    ppcmsg = Memory::VirtualToPhysical(&request);
    ppcctrl = X1;

    while ((ppcctrl & Y2) != Y2) {
        if ((ppcctrl & Y1) == Y1) { // Expected an ack but got a reply!
            ppcctrl = Y1;
            ppcctrl = X2;
        }
    }
    ppcctrl = Y2;

    u32 reply;
    do {
        while ((ppcctrl & Y1) != Y1) {
            if ((ppcctrl & Y2) == Y2) { // Expected a reply but got an ack!
                ppcctrl = Y2;
            }
            Clock::WaitMilliseconds(1);
        }
        reply = armmsg;
        ppcctrl = Y1;

        ppcctrl = X2;
    } while (reply != Memory::VirtualToPhysical(&request));

    DCache::Invalidate(&request, offsetof(Request, user));
}

} // namespace IOS
