#pragma once

extern "C" {
#include <dolphin/IPSocket.h>
}
#include <payload/Replace.hh>

class BBAMgr {
public:
    s32 REPLACED(processDHCP)();
    REPLACE s32 processDHCP();
    s32 REPLACED(processAutoIP)();
    REPLACE s32 processAutoIP();
    REPLACE bool disconnecting(bool force);
    u32 getState();

    static BBAMgr *Instance();
    static SOConfig &Config();

private:
    u8 _000[0x004 - 0x000];
    s32 m_state;
    u8 _008[0x014 - 0x008];
    f32 _014;
    f32 _018;
    u8 _01c[0x020 - 0x01c];
    u32 _020;
    u32 _024;
    f32 _028;
    bool _02c;
    s32 m_socket;
    SOSockAddr m_address;
    u8 _03c[0x9c0 - 0x03c];

    static BBAMgr *s_instance;
    static SOConfig s_config;
};
size_assert(BBAMgr, 0x9c0);
