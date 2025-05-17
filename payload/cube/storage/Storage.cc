#include <cube/storage/Storage.hh>

#include <cube/Arena.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

extern "C" {
#include <string.h>
}

void Storage::Init() {
    Array<OSMessage, 1> *messages = new (MEM1Arena::Instance(), 0x4) Array<OSMessage, 1>;
    OSInitMessageQueue(&s_queue, messages->values(), messages->count());
    Array<u8, 4 * 1024> *stack = new (MEM1Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM1Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Poll, nullptr, stack->values() + stack->count(), stack->count(), 12, 0);
    OSResumeThread(thread);
}

void Storage::notify() {
    OSSendMessage(&s_queue, this, OS_MESSAGE_BLOCK);
}

void Storage::remove() {
    Lock<NoInterrupts> lock;
    removeWithoutLocking();
}

void Storage::add() {
    Lock<NoInterrupts> lock;
    addWithoutLocking();
}

Storage::StorageHandle::StorageHandle(const char *path) : m_storage(nullptr), m_prefix(nullptr) {
    Lock<NoInterrupts> lock;
    acquireWithoutLocking(path);
    if (m_storage) {
        m_storage->m_mutex->lock();
    }
}

Storage::StorageHandle::StorageHandle(const FileHandle &file)
    : m_storage(nullptr), m_prefix(nullptr) {
    Lock<NoInterrupts> lock;
    acquireWithoutLocking(file);
    if (m_storage) {
        m_storage->m_mutex->lock();
    }
}

Storage::StorageHandle::StorageHandle(const DirHandle &dir)
    : m_storage(nullptr), m_prefix(nullptr) {
    Lock<NoInterrupts> lock;
    acquireWithoutLocking(dir);
    if (m_storage) {
        m_storage->m_mutex->lock();
    }
}

Storage::StorageHandle::~StorageHandle() {
    if (m_storage) {
        m_storage->m_mutex->unlock();
    }
}

void *Storage::Poll(void * /* param */) {
    while (true) {
        void *msg;
        OSReceiveMessage(&s_queue, &msg, OS_MESSAGE_BLOCK);
        Storage *storage = static_cast<Storage *>(msg);

        Lock<Mutex> lock(*storage->m_mutex);
        storage->poll();
    }
}

OSMessageQueue Storage::s_queue;
