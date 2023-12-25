#include <common/storage/Storage.hh>

#include <common/Arena.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

extern "C" {
#include <string.h>
}

void Storage::Init() {
    Array<OSMessage, 1> *messages = new (MEM2Arena::Instance(), 0x4) Array<OSMessage, 1>;
    OSInitMessageQueue(&s_queue, messages->values(), messages->count());
    Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Poll, nullptr, stack->values() + stack->count(), stack->count(), 11, 0);
    OSResumeThread(thread);
}

Storage::Storage(Mutex *mutex) : m_next(nullptr), m_isContained(false), m_mutex(mutex) {}

void Storage::notify() {
    OSSendMessage(&s_queue, this, OS_MESSAGE_BLOCK);
}

void Storage::remove() {
    Lock<NoInterrupts> lock;
    bool hasOldMain = s_head == this && s_head->priority() > 0;
    m_isContained = false;
    Storage **next;
    for (next = &s_head; *next != this; next = &(*next)->m_next) {}
    *next = m_next;
    bool hasNewMain = s_head && s_head->priority() > 0;
    for (Observer *observer = s_headObserver; observer; observer = observer->next()) {
        if (hasOldMain) {
            observer->onRemove("main:");
            if (hasNewMain) {
                observer->onAdd("main:");
            }
        }
        observer->onRemove(prefix());
    }
}

void Storage::add() {
    Lock<NoInterrupts> lock;
    bool hasOldMain = s_head && s_head->priority() > 0;
    Storage **next;
    for (next = &s_head; *next && (*next)->priority() >= priority(); next = &(*next)->m_next) {}
    m_next = *next;
    *next = this;
    m_isContained = true;
    bool hasNewMain = s_head == this && s_head->priority() > 0;
    for (Observer *observer = s_headObserver; observer; observer = observer->next()) {
        observer->onAdd(prefix());
        if (hasNewMain) {
            if (hasOldMain) {
                observer->onRemove("main:");
            }
            observer->onAdd("main:");
        }
    }
}

Storage::StorageHandle::StorageHandle(const char *path) : m_storage(nullptr), m_prefix(nullptr) {
    Lock<NoInterrupts> lock;
    if (s_head && s_head->priority() > 0) {
        if (!strncmp(path, "main:", strlen("main:"))) {
            m_storage = s_head;
            m_prefix = "main:";
        }
    }
    if (!m_storage) {
        for (Storage *storage = s_head; storage; storage = storage->m_next) {
            if (!strncmp(path, storage->prefix(), strlen(storage->prefix()))) {
                m_storage = storage;
                m_prefix = storage->prefix();
                break;
            }
        }
    }
    if (m_storage) {
        m_storage->m_mutex->lock();
    }
}

Storage::StorageHandle::StorageHandle(const FileHandle &file)
    : m_storage(nullptr), m_prefix(nullptr) {
    Lock<NoInterrupts> lock;
    if (file.m_file) {
        m_storage = file.m_file->storage();
    }
    if (m_storage) {
        m_storage->m_mutex->lock();
    }
}

Storage::StorageHandle::StorageHandle(const DirHandle &dir)
    : m_storage(nullptr), m_prefix(nullptr) {
    Lock<NoInterrupts> lock;
    if (dir.m_dir) {
        m_storage = dir.m_dir->storage();
    }
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
        Storage *storage = reinterpret_cast<Storage *>(msg);

        Lock<Mutex> lock(*storage->m_mutex);
        storage->poll();
    }
}

OSMessageQueue Storage::s_queue;
