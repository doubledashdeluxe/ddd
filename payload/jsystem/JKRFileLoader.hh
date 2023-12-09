#pragma once

#include "jsystem/JSUList.hh"

class JKRFileLoader {
public:
    JKRFileLoader();
    virtual ~JKRFileLoader();
    // ...

    static void *GetGlbResource(const char *name, JKRFileLoader *fileLoader);

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
