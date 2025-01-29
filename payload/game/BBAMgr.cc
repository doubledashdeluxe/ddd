#include "BBAMgr.hh"

#include "game/GameClock.hh"

extern "C" {
#include <dolphin/IPRoute.h>
#include <dolphin/UPnPHttpd.h>
#include <dolphin/UPnPSsdp.h>
}

s32 BBAMgr::processDHCP() {
    if (m_state != 2) {
        return REPLACED(processDHCP)();
    }

    s_config.flag = 1 << 0;
    if (SOStartup(&s_config) == SO_SUCCESS) {
        m_socket = SOSocket(AF_INET, SO_SOCK_DGRAM, 0);
        m_address.len = sizeof(m_address);
        m_address.family = AF_INET;
        m_address.port = 26512;
        m_address.addr = 0;
        SOBind(m_socket, &m_address);
        m_state = 3;
        _024 = 0;
        _020 = GameClock::GetTime();
        _02c = true;
        _014 = 0.0f;
    }
    return 1;
}

s32 BBAMgr::processAutoIP() {
    if (m_state != 2) {
        return REPLACED(processAutoIP)();
    }

    s_config.flag = 1 << 0;
    if (SOStartup(&s_config) == SO_SUCCESS) {
        m_socket = SOSocket(AF_INET, SO_SOCK_DGRAM, 0);
        m_address.len = sizeof(m_address);
        m_address.family = AF_INET;
        m_address.port = 26512;
        m_address.addr = 0;
        SOBind(m_socket, &m_address);
        m_state = 4;
        _024 = 0;
        _020 = GameClock::GetTime();
        _02c = true;
        _018 = 0.0f;
    }
    return 1;
}

bool BBAMgr::disconnecting(bool force) {
    switch (m_state) {
    case 0:
    case 1:
    case 2:
        IPClearConfigError(nullptr);
        return true;
    case 3:
    case 4:
        IPClearConfigError(nullptr);
        SOCleanup();
        m_state = 1;
        return true;
    case 5:
        IPClearConfigError(nullptr);
        if (force) {
            SOCleanup();
            m_state = 1;
            return true;
        }
        SSDPStop();
        m_state = 6;
        break;
    case 6:
    case 7:
        break;
    default:
        return false;
    }

    if (force) {
        SOCleanup();
        m_state = 1;
        return true;
    }

    if (m_state == 6) {
        if (SSDPGetState() == 1) {
            SOCleanup();
            m_state = 7;
        }
    }

    if (m_state == 7) {
        if (UPnPHttpGetState() == 1) {
            m_state = 1;
            return true;
        }
    }

    return false;
}

BBAMgr *BBAMgr::Instance() {
    return s_instance;
}

SOConfig &BBAMgr::Config() {
    return s_config;
}
