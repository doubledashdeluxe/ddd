extern "C" {
#include "GXFrameBuf.h"
}

#include <common/Console.hh>

extern "C" void GXCopyDisp(void *dest, BOOL clear) {
    if (!Console::Instance()->m_isActive) {
        REPLACED(GXCopyDisp)(dest, clear);
    }
}
