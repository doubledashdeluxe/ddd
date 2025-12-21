#pragma once

#include <portable/Types.hh>
#include <portable/UniquePtr.hh>

class ForceEffectMgr {
public:
    ForceEffectMgr();
    virtual ~ForceEffectMgr();

    static void Destroy();

private:
    u8 _04[0x1c - 0x04];

    static UniquePtr<ForceEffectMgr> s_instance;
};
size_assert(ForceEffectMgr, 0x1c);
