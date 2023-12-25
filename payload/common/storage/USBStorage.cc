#include <common/storage/USBStorage.hh>

#include <common/Arena.hh>
#include <common/Log.hh>

extern "C" {
#include <assert.h>
}

void USBStorage::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) USBStorage;
}

USBStorage::USBStorage() : FATStorage(&m_mutex), m_device(nullptr) {}

void USBStorage::onRemove(USB::Device *device) {
    assert(device == m_device);
    FATStorage::remove();
    m_device = nullptr;
    DEBUG("Removed device %p\n", device);
}

bool USBStorage::onAdd(const USB::DeviceInfo *deviceInfo, USB::Device *device) {
    if (m_device) {
        return false;
    }

    u8 interfaceClass = deviceInfo->interfaceDescriptor.interfaceClass;
    if (interfaceClass != USB::InterfaceClass::MassStorage) {
        return false;
    }

    u8 interfaceSubClass = deviceInfo->interfaceDescriptor.interfaceSubClass;
    if (interfaceSubClass != USB::InterfaceSubClass::MassStorageSCSI) {
        return false;
    }

    u8 interfaceProtocol = deviceInfo->interfaceDescriptor.interfaceProtocol;
    if (interfaceProtocol != USB::InterfaceProtocol::MassStorageBulkOnly) {
        return false;
    }

    m_interface = deviceInfo->interfaceDescriptor.interfaceNumber;

    u8 numEndpoints = deviceInfo->interfaceDescriptor.numEndpoints;
    assert(numEndpoints <= 16);
    bool outFound = false;
    bool inFound = false;
    for (u32 i = 0; i < numEndpoints; i++) {
        const USB::EndpointDescriptor &endpointDescriptor = deviceInfo->endpointDescriptors[i];

        u8 transferType = endpointDescriptor.attributes.transferType;
        if (transferType != USB::EndpointTransferType::Bulk) {
            continue;
        }

        u8 direction = endpointDescriptor.endpointAddress.direction;
        if (!outFound && direction == USB::EndpointDirection::HostToDevice) {
            m_outEndpointNumber = endpointDescriptor.endpointAddress.number;
            outFound = true;
        }
        if (!inFound && direction == USB::EndpointDirection::DeviceToHost) {
            m_inEndpointNumber = endpointDescriptor.endpointAddress.number;
            inFound = true;
        }
    }
    if (!outFound || !inFound) {
        return false;
    }

    m_tag = 0;

    u8 lunCount;
    if (!getLunCount(device, lunCount)) {
        return false;
    }
    DEBUG("Device has %u logical unit(s)\n", lunCount);

    if (!findLun(device, lunCount, m_lun)) {
        return false;
    }
    DEBUG("Using logical unit %u\n", m_lun);

    if (!readCapacity(device, m_lun, m_blockSize)) {
        return false;
    }
    DEBUG("Block size: %u bytes\n", m_blockSize);

    m_device = device;
    if (!FATStorage::add()) {
        m_device = nullptr;
        return false;
    }
    DEBUG("Added device %p\n", device);
    return true;
}

void USBStorage::notify() {
    Storage::notify();
}

void USBStorage::poll() {
    Handler::poll();
}
