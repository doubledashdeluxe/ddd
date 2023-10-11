#include <common/storage/SDStorage.hh>

#include <common/Arena.hh>
#include <common/Log.hh>

extern "C" {
#include <string.h>
}

void SDStorage::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) SDStorage;
    OSInitMessageQueue(&s_queue, s_messages.values(), s_messages.count());
    Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Run, nullptr, stack->values() + stack->count(), stack->count(), 10, 0);
    OSResumeThread(thread);
}

void SDStorage::poll() {
    (this->*m_pollCallback)();
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *SDStorage::run() {
    while (true) {
        Status addedStatus;
        memset(&addedStatus, 0, sizeof(addedStatus));
        addedStatus.wasAdded = true;
        waitFor(addedStatus);
        m_pollCallback = &SDStorage::pollAdd;
        notify();
        OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);

        Status removedStatus;
        memset(&removedStatus, 0, sizeof(removedStatus));
        removedStatus.wasRemoved = true;
        waitFor(removedStatus);
        m_pollCallback = &SDStorage::pollRemove;
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

void *SDStorage::Run(void * /* param */) {
    return s_instance->run();
}

OSMessageQueue SDStorage::s_queue;
Array<OSMessage, 1> SDStorage::s_messages;
