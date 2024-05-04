#include "USB.hh"

#include "common/Arena.hh"
#include "common/Bytes.hh"
#include "common/Log.hh"

extern "C" {
#include <assert.h>
#include <string.h>
}

USB::Handler::Handler() {
    for (u32 i = 0; i < s_backends.count(); i++) {
        assert(!s_backends[i]);
    }
    m_next = s_headHandler;
    s_headHandler = this;
}

void USB::Handler::pollRemove() {
    if (!isRemovalHandler()) {
        return;
    }

    onRemove(s_removalDevice);
    s_removalDevice->m_handler = nullptr;
    s_removalDevice = nullptr;
    s_removalHandler = nullptr;
}

void USB::Handler::pollAdd() {
    if (!isAdditionHandler()) {
        return;
    }

    s_additionDevice->m_handler = s_additionHandler;
    if (onAdd(s_additionDeviceInfo, s_additionDevice)) {
        s_additionDevice = nullptr;
        s_additionHandler = nullptr;
    } else {
        s_additionDevice->m_handler = nullptr;
        s_additionHandler = m_next;
    }
}

USB::Device::Device() : m_handler(nullptr), m_transfer(new(MEM2Arena::Instance(), 0x20) Transfer) {}

USB::Device::~Device() {}

bool USB::Device::ctrlTransfer(u8 endpointDirection, u8 requestType, u8 requestRecipient,
        u8 request, u16 value, u16 index, u16 length, void *data) {
    return m_resource->ctrlTransfer(m_id, m_transfer, endpointDirection, requestType,
            requestRecipient, request, value, index, length, data);
}

bool USB::Device::intrTransfer(void *data, u16 length, u8 endpointDirection, u8 endpointNumber) {
    return m_resource->intrTransfer(m_id, m_transfer, data, length, endpointDirection,
            endpointNumber);
}

bool USB::Device::isoTransfer(void *data, u16 *packetSizes, u8 packets, u8 endpointDirection,
        u8 endpointNumber) {
    return m_resource->isoTransfer(m_id, m_transfer, data, packetSizes, packets, endpointDirection,
            endpointNumber);
}

bool USB::Device::bulkTransfer(void *data, u16 length, u8 endpointDirection, u8 endpointNumber) {
    return m_resource->bulkTransfer(m_id, m_transfer, data, length, endpointDirection,
            endpointNumber);
}

USB::Resource::Resource(const char *name, Buffer &buffer)
    : IOS::Resource(name, IOS::Mode::None), m_buffer(buffer.buffer) {}

USB::Resource::~Resource() {}

bool USB::Resource::getVersion(u32 &version) {
    memset(m_buffer, 0, 0x20);
    s32 result = ioctl(Ioctl::GetVersion, nullptr, 0, m_buffer, 0x20);
    if (result < 0) {
        DEBUG("Failed to get version with error %d", result);
        return false;
    }
    version = Bytes::ReadBE<u32>(m_buffer, 0x0);
    return true;
}

s32 USB::Resource::getDeviceChange(Array<DeviceEntry, 0x20> &deviceEntries) {
    memset(m_buffer, 0, 0x180);
    s32 result = ioctl(Ioctl::GetDeviceChange, nullptr, 0, m_buffer, 0x180);
    if (result >= 0 && result <= 0x20) {
        static_assert(sizeof(deviceEntries) == 0x180);
        memcpy(deviceEntries.values(), m_buffer, sizeof(DeviceEntry) * result);
    }
    return result;
}

bool USB::Resource::attachFinish() {
    s32 result = ioctl(Ioctl::AttachFinish, nullptr, 0, nullptr, 0);
    return result >= 0;
}

bool USB::Resource::suspend(u32 id) {
    memset(m_buffer, 0, 0x20);
    Bytes::WriteBE<u32>(m_buffer, 0x0, id);
    Bytes::WriteBE<u32>(m_buffer, 0x8, 0);
    s32 result = ioctl(Ioctl::SuspendResume, m_buffer, 0x20, nullptr, 0);
    if (result < 0) {
        DEBUG("Failed to suspend device %u with error %d", id, result);
    }
    return result >= 0;
}

bool USB::Resource::resume(u32 id) {
    memset(m_buffer, 0, 0x20);
    Bytes::WriteBE<u32>(m_buffer, 0x0, id);
    Bytes::WriteBE<u32>(m_buffer, 0x8, 1);
    s32 result = ioctl(Ioctl::SuspendResume, m_buffer, 0x20, nullptr, 0);
    if (result < 0) {
        DEBUG("Failed to resume device %u with error %d", id, result);
    }
    return result >= 0;
}

