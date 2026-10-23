#pragma once

#include "jsystem/JKRThread.hh"
#include "jsystem/JUTConsole.hh"
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

    REPLACE void showStack(OSContext *context);

    void printContext(u16 error, OSContext *context, u32 dsisr, u32 dar);
    bool showMapInfo_subroutine(u32, bool);

    void *m_frameBuffer;
    JUTDirectPrint *m_directPrint;
    u8 _84[0x90 - 0x84];
    s32 m_traceWaitTime;
    u32 m_traceDepth;
    u8 _98[0xa0 - 0x98];
    u32 m_stackPointer;

    static void waitTime(s32 time);

    static JUTConsole *s_console;

    static OSMessageQueue s_queue;
    static Array<OSMessage, 1> s_messages;
};
size_assert(JUTException, 0xa4);
