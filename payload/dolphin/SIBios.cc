extern "C" {
#include "SIBios.h"
}

#include <payload/WUP028.hh>

extern "C" u32 SIProbe(s32 chan) {
    if (WUP028::Probe(chan)) {
        return 0x09000000;
    }

    return REPLACED(SIProbe)(chan);
}
