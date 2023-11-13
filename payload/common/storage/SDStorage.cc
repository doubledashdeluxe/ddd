#include <common/storage/SDStorage.hh>

#include <common/Arena.hh>
#include <common/Log.hh>

extern "C" {
#include <string.h>
}

void SDStorage::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) SDStorage;
}

SDStorage::SDStorage() : IOS::Resource("/dev/sdio/slot0", IOS::Mode::None) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    m_pollCallback = &SDStorage::pollAdd;
    notify();
    OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
    void *param = this;
    Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Run, param, stack->values() + stack->count(), stack->count(), 10, 0);
    OSResumeThread(thread);
}

void SDStorage::poll() {
    (this->*m_pollCallback)();
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
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
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
    }
}

bool SDStorage::waitFor(Status status) {
    u32 arg;
    memcpy(&arg, &status, sizeof(arg));
    return sendCommand(VirtualCommand::Wait, 0, 0, arg, 0, 0, nullptr, nullptr);
}

void SDStorage::pollAdd() {
    if (!resetCard()) {
        DEBUG("Failed to reset card\n");
        return;
    }

    Status status;
    if (!getStatus(status)) {
        DEBUG("Failed to get status\n");
        return;
    }

    if (!status.wasAdded) {
        DEBUG("No card inserted\n");
        return;
    }

    if (!status.isMemory) {
        DEBUG("Not a memory card\n");
        return;
    }

    m_isSDHC = status.isSDHC;

    if (!enable4BitBus()) {
        DEBUG("Failed to enable 4-bit bus\n");
        return;
    }

    if (!setClock(1)) {
        DEBUG("Failed to set clock\n");
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
