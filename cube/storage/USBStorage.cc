#include "USBStorage.hh"

#include "cube/Clock.hh"
#include "cube/Log.hh"
#include "cube/Memory.hh"

#include <portable/Algorithm.hh>
#include <portable/Bytes.hh>

extern "C" {
#include <assert.h>
#include <inttypes.h>
#include <string.h>
}

USBStorage::USBStorage() : FATStorage(s_mutex), m_device(nullptr) {}

void USBStorage::onRemove(USB::Device *device) {
    assert(device == m_device);
    FATStorage::remove();
    m_device = nullptr;
    DEBUG("Removed device %p", device);

    // This is an empirical workaround to prevent the first TEST UNIT READY command from hanging
    // during the second initialization on some devices.
    initLun(device, m_lun);
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
    DEBUG("Device has %u logical unit(s)", lunCount);

    if (!findLun(device, lunCount, m_lun)) {
        return false;
    }
    DEBUG("Using logical unit %u", m_lun);

    if (!readCapacity(device, m_lun, m_blockSize)) {
        return false;
    }
    DEBUG("Block size: %u bytes", m_blockSize);

    m_device = device;
    if (!FATStorage::add()) {
        m_device = nullptr;
        return false;
    }
    DEBUG("Added device %p", device);
    return true;
}

void USBStorage::notify() {
    Storage::notify();
}

void USBStorage::poll() {
    Handler::poll();
}

u32 USBStorage::priority() {
    return 2;
}

const char *USBStorage::prefix() {
    return "usb:";
}

u32 USBStorage::sectorSize() {
    return m_blockSize;
}

bool USBStorage::read(u32 firstSector, u32 sectorCount, void *buffer) {
    assert(sectorCount <= UINT16_MAX);

    for (u32 attempt = 0; attempt < 5; attempt++) {
        Array<u8, 10> command(0);
        Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::Read10);
        Bytes::WriteBE<u32>(command.values(), 0x2, firstSector);
        Bytes::WriteBE<u16>(command.values(), 0x7, sectorCount);

        u32 size = sectorCount * m_blockSize;
        if (scsiTransfer(m_device, false, size, buffer, m_lun, command.count(), command.values())) {
            return true;
        }

        Clock::WaitMilliseconds(attempt * 10);
    }

    return false;
}

bool USBStorage::write(u32 firstSector, u32 sectorCount, const void *buffer) {
    assert(sectorCount <= UINT16_MAX);

    for (u32 attempt = 0; attempt < 5; attempt++) {
        Array<u8, 10> command(0);
        Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::Write10);
        Bytes::WriteBE<u32>(command.values(), 0x2, firstSector);
        Bytes::WriteBE<u16>(command.values(), 0x7, sectorCount);

        u32 size = sectorCount * m_blockSize;
        if (scsiTransfer(m_device, true, size, const_cast<void *>(buffer), m_lun, command.count(),
                    command.values())) {
            return true;
        }

        Clock::WaitMilliseconds(attempt * 10);
    }

    return false;
}

bool USBStorage::erase(u32 /* firstSector */, u32 /* sectorCount */) {
    // This is not supported under Bulk Only Transport
    return true;
}

bool USBStorage::sync() {
    Array<u8, 10> command(0);
    Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::SynchronizeCache10);

    return scsiTransfer(m_device, false, 0, nullptr, m_lun, command.count(), command.values());
}

bool USBStorage::testUnitReady(USB::Device *device, u8 lun) {
    Array<u8, 6> command(0);
    Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::TestUnitReady);

    return scsiTransfer(device, false, 0, nullptr, lun, command.count(), command.values());
}

bool USBStorage::inquiry(USB::Device *device, u8 lun, u8 &type) {
    alignas(0x20) Array<u8, 36> response(0);
    Array<u8, 6> command(0);
    Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::Inquiry);
    Bytes::WriteBE<u8>(command.values(), 0x1, lun << 5);
    Bytes::WriteBE<u8>(command.values(), 0x4, response.count());

    if (!scsiTransfer(device, false, response.count(), response.values(), lun, command.count(),
                command.values())) {
        return false;
    }

    type = Bytes::ReadBE<u8>(response.values(), 0x0) & 0x1f;
    return true;
}

bool USBStorage::initLun(USB::Device *device, u8 lun) {
    if (!testUnitReady(device, lun)) {
        return false;
    }

    u8 type;
    if (!inquiry(device, lun, type)) {
        return false;
    }
    return type == SCSIDeviceType::DirectAccess;
}

bool USBStorage::requestSense(USB::Device *device, u8 lun) {
    alignas(0x20) Array<u8, 18> response(0);
    Array<u8, 6> command(0);
    Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::RequestSense);
    Bytes::WriteBE<u8>(command.values(), 0x4, response.count());

    if (!scsiTransfer(device, false, response.count(), response.values(), lun, command.count(),
                command.values())) {
        return false;
    }

    DEBUG("Sense key: %x", Bytes::ReadBE<u8>(response.values(), 0x2) & 0xf);
    return true;
}

