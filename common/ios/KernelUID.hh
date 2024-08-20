#pragma once

#include <common/Types.hh>

namespace IOS {

class KernelUID {
public:
    KernelUID(bool again);
    ~KernelUID();
    bool ok() const;

private:
    class SC {
    public:
        enum {
            IOS_SetUid = 0x2b,
            IOS_InvalidateDCache = 0x3f,
            IOS_FlushDCache = 0x40,
        };

    private:
        SC();
    };

    static bool Acquire(bool again);
    static void Release();

    static void SafeFlush(const void *start, size_t size);
    static u32 ReadMessage(u32 index);
    static void WriteMessage(u32 index, u32 message);
    static u32 Syscall(u32 id);

    bool m_ok;

    static const u32 ArmCode[0x64 / sizeof(u32)];
};

} // namespace IOS
