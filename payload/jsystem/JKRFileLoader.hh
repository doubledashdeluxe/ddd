#pragma once

#include "jsystem/JSUList.hh"

class JKRFileLoader {
public:
    JKRFileLoader();
    virtual ~JKRFileLoader();
    virtual void vf_0c();
    virtual void vf_10() = 0;
    virtual void *getResource(const char *path) = 0;
    virtual void vf_18() = 0;
    virtual void vf_1c() = 0;
    virtual void vf_20() = 0;
    virtual void vf_24() = 0;
    virtual void vf_28() = 0;
    virtual void vf_2c() = 0;
    virtual s32 getResSize(const void *ptr) const = 0;
    virtual void vf_34() = 0;
    virtual void vf_38() = 0;

    static void *GetGlbResource(const char *name, JKRFileLoader *fileLoader);
    static s32 GetGlbResSize(const void *ptr, JKRFileLoader *fileLoader);

protected:
    u8 _04[0x18 - 0x04];
    JSUPtrLink m_link;
    char *m_name;
    u32 m_signature;
    bool m_isMounted;
    u8 _31[0x34 - 0x31];
    u32 m_mountCount;

    static JSUPtrList s_volumeList;
};
size_assert(JKRFileLoader, 0x38);
