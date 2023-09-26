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

Lock<Mutex>::Lock(Mutex &mutex) : m_mutex(mutex) {
    m_mutex.lock();
}

Lock<Mutex>::~Lock() {
    m_mutex.unlock();
}
