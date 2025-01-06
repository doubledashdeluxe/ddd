#include <common/storage/WiiSDStorage.hh>

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

void WiiSDStorage::Init() {
    s_buffer = new (MEM2Arena::Instance(), 0x20) Array<u8, 0x4000>;
    s_mutex = new (MEM2Arena::Instance(), 0x4) Mutex;
    s_instance = new (MEM2Arena::Instance(), 0x4) WiiSDStorage;
}

WiiSDStorage::WiiSDStorage()
    : IOS::Resource("/dev/sdio/slot0", IOS::Mode::None), FATStorage(s_mutex) {
    Array<OSMessage, 1> *messages = new (MEM2Arena::Instance(), 0x4) Array<OSMessage, 1>;
    OSInitMessageQueue(&s_queue, messages->values(), messages->count());
    m_pollCallback = &WiiSDStorage::pollAdd;
    notify();
    OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
    void *param = this;
    Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
    OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
    OSCreateThread(thread, Run, param, stack->values() + stack->count(), stack->count(), 10, 0);
    OSResumeThread(thread);
}

void WiiSDStorage::poll() {
    (this->*m_pollCallback)();
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *WiiSDStorage::run() {
    while (true) {
        Status status;
        memset(&status, 0, sizeof(status));
        if (isContained()) {
            status.wasRemoved = true;
            waitFor(status);
            m_pollCallback = &WiiSDStorage::pollRemove;
        } else {
            status.wasAdded = true;
            waitFor(status);
            m_pollCallback = &WiiSDStorage::pollAdd;
        }
        notify();
        OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
    }
}

bool WiiSDStorage::waitFor(Status status) {
    u32 arg;
    memcpy(&arg, &status, sizeof(arg));
    return sendCommand(VirtualCommand::Wait, 0, 0, arg, 0, 0, nullptr, nullptr);
}

void *WiiSDStorage::Run(void *param) {
    return reinterpret_cast<WiiSDStorage *>(param)->run();
}

OSMessageQueue WiiSDStorage::s_queue;