bool USB::Resource::ctrlTransfer(u32 id, Transfer *transfer, u8 endpointDirection, u8 requestType,
        u8 requestRecipient, u8 request, u16 value, u16 index, u16 length, void *data) {
    memset(transfer, 0, sizeof(*transfer));

    transfer->id = id;
    transfer->ctrl.requestType.endpointDirection = endpointDirection;
    transfer->ctrl.requestType.type = requestType;
    transfer->ctrl.requestType.recipient = requestRecipient;
    transfer->ctrl.request = request;
    transfer->ctrl.value = value;
    transfer->ctrl.index = index;
    transfer->ctrl.length = length;
    transfer->ctrl.data = data;

    transfer->pairs[0].data = transfer;
    transfer->pairs[0].size = sizeof(*transfer);
    transfer->pairs[1].data = data;
    transfer->pairs[1].size = length;

    u32 inputCount = endpointDirection == EndpointDirection::HostToDevice ? 2 : 1;
    u32 outputCount = endpointDirection == EndpointDirection::HostToDevice ? 0 : 1;
    s32 result = ioctlv(Ioctlv::CtrlTransfer, inputCount, outputCount, transfer->pairs);
    return result == 0x8 + length;
}

USB::VENResource::VENResource(Buffer &buffer) : Resource("/dev/usb/ven", buffer) {}

USB::VENResource::~VENResource() {}

bool USB::VENResource::getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo) {
    memset(m_buffer, 0, 0x20 + 0xc0);
    Bytes::WriteBE<u32>(m_buffer, 0x0, id);
    Bytes::WriteBE<u32>(m_buffer, 0x8, alternateSetting);
    s32 result = ioctl(Ioctl::GetDeviceInfo, m_buffer, 0x20, m_buffer + 0x20, 0xc0);
    if (result < 0) {
        DEBUG("Failed to get device info for device %u and alternate setting %u with error %d", id,
                alternateSetting, result);
        return false;
    }
    memcpy(&deviceInfo, m_buffer + 0x20, sizeof(DeviceInfo));
    return true;
}

bool USB::VENResource::setAlternateSetting(u32 id, u8 alternateSetting) {
    memset(m_buffer, 0, 0x20);
    Bytes::WriteBE<u32>(m_buffer, 0x0, id);
    Bytes::WriteBE<u8>(m_buffer, 0x8, alternateSetting);
    s32 result = ioctl(Ioctl::SetAlternateSetting, m_buffer, 0x20, nullptr, 0);
    if (result < 0) {
        DEBUG("Failed to set alternate setting %u for device %u with error %d", alternateSetting,
                id, result);
    }
    return result >= 0;
}

bool USB::VENResource::intrTransfer(u32 id, Transfer *transfer, void *data, u16 length,
        u8 endpointDirection, u8 endpointNumber) {
    memset(transfer, 0, sizeof(*transfer));

    transfer->id = id;
    transfer->venIntr.data = data;
    transfer->venIntr.length = length;
    transfer->venIntr.endpoint.direction = endpointDirection;
    transfer->venIntr.endpoint.number = endpointNumber;

    transfer->pairs[0].data = transfer;
    transfer->pairs[0].size = sizeof(*transfer);
    transfer->pairs[1].data = data;
    transfer->pairs[1].size = length;

    u32 inputCount = endpointDirection == EndpointDirection::HostToDevice ? 2 : 1;
    u32 outputCount = endpointDirection == EndpointDirection::HostToDevice ? 0 : 1;
    s32 result = ioctlv(Ioctlv::IntrTransfer, inputCount, outputCount, transfer->pairs);
    return result == length;
}

bool USB::VENResource::isoTransfer(u32 id, Transfer *transfer, void *data, u16 *packetSizes,
        u8 packets, u8 endpointDirection, u8 endpointNumber) {
    u16 length = 0;
    for (u32 i = 0; i < packets; i++) {
        length += packetSizes[i];
    }

    memset(transfer, 0, sizeof(*transfer));

    transfer->id = id;
    transfer->iso.data = data;
    transfer->iso.packetSizes = packetSizes;
    transfer->iso.packets = packets;
    transfer->iso.endpoint.direction = endpointDirection;
    transfer->iso.endpoint.number = endpointNumber;

    transfer->pairs[0].data = transfer;
    transfer->pairs[0].size = sizeof(*transfer);
    transfer->pairs[1].data = packetSizes;
    transfer->pairs[1].size = sizeof(*packetSizes) * packets;
    transfer->pairs[2].data = data;
    transfer->pairs[2].size = length;
    transfer->pairs[3].data = packetSizes;
    transfer->pairs[3].size = sizeof(*packetSizes) * packets;

    u32 inputCount = endpointDirection == EndpointDirection::HostToDevice ? 3 : 2;
    u32 outputCount = endpointDirection == EndpointDirection::HostToDevice ? 1 : 2;
    s32 result = ioctlv(Ioctlv::IsoTransfer, inputCount, outputCount, transfer->pairs);
    return result == length;
}

bool USB::VENResource::bulkTransfer(u32 id, Transfer *transfer, void *data, u16 length,
        u8 endpointDirection, u8 endpointNumber) {
    memset(transfer, 0, sizeof(*transfer));

    transfer->id = id;
    transfer->bulk.data = data;
    transfer->bulk.length = length;
    transfer->bulk.endpoint.direction = endpointDirection;
    transfer->bulk.endpoint.number = endpointNumber;

    transfer->pairs[0].data = transfer;
    transfer->pairs[0].size = sizeof(*transfer);
    transfer->pairs[1].data = data;
    transfer->pairs[1].size = length;

    u32 inputCount = endpointDirection == EndpointDirection::HostToDevice ? 2 : 1;
    u32 outputCount = endpointDirection == EndpointDirection::HostToDevice ? 0 : 1;
    s32 result = ioctlv(Ioctlv::BulkTransfer, inputCount, outputCount, transfer->pairs);
    return result == length;
}

