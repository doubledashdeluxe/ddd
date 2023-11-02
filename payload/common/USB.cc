#include <common/USB.hh>

#include <common/Arena.hh>
#include <payload/Lock.hh>

extern "C" {
#include <assert.h>
}

USB::Handler::Handler() {
    assert(!s_venBackend && !s_hidBackend);
    m_next = s_headHandler;
    s_headHandler = this;
}

void USB::Handler::poll() {
    pollRemove();
    pollAdd();
}

void USB::Handler::pollRemove() {
    {
        Lock<NoInterrupts> lock;
        if (s_removalHandler != this) {
            return;
        }
    }

    onRemove(s_removalDevice);
    s_removalDevice->m_handler = nullptr;
    s_removalDevice = nullptr;
    s_removalHandler = nullptr;
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void USB::Handler::pollAdd() {
    {
        Lock<NoInterrupts> lock;
        if (s_additionHandler != this) {
            return;
        }
    }

    s_additionDevice->m_handler = s_additionHandler;
    if (onAdd(s_additionDeviceInfo, s_additionDevice)) {
        s_additionDevice = nullptr;
        s_additionHandler = nullptr;
    } else {
        s_additionDevice->m_handler = nullptr;
        s_additionHandler = m_next;
    }
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

USB::Device::Device() : m_handler(nullptr), m_transfer(new(MEM2Arena::Instance(), 0x20) Transfer) {}

void USB::Init() {
    s_additionDeviceInfo = new (MEM2Arena::Instance(), 0x20) DeviceInfo;
    s_venBackend = new (MEM2Arena::Instance(), 0x4) Backend;
    s_hidBackend = new (MEM2Arena::Instance(), 0x4) Backend;
    s_venBackend->resource = new (MEM2Arena::Instance(), 0x20) VENResource;
    s_hidBackend->resource = new (MEM2Arena::Instance(), 0x20) HIDResource;
    OSInitMessageQueue(&s_queue, s_messages.values(), s_messages.count());
    s_mutex = new (MEM2Arena::Instance(), 0x4) Mutex;
    Array<Backend *, 2> backends;
    backends[0] = s_venBackend;
    backends[1] = s_hidBackend;
    for (u32 i = 0; i < backends.count(); i++) {
        for (u32 j = 0; j < backends[i]->devices.count(); j++) {
            backends[i]->devices[j].m_resource = backends[i]->resource;
        }
        void *param = backends[i];
        Array<u8, 4 * 1024> *stack = new (MEM2Arena::Instance(), 0x8) Array<u8, 4 * 1024>;
        OSThread *thread = new (MEM2Arena::Instance(), 0x4) OSThread;
        OSCreateThread(thread, Run, param, stack->values() + stack->count(), stack->count(), 9, 0);
        OSResumeThread(thread);
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
    }
}

void USB::HandleRemovals(Backend *backend, u32 deviceEntryCount,
        const Array<Resource::DeviceEntry, 0x20> &deviceEntries) {
    for (u32 i = 0; i < backend->devices.count(); i++) {
        if (!backend->devices[i].m_handler) {
            continue;
        }
        bool wasRemoved = true;
        for (u32 j = 0; j < deviceEntryCount; j++) {
            if (deviceEntries[j].id == backend->devices[i].m_id) {
                wasRemoved = false;
                break;
            }
        }
        if (!wasRemoved) {
            continue;
        }

        s_removalHandler = backend->devices[i].m_handler;
        s_removalDevice = &backend->devices[i];
        {
            Lock<NoInterrupts> lock;
            s_removalHandler->notify();
            OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
        }
    }
}

void USB::HandleAdditions(Backend *backend, u32 deviceEntryCount,
        const Array<Resource::DeviceEntry, 0x20> &deviceEntries) {
    for (u32 i = 0; i < deviceEntryCount; i++) {
        bool wasAdded = true;
        for (u32 j = 0; j < backend->devices.count(); j++) {
            if (!backend->devices[j].m_handler) {
                continue;
            }
            if (backend->devices[j].m_id == deviceEntries[i].id) {
                wasAdded = false;
            }
        }
        if (!wasAdded) {
            continue;
        }

        u32 j;
        for (j = 0; j < backend->devices.count(); j++) {
            if (!backend->devices[j].m_handler) {
                break;
            }
        }
        assert(j < backend->devices.count());
        backend->devices[j].m_id = deviceEntries[i].id;

        if (!backend->resource->resume(backend->devices[j].m_id)) {
            continue;
        }

        for (u32 k = 0; k < deviceEntries[i].alternateSettingCount; k++) {
            if (!backend->resource->getDeviceInfo(backend->devices[j].m_id, k,
                        *s_additionDeviceInfo)) {
                continue;
            }

            if (deviceEntries[i].alternateSettingCount > 1) {
                if (!backend->resource->setAlternateSetting(backend->devices[j].m_id, k)) {
                    continue;
                }
            }

            s_additionHandler = s_headHandler;
            s_additionDevice = &backend->devices[j];
            while (s_additionHandler) {
                Lock<NoInterrupts> lock;
                s_additionHandler->notify();
                OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);
            }

            if (backend->devices[j].m_handler) {
                break;
            }
        }

        if (!backend->devices[j].m_handler) {
            backend->resource->suspend(backend->devices[j].m_id);
        }
    }
}

OSMessageQueue USB::s_queue;
Array<OSMessage, 1> USB::s_messages;
Mutex *USB::s_mutex = nullptr;
