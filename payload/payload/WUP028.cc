#include "WUP028.hh"

#include "payload/Lock.hh"

#include <common/Arena.hh>
#include <common/Log.hh>
#include <portable/Bytes.hh>

extern "C" {
#include <assert.h>
#include <string.h>
}

void WUP028::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) WUP028;
}

bool WUP028::Probe(s32 chan) {
    if (!s_instance || !s_instance->m_device) {
        return false;
    }

    if (s_instance->m_readBuffer[0] != 0x21) {
        return false;
    }

    u8 *data = s_instance->m_readBuffer.values() + 1 + chan * 9;
    return data[0] & 1 << 5 || data[0] & 1 << 4;
}

u32 WUP028::Read(PADStatus *status) {
    for (u32 i = 0; i < 4; i++) {
        status[i].err = PAD_ERR_NO_CONTROLLER;
    }
    u32 rumbleSupported = 0;

    if (!s_instance || !s_instance->m_device) {
        return rumbleSupported;
    }

    if (s_instance->m_readBuffer[0] != 0x21) {
        return rumbleSupported;
    }

    Lock<NoInterrupts> lock;
    for (u32 i = 0; i < 4; i++) {
        u8 *data = s_instance->m_readBuffer.values() + 1 + i * 9;
        if (data[0] & 1 << 4) {
            rumbleSupported |= 1 << (31 - i);
        } else if (!(data[0] & 1 << 5)) {
            continue;
        }
        status[i].err = PAD_ERR_NONE;
        status[i].button = 0;
        if (data[1] & 1 << 0) {
            status[i].button |= PAD_BUTTON_A;
        }
        if (data[1] & 1 << 1) {
            status[i].button |= PAD_BUTTON_B;
        }
        if (data[1] & 1 << 2) {
            status[i].button |= PAD_BUTTON_X;
        }
        if (data[1] & 1 << 3) {
            status[i].button |= PAD_BUTTON_Y;
        }
        if (data[1] & 1 << 4) {
            status[i].button |= PAD_BUTTON_LEFT;
        }
        if (data[1] & 1 << 5) {
            status[i].button |= PAD_BUTTON_RIGHT;
        }
        if (data[1] & 1 << 6) {
            status[i].button |= PAD_BUTTON_DOWN;
        }
        if (data[1] & 1 << 7) {
            status[i].button |= PAD_BUTTON_UP;
        }
        if (data[2] & 1 << 0) {
            status[i].button |= PAD_BUTTON_START;
        }
        if (data[2] & 1 << 1) {
            status[i].button |= PAD_TRIGGER_Z;
        }
        if (data[2] & 1 << 2) {
            status[i].button |= PAD_TRIGGER_R;
        }
        if (data[2] & 1 << 3) {
            status[i].button |= PAD_TRIGGER_L;
        }
        status[i].stickX = data[3] - 128;
        status[i].stickY = data[4] - 128;
        status[i].substickX = data[5] - 128;
        status[i].substickY = data[6] - 128;
        status[i].triggerLeft = data[7];
        status[i].triggerRight = data[8];
        status[i].analogA = 0;
        status[i].analogB = 0;
    }
    return rumbleSupported;
}

void WUP028::ControlMotor(s32 chan, u32 command) {
    if (!s_instance || !s_instance->m_device) {
        return;
    }

    Lock<NoInterrupts> lock;
    s_instance->m_writeBuffer[1 + chan] = command;
}

WUP028::WUP028() : m_device(nullptr) {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    OSCreateThread(&m_pollThread, Poll, this, m_pollStack.values() + m_pollStack.count(),
            m_pollStack.count(), 3, 0);
    OSCreateThread(&m_transferThread, Transfer, this,
            m_transferStack.values() + m_transferStack.count(), m_transferStack.count(), 4, 0);
    OSResumeThread(&m_pollThread);
}

void WUP028::onRemove(USB::Device *device) {
    assert(device == m_device);
    m_device = nullptr;
    DEBUG("Removed device %p", device);
}

bool WUP028::onAdd(const USB::DeviceInfo *deviceInfo, USB::Device *device) {
    if (m_device) {
        return false;
    }

    u16 vendorId = deviceInfo->deviceDescriptor.vendorId;
    u16 productId = deviceInfo->deviceDescriptor.productId;
    if (vendorId != 0x057e || productId != 0x0337) {
        return false;
    }

    u8 numEndpoints = deviceInfo->interfaceDescriptor.numEndpoints;
    if (numEndpoints != 2) {
        DEBUG("Unexpected number of endpoints: %u", numEndpoints);
        return false;
    }

    memset(m_initBuffer.values(), 0, m_initBuffer.count());
    m_initBuffer[0] = 0x13;
    void *data = m_initBuffer.values();
    u16 length = m_initBuffer.count();
    u8 endpointDirection = USB::EndpointDirection::HostToDevice;
    u8 endpointNumber = 2;
    if (!device->intrTransfer(data, length, endpointDirection, endpointNumber)) {
        DEBUG("Failed to send init transfer");
        return false;
    }

    memset(m_readBuffer.values(), 0, m_readBuffer.count());
    memset(m_writeBuffer.values(), 0, m_writeBuffer.count());
    m_writeBuffer[0] = 0x11;

    DEBUG("Added device %p", device);
    m_device = device;
    return true;
}

void WUP028::notify() {
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *WUP028::poll() {
    m_mutex.lock();

    OSResumeThread(&m_transferThread);

    while (true) {
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);

        if (m_device) {
            m_mutex.lock();
        }

        Handler::poll();

        if (m_device) {
            m_mutex.unlock();
        }
    }
}

void *WUP028::transfer() {
    alignas(0x20) Array<u8, 0x25> readBuffer;
    alignas(0x20) Array<u8, 0x5> writeBuffer;

    while (true) {
        Lock<Mutex> deviceLock(m_mutex);

        {
            void *data = readBuffer.values();
            u16 length = readBuffer.count();
            u8 endpointDirection = USB::EndpointDirection::DeviceToHost;
            u8 endpointNumber = 1;
            if (!m_device->intrTransfer(data, length, endpointDirection, endpointNumber)) {
                continue;
            }
        }

        {
            Lock<NoInterrupts> bufferLock;
            m_readBuffer = readBuffer;
            if (!memcmp(m_writeBuffer.values(), writeBuffer.values(), writeBuffer.count())) {
                continue;
            }
            writeBuffer = m_writeBuffer;
        }

        {
            void *data = writeBuffer.values();
            u16 length = writeBuffer.count();
            u8 endpointDirection = USB::EndpointDirection::HostToDevice;
            u8 endpointNumber = 2;
            if (!m_device->intrTransfer(data, length, endpointDirection, endpointNumber)) {
                continue;
            }
        }
    }
}

void *WUP028::Poll(void *param) {
    return reinterpret_cast<WUP028 *>(param)->poll();
}

void *WUP028::Transfer(void *param) {
    return reinterpret_cast<WUP028 *>(param)->transfer();
}

WUP028 *WUP028::s_instance = nullptr;
