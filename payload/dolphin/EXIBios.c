#include "EXIBios.h"

#include <common/Types.h>

typedef struct EXIControl {
    u8 _00[0x20 - 0x00];
    u32 _20;
    u8 _24[0x40 - 0x24];
} EXIControl;
size_assert(EXIControl, 0x40);

extern u32 exiProbeStartTimes[2];

extern EXIControl Ecb[];

BOOL __EXIProbe(s32 chan);

void EXIProbeReset(void) {
    exiProbeStartTimes[0] = 0;
    exiProbeStartTimes[1] = 0;
    Ecb[0]._20 = 0;
    Ecb[1]._20 = 0;
    __EXIProbe(0);
    __EXIProbe(1);
}
