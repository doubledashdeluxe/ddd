#include "Lock.hh"

extern "C" {
#include <dolphin/OSInterrupt.h>
}

Lock<NoInterrupts>::Lock() {
    m_isr = OSDisableInterrupts();
}

Lock<NoInterrupts>::~Lock() {
    OSRestoreInterrupts(m_isr);
}
