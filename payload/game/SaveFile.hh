#pragma once

#include <common/Types.hh>

class SaveFile {
public:
    virtual void vf_08() = 0;
    virtual void vf_0c() = 0;
    virtual void store() = 0;
    virtual void vf_14() = 0;
    virtual void vf_18() = 0;
    virtual void vf_1c() = 0;
    virtual void vf_20() = 0;
    virtual void vf_24() = 0;
    virtual void vf_28() = 0;
    virtual void vf_2c() = 0;
    virtual void vf_30() = 0;
    virtual void vf_34() = 0;
    virtual void vf_38() = 0;
    virtual void vf_3c() = 0;
    virtual void vf_40() = 0;
    virtual void vf_44() = 0;
    virtual void vf_48() = 0;
    virtual void vf_4c() = 0;
    virtual void vf_50() = 0;
    virtual ~SaveFile() = 0;
};
size_assert(SaveFile, 0x4);
