#include <common/USB.hh>

#include <common/Arena.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

extern "C" {
#include <assert.h>
}

void USB::Handler::poll() {
    pollRemove();
    pollAdd();
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

bool USB::Handler::isRemovalHandler() const {
    Lock<NoInterrupts> lock;
    return this == s_removalHandler;
}

bool USB::Handler::isAdditionHandler() const {
    Lock<NoInterrupts> lock;
    return this == s_additionHandler;
}

void USB::Init() {
    s_additionDeviceInfo = new (MEM2Arena::Instance(), 0x20) DeviceInfo;
    for (u32 i = 0; i < s_backends.count(); i++) {
        s_backends[i] = new (MEM2Arena::Instance(), 0x20) Backend;
    }
    s_backends[0]->resource = new (MEM2Arena::Instance(), 0x20) VENResource(s_backends[0]->buffer);
    s_backends[1]->resource = new (MEM2Arena::Instance(), 0x20) HIDResource(s_backends[1]->buffer);
    s_mutex = new (MEM2Arena::Instance(), 0x4) Mutex;
    Array<OSMessage, 1> *messages = new (MEM2Arena::Instance(), 0x4) Array<OSMessage, 1>;
    OSInitMessageQueue(&s_queue, messages->values(), messages->count());
    for (u32 i = 0; i < s_backends.count(); i++) {
        for (u32 j = 0; j < s_backends[i]->devices.count(); j++) {
            s_backends[i]->devices[j].m_resource = s_backends[i]->resource;
        }
        s_backends[i]->initQueue = new (MEM2Arena::Instance(), 0x4) OSMessageQueue;
        Array<OSMessage, 1> *initMessages = new (MEM2Arena::Instance(), 0x4) Array<OSMessage, 1>;
        OSInitMessageQueue(s_backends[i]->initQueue, initMessages->values(), initMessages->count());
        void *param = s_backends[i];
        Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
        OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
        OSCreateThread(thread, Run, param, stack->values() + stack->count(), stack->count(), 9, 0);
        OSResumeThread(thread);
    }
    for (u32 i = 0; i < s_backends.count(); i++) {
        OSReceiveMessage(s_backends[i]->initQueue, nullptr, OS_MESSAGE_BLOCK);
    }
}

void *USB::Run(void *param) {
    Backend *backend = reinterpret_cast<Backend *>(param);
    while (true) {
        Array<Resource::DeviceEntry, 0x20> deviceEntries;
        s32 result = backend->resource->getDeviceChange(deviceEntries);
        assert(result >= 0 && result <= static_cast<s32>(deviceEntries.count()));
        u32 deviceEntryCount = result;
        Lock<Mutex> lock(*s_mutex);

        PrintDeviceEntries(deviceEntryCount, deviceEntries);
        CheckDeviceEntries(deviceEntryCount, deviceEntries);
        HandleRemovals(backend, deviceEntryCount, deviceEntries);
        HandleAdditions(backend, deviceEntryCount, deviceEntries);

        assert(backend->resource->attachFinish());

        OSSendMessage(backend->initQueue, nullptr, OS_MESSAGE_NOBLOCK);
    }
}

void USB::HandleRemoval() {
    Lock<NoInterrupts> lock;
    s_removalHandler->notify();
    OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
}

void USB::HandleAddition() {
    Lock<NoInterrupts> lock;
    s_additionHandler->notify();
    OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
}

OSMessageQueue USB::s_queue;
Mutex *USB::s_mutex;
