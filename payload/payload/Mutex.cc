#include "Mutex.hh"

Mutex::Mutex() {
    OSInitMutex(&m_mutex);
}

Mutex::~Mutex() {}

void Mutex::lock() {
    OSLockMutex(&m_mutex);
}

void Mutex::unlock() {
    OSUnlockMutex(&m_mutex);
}
