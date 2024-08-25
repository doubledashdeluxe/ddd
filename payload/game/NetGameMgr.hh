#pragma once

#include <common/Types.hh>

class NetGameMgr {
public:
    bool isActive() const;
    void framework();
    void adjustFrame();

    static NetGameMgr *Instance();

private:
    NetGameMgr();

    u8 _0000[0x096c - 0x0000];
    s32 m_state;
    u8 _0970[0x1308 - 0x0970];

    static NetGameMgr *s_instance;
};
size_assert(NetGameMgr, 0x1308);
