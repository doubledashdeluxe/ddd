#include <cube/USB.hh>

#include <cube/Arena.hh>

extern "C" {
#include <assert.h>
}

void USB::Handler::poll() {
    pollRemove();
    pollAdd();
}

bool USB::Handler::isRemovalHandler() const {
    return this == s_removalHandler;
}

bool USB::Handler::isAdditionHandler() const {
    return this == s_additionHandler;
}

USB::Handle::Handle() : m_venResource(s_backends[0]->buffer), m_hidResource(s_backends[1]->buffer) {
    s_backends[0]->resource = &m_venResource;
    s_backends[1]->resource = &m_hidResource;
    for (u32 i = 0; i < s_backends.count(); i++) {
        for (u32 j = 0; j < s_backends[i]->devices.count(); j++) {
            s_backends[i]->devices[j].m_resource = s_backends[i]->resource;
        }

        Array<Resource::DeviceEntry, 0x20> deviceEntries;
        s32 result = s_backends[i]->resource->getDeviceChange(deviceEntries);
        assert(result >= 0 && result <= static_cast<s32>(deviceEntries.count()));
        u32 deviceEntryCount = result;

        PrintDeviceEntries(deviceEntryCount, deviceEntries);
        CheckDeviceEntries(deviceEntryCount, deviceEntries);
        HandleRemovals(s_backends[i], deviceEntryCount, deviceEntries);
        HandleAdditions(s_backends[i], deviceEntryCount, deviceEntries);

        assert(s_backends[i]->resource->attachFinish());
    }
}

USB::Handle::~Handle() {
    for (u32 i = 0; i < s_backends.count(); i++) {
        for (u32 j = 0; j < s_backends[i]->devices.count(); j++) {
            if (!s_backends[i]->devices[j].m_handler) {
                continue;
            }

            s_removalHandler = s_backends[i]->devices[j].m_handler;
            s_removalDevice = &s_backends[i]->devices[j];
            HandleRemoval();

            s_backends[i]->resource->suspend(s_backends[i]->devices[j].m_id);
        }

        for (u32 j = 0; j < s_backends[i]->devices.count(); j++) {
            s_backends[i]->devices[j].m_resource = nullptr;
        }
        s_backends[i]->resource = nullptr;
    }
}

void USB::Init() {
    s_additionDeviceInfo = new (MEM2Arena::Instance(), 0x20) DeviceInfo;
    for (u32 i = 0; i < s_backends.count(); i++) {
        s_backends[i] = new (MEM2Arena::Instance(), 0x20) Backend;
    }
}

void USB::HandleRemoval() {
    s_removalHandler->notify();
}

void USB::HandleAddition() {
    s_additionHandler->notify();
}
