extern "C" {
#include "EXIBios.h"

#include "dolphin/OSInterrupt.h"
}

#include <cube/Memory.hh>
#include <cube/Platform.hh>
#include <payload/Lock.hh>

extern "C" {
#include <string.h>
}

struct EXIControl {
    u8 _00[0x04 - 0x00];
    EXICallback callback;
    u8 _08[0x0c - 0x08];
    u32 flags;
    s32 len;
    void *buf;
    u8 _18[0x20 - 0x18];
    u32 _20;
    u8 _24[0x40 - 0x24];
};
size_assert(EXIControl, 0x40);

struct EXIChannel {
    u32 cpr;
    u32 mar;
    u32 length;
    u32 cr;
    u32 data;
};
size_assert(EXIChannel, 0x14);

extern "C" u32 exiProbeStartTimes[2];
extern "C" EXIControl Ecb[];

extern "C" volatile EXIChannel exi[3];

extern "C" BOOL __EXIProbe(s32 chan);

void EXIProbeReset(void) {
    exiProbeStartTimes[0] = 0;
    exiProbeStartTimes[1] = 0;
    Ecb[0]._20 = 0;
    Ecb[1]._20 = 0;
    __EXIProbe(0);
    __EXIProbe(1);
}

BOOL EXIDma(s32 chan, void *buf, s32 len, u32 type, EXICallback callback) {
    buf = reinterpret_cast<void *>(Memory::CachedToPhysical(buf));
    return REPLACED(EXIDma)(chan, buf, len, type, callback);
}

BOOL EXIImm(s32 chan, void *buf, s32 len, u32 type, EXICallback callback) {
    Lock<NoInterrupts> lock;

    if (Ecb[chan].flags & (1 << 1 | 1 << 0)) {
        return false;
    }

    if (!(Ecb[chan].flags & (1 << 2))) {
        return false;
    }

    Ecb[chan].callback = callback;
    if (callback) {
        EXIClearInterrupts(chan, false, true, false);
        OSUnmaskInterrupts(1 << (21 - chan * 3));
    }
    Ecb[chan].flags |= 1 << 1;
    u32 data = ~0;
    if (type != EXI_READ) {
        memcpy(&data, buf, len);
    }
    exi[chan].data = data;
    Ecb[chan].buf = buf;
    Ecb[chan].len = type == EXI_WRITE ? 0 : len;
    exi[chan].cr = (len - 1) << 4 | type << 2 | 1 << 0;
    return true;
}

s32 EXIGetType(s32 chan, u32 dev, u32 *type) {
    if (chan == 0 && dev == 2) {
        *type = 0x04020200;
        return true;
    }

    return REPLACED(EXIGetType)(chan, dev, type);
}
