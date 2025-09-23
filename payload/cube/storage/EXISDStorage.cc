#include <cube/storage/EXISDStorage.hh>

#include <cube/Arena.hh>
#include <cube/Clock.hh>
extern "C" {
#include <dolphin/EXIBios.h>
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>
#include <payload/Mutex.hh>

void EXISDStorage::Init() {
    for (u32 i = 0; i < s_instances.count(); i++) {
        s_instances[i] = new (MEM1Arena::Instance(), 0x4) EXISDStorage(i);
    }
}

EXISDStorage::EXISDStorage(u32 channel)
    : SDStorage(new(MEM1Arena::Instance(), 0x4) Mutex), m_channel(channel),
      m_queue(new(MEM1Arena::Instance(), 0x4) OSMessageQueue),
      m_transferQueue(new(MEM1Arena::Instance(), 0x4) OSMessageQueue) {
    Array<OSMessage, 1> *transferMessages = new (MEM1Arena::Instance(), 0x4) Array<OSMessage, 1>;
    OSInitMessageQueue(m_transferQueue, transferMessages->values(), transferMessages->count());
    void *param = this;
    Array<u8, 4 * 1024> *stack = new (MEM1Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM1Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Transfer, param, stack->values() + stack->count(), stack->count(), 31,
            0);
    OSResumeThread(thread);
    Array<OSMessage, 2> *messages = new (MEM1Arena::Instance(), 0x4) Array<OSMessage, 2>;
    OSInitMessageQueue(m_queue, messages->values(), messages->count());
    notify();
    OSReceiveMessage(m_queue, nullptr, OS_MESSAGE_BLOCK);
    if (m_channel != 2) {
        stack = new (MEM1Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
        thread = new (MEM1Arena::Instance(), 0x4) OSThread;
        OSCreateThread(thread, Run, param, stack->values() + stack->count(), stack->count(), 11, 0);
        OSResumeThread(thread);
    }
}

void EXISDStorage::poll() {
    if (isContained()) {
        pollRemove();
        detach();
    } else {
        m_wasDetached = false;
        if (attach()) {
            pollAdd();
            if (!isContained()) {
                detach();
            }
        }
    }
    OSSendMessage(m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *EXISDStorage::run() {
    while (true) {
        if (isContained()) {
            OSReceiveMessage(m_queue, nullptr, OS_MESSAGE_BLOCK);
        } else {
            Clock::WaitMilliseconds(50);
        }
        notify();
        OSReceiveMessage(m_queue, nullptr, OS_MESSAGE_BLOCK);
    }
}

void *EXISDStorage::transfer() {
    while (true) {
        const struct Transfer *transfer;
        OSReceiveMessage(m_transferQueue,
                reinterpret_cast<void **>(const_cast<struct Transfer **>(&transfer)),
                OS_MESSAGE_BLOCK);
        uintptr_t result = execute(transfer);
        OSSendMessage(transfer->queue, reinterpret_cast<void *>(result), OS_MESSAGE_BLOCK);
    }
}

void EXISDStorage::handleEXT() {
    m_wasDetached = true;
    OSSendMessage(m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

bool EXISDStorage::attach() {
    if (m_channel == 2) {
        return true;
    }
    return EXIAttach(m_channel, HandleEXT);
}

void EXISDStorage::detach() {
    if (m_channel == 2) {
        return;
    }
    Lock<NoInterrupts> lock;
    if (!m_wasDetached) {
        EXIDetach(m_channel);
    }
}

bool EXISDStorage::dispatch(struct Transfer *transfer) {
    OSMessageQueue queue;
    Array<OSMessage, 1> messages;
    OSInitMessageQueue(&queue, messages.values(), messages.count());
    transfer->queue = &queue;
    OSSendMessage(m_transferQueue, const_cast<struct Transfer *>(transfer), OS_MESSAGE_BLOCK);
    uintptr_t result;
    OSReceiveMessage(&queue, reinterpret_cast<void **>(&result), OS_MESSAGE_BLOCK);
    return result;
}

void *EXISDStorage::Run(void *param) {
    return static_cast<EXISDStorage *>(param)->run();
}

void *EXISDStorage::Transfer(void *param) {
    return static_cast<EXISDStorage *>(param)->transfer();
}

void EXISDStorage::HandleEXT(s32 chan, struct OSContext * /* context */) {
    s_instances[chan]->handleEXT();
}

Array<EXISDStorage *, 3> EXISDStorage::s_instances;