USB::HIDResource::HIDResource(Buffer &buffer) : Resource("/dev/usb/hid", buffer) {}

USB::HIDResource::~HIDResource() {}

bool USB::HIDResource::getDeviceInfo(u32 id, u8 alternateSetting, DeviceInfo &deviceInfo) {
    memset(m_buffer, 0, 0x20 + 0x60);
    Bytes::WriteBE<u32>(m_buffer, 0x0, id);
    Bytes::WriteBE<u32>(m_buffer, 0x8, alternateSetting);
    s32 result = ioctl(Ioctl::GetDeviceInfo, m_buffer, 0x20, m_buffer + 0x20, 0x60);
    if (result < 0) {
        DEBUG("Failed to get device info for device %u and alternate setting %u with error %d", id,
                alternateSetting, result);
        return false;
    }
    memcpy(&deviceInfo.id, m_buffer + 0x20 + 0x00, sizeof(deviceInfo.id));
    memcpy(&deviceInfo.deviceDescriptor, m_buffer + 0x20 + 0x24,
            sizeof(deviceInfo.deviceDescriptor));
    memcpy(&deviceInfo.configDescriptor, m_buffer + 0x20 + 0x38,
            sizeof(deviceInfo.configDescriptor));
    memcpy(&deviceInfo.interfaceDescriptor, m_buffer + 0x20 + 0x44,
            sizeof(deviceInfo.interfaceDescriptor));
    memcpy(&deviceInfo.endpointDescriptors[0], m_buffer + 0x20 + 0x50,
            sizeof(deviceInfo.endpointDescriptors[0]));
    memcpy(&deviceInfo.endpointDescriptors[1], m_buffer + 0x20 + 0x58,
            sizeof(deviceInfo.endpointDescriptors[1]));
    return true;
}

bool USB::HIDResource::setAlternateSetting(u32 /* id */, u8 /* alternateSetting */) {
    return true;
}

bool USB::HIDResource::intrTransfer(u32 id, Transfer *transfer, void *data, u16 length,
        u8 endpointDirection, u8 /* endpointNumber */) {
    memset(transfer, 0, sizeof(*transfer));

    transfer->id = id;
    transfer->hidIntr.endpointDirection = !endpointDirection;

    transfer->pairs[0].data = transfer;
    transfer->pairs[0].size = sizeof(*transfer);
    transfer->pairs[1].data = data;
    transfer->pairs[1].size = length;

    u32 inputCount = endpointDirection == EndpointDirection::HostToDevice ? 2 : 1;
    u32 outputCount = endpointDirection == EndpointDirection::HostToDevice ? 0 : 1;
    s32 result = ioctlv(Ioctlv::IntrTransfer, inputCount, outputCount, transfer->pairs);
    return result == length;
}

bool USB::HIDResource::isoTransfer(u32 /* id */, Transfer * /* transfer */, void * /* data */,
        u16 * /* packetSizes */, u8 /* packets */, u8 /* endpointDirection */,
        u8 /* endpointNumber */) {
    return false;
}

bool USB::HIDResource::bulkTransfer(u32 /* id */, Transfer * /* transfer */, void * /* data */,
        u16 /* length */, u8 /* endpointDirection */, u8 /* endpointNumber */) {
    return false;
}

void USB::PrintDeviceEntries(u32 deviceEntryCount,
        const Array<Resource::DeviceEntry, 0x20> &deviceEntries) {
    for (u32 i = 0; i < deviceEntryCount; i++) {
        DEBUG("DeviceEntry(%x):", i);
        DEBUG("    id: %x", deviceEntries[i].id);
        DEBUG("    vendorId: %x", deviceEntries[i].vendorId);
        DEBUG("    productId: %x", deviceEntries[i].productId);
        DEBUG("    interfaceNumber: %x", deviceEntries[i].interfaceNumber);
        DEBUG("    alternateSettingCount: %x", deviceEntries[i].alternateSettingCount);
    }
}

void USB::CheckDeviceEntries(u32 deviceEntryCount,
        const Array<Resource::DeviceEntry, 0x20> &deviceEntries) {
    for (u32 i = 0; i < deviceEntryCount; i++) {
        for (u32 j = i + 1; j < deviceEntryCount; j++) {
            assert(deviceEntries[i].id != deviceEntries[j].id);
        }
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
        HandleRemoval();
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
                HandleAddition();
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

USB::DeviceInfo *USB::s_additionDeviceInfo = nullptr;
USB::Handler *USB::s_headHandler = nullptr;
USB::Handler *USB::s_removalHandler = nullptr;
USB::Handler *USB::s_additionHandler = nullptr;
USB::Device *USB::s_removalDevice = nullptr;
USB::Device *USB::s_additionDevice = nullptr;
Array<USB::Backend *, 2> USB::s_backends;
