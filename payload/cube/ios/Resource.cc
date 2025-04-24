#include <cube/ios/Resource.hh>

#include <cube/DCache.hh>
#include <cube/Memory.hh>
extern "C" {
#include <dolphin/OSInterrupt.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

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
extern "C" volatile u32 ppcirqflag;

namespace IOS {

void Resource::Init() {
    OSSetInterruptHandler(27, HandleInterrupt);
    OSUnmaskInterrupts(1 << (31 - 27));
    ppcctrl = IY2 | IY1 | X2;
}

void Resource::Sync(Request &request) {
    DCache::Flush(&request, offsetof(Request, user));

    Lock<NoInterrupts> lock;

    request.user.next = nullptr;
    if (s_request) {
        Request *prev;
        for (prev = s_request; prev->user.next; prev = prev->user.next) {}
        prev->user.next = &request;
    } else {
        ppcmsg = Memory::CachedToPhysical(&request);
        ppcctrl = IY2 | IY1 | X1;
        s_request = &request;
    }

    OSThreadQueue *queue = reinterpret_cast<OSThreadQueue *>(request.user._04);
    OSInitThreadQueue(queue);
    OSSleepThread(queue);
}

bool Resource::SyncReboot(Request & /* request */) {
    return false;
}

void Resource::HandleInterrupt(s16 /* interrupt */, OSContext * /* context */) {
    if (ppcctrl & Y1) {
        Request *reply = Memory::PhysicalToCached<Request>(armmsg);

        ppcctrl = IY2 | IY1 | Y1;
        ppcirqflag = 1 << 30;

        DCache::Invalidate(reply, offsetof(Request, user));
        OSThreadQueue *queue = reinterpret_cast<OSThreadQueue *>(reply->user._04);
        OSWakeupThread(queue);

        ppcctrl = IY2 | IY1 | X2;
    }

    if (ppcctrl & Y2) {
        ppcctrl = IY2 | IY1 | Y2;
        ppcirqflag = 1 << 30;

        if (s_request) {
            s_request = s_request->user.next;
            if (s_request) {
                ppcmsg = Memory::CachedToPhysical(s_request);
                ppcctrl = IY2 | IY1 | X1;
            }
        }
    }
}

Resource::Request *Resource::s_request = nullptr;

} // namespace IOS