bool USBStorage::findLun(USB::Device *device, u8 lunCount, u8 &lun) {
    for (lun = 0; lun < lunCount; lun++) {
        for (u32 attempt = 0; attempt < 5; attempt++) {
            if (initLun(device, lun)) {
                return true;
            }

            // This can clear a UNIT ATTENTION condition
            requestSense(device, lun);

            Clock::WaitMilliseconds(attempt * 10);
        }
    }

    return false;
}

bool USBStorage::readCapacity(USB::Device *device, u8 lun, u32 &blockSize) {
    alignas(0x20) Array<u8, 8> response(0);
    Array<u8, 10> command(0);
    Bytes::WriteBE<u8>(command.values(), 0x0, SCSICommand::ReadCapacity10);

    if (!scsiTransfer(device, false, response.count(), response.values(), lun, command.count(),
                command.values())) {
        return false;
    }

    blockSize = Bytes::ReadBE<u32>(response.values(), 0x4);
    return true;
}

bool USBStorage::getLunCount(USB::Device *device, u8 &lunCount) {
    u8 endpointDirection = USB::EndpointDirection::DeviceToHost;
    u8 requestType = USB::RequestType::Class;
    u8 requestRecipient = USB::RequestRecipient::Interface;
    if (!device->ctrlTransfer(endpointDirection, requestType, requestRecipient,
                MSCRequest::GetMaxLUN, 0, m_interface, 0x1, m_buffer.values())) {
        return false;
    }
    lunCount = Bytes::ReadBE<u8>(m_buffer.values(), 0x0) + 1;
    return lunCount >= 1 && lunCount <= 16;
}

bool USBStorage::scsiTransfer(USB::Device *device, bool isWrite, u32 size, void *data, u8 lun,
        u8 cbSize, void *cb) {
    assert(device);
    assert(!!size == !!data);
    assert(lun < 16);
    assert(cbSize >= 1 && cbSize < m_csw.count() - 0xf);
    assert(cb);

    u8 outEndpointDirection = USB::EndpointDirection::HostToDevice;
    u8 inEndpointDirection = USB::EndpointDirection::DeviceToHost;

    memset(m_csw.values(), 0, m_csw.count());
    Bytes::WriteLE<u32>(m_csw.values(), 0x0, 0x43425355);
    Bytes::WriteLE<u32>(m_csw.values(), 0x4, m_tag++);
    Bytes::WriteLE<u32>(m_csw.values(), 0x8, size);
    Bytes::WriteLE<u8>(m_csw.values(), 0xc, !isWrite << 7);
    Bytes::WriteLE<u8>(m_csw.values(), 0xd, lun);
    Bytes::WriteLE<u8>(m_csw.values(), 0xe, cbSize);
    memcpy(m_csw.values() + 0xf, cb, cbSize);

    if (!device->bulkTransfer(m_csw.values(), m_csw.count(), outEndpointDirection,
                m_outEndpointNumber)) {
        return false;
    }

    u8 endpointDirection = isWrite ? outEndpointDirection : inEndpointDirection;
    u8 endpointNumber = isWrite ? m_outEndpointNumber : m_inEndpointNumber;
    if (Memory::IsMEM2(data) && Memory::IsAligned(data, 0x20)) {
        if (!device->bulkTransfer(data, size, endpointDirection, endpointNumber)) {
            return false;
        }
    } else {
        for (u32 offset = 0; offset < size; offset += m_buffer.count()) {
            u32 chunkSize = Min<u32>(size - offset, m_buffer.count());
            if (isWrite) {
                memcpy(m_buffer.values(), reinterpret_cast<u8 *>(data) + offset, chunkSize);
            }
            if (!device->bulkTransfer(m_buffer.values(), chunkSize, endpointDirection,
                        endpointNumber)) {
                return false;
            }
            if (!isWrite) {
                memcpy(reinterpret_cast<u8 *>(data) + offset, m_buffer.values(), chunkSize);
            }
        }
    }

    memset(m_cbw.values(), 0, m_cbw.count());

    if (!device->bulkTransfer(m_cbw.values(), m_cbw.count(), inEndpointDirection,
                m_inEndpointNumber)) {
        return false;
    }

    if (Bytes::ReadLE<u32>(m_cbw.values(), 0x0) != 0x53425355) {
        return false;
    }
    if (Bytes::ReadLE<u32>(m_cbw.values(), 0x4) != m_tag - 1) {
        return false;
    }
    if (Bytes::ReadLE<u32>(m_cbw.values(), 0x8) != 0) {
        return false;
    }
    if (Bytes::ReadLE<u8>(m_cbw.values(), 0xc) != 0) {
        return false;
    }

    return true;
}

USBStorage *USBStorage::s_instance = nullptr;
Mutex *USBStorage::s_mutex = nullptr;
