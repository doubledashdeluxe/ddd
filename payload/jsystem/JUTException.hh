#pragma once

#include "jsystem/JKRThread.hh"
#include "jsystem/JUTDirectPrint.hh"

extern "C" {
#include <dolphin/OSContext.h>
#include <dolphin/OSMessage.h>
}
#include <payload/Replace.hh>
#include <portable/Array.hh>

class JUTException : public JKRThread {
public:
    REPLACE void *run() override;

    __attribute__((noreturn)) static void panic_f(const char *file, int line, const char *message,
            ...);

private:
    struct CallbackObject {
        u8 _00[0x04 - 0x00];
        u16 error;
        OSContext *context;
        u32 dsisr;
        u32 dar;
    };
    size_assert(CallbackObject, 0x14);

    void printContext(u16 error, OSContext *context, u32 dsisr, u32 dar);

    void *m_frameBuffer;
    JUTDirectPrint *m_directPrint;
    u8 _84[0xa0 - 0x84];
    u32 m_stackPointer;

    static OSMessageQueue s_queue;
    static Array<OSMessage, 1> s_messages;
};
size_assert(JUTException, 0xa4);
