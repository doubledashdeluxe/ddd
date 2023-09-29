#include <common/storage/Storage.hh>

#include <common/Arena.hh>
#include <payload/Lock.hh>

void Storage::Init() {
    OSInitMessageQueue(&s_queue, s_messages.values(), s_messages.count());
    Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Poll, nullptr, stack->values() + stack->count(), stack->count(), 11, 0);
    OSResumeThread(thread);
}

Storage::Storage() : m_next(nullptr) {}

void Storage::notify() {
    OSSendMessage(&s_queue, this, OS_MESSAGE_NOBLOCK);
}

void Storage::remove() {
    Lock<NoInterrupts> lock;
    Storage **next;
    for (next = &s_head; *next != this; next = &(*next)->m_next) {}
    *next = m_next;
}

void Storage::add() {
    Lock<NoInterrupts> lock;
    m_next = s_head;
    s_head = this;
}

void *Storage::Poll(void * /* param */) {
    while (true) {
        void *msg;
        OSReceiveMessage(&s_queue, &msg, OS_MESSAGE_BLOCK);
        Storage *storage = reinterpret_cast<Storage *>(msg);

        Lock<Mutex> lock(storage->m_mutex);
        storage->poll();
    }
}

OSMessageQueue Storage::s_queue;
Array<OSMessage, 1> Storage::s_messages;
