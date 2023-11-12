#pragma once

#include <common/Types.hh>

class JKRFileLoader {
public:
    JKRFileLoader();
    virtual ~JKRFileLoader();
    // ...

    static void *GetGlbResource(const char *name, JKRFileLoader *fileLoader);

protected:
    u8 _04[0x38 - 0x04];
};
size_assert(JKRFileLoader, 0x38);
