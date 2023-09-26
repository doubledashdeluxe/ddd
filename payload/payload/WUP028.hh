#pragma once

#include "payload/Mutex.hh"

#include <common/USB.hh>
extern "C" {
#include <dolphin/PAD.h>
}

class WUP028 : private USB::Handler {
public:
    static void Init();
    static bool Probe(s32 chan);
    static u32 Read(PADStatus *status);
    static void ControlMotor(s32 chan, u32 command);

private:
    WUP028();
    ~WUP028();

    void onRemove(USB::Device *device);
    bool onAdd(const USB::DeviceInfo *deviceInfo, USB::Device *device);
    void notify();

    void *poll();
    void *transfer();

    static void *Poll(void *param);
    static void *Transfer(void *param);

    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    Mutex m_mutex;
    Array<u8, 4 * 1024> m_pollStack;
    Array<u8, 4 * 1024> m_transferStack;
    OSThread m_pollThread;
    OSThread m_transferThread;
    USB::Device *m_device;
    alignas(0x20) Array<u8, 0x1> m_initBuffer;
    alignas(0x20) Array<u8, 0x25> m_readBuffer;
    alignas(0x20) Array<u8, 0x5> m_writeBuffer;

    static WUP028 *s_instance;
};
