#pragma once

#include "jsystem/JSUList.hh"

class JKRFileLoader {
public:
    JKRFileLoader();
    virtual ~JKRFileLoader();
    virtual void vf_0c();
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1c();
    virtual void vf_20();
    virtual void vf_24();
    virtual void vf_28();
    virtual void vf_2c();
    virtual s32 getResSize(const void *ptr) const;
    // ...

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
