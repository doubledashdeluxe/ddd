#include "JUTException.hh"

#include <cube/Memory.hh>
#include <cube/VI.hh>
extern "C" {
#include <cube/StackCanary.h>
#include <dolphin/OSInterrupt.h>
#include <dolphin/PPCArch.h>
#include <dolphin/VI.h>
}

void *JUTException::run() {
    u32 msr = PPCMfmsr();
    msr &= ~(1 << 11 | 1 << 8);
    PPCMtmsr(msr);

    OSInitMessageQueue(&s_queue, s_messages.values(), s_messages.count());
    while (true) {
        OSMessage message;
        OSReceiveMessage(&s_queue, &message, OS_MESSAGE_BLOCK);

        VISetPreRetraceCallback(nullptr);
        VISetPostRetraceCallback(nullptr);

        CallbackObject *co = static_cast<CallbackObject *>(message);
        if (co->error < 0x11) {
            m_stackPointer = co->context->gprs[1];
        }

        m_frameBuffer = reinterpret_cast<void *>(0x800056c0);
        VISetNextFrameBuffer(m_frameBuffer);
        VIFlush();
        VIFlush();
        for (u32 i = 0; i < 3; i++) {
            for (u32 retraceCount = VIGetRetraceCount(); VIGetRetraceCount() == retraceCount;) {}
        }

        OSDisableInterrupts();

        VI *vi = VI::Instance();
        m_directPrint->changeFrameBuffer(m_frameBuffer, vi->getXFBWidth(), vi->getXFBHeight());
        printContext(co->error, co->context, co->dsisr, co->dar);
    }
}

void JUTException::showStack(OSContext * /* context */) {
    if (!s_console) {
        return;
    }

    s_console->print("-------------------------------- TRACE\n");
    struct StackFrame {
        StackFrame *backChain;
        u32 lrSave;
    };
    StackFrame *stackFrame = reinterpret_cast<StackFrame *>(m_stackPointer);
    s_console->print_f("Address:   BackChain   LR save\n");

    for (u32 i = 0; stackFrame && reinterpret_cast<u32>(stackFrame) != 0xffffffff && i++ < 0x10;
            stackFrame = stackFrame->backChain) {
        if (i > m_traceDepth) {
            s_console->print("Suppress trace.");
            return;
        }

        // The stack canary always XORs the MSB
        u32 lrSave = stackFrame->lrSave;
        if ((lrSave & 0x80000000) == 0) {
            lrSave ^= StackCanary_Canary();
        }

        // If we're not executing in code, mark it on the stack trace
        bool ok = Memory::IsCode(lrSave);

        s_console->print_f("%08X:  %08X    %08X%c\n", reinterpret_cast<u32>(stackFrame),
                reinterpret_cast<u32>(stackFrame->backChain), lrSave, ok ? ' ' : '*');
        showMapInfo_subroutine(lrSave, false);
        JUTConsoleManager::Instance()->drawDirect(true);
        waitTime(m_traceWaitTime);
    }
}
