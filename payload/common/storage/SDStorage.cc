#include <common/storage/SDStorage.hh>

#include <common/Arena.hh>
#include <common/Log.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Mutex.hh>

extern "C" {
#include <string.h>
}

void SDStorage::Init() {
    s_mutex = new (MEM2Arena::Instance(), 0x4) Mutex;
    s_instance = new (MEM2Arena::Instance(), 0x20) SDStorage;
}

SDStorage::SDStorage() : IOS::Resource("/dev/sdio/slot0", IOS::Mode::None), FATStorage(s_mutex) {
    Array<OSMessage, 1> *messages = new (MEM2Arena::Instance(), 0x4) Array<OSMessage, 1>;
    OSInitMessageQueue(&s_queue, messages->values(), messages->count());
    m_pollCallback = &SDStorage::pollAdd;
    notify();
    OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
    void *param = this;
    Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Run, param, stack->values() + stack->count(), stack->count(), 10, 0);
    OSResumeThread(thread);
}

void SDStorage::poll() {
    (this->*m_pollCallback)();
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *SDStorage::run() {
    while (true) {
        Status status;
        memset(&status, 0, sizeof(status));
        if (isContained()) {
            status.wasRemoved = true;
            waitFor(status);
            m_pollCallback = &SDStorage::pollRemove;
        } else {
            status.wasAdded = true;
            waitFor(status);
            m_pollCallback = &SDStorage::pollAdd;
        }
        notify();
        OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
    }
}

bool SDStorage::waitFor(Status status) {
    u32 arg;
    memcpy(&arg, &status, sizeof(arg));
    return sendCommand(VirtualCommand::Wait, 0, 0, arg, 0, 0, nullptr, nullptr);
}

void SDStorage::pollAdd() {
    if (!resetCard()) {
        DEBUG("Failed to reset card");
        return;
    }

    Status status;
    if (!getStatus(status)) {
        DEBUG("Failed to get status");
        return;
    }

    if (!status.wasAdded) {
        DEBUG("No card inserted");
        return;
    }

    if (!status.isMemory) {
        DEBUG("Not a memory card");
        return;
    }

    m_isSDHC = status.isSDHC;

    if (!enable4BitBus()) {
        DEBUG("Failed to enable 4-bit bus");
        return;
    }

    if (!setClock(1)) {
        DEBUG("Failed to set clock");
        return;
    }

    {
        CardHandle cardHandle(this);
        if (!cardHandle.ok()) {
            return;
        }

        if (!setCardBlockLength(sectorSize())) {
            return;
        }

        if (!enableCard4BitBus()) {
            return;
        }
    }

    add();
}

void SDStorage::pollRemove() {
    remove();
}

void *SDStorage::Run(void *param) {
    return reinterpret_cast<SDStorage *>(param)->run();
}

Mutex *SDStorage::s_mutex;
OSMessageQueue SDStorage::s_queue;
