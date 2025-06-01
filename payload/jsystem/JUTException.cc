#include "JUTException.hh"

#include <cube/VI.hh>
extern "C" {
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
